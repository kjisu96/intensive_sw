/**********************************************************************************************************************
 * \file Cpu0_Main.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are solely in the form of
 * machine-executable object code generated by a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "IfxCcu6_reg.h"        // for CCU60 clock enable request
#include "IfxVadc_reg.h"        // VADC
#include "IfxGtm_reg.h"

// Port registers
#define PC0_BIT_LSB_IDX     3       // SW2
#define PC1_BIT_LSB_IDX     11      // SW3
#define PC2_BIT_LSB_IDX     19
#define PC4_BIT_LSB_IDX     3
#define PC6_BIT_LSB_IDX     19
#define P0_BIT_LSB_IDX      0
#define P1_BIT_LSB_IDX      1
#define P2_BIT_LSB_IDX      2
#define P6_BIT_LSB_IDX      6

// SCU registers
#define LCK_BIT_LSB_IDX     1
#define ENDINIT_BIT_LSB_IDX 0
#define EXIS0_BIT_LSB_IDX   4
#define FEN0_BIT_LSB_IDX    8       // falling edge enable
#define REN0_BIT_LSB_IDX    9       // rising edge enable
#define EIEN0_BIT_LSB_IDX   11
#define INP0_BIT_LSB_IDX    12
#define IGP0_BIT_LSB_IDX    14

// SRC registers
#define SRPN_BIT_LSB_IDX    0
#define TOS_BIT_LSB_IDX     11
#define SRE_BIT_LSB_IDX     10

// CCU60 registers
#define DISS_BIT_LSB_IDX    1
#define DISR_BIT_LSB_IDX    0
#define CTM_BIT_LSB_IDX     7
#define T12PRE_BIT_LSB_IDX  3
#define T12CLK_BIT_LSB_IDX  0
#define T12STR_BIT_LSB_IDX  6
#define T12RS_BIT_LSB_IDX   1
#define ENT12PM_BIT_LSB_IDX 7
#define INPT12_BIT_LSB_IDX  10
#define SRPN_BIT_LSB_IDX    0
#define TOS_BIT_LSB_IDX     11
#define SRE_BIT_LSB_IDX     10

// RGBLED registers
#define PC7_BIT_LSB_IDX     27  // P2.7
#define PC5_BIT_LSB_IDX     11  // P10.5
#define PC3_BIT_LSB_IDX     27  // P10.3
#define P7_BIT_LSB_IDX      7
#define P5_BIT_LSB_IDX      5
#define P4_BIT_LSB_IDX      4
#define P3_BIT_LSB_IDX      3
#define PRIO0_BIT_LSB_IDX   0
#define CSM0_BIT_LSB_IDX    3
#define ASEN0_BIT_LSB_IDX   24
#define ENGT_BIT_LSB_IDX    0
#define FLUSH_BIT_LSB_IDX   10
#define ANONC_BIT_LSB_IDX   0
#define CMS_BIT_LSB_IDX     8
#define RESPOS_BIT_LSB_IDX  21
#define RESREG_BIT_LSB_IDX  16
#define ICLSEL_BIT_LSB_IDX  0
#define ASSCH7_BIT_LSB_IDX  7
#define TREV_BIT_LSB_IDX    9
#define VF_BIT_LSB_IDX      31
#define RESULT_BIT_LSB_IDX  0

// GTM registers
#define FXCLK_SEL_BIT_LSB_IDX   0
#define EN_FXCLK_BIT_LSB_IDX    22
#define SEL7_BIT_LSB_IDX        14
#define UPEN_CTRL1_BIT_LSB_IDX  18
#define ENDIS_CTRL1_BIT_LSB_IDX 2
#define OUTEN_CTRL1_BIT_LSB_IDX 2
#define SL_BIT_LSB_IDX          11
#define CLK_SRC_CR_BIT_LSB_IDX  12
#define OSM_BIT_LSB_IDX         26
#define TRIGOUT_BIT_LSB_IDX     24
#define HOST_TRIG_BIT_LSB_IDX   0
#define SEL7_BIT_LSB_IDX        14
#define SEL9_BIT_LSB_IDX        18
#define SEL11_BIT_LSB_IDX       22
#define CLK_SRC_SR_BIT_LSB_IDX  12

IfxCpu_syncEvent g_cpuSyncEvent = 0;

unsigned int range;
unsigned char range_valid_flag = 0;
unsigned int glb_start_flag = 0;

void initUSonic(void)
{
    P02_IOCR4.U &= ~(0x1F << PC6_BIT_LSB_IDX);  // reset P02_IOCR4 PC6
    P00_IOCR4.U &= ~(0x1F << PC4_BIT_LSB_IDX);  // reset P00_IOCR4 PC4

    P00_IOCR4.U |= 0x01 << PC4_BIT_LSB_IDX;     // (***)set P00.4 general input (pull-down connected) [Echo]
    P02_IOCR4.U |= 0x10 << PC6_BIT_LSB_IDX;     // set P02.6 push-pull general output                 [Trig]

    P02_OUT.U &= ~(0x1 << P6_BIT_LSB_IDX);
}

void usonicTrigger(void)
{
    // start of 10us Trigger Pulse
    // GPIO P02.0 --> HIGH
    P02_OUT.U |= 0x1 << P6_BIT_LSB_IDX;
    range_valid_flag = 0;
    CCU60_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;       // T12 start counting

}

void initGTM(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);    // wait until locked

    GTM_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);    // enable GTM

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);    // wait until unlocked

    // Modify Access to set ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);

    while ((GTM_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0); // wait until GTM module enabled


    // GTM clock configuration
    GTM_CMU_FXCLK_CTRL.U &= ~(0xF << FXCLK_SEL_BIT_LSB_IDX);  // input clock of CMU_FXCLK --> CMU_GCLK_EN
    GTM_CMU_CLK_EN.U |= 0x2 << EN_FXCLK_BIT_LSB_IDX;        // enable all CMU_FXCLK

    // set TGC0 to enable GTM TOM0 channel 1
    GTM_TOM0_TGC0_GLB_CTRL.U |= 0x2 << UPEN_CTRL1_BIT_LSB_IDX;  // TOM channel 1 enable
    GTM_TOM0_TGC0_ENDIS_CTRL.U |= 0x2 << ENDIS_CTRL1_BIT_LSB_IDX;   // enable channel 1 on update trigger
    GTM_TOM0_TGC0_OUTEN_CTRL.U |= 0x2 << OUTEN_CTRL1_BIT_LSB_IDX;   // enable channel 1 output on update trigger


    // set TGC0 to enable GTM TOM0 channel 2, 3, 15
    GTM_TOM0_TGC0_GLB_CTRL.B.UPEN_CTRL2 |= 0x2;  // TOM0 channel 2 enable
    GTM_TOM0_TGC0_GLB_CTRL.B.UPEN_CTRL3 |= 0x2;  // TOM0 channel 3 enable
    GTM_TOM0_TGC1_GLB_CTRL.B.UPEN_CTRL7 |= 0x2;  // TOM0 channel 15 enable

    GTM_TOM0_TGC0_ENDIS_CTRL.B.ENDIS_CTRL2 |= 0x2;   // enable channel 2 on update trigger
    GTM_TOM0_TGC0_ENDIS_CTRL.B.ENDIS_CTRL3 |= 0x2;   // enable channel 3 on update trigger
    GTM_TOM0_TGC1_ENDIS_CTRL.B.ENDIS_CTRL7 |= 0x2;   // enable channel 15 on update trigger

    GTM_TOM0_TGC0_OUTEN_CTRL.B.OUTEN_CTRL2 |= 0x2;   // enable channel 2 output on update trigger
    GTM_TOM0_TGC0_OUTEN_CTRL.B.OUTEN_CTRL3 |= 0x2;   // enable channel 3 output on update trigger
    GTM_TOM0_TGC1_OUTEN_CTRL.B.OUTEN_CTRL7 |= 0x2;   // enable channel 15 output on update trigger


    // TOM 0_1
    GTM_TOM0_CH1_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;                   // high signal level for duty cycle

    GTM_TOM0_CH1_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH1_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;           // clock source --> CMU_FXCLK(1) = 6250 kHz

    GTM_TOM0_CH1_SR0.U = 12500 - 1;                                 // PWM freq. = 6250 kHz / 12500 = 250 kHz
    GTM_TOM0_CH1_SR1.U = 1250 - 1;                                  // duty cycle = 6250 / 12500 = 50 %

    // TOM 0_2
    GTM_TOM0_CH2_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;                   // high signal level for duty cycle

    GTM_TOM0_CH2_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH2_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;           // clock source --> CMU_FXCLK(1) = 6250 kHz

    GTM_TOM0_CH2_SR0.U = 12500 - 1;                                 // PWM freq. = 6250 kHz / 12500 = 250 kHz
    //GTM_TOM0_CH2_SR1.U = 12500 - 1;                               // duty cycle = 6250 / 12500 = 50 %

    // TOM 0_3
    GTM_TOM0_CH3_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;                   // high signal level for duty cycle

    GTM_TOM0_CH3_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH3_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;           // clock source --> CMU_FXCLK(1) = 6250 kHz

    GTM_TOM0_CH3_SR0.U = 12500 - 1;                                 // PWM freq. = 6250 kHz / 12500 = 250 kHz
    //GTM_TOM0_CH3_SR1.U = 125 - 1;                                 // duty cycle = 6250 / 12500 = 50 %

    // TOM 0_15
    GTM_TOM0_CH15_CTRL.B.SL |= 0x1;                                 // high signal level for duty cycle
    GTM_TOM0_CH15_CTRL.B.CLK_SRC_SR |= 0x1;                         // clock source --> CMU_FXCLK(1) = 6250 kHz

    GTM_TOM0_CH15_SR0.U = 12500 - 1;                                // PWM freq. = 6250 kHz / 12500 = 250 kHz
    //GTM_TOM0_CH15_SR1.U = 125 - 1;                                // duty cycle = 6250 / 12500 = 50 %


    // TOUT pin selection
    GTM_TOUTSEL6.U &= ~(0x3 << SEL7_BIT_LSB_IDX);                   // TOUT103  --> TOM0 channel 1
    GTM_TOUTSEL0.U &= ~(0x3 << SEL7_BIT_LSB_IDX);                   // TOUT7    --> TOM0 channel 15
    GTM_TOUTSEL6.U &= ~(0x3 << SEL11_BIT_LSB_IDX);                  // TOUT103  --> TOM0 channel 2
    GTM_TOUTSEL6.U &= ~(0x3 << SEL9_BIT_LSB_IDX);                   // TOUT105  --> TOM0 channel 3





    // set GTM TOM0 channel 11 - Buzzer
    GTM_TOM0_TGC1_GLB_CTRL.B.UPEN_CTRL3 |= 0x2;                      // TOM0 channel 11 enable
    GTM_TOM0_TGC1_ENDIS_CTRL.B.ENDIS_CTRL3 |= 0x2;                   // enable channel 11 on update trigger
    GTM_TOM0_TGC1_OUTEN_CTRL.B.OUTEN_CTRL3 |= 0x2;                   // enable channel 11 output on update trigger


    // TOM 0_11
    GTM_TOM0_CH11_CTRL.B.SL = 0x1;                                  // high signal level for duty cycle
    GTM_TOM0_CH11_CTRL.B.CLK_SRC_SR = 0x1;                          // clock source --> CMU_FXCLK(1) = 6250 kHz
    GTM_TOM0_CH11_SR0.B.SR0 = 12500 - 1;                            // PWM freq. = 6250 kHz / 12500 = 500 Hz
    GTM_TOM0_CH11_SR1.B.SR1 = 6250 - 1;                             // duty cycle = 6250 / 12500 = 50 %

    // TOUT pin selection
    GTM_TOUTSEL0.B.SEL3 = 0x0;                                      // TOUT3  --> TOM0 channel 11
}

void initPWMLED(void)
{
    P10_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);  // reset P10_IOCR0 PC1
    P10_IOCR0.U &= ~(0x1F << PC2_BIT_LSB_IDX);  // reset P10_IOCR0 PC2

    P10_IOCR0.U |= 0x11 << PC1_BIT_LSB_IDX;     // (***)set P10.1 GTM Output (PWM !!!)
    P10_IOCR0.U |= 0x10 << PC2_BIT_LSB_IDX;     // set P10.2 push-pull general output
}

void initVADC(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);      // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);      // wait until locked

    VADC_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);     // enable VADC

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);      // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);      // wait until locked

    // VADC configurations
    while((VADC_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);     // wait until VADC module enabled

    VADC_G4_ARBPR.U |= 0x3 << PRIO0_BIT_LSB_IDX;            // highest priority for Request Source 0
    VADC_G4_ARBPR.U &= ~(0x1 << CSM0_BIT_LSB_IDX);          // Wait-for-Start Mode
    VADC_G4_ARBPR.U |= 0x1 << ASEN0_BIT_LSB_IDX;            // Arbitration Source Input 0 Enable

    VADC_G4_QMR0.U &= ~(0x3 << ENGT_BIT_LSB_IDX);
    VADC_G4_QMR0.U |= 0x1 << ENGT_BIT_LSB_IDX;              // enable conversion request
    VADC_G4_QMR0.U |= 0x1 << FLUSH_BIT_LSB_IDX;             // clear ADC queue

    VADC_G4_ARBCFG.U |= 0x3 << ANONC_BIT_LSB_IDX;           // ADC normal operation

    VADC_G4_ICLASS0.U &= ~(0x7 << CMS_BIT_LSB_IDX);         // Class 0 Standard Conversion (12-bit)


    // VADC Group 4 Channel 7 configuration
    VADC_G4_CHCTR7.U |= 0x1 << RESPOS_BIT_LSB_IDX;          // result right-aligned
    VADC_G4_CHCTR7.U &= ~(0xF << RESREG_BIT_LSB_IDX);       // store result @ Result Register G4RES0
    VADC_G4_CHCTR7.U &= ~(0x3 << ICLSEL_BIT_LSB_IDX);       // Class 0

    VADC_G4_CHASS.U |= 0x1 << ASSCH7_BIT_LSB_IDX;
}

void initRGBLED(void)
{
    // reset Port IOCR register
    P02_IOCR4.U &= ~(0x1F << PC7_BIT_LSB_IDX);  // P2.7 setting
    P10_IOCR4.U &= ~(0x1F << PC5_BIT_LSB_IDX);  // P10.5
    P10_IOCR0.U &= ~(0x1F << PC3_BIT_LSB_IDX);  // P10.3

    // set Port as general purpose output (push-pull)
    P02_IOCR4.U |= 0x10 << PC7_BIT_LSB_IDX;     // set Port02's pin 7 as OUTPUT mode.
    P10_IOCR4.U |= 0x10 << PC5_BIT_LSB_IDX;
    P10_IOCR0.U |= 0x10 << PC3_BIT_LSB_IDX;
}

void VADC_startConversion(void)
{
    VADC_G4_QINR0.U |= 0x07;                    // no. of Request Channel = 7

    VADC_G4_QMR0.U |= 0x1 << TREV_BIT_LSB_IDX;  // Generate Conversion Start Trigger Event
}

unsigned int VADC_readResult(void)
{
    unsigned int result;

    while( (VADC_G4_RES0.U & (0x1 << VF_BIT_LSB_IDX)) == 0 );   // wait until read available
    result = VADC_G4_RES0.U & (0xFFF << RESULT_BIT_LSB_IDX);    // read ADC value

    return result;
}

void initButton(void)
{
    /* SW 3 */
    P02_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);  // reset P02_IOCR0 PC1

    P02_IOCR0.U |= 0x02 << PC1_BIT_LSB_IDX;    // set P02.1 general input (pull-up connected)

    /* SW 2 */
    //P02_IOCR0.U &= ~(0x1F << PC0_BIT_LSB_IDX);  // reset P02_IOCR0 PC0

    //P02_IOCR0.U |= 0x02 << P0_BIT_LSB_IDX;    // set P02.1 general input (pull-up connected)

}

void initLED(void)
{
    P10_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);  // reset P10_IOCR0 PC1
    P10_IOCR0.U &= ~(0x1F << PC2_BIT_LSB_IDX);  // reset P10_IOCR0 PC2

    P10_IOCR0.U |= 0x10 << PC1_BIT_LSB_IDX;     // set P10.1 push-pull general output
    P10_IOCR0.U |= 0x10 << PC2_BIT_LSB_IDX;     // set P10.2 push-pull general output

}

void initERU(void)
{
    // ERU setting
    SCU_EICR1.U &= ~(0x7 << EXIS0_BIT_LSB_IDX);
    SCU_EICR1.U |= (0x2  << EXIS0_BIT_LSB_IDX);  // ERS2 - Input 2

    /****/
    SCU_EICR1.U |= 0x1  << FEN0_BIT_LSB_IDX;    // falling edge
    SCU_EICR1.U |= 0x1  << REN0_BIT_LSB_IDX;    // rising edge
    /****/

    SCU_EICR1.U |= 0x1  << EIEN0_BIT_LSB_IDX;

    SCU_EICR1.U &= ~(0x7 << INP0_BIT_LSB_IDX);

    SCU_IGCR0.U &= ~(0x3 << IGP0_BIT_LSB_IDX);
    SCU_IGCR0.U |= 0x1  << IGP0_BIT_LSB_IDX;

    // SRC Interrupt setting
    SRC_SCU_SCU_ERU0.U &= ~(0xFF << SRPN_BIT_LSB_IDX);  // clear (8-bit)
    SRC_SCU_SCU_ERU0.U |= 0x0A   << SRPN_BIT_LSB_IDX;   // set 0x0A


    SRC_SCU_SCU_ERU0.U &= ~(0x3 << TOS_BIT_LSB_IDX);

    SRC_SCU_SCU_ERU0.U |= 1     << SRE_BIT_LSB_IDX;
}

void initCCU60(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);      // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);      // wait until locked

    CCU60_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);    // enable CCY

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);  // ?�? ??�씪
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);      // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);   // ENDINIT_BIT_LSB_IDX
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);      // wait until locked

    // CCU60 T12 configurations
    while((CCU60_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);    // wait until CCU60 module enabled
/*
    CCU60_TCTR0.U &= ~(0x7 << T12CLK_BIT_LSB_IDX);      // f_T12 = f_CCU6 / prescaler
    CCU60_TCTR0.U |= 0x2 << T12CLK_BIT_LSB_IDX;         // f_CCU6 = 50 Mhz, prescaler = 1024
    CCU60_TCTR0.U |= 0x1 << T12PRE_BIT_LSB_IDX;         // f_T12 = 48,828 Hz
*/
    /* 12.5 Hz setting */
    CCU60_TCTR0.U &= ~(0x7 << T12CLK_BIT_LSB_IDX);      // f_T12 = f_CCU6 / prescaler
    CCU60_TCTR0.U |= 0x2 << T12CLK_BIT_LSB_IDX;         // f_CCU6 = 50 Mhz, prescaler = 4
    //CCU60_TCTR0.U |= 0x1 << T12PRE_BIT_LSB_IDX;         // f_T12 = 12.5 Hz

    CCU60_TCTR0.U &= ~(0x1 << CTM_BIT_LSB_IDX);         // T12 auto reset when period match (PM) occur


    CCU60_T12PR.U =  12207 -1;                      // PM interrupt freq. = f T12 / (T12PR + 1) (24414 : 0.5s)
    CCU60_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;     // load T112PR from shadow register


    CCU60_TCTR2.B.T12SSC = 0x1;     // Single Shot Control

    CCU60_T12.U = 0;                                // clear T12 counter register

    CCU60_IEN.U |= 0x1 << ENT12PM_BIT_LSB_IDX;      // enable T12 PM interrupt
    CCU60_INP.U &= ~(0x3 << INPT12_BIT_LSB_IDX);    // service request output SR0 selected

    // SRC setting for CCU60
    SRC_CCU6_CCU60_SR0.U &= ~(0xff << SRPN_BIT_LSB_IDX);
    SRC_CCU6_CCU60_SR0.U |= 0x0B << SRPN_BIT_LSB_IDX;       // set priority 0x0B

    SRC_CCU6_CCU60_SR0.U &= ~(0x3 << TOS_BIT_LSB_IDX);      // CPU0 service T12 PM interrupt

    SRC_CCU6_CCU60_SR0.U |= 0x1 << SRE_BIT_LSB_IDX;         // SR0 enabled

    // T13 configurations for led blink
    CCU60_TCTR0.B.T13CLK = 0x2;     // f_CCU6 = 50 MHz, prescaler = 1024
    CCU60_TCTR0.B.T13PRE = 0x1;     // prescaler enable
    CCU60_T13PR.B.T13PV = 36621 - 1;
    CCU60_TCTR4.B.T13STR = 0x1;
    CCU60_T13.B.T13CV = 0x0;
    CCU60_IEN.B.ENT13PM = 0x1;
    CCU60_INP.B.INPT13 = 0x1;       // SR1
    SRC_CCU6_CCU60_SR1.B.SRPN = 0x0C;
    SRC_CCU6_CCU60_SR1.B.TOS = 0x0;
    SRC_CCU6_CCU60_SR1.B.SRE = 0x1;
    CCU60_TCTR4.B.T13RS = 0x1;
}

void initCCU61(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);      // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);      // wait until locked

    CCU61_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);    // enable CCY

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);      // wait until unlocked

    // Modify Access to clear ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);      // wait until locked

    // CCU61 T12 configurations
    while((CCU61_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);// wait until CCU61 module enabled  => 0

    CCU61_TCTR0.U &= ~(0x7 << T12CLK_BIT_LSB_IDX);      // f_T12 = f_CCU6 / prescaler
    CCU61_TCTR0.U |= 0x2 << T12CLK_BIT_LSB_IDX;         // f_CCU6 = 50 Mhz, prescaler = 4

    CCU61_TCTR0.U |= 0x1 << T12PRE_BIT_LSB_IDX;         // f_T12 = f_CCU6 / 255 = 48,828 Hz

    CCU61_T12PR.U =  100000 -1;                         // PM interrupt freq. = f T12 / (T12PR + 1) (24414 : 0.5s)
    CCU61_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;         // load T112PR from shadow register

    CCU61_TCTR2.B.T12SSC = 0x1;                         // Single Shot Control

    CCU61_T12.U = 0;                                    // clear T12 counter register

}

void initBuzzer(void)
{
    P02_IOCR0.B.PC3 = 0x11;
}

__interrupt(0x0A) __vector_table(0)
void ERU0_ISR(void)
{
    if( (P00_IN.U & (0x1 << P4_BIT_LSB_IDX)) != 0 )     // rising edge of echo
    {
        //                     ___________
        //      echo _________|
        //                    ^
        CCU61_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;       // start CCU61 T12 counter
    }
    else                                                // falling edge of echo
    {
        //           _________
        //      echo          |___________
        //                    ^
        CCU61_TCTR4.B.T12RR = 0x1;      // stop CCU61 T12 counter

        // (1 / t_freq) * counter * 1000000 / 58 = centimeter
        range = ((CCU61_T12.B.T12CV * 1000000) / 48828) / 58;
        range_valid_flag = 1;

        CCU61_TCTR4.B.T12RES = 0x1;    // reset CCU61 T12 counter
    }
}

__interrupt(0x0B) __vector_table(0)
void CCU60_T12_ISR(void)
{
    // end of 10us Trig
    // GPIO P02.6 --> LOW
    P02_OUT.U &= ~(0x1 << P6_BIT_LSB_IDX);
}

__interrupt(0x0C) __vector_table(0)
void CCU60_T13_ISR(void)
{
    P10_OUT.U ^= 0x1 << P2_BIT_LSB_IDX;  // toggle P10.1 (LED D12 RED)
    P10_OUT.U ^= 0x1 << P1_BIT_LSB_IDX;  // toggle P10.2 (LED D13 BLUE)
}

void systemoff(void)
{
    // LED off
    P10_OUT.U &= ~(0x1 << P2_BIT_LSB_IDX);  // toggle P10.1 (LED D12 RED)
    P10_OUT.U &= ~(0x1 << P1_BIT_LSB_IDX);  // toggle P10.2 (LED D13 BLUE)

    // RGBLED off
    P02_OUT.U &= ~(0x1 << P7_BIT_LSB_IDX);
    P10_OUT.U &= ~(0x1 << P5_BIT_LSB_IDX);
    P10_OUT.U &= ~(0x1 << P3_BIT_LSB_IDX);

    // Buzzer - program off sound
    // from 3 octave C ~ 4 octave C {C, D, E, F, G, A, B, C}
    unsigned int duty[8] = { 130, 146, 164, 174, 195, 220, 246, 262 };

    GTM_TOM0_CH11_SR0.B.SR0 = 6250000 / duty[7];
    GTM_TOM0_CH11_SR1.B.SR1 = 3125000 / duty[7];

    for(unsigned int i = 0; i < 20000000; i++);

    GTM_TOM0_CH11_SR0.B.SR0 = 6250000 / duty[0];
    GTM_TOM0_CH11_SR1.B.SR1 = 3125000 / duty[0];

    for(unsigned int i = 0; i < 20000000; i++);

    // buzzer off
    GTM_TOM0_CH11_SR0.B.SR0 = 0;
    GTM_TOM0_CH11_SR1.B.SR1 = 0;

}

int core0_main(void)
{
    IfxCpu_enableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    initButton();
    initRGBLED();
    initLED();

    while(1)
    {
        int pre = P02_IN.U & (0x1 << P1_BIT_LSB_IDX);
        for(uint32 j = 0; j < 100; j++);
        int cur = P02_IN.U & (0x1 << P1_BIT_LSB_IDX);

        if(pre ^ cur)   // sw3 button pushed
        {
            glb_start_flag = 1;
            break;
        }
    }
    for (unsigned int i = 0; i < 20000000; i++);    // delay

    initERU();
    initCCU60();
    initCCU61();
    initVADC();
    initGTM();
    initBuzzer();
    initUSonic();

    GTM_TOM0_TGC0_GLB_CTRL.U |= 0x1 << HOST_TRIG_BIT_LSB_IDX;       // trigger update request signal
    GTM_TOM0_TGC1_GLB_CTRL.U |= 0x1 << HOST_TRIG_BIT_LSB_IDX;       // trigger update request signal

    // buzzer off
    GTM_TOM0_CH11_SR0.B.SR0 = 0;
    GTM_TOM0_CH11_SR1.B.SR1 = 0;

    // from 3 octave C ~ 4 octave C {C, D, E, F, G, A, B, C}
    unsigned int duty[8] = { 130, 146, 164, 174, 195, 220, 246, 262 };

    while(1)
    {
        // Detect program off button pushed
        if((P02_IN.U & (0x1 << P1_BIT_LSB_IDX)) == 0)
        {
            if( glb_start_flag == 1 ) break;
        }


        // get ADC result
        unsigned int adcResult;
        VADC_startConversion();
        adcResult = VADC_readResult();


        // usonic sensor
        usonicTrigger();
        while (range_valid_flag == 0)
        {
            if((P02_IN.U & (0x1 << P1_BIT_LSB_IDX)) == 0)
            {
                if( glb_start_flag == 1 ) break;
            }
        }

        int delay_idx = 0;
        unsigned int delay[3] = {10000000, 5000000, 1000000};   // delay for RGB LED blink & buzzer

        if (range >= 50) // blue
        {
            P02_OUT.U &= ~(0x1 << P7_BIT_LSB_IDX);
            P10_OUT.U &= ~(0x1 << P5_BIT_LSB_IDX);
            P10_OUT.U |= (0x1 << P3_BIT_LSB_IDX);

            for (unsigned int i = 0; i < 20000000; i++);

            continue;     // don't blink when distance >= 50cm
        }

        else if (range >= 30) // green
        {

            P02_OUT.U &= ~(0x1 << P7_BIT_LSB_IDX);
            P10_OUT.U |= (0x1 << P5_BIT_LSB_IDX);
            P10_OUT.U &= ~(0x1 << P3_BIT_LSB_IDX);

            delay_idx = 0;
        }

        else if (range >= 15) // orange
        {

            P02_OUT.U |= (0x1 << P7_BIT_LSB_IDX);
            P10_OUT.U |= (0x1 << P5_BIT_LSB_IDX);
            P10_OUT.U &= ~(0x1 << P3_BIT_LSB_IDX);

            delay_idx = 1;
        }

        else // red
        {
            P02_OUT.U |= 0x1 << P7_BIT_LSB_IDX;
            P10_OUT.U &= ~(0x1 << P5_BIT_LSB_IDX);
            P10_OUT.U &= ~(0x1 << P3_BIT_LSB_IDX);

            delay_idx = 2;
        }

        for (unsigned int i = 0; i < delay[delay_idx]; i++);    // RGB LED delay

        // rgb led off
        P02_OUT.U &= ~(0x1 << P7_BIT_LSB_IDX);
        P10_OUT.U &= ~(0x1 << P5_BIT_LSB_IDX);
        P10_OUT.U &= ~(0x1 << P3_BIT_LSB_IDX);


        // run buzzer
        for (unsigned int i = 0; i < 1000000; i++);

        int idx_duty = 2  * (3 - adcResult / 1024) + 1;         // set buzzer delay idx
        if (idx_duty < 1) idx_duty = 0;

        GTM_TOM0_CH11_SR0.B.SR0 = 6250000 / duty[idx_duty];     // set buzzer key
        GTM_TOM0_CH11_SR1.B.SR1 = 3125000 / duty[idx_duty];

        for (unsigned int i = 0; i < delay[delay_idx]; i++);    // buzzer delay


        // buzzer off
        GTM_TOM0_CH11_SR0.B.SR0 = 0;
        GTM_TOM0_CH11_SR1.B.SR1 = 0;

    }

    systemoff();
    return (1);
}

