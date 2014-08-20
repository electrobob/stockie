//****************************************************************************
//  ws2812 - Serial driver for ws2812 RGB leds using fastpwm
//
//  ws2812_tinymega - driver for attiny and atmega architecture
//
//  2013 by M. Marquardt (adrock0905@alice.de)
//
//  Please see README.TXT for more information
//

//----- Includes
#ifdef __cplusplus
extern "C" {
	#endif
#include <avr/io.h>
#include "asmdefs.h"
#include "ws2812.h"

#if defined WS_ARCH_TINY || defined WS_ARCH_MEGA

//----- Functions
//
//      GCC style parameter register usage:
//      r0: scratch
//      r1: always zero
//      r2-r17, r28-r29: leave unchanged/saved
//      r18-r27, r30-r31: scratch
//      Function arguments - allocated left to right, r25 to r8.
//      All arguments are aligned to start in even-numbered registers
//      (odd-sized arguments, including char, have one free register above
//      them).
//      Return values: 8-bit in r24 (not r25!), 16-bit in r25:r24

//----- Some macros used later for loop unrolling

//  BITOUT - check for bitnum set in reg and set OCR0B val accordingly

.macro  BITOUT reg, bitnum
    STORE   WS_TIFR, r22                // clear OCF0A flag

    STORE   WS_OCRB, r18                // set OCR0B to low val
    sbrc    \reg, \bitnum               // skip if bit is cleared
    STORE   WS_OCRB, r19                // set OCR0B to high val
.endm

//  BITWAIT - wait for actual pwm cycle to finish

.macro  BITWAIT
1:
    LOAD    r20, WS_TIFR
    sbrs    r20, WS_OCFA
    rjmp    1b
.endm



//****************************************************************************
//  void WS_init(void)
//
//  DESCR:
//      Initialize hardware
//
.global WS_init

WS_init:
    cbi     _SFR_IO_ADDR(WS_OUTPORT), WS_OUTPIN
    sbi     _SFR_IO_ADDR(WS_OUTDDR), WS_OUTPIN
    ldi     r24, (1<<COM0B1)|(1<<WGM01)|(1<<WGM00)
    STORE   WS_TCCRA, r24
    ldi     r24, WS_CNT_TOTAL
    STORE   WS_OCRA, r24
    ret



//****************************************************************************
//      void WS_out(uint8_t *data, uint16_t count, const uint8_t *gamma);
//
//  DESCR:
//      Output bytes to ws2812 leds using fastpwm with gamma correction using
//      the lookup table specified.
//
//      ATTINY+ATMEGA: When gamma is non-null, due to implementation
//      constraints, there is ONE MORE byte read at end of data from memory,
//      but not output.
//
//  IN:
//      r24: uint8_t *data      Ptr. to bytes to output
//      r22/r23: uint16_t count     Number of bytes
//      r20/r21: uint16_t *gamma    Ptr. to GammaTable in FLASH, NULL = none
//
.global WS_out

WS_out:
    clr     r1

    mov     r26, r20
    or      r26, r21                    // check if gamma == 0
    breq    WS_out_nogamma
    rjmp    WS_outG

WS_out_nogamma:
    movw    r26, r24                    // X = Ptr to output data
    movw    r24, r22                    // r24/r25 = Output counter

    ldi     r22, (1<<WS_OCFA)           // r22 = value to reset OCF0A
    ldi     r23, (1<<CS00)|(1<<WGM02)   // r23 = value to start counter
    ldi     r18, WS_CNT_LOW             // r18 = low OCR0B value
    ldi     r19, WS_CNT_HIGH            // r19 = high OCR0B value

    STORE   WS_OCRB, r1                 // OCR0B = 0 -> zero cycle at beginning
    ldi     r30, 0x01                   // init. timer counter
    STORE   WS_TCNT, r30
    STORE   WS_TCCRB, r23               // start timer

    // X: ptr to next byte, r24/r25: Output byte counter

WS_out1:
    sbiw    r24, 0x01                   // dec counter
    brcs    WS_out_end

    ld      r0, X+                      // Get next byte

    BITOUT  r0, 7
    BITWAIT
    BITOUT  r0, 6
    BITWAIT
    BITOUT  r0, 5
    BITWAIT
    BITOUT  r0, 4
    BITWAIT
    BITOUT  r0, 3
    BITWAIT
    BITOUT  r0, 2
    BITWAIT
    BITOUT  r0, 1
    BITWAIT
    BITOUT  r0, 0
    BITWAIT
    rjmp    WS_out1

WS_out_end:
    STORE   WS_TIFR, r22                // clear OCF0A flag
    STORE   WS_OCRB, r1                 // last OCR0B = 0 

WS_out_waitfinal:
    LOAD    r20, WS_TIFR
    sbrs    r20, WS_OCFA
    rjmp    WS_out_waitfinal

    STORE   WS_TCCRB, r1                // stop timer
    ret

    // Output with GammaTable supplied

WS_outG:
    push    r28
    push    r29
    movw    r28, r20                    // Y = Ptr. to gamme table

    movw    r26, r24                    // X = Ptr to output data
    movw    r24, r22                    // r24/r25 = Output counter

    ldi     r22, (1<<WS_OCFA)           // r22 = value to reset OCF0A
    ldi     r23, (1<<CS00)|(1<<WGM02)   // r23 = value to start counter
    ldi     r18, WS_CNT_LOW             // r18 = low OCR0B value
    ldi     r19, WS_CNT_HIGH            // r19 = high OCR0B value

    STORE   WS_OCRB, r1                 // OCR0B = 0 -> zero cycle at beginning
    ldi     r30, 0x01                   // init. timer counter
    STORE   WS_TCNT, r30
    STORE   WS_TCCRB, r23               // start timer

    // X: ptr to next byte, r24/r25: Output byte counter

    ld      r0, X+                      // Get next byte
    movw    r30, r28                    // gamma table into Z
    add     r30, r0                     // add byte as offset
    adc     r31, r1
    lpm     r0, Z                       // lookup byte -> r0

WS_outG1:
    sbiw    r24, 0x01                   // dec counter
    brcs    WS_outG_end

    // Some instructions for gamma table lookup for next byte are spread
    // between bit outputs for timing reasons

    BITOUT  r0, 7
    BITWAIT
    BITOUT  r0, 6
    BITWAIT
    BITOUT  r0, 5
    BITWAIT
    BITOUT  r0, 4
    BITWAIT
    BITOUT  r0, 3
    ld      r21, X+                     // Get next byte
    BITWAIT
    BITOUT  r0, 2
    movw    r30, r28                    // gamma table into Z
    BITWAIT
    BITOUT  r0, 1
    add     r30, r21                    // add byte as offset
    adc     r31, r1
    BITWAIT
    BITOUT  r0, 0
    lpm     r0, Z                       // lookup byte -> r0
    BITWAIT

    rjmp    WS_outG1

WS_outG_end:
    STORE   WS_TIFR, r22                // clear OCF0A flag
    STORE   WS_OCRB, r1                 // last OCR0B = 0 

WS_outG_waitfinal:
    LOAD    r20, WS_TIFR
    sbrs    r20, WS_OCFA
    rjmp    WS_outG_waitfinal

    STORE   WS_TCCRB, r1                // stop timer

    pop     r29
    pop     r28
    ret



#endif // WS_ARCH
#ifdef __cplusplus
}
#endif