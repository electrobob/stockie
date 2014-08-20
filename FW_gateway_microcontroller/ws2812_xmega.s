//****************************************************************************
//  ws2812 - Serial driver for ws2812 RGB leds using fastpwm
//
//  ws2812_xmega - driver for atxmega architecture
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

#if defined WS_ARCH_XMEGA

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


//  Private data struct for DMA mode

    STRUCT  WSD
    PTR     WSD_OutPtr                  // Ptr. to output data
    UWORD   WSD_OutCnt                  // Output bytes left
    PTR     WSD_BufAPtr                 // Ptr to Buffer A
    PTR     WSD_BufBPtr                 // Ptr to Buffer B
    PTR     WSD_GammaPtr                // Ptr to gammatable
    UBYTE   WSD_EndFlag                 // Flag for end
    UBYTE   WSD_Fill
    LABEL   WSD_SizeOf
    
// Some brain-twisting macros used in asm source

#define     WS_DMA(CH, REG) WS_DMA_(WS_DMA_ ## CH, REG)
#define     WS_DMA_(CH, REG) WS_DMA__(CH, REG)
#define     WS_DMA__(CH, REG) DMA_ ## CH ## _ ## REG
#define     WS_DMA_R(CH, REG) WS_DMA(CH, REG)-WS_DMA(CH, CTRLA)
#define     WS_TC(TCC, REG) WS_TC_(TCC, REG)
#define     WS_TC_(TCC, REG) TCC ## _ ## REG
#define     WS_TC_R(TCC, REG) WS_TC(TCC, REG)-WS_TC(TCC, CTRLA)
#define     WS_PORT(PORT, REG) WS_PORT_(PORT, REG)
#define     WS_PORT_(PORT, REG) PORT ## _ ## REG
#define     WS_ENDBYTE  0xff
#define     WS_MERGE(A, B, C) WS_MERGE_(A, B, C)
#define     WS_MERGE_(A, B, C) A ## B ## C

#define     WS_EVSYS_MUX    WS_MERGE(EVSYS_CH, WS_EVSYS_CH, MUX) // EVSYS mux to use (CH0)
#define     WS_EVSYS_CTRL   WS_MERGE(EVSYS_CH, WS_EVSYS_CH, CTRL) // EVSYS ctrl to use (Ch0)



//****************************************************************************
//  void WS_init(void)
//
//  DESCR:
//      Initialize hardware
//
//  SCRATCH:
//      r24, r25, r26, r27, r30, r31
//
.global WS_init

WS_init:
    push    r28
    push    r29

    clr     r26

    // Initialize internal data structure

    ldi     r30, lo8(WSData)            // Z = Ptr. to internal data
    ldi     r31, hi8(WSData)

    ldi     r24, lo8(WS_BufferA)        // init. BufAPtr
    std     Z+WSD_BufAPtr, r24
    ldi     r24, hi8(WS_BufferA)
    std     Z+WSD_BufAPtr+1, r24

    ldi     r24, lo8(WS_BufferB)        // init. BufBPtr
    std     Z+WSD_BufBPtr, r24
    ldi     r24, hi8(WS_BufferB)
    std     Z+WSD_BufBPtr+1, r24

    // Init IO port

    ldi     r24, _BV(WS_OUTPIN)
    sts     WS_MERGE(WS_OUTPORT, _OUTCLR,), r24 // Set output low
    sts     WS_MERGE(WS_OUTPORT, _DIRSET,), r24 // Set DDR for output

    // Initialize DMA stuff

    lds     r24, DMA_CTRL
    ori     r24, DMA_ENABLE_bm|WS_DMA_DBUFMODE_bm
    sts     DMA_CTRL, r24

    ldi     r28, lo8(WS_DMA(A, CTRLA))  // Y = DMA ch A BASE
    ldi     r29, hi8(WS_DMA(A, CTRLA))
    ldi     r30, lo8(WS_DMA(B, CTRLA))  // Z = DMA ch B BASE
    ldi     r31, hi8(WS_DMA(B, CTRLA))

    ldi     r24, DMA_CH_SINGLE_bm|DMA_CH_REPEAT_bm|(0x00<<DMA_CH_BURSTLEN_gp)
    std     Y+WS_DMA_R(A, CTRLA), r24
    std     Z+WS_DMA_R(B, CTRLA), r24

    ldi     r24, WS_DMA_INTLVL<<DMA_CH_TRNINTLVL_gp
    std     Y+WS_DMA_R(A, CTRLB), r24
    std     Z+WS_DMA_R(B, CTRLB), r24

    ldi     r24, DMA_CH_SRCRELOAD0_bm|DMA_CH_SRCRELOAD1_bm|DMA_CH_SRCDIR0_bm
    std     Y+WS_DMA_R(A, ADDRCTRL), r24
    std     Z+WS_DMA_R(B, ADDRCTRL), r24

    ldi     r24, WS_DMA_TRIGSRC<<DMA_CH_TRIGSRC_gp
    std     Y+WS_DMA_R(A, TRIGSRC), r24
    std     Z+WS_DMA_R(B, TRIGSRC), r24

    std     Y+WS_DMA_R(A, REPCNT), r26
    std     Y+WS_DMA_R(A, REPCNT+1), r26
    std     Z+WS_DMA_R(B, REPCNT), r26
    std     Z+WS_DMA_R(B, REPCNT+1), r26

    ldi     r24, lo8(WS_BufferA)
    ldi     r25, hi8(WS_BufferA)
    std     Y+WS_DMA_R(A, SRCADDR0), r24
    std     Y+WS_DMA_R(A, SRCADDR1), r25
    std     Y+WS_DMA_R(A, SRCADDR2), r26

    ldi     r24, lo8(WS_BufferB)
    ldi     r25, hi8(WS_BufferB)
    std     Z+WS_DMA_R(B, SRCADDR0), r24
    std     Z+WS_DMA_R(B, SRCADDR1), r25
    std     Z+WS_DMA_R(B, SRCADDR2), r26

    ldi     r24, (WS_TC(WS_TCC, CCABUF))&0xff
    ldi     r25, (WS_TC(WS_TCC, CCABUF))>>8
    std     Y+WS_DMA_R(A, DESTADDR0), r24
    std     Y+WS_DMA_R(A, DESTADDR1), r25
    std     Y+WS_DMA_R(A, DESTADDR2), r26
    std     Z+WS_DMA_R(B, DESTADDR0), r24
    std     Z+WS_DMA_R(B, DESTADDR1), r25
    std     Z+WS_DMA_R(B, DESTADDR2), r26

    // Init event system to trigger DMA transfer for each timer overflow

    ldi     r24, WS_EVSYS_MUXVAL
    sts     WS_EVSYS_MUX, r24
    sts     WS_EVSYS_CTRL, r26

    // Init timer stuff

    ldi     r30, lo8(WS_TC(WS_TCC, CTRLA))  // Z = TCx base
    ldi     r31, hi8(WS_TC(WS_TCC, CTRLB))

    ldi     r24, TC0_CCAEN_bm|(WS_WGMODE<<TC0_WGMODE_gp) // enable timer
    std     Z+WS_TC_R(WS_TCC, CTRLB), r24
    std     Z+WS_TC_R(WS_TCC, CTRLC), r26

    ldi     r24, TC0_BYTEM0_bm
    std     Z+WS_TC_R(WS_TCC, CTRLE), r24

    ldi     r24, WS_CNT_TOTAL
    std     Z+WS_TC_R(WS_TCC, PERBUF), r24
    std     Z+WS_TC_R(WS_TCC, PERBUF+1), r26
    std     Z+WS_TC_R(WS_TCC, PER), r24
    std     Z+WS_TC_R(WS_TCC, PER+1), r26

    pop     r29
    pop     r28
    ret



//****************************************************************************
//      uint8_t WS_out(uint8_t *data, uint16_t count, const uint8_t *gamma);
//
//  DESCR:
//      Output bytes to ws2812 leds using fastpwm with gamma correction using
//      the lookup table specified.
//
//      If output is already in progress (DMA), this function will
//      return an error.
//
//  IN:
//      r24: uint8_t *data      Ptr. to bytes to output
//      r22: uint16_t count     Number of bytes
//      r20: uint8_t *gamma     Ptr. to GammaTable in FLASH, NULL = none
//
//  OUT:
//      r24: uint8_t status      0=OK, 1: DMA Busy
//
//  SCRATCH:
//      r0, r18, r19, r20, r21, r22/r23, r24/r25, r26/r27, r30/r31
//
.global WS_out

WS_out:
    push    r28
    push    r29

    lds     r18, DMA_STATUS             // check if DMA is still in progress
    andi    r18, WS_DMA_BUSY_bm
    breq    WS_out_notbusy              // zero -> no active DMA

    ldi     r24, 1                      // ret = 1 (busy)
    rjmp    WS_out0

WS_out_notbusy:

    // Fill internal structure with data

    ldi     r30, lo8(WSData)
    ldi     r31, hi8(WSData)

    std     Z+WSD_OutPtr, r24
    std     Z+WSD_OutPtr+1, r25
    std     Z+WSD_OutCnt, r22
    std     Z+WSD_OutCnt+1, r23
    std     Z+WSD_GammaPtr, r20
    std     Z+WSD_GammaPtr+1, r21
    std     Z+WSD_EndFlag, r1

    // Initially disable repeat mode for DMA channel A+B

    lds     r24, WS_DMA(A, CTRLA)       // disable repeat for channel A
    andi    r24, ~DMA_CH_REPEAT_bm
    sts     WS_DMA(A, CTRLA), r24

    lds     r24, WS_DMA(B, CTRLA)       // disable repeat for channel B
    andi    r24, ~DMA_CH_REPEAT_bm
    sts     WS_DMA(B, CTRLA), r24

    // At beginning, fill both buffers with data

    clr     r24                         // Buffer 0
    rcall   WS_WriteBuf

    sts     WS_DMA(A, TRFCNT), r24      // set number of bytes for DMA
    sts     WS_DMA(A, TRFCNT+1), r25

    ldd     r24, Z+WSD_EndFlag
    tst     r24                         // already end?
    brne    WS_out_start                // yes!

    lds     r24, WS_DMA(B, CTRLA)       // enable repeat for channel B
    ori     r24, DMA_CH_REPEAT_bm
    sts     WS_DMA(B, CTRLA), r24

    ldi     r24, 1                      // Buffer 1
    rcall   WS_WriteBuf

    sts     WS_DMA(B, TRFCNT), r24      // set number of byes for DMA
    sts     WS_DMA(B, TRFCNT+1), r25

    ldd     r24, Z+WSD_EndFlag
    tst     r24                         // already end?
    brne    WS_out_start                // yes!

    lds     r24, WS_DMA(A, CTRLA)       // enable repeat for channel A
    ori     r24, DMA_CH_REPEAT_bm
    sts     WS_DMA(A, CTRLA), r24

    // Enable DMA and start counter

WS_out_start:
    ldi     r24, 1
    sts     WS_TC(WS_TCC, CNT), r24     // Init counter with 1 to avoid HIGH state on first run
    sts     WS_TC(WS_TCC, CCA), r1

    lds     r24, WS_DMA(A, CTRLA)       // Enable DMA channel A
    ori     r24, DMA_CH_ENABLE_bm
    sts     WS_DMA(A, CTRLA), r24

    ldi     r24, (0x01<<TC0_CLKSEL_gp)  // Start timer
    sts     WS_TC(WS_TCC, CTRLA), r24

    ldi     r24, 1<<WS_EVSYS_CH         // STROBE EVSYS
    sts     EVSYS_STROBE, r24

    clr     r24

WS_out0:
    pop     r29
    pop     r28
    ret



//****************************************************************************
//  DMA_CHx_vect (private)
//
//  DESCR:
//      Interrupt subroutines called when DMA for channel a or b finishes.
//      As we are using double buffered mode, when one DMA finishes, we can
//      refill the associated buffer with new data.
//      When there is no more data available, the DMA for this channel will
//      be disabled.
//
//  IN:
//
//  OUT:
//
//  SCRATCH:
//      We have to save everything
//

    // Funny thing to mention: Disabling repeat for a channel in dbufmode
    // means that output will stop BEFORE it starts when the channel would
    // have becom active

    // Channel A finished

.global WS_MERGE(DMA_, WS_DMA_A, _vect)
WS_MERGE(DMA_, WS_DMA_A, _vect):
    MPUSH   r20, r21, r22, r23, r24
    in      r20, SREG
    push    r20

    ldi     r20, lo8(WS_DMA(A, CTRLA))  // r20/r21 = Base DMA A
    ldi     r21, hi8(WS_DMA(A, CTRLA))
    ldi     r22, lo8(WS_DMA(B, CTRLA))  // r22/r23 = Base DMA B
    ldi     r23, hi8(WS_DMA(B, CTRLA))
    clr     r24                         // Buffer = 0
    rcall   WS_DMA_ISR

    pop     r20
    out     SREG, r20
    MPOP    r24, r23, r22, r21, r20
    reti

.global WS_MERGE(DMA_, WS_DMA_B, _vect)
WS_MERGE(DMA_, WS_DMA_B, _vect):
    MPUSH   r20, r21, r22, r23, r24
    in      r20, SREG
    push    r20

    ldi     r20, lo8(WS_DMA(B, CTRLA))  // r20/r21 = Base DMA A
    ldi     r21, hi8(WS_DMA(B, CTRLA))
    ldi     r22, lo8(WS_DMA(A, CTRLA))  // r22/r23 = Base DMA B
    ldi     r23, hi8(WS_DMA(A, CTRLA))
    ldi     r24, 1                      // Buffer = 1
    rcall   WS_DMA_ISR

    pop     r20
    out     SREG, r20
    MPOP    r24, r23, r22, r21, r20
    reti

WS_DMA_ISR:
    MPUSH   r0, r1, r18, r19, r25, r26, r27, r28, r29, r30, r31

    mov     r0, r24                     // Signal for Debugging
    inc     r0
    lsl     r0
    sts     PORTC_OUTSET, r0

    clr     r1

    ldi     r30, lo8(WSData)            // Z = Ptr. to data struct
    ldi     r31, hi8(WSData)

    ldd     r0, Z+WSD_EndFlag           // already finished?
    tst     r0
    brne    WS_DMA_ISR_End              // yes!

    MPUSH   r20, r21, r22, r23
    rcall   WS_WriteBuf                 // Prepare DMA buffer
    MPOP    r23, r22, r21, r20

    movw    r28, r20                    // Y = Base of own DMA
    std     Y+WS_DMA_R(A, TRFCNT), r24  // set number of bytes for DMA
    std     Y+WS_DMA_R(A, TRFCNT+1), r25

    ldd     r0, Z+WSD_EndFlag
    tst     r0                          // end?
    breq    WS_DMA_ISR0                 // no

WS_DMA_ISR_stop:
    movw    r28, r22                    // Y = Base of other DMA
    ldd     r24, Y+WS_DMA_R(B, CTRLA)   // disable repeat
    andi    r24, ~DMA_CH_REPEAT_bm
    std     Y+WS_DMA_R(B, CTRLA), r24    

WS_DMA_ISR0:
    movw    r28, r20                    // Y = Base of own DMA
    ldd     r0, Y+WS_DMA_R(A, CTRLB)    // clear TRNIF flag
    std     Y+WS_DMA_R(A, CTRLB), r0

    ldi     r24, 0x06                   // clear debug signal
    sts     PORTC_OUTCLR, r24

    MPOP    r31, r30, r29, r28, r27, r26, r25, r19, r18, r1, r0
    ret

WS_DMA_ISR_End:
    movw    r28, r20                    // Y = Base of own DMA
    ldd     r24, Y+WS_DMA_R(A, CTRLA)   // disable DMA channel
    andi    r24, ~DMA_CH_ENABLE_bm
    std     Y+WS_DMA_R(A, CTRLA), r24

    movw    r28, r22                    // Y = Base of other DMA
    ldd     r24, Y+WS_DMA_R(B, CTRLA)   // still active?
    andi    r24, DMA_CH_REPEAT_bm
    brne    WS_DMA_ISR_stop             // yes

WS_DMA_ISR_WaitEnd:
    lds     r24, WS_TC(WS_TCC, CCA)     // endbyte in CCA set?
    cpi     r24, WS_ENDBYTE
    brne    WS_DMA_ISR_WaitEnd          // no -> wait

    sts     WS_TC(WS_TCC, CTRLA), r1    // stop timer
    sts     WS_TC(WS_TCC, CTRLC), r1    // force output low

    rjmp    WS_DMA_ISR0



//****************************************************************************
//  WS_WriteBuf (private)
//
//  DESCR:
//      Read bytes from input ptr, convert them into pwm values and write
//      them into the next available dma buffer.
//      When end of input data is reached, write 0xff as end byte.
//
//  IN:
//      r30/r31 (Z) : Ptr. to WSD
//      r24         : BUffer to write 0/1
//
//  OUT:
//      r24/r25 : Num bytes written
//
//  SCRATCH:
//      r0, r18, r19, r20, r21, r22/r23, r24/r25, r26/r27, r28/r29
//
WS_WriteBuf:
    clr     r1

    movw    r26, r30                    // WSD -> X
    lsl     r24                         // Buffer number *2
    ldi     r25, WSD_BufAPtr
    add     r24, r25
    add     r26, r24                    // Get adr. of WSD_BufxPtr
    adc     r27, r1

    ld      r28, X+                     // Get ptr. in Y
    ld      r29, X+

    ldd     r22, Z+WSD_OutCnt           // r22/r23,r24/r25 = Number of bytes left
    ldd     r23, Z+WSD_OutCnt+1
    movw    r24, r22
    adiw    r24, 0                      // zero bytes left?
    breq    WS_WriteBuf_OutEnd          // yes!

    ldi     r18, (WS_BUFNUM/2)&0xff
    ldi     r19, (WS_BUFNUM/2)>>8
    cp      r18, r24                    // bytes to output > numbuf/2?
    cpc     r19, r25
    brsh    WS_WriteBuf1

    mov     r24, r18                    // yes, bytes to output = numbuf/2
    mov     r25, r19

WS_WriteBuf1:
    sub     r22, r24                    // Num of bytes left -= bytes to output
    sbc     r23, r25
    std     Z+WSD_OutCnt, r22           // store
    std     Z+WSD_OutCnt+1, r23

    ldd     r26, Z+WSD_OutPtr           // r26/r27 (X) = ptr to output data
    ldd     r27, Z+WSD_OutPtr+1

    ldd     r22, Z+WSD_GammaPtr         // r22/r23 = Ptr. to gamma table
    ldd     r23, Z+WSD_GammaPtr+1

    push    r28
    push    r29
    push    r30
    push    r31

    rcall   WS_Byte2PWM

    pop     r31
    pop     r30

    std     Z+WSD_OutPtr, r26           // store new OutPtr
    std     Z+WSD_OutPtr+1, r27

    pop     r27
    pop     r26

    movw    r24, r28                    // calc. number bytes written in r24
    sub     r24, r26
    sbc     r25, r27   

    // Check if we have used whole buffer

    cpi     r24, (WS_BUFSIZE/2)&0xff
    brne    WS_WriteBuf_OutEnd
    cpi     r25, (WS_BUFSIZE/2)>>8
    breq    WS_WriteBuf0

    // We have no end of buffer reached, that means: We are finished with data
    // and also there is some place left in buffer to store the "end byte" 0xff

WS_WriteBuf_OutEnd:
    ldi     r22, WS_ENDBYTE             // write "end byte"
    st      Y, r22
    ldi     r22, 1                      // Set Endflag
    std     Z+WSD_EndFlag, r22
    adiw    r24, 1

WS_WriteBuf0:
    ret



//****************************************************************************
//  WS_Byte2PWM (private)
//
//  DESCR:
//      Convert input bytes to stream of PWM values
//
//  IN:
//      r22/r23     : Ptr. to Gammatable
//      r24/r25     : Number of bytes
//      r26/r27 (X) : Ptr. to bytes
//      r28/r29 (Y) : Ptr. to dest. buffer
//
//  OUT:
//      r26/r27 (X) : New ptr. to bytes
//      r28/r29 (Y) : New ptr. to dest. buffer behind
//
//  SCRATCH:
//      r0, r18, r19, r20, r21, r24/r25, r30/r31
//
WS_Byte2PWM:
    ldi     r19, WS_CNT_LOW             // r19 = value for low
    ldi     r20, WS_CNT_HIGH            // r20 = value for high
    mov     r21, r22
    or      r21, r23                    // r21 = flag for Gammatable

WS_Byte2PWM_loop2:
    sbiw    r24, 1
    brlt    WS_Byte2PWM0

    ld      r0, X+                      // Get next byte
    tst     r21                         // Gammatable?
    breq    WS_Byte2PWM_nogamma

    movw    r30, r22                    // start of gammatable into r30
    add     r30, r0                     // add byte val.
    adc     r31, r1
    lpm     r0, Z                       // get new value from table

WS_Byte2PWM_nogamma:
    ldi     r18, 8                      // r18 = Bitcounter

WS_Byte2PWM_loop:
    lsl     r0
    brcs    WS_Byte2PWM_high
    st      Y+, r19
    dec     r18
    brne    WS_Byte2PWM_loop
    rjmp    WS_Byte2PWM_loop2

WS_Byte2PWM_high:
    st      Y+, r20
    dec     r18
    brne    WS_Byte2PWM_loop
    rjmp    WS_Byte2PWM_loop2

WS_Byte2PWM0:
    ret



//****************************************************************************
//      uint8_t WS_outCheck(void)
//
//  DESCR
//      Check if output of last data is finished.
//
//  OUT:
//      r24: uint8_t status      0=OK, 1=DMA busy
//
.global WS_outCheck
    lds     r24, DMA_STATUS             // check if DMA is still in progress
    andi    r24, WS_DMA_BUSY_bm
    breq    WS_outCheck0                // zero -> no active DMA

    ldi     r24, 1                      // ret = 1 (busy)

WS_outCheck0:
    ret



//****************************************************************************
//  DATA
//
.section .bss
WSData:
    .space  WSD_SizeOf
WS_BufferA:
    .space  WS_BUFSIZE/2
WS_BufferA_End:
WS_BufferB:
    .space  WS_BUFSIZE/2
WS_BufferB_End:



#endif // WS_ARCH
#ifdef __cplusplus
}
#endif