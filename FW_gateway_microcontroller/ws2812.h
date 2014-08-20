//****************************************************************************
//  ws2812 - Serial driver for ws2812 RGB leds using fastpwm
//
//  Include
//
//  2013 by M. Marquardt (adrock0905@alice.de)
//
//  Please see README.TXT for more information
//

#ifdef __cplusplus
extern "C" {
	#endif
#ifndef WS2812_H_
#define WS2812_H_

#include <avr/io.h>


//----- Defaults for configurable stuff

//  If undefined, try to find out arch

#if ! ( defined WS_ARCH_TINY || defined WS_ARCH_MEGA || defined WS_ARCH_XMEGA )
    #if __AVR_ARCH__ == 25              // ATTINY
        #define WS_ARCH_TINY
    #elif __AVR_ARCH__ == 5             // ATMEGA
        #define WS_ARCH_MEGA
    #elif __AVR_ARCH__ >= 100           // XMEGA
        #define WS_ARCH_XMEGA
    #else
        #error "Unrecognized __AVR_ARCH__ - please define WS_ARCH_..."
    #endif
#endif

//  Constants for timing (common for all arch)

#ifndef WS_F_OUT
    #define     WS_F_OUT        800000                  // Output frequency
#endif
#ifndef WS_CNT_TOTAL
    #define     WS_CNT_TOTAL    ((F_CPU/WS_F_OUT)-1)    // # cycles for one bit
#endif
#ifndef WS_CNT_LOW
    #define     WS_CNT_LOW      (((F_CPU/WS_F_OUT)*1)/3-1)  // # cycles for T0H
#endif
#ifndef WS_CNT_HIGH
    #define     WS_CNT_HIGH     (((F_CPU/WS_F_OUT)*2)/3-1) // # cycles for T1H
#endif


//----- Device specific stuff

//  ATTINY

#if defined WS_ARCH_TINY

#define     WS_OCRA     OCR0A           // OCRxA to use
#define     WS_OCRB     OCR0B           // OCRxB to use
#define     WS_OCFA     OCF0A           // Flag in TIFR for OCRxA
#define     WS_TCCRA    TCCR0A          // TCCRxA to use
#define     WS_TCCRB    TCCR0B          // TCCRxB to use
#define     WS_TCNT     TCNT0           // TCNTx to use
#define     WS_TIFR     TIFR            // TIFR to use ATtiny
#define     WS_OUTPORT  PORTB           // data output port ATtiny
#define     WS_OUTDDR   DDRB            // data output ddr ATtiny
#define     WS_OUTPIN   PB1             // data output pin ATtiny

//  ATMEGA

#elif defined WS_ARCH_MEGA

#define     WS_OCRA     OCR0A           // OCRxA to use
#define     WS_OCRB     OCR0B           // OCRxB to use
#define     WS_OCFA     OCF0A           // Flag in TIFR for OCRxA
#define     WS_TIFR     TIFR0           // TIFR to use ATmega
#define     WS_TCCRA    TCCR0A          // TCCRxA to use
#define     WS_TCCRB    TCCR0B          // TCCRxB to use
#define     WS_TCNT     TCNT0           // TCNTx to use
#define     WS_OUTPORT  PORTD           // data output port ATmega
#define     WS_OUTDDR   DDRD            // data output ddr ATmega
#define     WS_OUTPIN   PD5             // datat output pin ATmega

//  XMEGA

#elif defined WS_ARCH_XMEGA

#define     WS_BUFNUM       6              // Output bytes to buffer
#define     WS_BUFSIZE      (WS_BUFNUM*8)   // required buffer size for this

#define     WS_DMA_A		CH0         // DMA channel A to use
#define     WS_DMA_B		CH1         // DMA channel B to use
#define     WS_DMA_DBUFMODE DMA_DBUFMODE_CH01_gc // DMA DBUFMODE to use
#define     WS_DMA_DBUFMODE_bm DMA_DBUFMODE0_bm
#define     WS_DMA_BUSY_bm  (DMA_CH0BUSY_bm|DMA_CH1BUSY_bm)
#define     WS_DMA_INTLVL   0x02        // Interruptlevel for DMA
#define     WS_TCC          TCC0        // Timer/Counter to use

#define     WS_DMA_TRIGSRC  0x01        // TRIGSRC = EVSYS CH0
#define     WS_EVSYS_CH     0           // Eventchannel to use
#define     WS_EVSYS_MUXVAL 0b11000000  // Event selection == TCC0OVF
//#define     WS_EVSYS_MUXVAL 0b11000000
#define     WS_WGMODE       0x03        // WGMODE_SS
#define     WS_OUTPORT      PORTC       // output port
#define     WS_OUTPIN       0           // output pin

#endif


//----- Functions

#ifndef __ASSEMBLER__

//****************************************************************************
//  void WS_init(void)
//
//  DESCR:
//      Initialize hardware
//
void WS_init(void);


//****************************************************************************
//  TINY+ATMEGA:
//      void WS_out(uint8_t *data, uint16_t count, const uint8_t *gamma);
//
//  XMEGA:
//      uint8_t WS_out(uint8_t *data, uint16_t count, const uint8_t *gamma);
//
//  DESCR:
//      Output bytes to ws2812 leds using fastpwm with gamma correction using
//      the lookup table specified.
//
//      ATTINY+ATMEGA: When gamma is non-null, due to implementation
//      constraints, there is ONE MORE byte read at end of data from memory,
//      but not output.
//
//      XMEGA: If output is already in progress (DMA), this function will
//      return an error.
//
//  IN:
//      r24: uint8_t *data      Ptr. to bytes to output
//      r22: uint16_t count     Number of bytes
//      r20: uint8_t *gamma     Ptr. to GammaTable in FLASH, NULL = none
//
//  OUT: (XMega only)
//      r24: uint8_t status      0=OK, 1=DMA busy
//
#if defined WS_ARCH_TINY || defined WS_ARCH_MEGA
void WS_out(uint8_t *data, uint16_t count, const uint8_t *gamma);
#else
uint8_t WS_out(uint8_t *data, uint16_t count, const uint8_t *gamma);
#endif


//****************************************************************************
//  XMEGA:
//      uint8_t WS_outCheck(void)
//
//  DESCR
//      Check if output of last data is finished.
//
//  IN:
//
//  OUT:
//      r24: uint8_t status      0=OK, 1=DMA busy
//
#ifdef WS_ARCH_XMEGA
uint8_t WS_outCheck(void);
#endif

#endif // __ASSEMBLER__

#endif // WS2812_H_
#ifdef __cplusplus
}
#endif