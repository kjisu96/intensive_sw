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
#include <stdio.h>

#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "IfxCcu6_reg.h"
#include "IfxVadc_reg.h"
#include "IfxGtm_reg.h"

 // Port registers
#define PC1_BIT_LSB_IDX         11
#define PC2_BIT_LSB_IDX         19
#define PC3_BIT_LSB_IDX         27
#define PC4_BIT_LSB_IDX         3
#define PC5_BIT_LSB_IDX         11
#define PC6_BIT_LSB_IDX         19
#define PC7_BIT_LSB_IDX         27
#define P1_BIT_LSB_IDX          1
#define P2_BIT_LSB_IDX          2
#define P3_BIT_LSB_IDX          3
#define P4_BIT_LSB_IDX          4
#define P5_BIT_LSB_IDX          5
#define P6_BIT_LSB_IDX          6
#define P7_BIT_LSB_IDX          7

// SCU REGISTER
#define LCK_BIT_LSB_IDX         1
#define ENDINIT_BIT_LSB_IDX     0
#define EXIS0_BIT_LSB_IDX       4
#define FEN0_BIT_LSB_IDX        8
#define REN0_BIT_LSB_IDX        9
#define EIEN0_BIT_LSB_IDX       11
#define INP0_BIT_LSB_IDX        12
#define IGP0_BIT_LSB_IDX        14

// SRC REGISTERS
#define SRPN_BIT_LSB_IDX        0
#define TOS_BIT_LSB_IDX         11
#define SRE_BIT_LSB_IDX         10

// CCU60 REGISTERS
#define DISS_BIT_LSB_IDX        1
#define DISR_BIT_LSB_IDX        0
#define CTM_BIT_LSB_IDX         7
#define T12PRE_BIT_LSB_IDX      3
#define T12CLK_BIT_LSB_IDX      0
#define T12STR_BIT_LSB_IDX      6
#define T12RS_BIT_LSB_IDX       1
#define INPT12_BIT_LSB_IDX      10
#define ENT12PM_BIT_LSB_IDX     7
#define T12SSC_BIT_LSB_IDX      0

// VADC registers
#define ANONC_BIT_LSB_IDX       0
#define ASEN0_BIT_LSB_IDX       24
#define CSM0_BIT_LSB_IDX        3
#define PRIO0_BIT_LSB_IDX       0
#define CMS_BIT_LSB_IDX         8
#define FLUSH_BIT_LSB_IDX       10
#define TREV_BIT_LSB_IDX        9
#define ENGT_BIT_LSB_IDX        0
#define RESPOS_BIT_LSB_IDX      21
#define RESREG_BIT_LSB_IDX      16
#define ICLSEL_BIT_LSB_IDX      0
#define VF_BIT_LSB_IDX          31
#define RESULT_BIT_LSB_IDX      0
#define ASSCH7_BIT_LSB_IDX      7

// GTM registers
#define SEL7_BIT_LSB_IDX        14
#define SEL9_BIT_LSB_IDX        18
#define SEL11_BIT_LSB_IDX       22
#define EN_FXCLK_BIT_LSB_IDX    22
#define FXCLK_SEL_BIT_LSB_IDX   0

// GTM - TOM0 registers
#define UPEN_CTRL1_BIT_LSB_IDX  18
#define HOST_TRIG_BIT_LSB_IDX   0
#define ENDIS_CTRL1_BIT_LSB_IDX 2
#define OUTEN_CTRL1_BIT_LSB_IDX 2
#define CLK_SRC_SR_BIT_LSB_IDX  12
#define OSM_BIT_LSB_IDX         26
#define TRIGOUT_BIT_LSB_IDX     24
#define SL_BIT_LSB_IDX          11

// Handle (Potentiometer)
#define THRESHOLD_R 3096
#define THRESHOLD_L 1024
#define THRESHOLD_S 2048

#define DIR_R          2
#define DIR_L          1
#define DIR_S          0


IfxCpu_syncEvent g_cpuSyncEvent = 0;

/* Declaration Part */
void DriveStateChange(void);

void initCCU60(void);
void initCCU61(void);
void initERU(void);
void initLED(void);
void initGTM(void);
void initBuzzer(void);
void initUSonic(void);
void initRGBLED(void);
void disableBuzzer(void);

void PW_ULK(void);
void usonicTrigger(void);
void buzzerChange(unsigned short, int);
void RGBChange(void);
void RGBZero(void);

void SendMotorControl(int Direction, int Velocity);
void initGPIOMaster(void);

void initVADC(void) ;
void VADC_StartConvesrion(void);
unsigned int VADC_ReadResult(void);
void SetStateAfterSteering(int LED_FLAG);

/* Interrupt Part */
// USONIC activate
__interrupt(0x0A)   __vector_table(0) // interrupt on =>
void ERU0_ISR(void);

// ERU  activate [L]
__interrupt(0x0B)   __vector_table(0) // interrupt on =>
void ERU_T12_ISR_L(void);

// ERU  activate [R]
__interrupt(0x0C)   __vector_table(0) // interrupt on =>
void ERU_T12_ISR_R(void);

// Clock T12 activate
__interrupt(0x0D)   __vector_table(0) // interrupt on =>
void CCU60_T12_ISR(void);

// Clock T13 activate
__interrupt(0x0E)   __vector_table(0) // interrupt on =>
void CCU60_T13_ISR(void);


/* Variable Declare */
unsigned int    adcResult;
unsigned char   range_valid_flag = 0;
int             range;
unsigned int    duty[8] = { 130, 146, 164, 174, 195, 220, 246, 262 };
unsigned short  RGB_VAR = 0;
unsigned short  warning_array[3] = { 1, 15, 25 };
int             range_array[2] = { 45, 75 };
unsigned short  warning;
int             bright;
unsigned int adcResult ;
unsigned int step ;
unsigned int LED_OFF_FLAG = 0;

// LED [ H S J ]
int LeftT = 0, RightT = 0;
int  DRIVE_STATE = 0;    // 0 FORWARD 1 BACK


// Temp
unsigned short  BUZZER_CHECK = 1;

int core0_main(void)
{
    IfxCpu_enableInterrupts();

    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    // ***Init ACTIVATE PART
    initERU();
    initCCU60();
    initCCU61();
    initLED();      // initLED (PWM VERSION)
    initRGBLED();
    initUSonic();
    initGTM();      // generic timer module
    initBuzzer();
    initGPIOMaster();
    initVADC();

    step = 0 ;

    // PWM Activate Variable (only one time)
    GTM_TOM0_TGC0_GLB_CTRL.U |= 0x1 << HOST_TRIG_BIT_LSB_IDX;  // trigger update request signal
    GTM_TOM0_TGC1_GLB_CTRL.U |= 0x1 << HOST_TRIG_BIT_LSB_IDX;  // trigger update request signal

    static int temp_cnt = 0;

    while (1)
    {

        for (unsigned int i = 0; i < 10000000; i++);
        usonicTrigger();    // trigger ON if Button On state (HSJ)
        while (range_valid_flag == 0);

        if (DRIVE_STATE == 0)   // Drive Gear (Forward)
        {
            if (LeftT != 0 || RightT != 0)    // When Blinking Turn Signal ON
            {
                buzzerChange(warning, DRIVE_STATE);
                RGBChange();    /* RGB CHANGE */
            }
            else // When Blinking Turn Signal OFF -> Straight Drive -> AEB MODE
            {
                GTM_TOM0_CH11_SR1.U = 1;
                RGBZero();

                if (range <15)
                {
                    buzzerChange(15, DRIVE_STATE); RGBChange();
                }
            }
        }
        else if (DRIVE_STATE == 1) // Rear Gear (Back)
        {
            RGBChange();    /* RGB CHANGE */
            buzzerChange(warning, DRIVE_STATE);
        }

        /* Motor Control using UltraSonic range Info. */
        if (LeftT == 0 && RightT == 0)
        {
            if ((range >= 75) && (range<= 10000))
            {
                SendMotorControl(DRIVE_STATE, 3);
            }
            else if (range >= 45)
            {
                SendMotorControl(DRIVE_STATE, 2);
            }
            else if (range >=15)
            {
                SendMotorControl(DRIVE_STATE, 1);
            }
            else
            {
                SendMotorControl(DRIVE_STATE, 0);
            }
        }


        /* Turn Signal Light Off Decision using Handle Movement. */
        int TurnSignalLight = 0;
        if(LeftT == 1)
        {
            TurnSignalLight = DIR_L;
        }
        else if (RightT == 1)
        {
            TurnSignalLight = DIR_R;
        }
        else
        {
            TurnSignalLight = DIR_S;
        }
        SetStateAfterSteering(TurnSignalLight);


    }
    return (1);
}


void initVADC(void){

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~( 1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX) ;
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0) ;

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) |  ( 1 << LCK_BIT_LSB_IDX)) &~(1 << ENDINIT_BIT_LSB_IDX) ;
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0) ;

    VADC_CLC.U &= ~ ( 1 << DISR_BIT_LSB_IDX) ;

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~( 1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX) ;
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0) ;

    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) |  ( 1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX) ;
    while((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0) ;

    while((VADC_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0 ) ;

    VADC_G4_ARBPR.B.PRIO0 = 0x3;
    VADC_G4_ARBPR.B.CSM0 = 0;
    VADC_G4_ARBPR.B.ASEN0 = 1;

    VADC_G4_QMR0.B.ENGT = 0x1;   // enable conversion request
    VADC_G4_QMR0.B.FLUSH = 0x1;  // clear ADC queue

    VADC_G4_ARBCFG.B.ANONC = 0x3;    // ADC normal operation

    VADC_G4_ICLASS0.B.CMS = 0;       // Class 0 standard Conversion (12bit)

    // VADC Group 4 channel 7 configuration.
    VADC_G4_CHCTR7.B.RESPOS = 1;     // result right-aligned.
    VADC_G4_CHCTR7.B.RESREG = 0x0;   // store result @ Result Register G4RES0

    VADC_G4_CHASS.B.ASSCH7 = 1;

}

void VADC_StartConvesrion(void){
    VADC_G4_QINR0.U |= 0x07 ;
    VADC_G4_QMR0.U  |= 0x1 << TREV_BIT_LSB_IDX ;
}

unsigned int VADC_ReadResult(void){

    while( ( VADC_G4_RES0.U & (0x1 << VF_BIT_LSB_IDX)) == 0 ) ;
    return VADC_G4_RES0.U & (0xFFF << RESULT_BIT_LSB_IDX) ;
}

void SetStateAfterSteering(int LED_FLAG){

    /*
     * INPUT
     *      LED_FLAG =
     *               1 : left direction led
     *               0 : straight
     *               2 : Right direction led
     */

    if(LED_FLAG == DIR_R){
        if(step == 0){
            VADC_StartConvesrion() ;
            adcResult = VADC_ReadResult() ;
            if(adcResult > THRESHOLD_R){
                step = 1 ;
                LED_OFF_FLAG = 0 ;
            }
        }
        else if(step == 1){
            VADC_StartConvesrion() ;
            adcResult = VADC_ReadResult() ;
            if(adcResult <= THRESHOLD_S){
                step = 0 ;
                LED_OFF_FLAG = 1 ;
            }
        }
    }
    else if(LED_FLAG == DIR_L){
        if(step == 0){
            VADC_StartConvesrion() ;
            adcResult = VADC_ReadResult() ;
            if(adcResult < THRESHOLD_L){
                step = 1 ;
                LED_OFF_FLAG = 0 ;
            }
        }
        else if(step == 1){
            VADC_StartConvesrion() ;
            adcResult = VADC_ReadResult() ;
            if(adcResult >= THRESHOLD_S){
                step = 0 ;
                LED_OFF_FLAG = 1 ;
            }
        }
    }
    else{ // go straight
        LED_OFF_FLAG = 0 ;
    }

}



/*         init            */
void initLED(void)
{
    // RESET P10_IOCR0 PC1,2
    P10_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);
    P10_IOCR0.U &= ~(0x1F << PC2_BIT_LSB_IDX);

    // SET P10.1,2 push-pull general output
    P10_IOCR0.U |= (0x10 << PC1_BIT_LSB_IDX);
    P10_IOCR0.U |= (0x10 << PC2_BIT_LSB_IDX);
}

void initRGBLED(void)
{
    // reset port IOCR regist
    P02_IOCR4.U &= ~(0x1F << PC7_BIT_LSB_IDX);
    P10_IOCR4.U &= ~(0x1F << PC5_BIT_LSB_IDX);
    P10_IOCR0.U &= ~(0x1F << PC3_BIT_LSB_IDX);

    // set port as general purpose output
    P02_IOCR4.U |= (0x11 << PC7_BIT_LSB_IDX);
    P10_IOCR4.U |= (0x11 << PC5_BIT_LSB_IDX);
    P10_IOCR0.U |= (0x11 << PC3_BIT_LSB_IDX);
}

/* Ultra Sonic version ERU */
void initERU(void)
{
    // U SONIC
    SCU_EICR3.B.EXIS0 = 0x3;    // 0x1

    // ETL setting (Falling edge)
    SCU_EICR3.B.FEN0 = 1;

    // (BUZ - Rising edge)
    SCU_EICR3.B.REN0 = 1;
    // (BUZ - Falling edge)
    SCU_EICR3.B.EIEN0 = 1;

    // OGU - Output Gating Unit - Trigger  Request
    SCU_EICR3.B.INP0 = 0x2;

    // IGCR - INPUT TRIGGER CONNECT
    SCU_IGCR1.B.IGP0 = 0x1;

    // SRC Interrupt setting
    SRC_SCU_SCU_ERU2.B.SRPN = 0x0A;
    SRC_SCU_SCU_ERU2.B.TOS = 0x0;
    SRC_SCU_SCU_ERU2.B.SRE = 1;


    // LED
// @@@@@@@@@@@@ LEFT  @@@@@@@@@@@@@@@
    SCU_EICR1.B.EXIS0 = 0x1;
    SCU_EICR1.B.FEN0 = 1;
    SCU_EICR1.B.EIEN0 = 1;
    SCU_EICR1.B.INP0 = 0x0;
    SCU_IGCR0.B.IGP0 = 0x1;
    SRC_SCU_SCU_ERU0.B.SRPN = 0x0B;
    SRC_SCU_SCU_ERU0.B.TOS = 0x0;
    SRC_SCU_SCU_ERU0.B.SRE = 1;

    // @@@@@@@@@@@@ Right  @@@@@@@@@@@@@@@
    SCU_EICR1.B.EXIS1 = 0x2;
    SCU_EICR1.B.FEN1 = 1;
    SCU_EICR1.B.EIEN1 = 1;
    SCU_EICR1.B.INP1 = 0x1;
    SCU_IGCR0.B.IGP1 = 0x1;
    SRC_SCU_SCU_ERU1.B.SRPN = 0x0C;
    SRC_SCU_SCU_ERU1.B.TOS = 0x0;
    SRC_SCU_SCU_ERU1.B.SRE = 1;

}

// ultra sonic 10 us - pulse distance check
void initCCU60(void)
{
    PW_ULK();
    CCU60_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);
    PW_ULK();
    while ((CCU60_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0); // wait until CCU60 module enabled

    // T12 configurations [ LED ]
    CCU60_TCTR0.B.T12CLK = 0x2;     // f_CCU6 = 50 MHz, prescaler = 1024
    CCU60_TCTR0.B.T12PRE = 0x1;     // prescaler enable
    CCU60_TCTR0.U &= ~(0x1 << CTM_BIT_LSB_IDX);
    CCU60_T12PR.U = 24414 - 1;          // CCU60_T12PR.B.T12PV = 24414 - 1;
    CCU60_TCTR4.B.T12STR = 0x1;
    CCU60_T12.U = 0;    // clear T12 counter register
    // CCU60_T12.B.T12CV = 0x1;    // HSJ NOT
    CCU60_INP.U &= ~(0x3 << INPT12_BIT_LSB_IDX);        // CCU60_INP.B.INPT12 = 0x1;
    CCU60_IEN.U |= 0x1 << ENT12PM_BIT_LSB_IDX;          // CCU60_IEN.B.ENT12PM = 0x1;
    SRC_CCU6_CCU60_SR0.B.SRPN = 0x0D;
    SRC_CCU6_CCU60_SR0.U &= ~(0x3 << TOS_BIT_LSB_IDX);  // // *hg SRC_CCU6_CCU60_SR1.B.TOS = 0x0;
    SRC_CCU6_CCU60_SR0.B.SRE = 0x1;
    CCU60_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;   // T12 start counting

    // T13 configurations [ ULTRA SONIC ]
    CCU60_TCTR0.B.T13CLK = 0x2;     // f_CCU6 = 50 MHz, prescaler = 1024
    CCU60_TCTR0.B.T13PRE = 0x1;     // prescaler enable
    CCU60_T13PR.B.T13PV = 125 - 1;
    CCU60_TCTR4.B.T13STR = 0x1;
    CCU60_TCTR2.B.T13SSC = 0x1;     // Single Shot Control - U Sonic
    CCU60_T13.B.T13CV = 0x1;
    CCU60_IEN.B.ENT13PM = 0x1;
    CCU60_INP.B.INPT13 = 0x1;       // SR1
    SRC_CCU6_CCU60_SR1.B.SRPN = 0x0E;
    SRC_CCU6_CCU60_SR1.B.TOS = 0x0;
    SRC_CCU6_CCU60_SR1.B.SRE = 0x1;
    // CCU60_TCTR4.B.T13RS = 0x1;       // T13 start counting

}

// Ehco signal time check
void initCCU61(void)
{
    PW_ULK();
    CCU61_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);
    PW_ULK();
    while ((CCU61_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0); // wait until CCU60 module enabled

    // 占쏙옙占쏙옙 clock 占쏙옙占식쇽옙 占쏙옙占쏙옙
    CCU61_TCTR0.U &= ~(0x7 << T12CLK_BIT_LSB_IDX);    // f_T12 = f_CCU6 / prescaler
    CCU61_TCTR0.U |= 0x2 << T12CLK_BIT_LSB_IDX;       // f_CCU6 = 50MHz, prescaler = 1024
    CCU61_TCTR0.U |= 0x1 << T12PRE_BIT_LSB_IDX;       // f_T12 = 48,828 Hz

        // --- 占쏙옙占쏙옙 T12 占쏙옙占� 占쏙옙占쏙옙

    // do not use Period Match => use temp value  [ 24414 - 1 => 48828 / 2 - 1==> 0.5s ]
    CCU61_T12PR.U = 100000 - 1;                           // PM interrupt freq. = f_T12 / (T12PR + 1)

    // T12PR -> Write 占쏙옙占쏙옙 Period Shadow Register 占쌈뱄옙占쏙옙 -> T12STR占쏙옙 1占쏙옙 占실몌옙 Shadow Enable
    CCU61_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;         // Load T12PR from shadow regist

    // T1 counter init => counter will be rised so need to init
    CCU61_T12.U = 0;
}

void initGTM(void)
{
    // PW Access to unlock SCU_WDTSCON0
    PW_ULK();

    // Enable GTM [CLC 占쏙옙占쏙옙]
    GTM_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);

    // Password Access to unlock SCU_WDTSCON0
    PW_ULK();

    // GTM configurations
    while ((GTM_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0); // ENABLE占쏙옙占쏙옙 占쏙옙占�

    // GTM clock config
    GTM_CMU_FXCLK_CTRL.U &= ~(0xF << FXCLK_SEL_BIT_LSB_IDX);        // input clock of CMU_FXCLK --> CMU_GCLK_EN
    GTM_CMU_CLK_EN.U |= 0x2 << EN_FXCLK_BIT_LSB_IDX;                // enable all CUM_FXCLK

    // GTM TOM0 PWM CONFIG
    GTM_TOM0_TGC0_GLB_CTRL.U |= 0x2 << UPEN_CTRL1_BIT_LSB_IDX;      // TOM ch 1 update enable
    GTM_TOM0_TGC0_ENDIS_CTRL.U |= 0x2 << ENDIS_CTRL1_BIT_LSB_IDX;   // enable ch 1 on update trigger
    GTM_TOM0_TGC0_OUTEN_CTRL.U |= 0x2 << OUTEN_CTRL1_BIT_LSB_IDX;   // enable ch1 output on update trigger

    GTM_TOM0_CH1_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;
    GTM_TOM0_CH1_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH1_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;
    GTM_TOM0_CH1_CTRL.U &= ~(0x1 << OSM_BIT_LSB_IDX);
    GTM_TOM0_CH1_CTRL.U &= ~(0x1 << TRIGOUT_BIT_LSB_IDX);

    GTM_TOUTSEL6.U &= ~(0x3 << SEL7_BIT_LSB_IDX);   // TOUT103 --> TOM0 ch 1

                // [RGB]
    // 占쏙옙占쏙옙 LAB => set TGC0 to enable GTM TOM0 CH 2, 3, 15
    GTM_TOM0_TGC0_GLB_CTRL.B.UPEN_CTRL2 |= 0x2;
    GTM_TOM0_TGC0_GLB_CTRL.B.UPEN_CTRL3 |= 0x2;
    GTM_TOM0_TGC1_GLB_CTRL.B.UPEN_CTRL7 |= 0x2;

    GTM_TOM0_TGC0_ENDIS_CTRL.B.ENDIS_CTRL2 |= 0x2;
    GTM_TOM0_TGC0_ENDIS_CTRL.B.ENDIS_CTRL3 |= 0x2;
    GTM_TOM0_TGC1_ENDIS_CTRL.B.ENDIS_CTRL7 |= 0x2;

    GTM_TOM0_TGC0_OUTEN_CTRL.B.OUTEN_CTRL2 |= 0x2;
    GTM_TOM0_TGC0_OUTEN_CTRL.B.OUTEN_CTRL3 |= 0x2;
    GTM_TOM0_TGC1_OUTEN_CTRL.B.OUTEN_CTRL7 |= 0x2;

    // TOM 0_2
    GTM_TOM0_CH2_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;
    GTM_TOM0_CH2_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH2_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;
    GTM_TOM0_CH2_SR0.U = 12500 - 1;

    // TOM 0_3
    GTM_TOM0_CH3_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;
    GTM_TOM0_CH3_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH3_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;
    GTM_TOM0_CH3_SR0.U = 12500 - 1;

    // TOM 0_15
    GTM_TOM0_CH15_CTRL.U |= 0x1 << SL_BIT_LSB_IDX;
    GTM_TOM0_CH15_CTRL.U &= ~(0x7 << CLK_SRC_SR_BIT_LSB_IDX);
    GTM_TOM0_CH15_CTRL.U |= 0x1 << CLK_SRC_SR_BIT_LSB_IDX;
    GTM_TOM0_CH15_SR0.U = 12500 - 1;

    // [buzzer]
// 占쏙옙占쏙옙 LAB => set GTM TOM0 ch 11
    GTM_TOM0_TGC1_GLB_CTRL.B.UPEN_CTRL3 |= 0x2;      // TOM ch 1 update enable
    GTM_TOM0_TGC1_ENDIS_CTRL.B.ENDIS_CTRL3 |= 0x2;   // enable ch 1 on update trigger
    GTM_TOM0_TGC1_OUTEN_CTRL.B.OUTEN_CTRL3 |= 0x2;   // enable ch1 output on update trigger
    GTM_TOM0_CH1_SR0.U = 12500 - 1;         // PWM FREQ. = 6250 kHz / 12500 = 500 Hz [2ms] => 1ms : 6250 | 2ms : 12500
    GTM_TOM0_CH1_SR1.U = 1250 - 1;          // Duty Cycle = 1250 / 12500 = 10 %
                                            // 103 = 16 * 6 + 7
    // TOM 0_11
    GTM_TOM0_CH11_CTRL.B.SL = 0x1;
    GTM_TOM0_CH11_CTRL.B.CLK_SRC_SR = 0x1;
    GTM_TOM0_CH11_CTRL.U &= ~(0x0 << OSM_BIT_LSB_IDX);              // continuous mode
    GTM_TOM0_CH11_CTRL.U &= ~(0x0 << TRIGOUT_BIT_LSB_IDX);
    GTM_TOM0_CH11_SR0.B.SR0 = 12500 - 1;         // PWM FREQ. = 6250 kHz / 12500 = 500 Hz [2ms] => 1ms : 6250 | 2ms : 12500
    GTM_TOM0_CH11_SR1.B.SR1 = 1250 - 1;          // Duty Cycle = 1250 / 12500 = 10 %

    // TOUT pin selection
    GTM_TOUTSEL0.B.SEL3 = 0x0;
    // [RGB]
    GTM_TOUTSEL6.U &= ~(0x3 << SEL7_BIT_LSB_IDX);                   // TOUT103  --> TOM0 channel 1
    GTM_TOUTSEL0.U &= ~(0x3 << SEL7_BIT_LSB_IDX);                   // TOUT7    --> TOM0 channel 15
    GTM_TOUTSEL6.U &= ~(0x3 << SEL11_BIT_LSB_IDX);                  // TOUT103  --> TOM0 channel 2
    GTM_TOUTSEL6.U &= ~(0x3 << SEL9_BIT_LSB_IDX);                   // TOUT105  --> TOM0 channel 3
}

void initBuzzer(void)
{
    P02_IOCR0.B.PC3 = 0x11;
}

void disableBuzzer(void)
{
    P02_IOCR0.B.PC3 = 0x00;
}

void initUSonic(void)
{
    P02_IOCR4.B.PC6 = 0x10;
    P11_IOCR8.B.PC10 = 0x01;

    P02_OUT.U &= ~(0x1 << P6_BIT_LSB_IDX);      // PIN 2.6 占십깍옙 占쏙옙째占� 占쏙옙 占실깍옙화
}


/*         function            */

void PW_ULK(void)
{
    // PW Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);  // unlock 占쏙옙占�

    // Modify Access to clear ENDINIT => Mask 占쌜억옙
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0xFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0);  // lock 占쏙옙占�
}

void ERU0_ISR(void)
{
    // ultra sonic
    if (P11_IN.B.P10 != 0)   // rising edge of echo
    {
        // echo ______|

        CCU61_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;  // shadow register占쏙옙 占쏙옙占쏙옙占쏙옙
    }
    else    // Button released
    {
        CCU61_TCTR4.B.T12RR = 0x1;

        // (1 / t_freq) * counter * 1000000 / 58 = centimeter
        range = ((CCU61_T12.B.T12CV * 1000000) / 48828) / 58;
        range_valid_flag = 1;

        CCU61_TCTR4.B.T12RES = 0x1;     // reset CCU61 T12 counter
    }
}

// LED
void CCU60_T12_ISR(void) {
    if(DRIVE_STATE == 0)
    {
        if (LED_OFF_FLAG == 1)
        {
            LeftT = 0;
            P10_OUT.B.P1 = 0;
            RightT = 0;
            P10_OUT.B.P2 = 0;
            LED_OFF_FLAG = 0;
        }

        if (LeftT == 1)
            P10_OUT.B.P1 ^= 0x1;

        if (RightT == 1)
            P10_OUT.B.P2 ^= 0x1;
    }

    else
    {
        P10_OUT.B.P1 ^= 0x1;
        P10_OUT.B.P2 ^= 0x1;
    }

    DriveStateChange();
}

void ERU_T12_ISR_L(void)
{
    if (DRIVE_STATE == 0)
    {
        LeftT ^= 1;
        RightT = 0;
        P10_OUT.B.P2 = 0;
        if (LeftT == 0)
        {
            P10_OUT.B.P1 = 0x0;
        }
    }

}

void ERU_T12_ISR_R(void)
{
    if (DRIVE_STATE == 0)
    {
        RightT ^= 1;
        LeftT = 0;
        P10_OUT.B.P1 = 0;
        if (RightT == 0)
        {
            P10_OUT.B.P2 = 0x0;
        }
    }
}


// ULTRA SONIC
void CCU60_T13_ISR(void)
{
    // END OF 10us Trig
    // GPIO P02.6 --> LOW
    P02_OUT.U &= ~(0x1 << P6_BIT_LSB_IDX);
}


void usonicTrigger(void)
{
    // start of 10us Trigger Pulse
    P02_OUT.U |= 0x1 << P6_BIT_LSB_IDX; // 0(占십깍옙화)占쏙옙占쏙옙 1占쏙옙 占실몌옙 占쏙옙占쏙옙
    range_valid_flag = 0;
    // CCU60_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;   //T12 START COUNTING

    CCU60_TCTR4.B.T13RS = 0x1;
}

void RGBChange(void)
{
    if (range <= range_array[0])        // red
    {
        bright = 12500 * 1 / (range / 5 + 1);

        GTM_TOM0_CH2_SR1.U = 0;                 // G
        GTM_TOM0_CH3_SR1.U = 0;                 // B
        GTM_TOM0_CH15_SR1.U = bright;            // R

        warning = warning_array[0];

    }
    else if (range <= range_array[1])   // green
    {
        bright = 12500 * 1 / (range - range_array[0] + 1);

        GTM_TOM0_CH2_SR1.U = bright;
        GTM_TOM0_CH3_SR1.U = 0;
        GTM_TOM0_CH15_SR1.U = 0;

        warning = warning_array[1];
    }
    else                    // blue
    {
        bright = 12500 * 1 / (range - range_array[1] + 1);

        GTM_TOM0_CH2_SR1.U = 0;
        GTM_TOM0_CH3_SR1.U = bright;
        GTM_TOM0_CH15_SR1.U = 0;

        warning = warning_array[2];
    }
}

void RGBZero(void)
{
    GTM_TOM0_CH2_SR1.U = 0;                 // G
    GTM_TOM0_CH3_SR1.U = 0;                 // B
    GTM_TOM0_CH15_SR1.U = 0;               // R
}

void buzzerChange_temp(unsigned short warning, unsigned short l_DRIVE_STATE)
{
    unsigned int cycle = warning * 100000;

    static int delay_cnt = 0;
    static int duty_idx = 0;
    if (l_DRIVE_STATE == 0 && warning == warning_array[2])
    {
        GTM_TOM0_CH11_SR1.U = 1;
    }
    else
    {
        if (delay_cnt % 300000 == 0)
        {
            GTM_TOM0_CH11_SR0.U = (6250000 / duty[duty_idx]) - 1;      // Freq
            GTM_TOM0_CH11_SR1.U = (3125000 / duty[duty_idx]) - 1;      // volume
            delay_cnt = 0;
            duty_idx++;
            if (duty_idx % 8 == 0)
            {
                duty_idx = 0;
            }
        }


    }


    delay_cnt++;
}

void buzzerChange(unsigned short warning, int l_DRIVE_STATE)
{
    unsigned int cycle = warning * 100000;

    static int delay_cnt = 0;

    if (l_DRIVE_STATE == 0 && warning == warning_array[2])
    {
        GTM_TOM0_CH11_SR1.U = 1;
    }
    else
    {
        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[0]) - 1;      // Freq
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[0]) - 1;      // volume

        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[1]) - 1;
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[1]) - 1;

        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[2]) - 1;
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[2]) - 1;

        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[3]) - 1;
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[3]) - 1;

        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[4]) - 1;
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[4]) - 1;

        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[5]) - 1;
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[5]) - 1;

        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[6]) - 1;
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[6]) - 1;

        for (unsigned int i = 0; i < cycle; i++);
        GTM_TOM0_CH11_SR0.U = (6250000 / duty[7]) - 1;
        GTM_TOM0_CH11_SR1.U = (3125000 / duty[7]) - 1;
    }


    delay_cnt++;
}

void DriveStateChange()
{
    static int change_cnt = 0;
    if ((P02_IN.B.P1 == 0) && (P02_IN.B.P0 == 0))
    {
        change_cnt++;
    }
    else
    {
        change_cnt = 0;
    }

    if (change_cnt >= 4)
    {
        DRIVE_STATE ^= 1;
        P10_OUT.B.P1 = 0; // 깜빡이 꺼
        P10_OUT.B.P2 = 0; // 깜빡이 꺼
        RightT = 0;
        LeftT = 0;
        change_cnt = 0;
        SendMotorControl(0, 0);
    }
}


// [Master] Motor Control Output to Slave Board
void initGPIOMaster()
{
    P14_IOCR0.B.PC1 = 0x10; // PIN23, P14.1 Output
    P00_IOCR0.B.PC0 = 0x10; // PIN25, P00.0 Output
    P00_IOCR0.B.PC1 = 0x10; // PIN27, P00.1 Output
}

void SendMotorControl(int Direction, int Velocity)
{
    //P14_OUT.B.P1;
    //P00_OUT.B.P0;
    //P00_OUT.B.P1;
    if (Direction == 0) // 전진
    {
        P14_OUT.B.P1 = 0;
    }
    else if (Direction == 1) // 후진
    {
        P14_OUT.B.P1 = 1;
    }

    if(Velocity == 0)
    {
        P00_OUT.B.P0 = 0;
        P00_OUT.B.P1 = 0;

    }
    else if (Velocity == 1)
    {
        P00_OUT.B.P0 = 0;
        P00_OUT.B.P1 = 1;
    }
    else if (Velocity == 2)
    {
        P00_OUT.B.P0 = 1;
        P00_OUT.B.P1 = 0;
    }
    else if (Velocity == 3)
    {
        P00_OUT.B.P0 = 1;
        P00_OUT.B.P1 = 1;
    }



    //printf("%d %d\n", Direction, Velocity);

}




