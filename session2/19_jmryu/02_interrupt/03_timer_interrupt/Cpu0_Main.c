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
#include "IfxCcu6_reg.h"

// PORT REGISTERS
#define PC1_BIT_LSB_IDX 11
#define PC2_BIT_LSB_IDX 19
#define P1_BIT_LSB_IDX 1
#define P2_BIT_LSB_IDX 2

// SCU REGISTERS
#define EXIS0_BIT_LSB_IDX 4
#define FEN0_BIT_LSB_IDX 8
#define REN0_BIT_LSB_IDX 9
#define EIEN0_BIT_LSB_IDX 11
#define INP0_BIT_LSB_IDX 12
#define IGP0_BIT_LSB_IDX 14

#define LCK_BIT_LSB_IDX 1
#define ENDINIT_BIT_LSB_IDX 0

// SRC REGISTERS
#define SRPN_BIT_LSB_IDX 0
#define TOS_BIT_LSB_IDX 11
#define SRE_BIT_LSB_IDX 10

// CCU60 registers
#define DISS_BIT_LSB_IDX 1
#define DISR_BIT_LSB_IDX 0

#define CTM_BIT_LSB_IDX 7
#define T12PRE_BIT_LSB_IDX 3
#define T12CLK_BIT_LSB_IDX 0
#define T12STR_BIT_LSB_IDX 6
#define T12RS_BIT_LSB_IDX 1

#define INPT12_BIT_LSB_IDX 10
#define ENT12PM_BIT_LSB_IDX 7


IfxCpu_syncEvent g_cpuSyncEvent = 0;

__interrupt(0x0A) __vector_table(0)

void ERU0_ISR(void)
{
    // button status check
    if ((P02_IN.U & (0x1 << P1_BIT_LSB_IDX)) == 0)      // button pushedd
    {
        P10_OUT.B.P1 = 0x1;
        CCU60_T12PR.U = 12207 - 1;                      // PM interrupt freq. = f_T12 / (T12PR + 1)
        CCU60_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;     // load T12PR from shadow register
    }
    else                                                // button released
    {
        P10_OUT.B.P1 = 0x0;
        CCU60_T12PR.U = 24414 - 1;                      // PM interrupt freq. = f_T12 / (T12PR + 1)
        CCU60_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;     // load T12PR from shadow register
    }
}

__interrupt(0x0B) __vector_table(0)

void CCU60_T12_ISR(void)
{
    P10_OUT.U ^= 0x1 << P2_BIT_LSB_IDX;         // toggle P10.2 (LED D13 BLUE)
}

void initERU(void)
{
   // ERU setting
    SCU_EICR1.U &= ~(0x7 << EXIS0_BIT_LSB_IDX); // 000으로 설정
    SCU_EICR1.U |= (0x1 << EXIS0_BIT_LSB_IDX);  // 001설정


    SCU_EICR1.U |= (0x1 << FEN0_BIT_LSB_IDX);   // falling edge
    SCU_EICR1.U |= (0x1 << REN0_BIT_LSB_IDX);   // rising edge


    SCU_EICR1.U |= (0x1 << EIEN0_BIT_LSB_IDX);  // external interrupt interrupt

    SCU_EICR1.U &= ~(0x7 << INP0_BIT_LSB_IDX);  // OGU output channel

    SCU_IGCR0.U &= ~(0x3 << IGP0_BIT_LSB_IDX);
    SCU_IGCR0.U |= (0x1 << IGP0_BIT_LSB_IDX);   // output trigger event to IOUT port in ERU

    //SRC Interrupt setting
    SRC_SCU_SCU_ERU0.U &= ~(0xFF << SRPN_BIT_LSB_IDX);
    SRC_SCU_SCU_ERU0.U |= (0x0A << SRPN_BIT_LSB_IDX);

    SRC_SCU_SCU_ERU0.U &= ~(0x3 << TOS_BIT_LSB_IDX);

    SRC_SCU_SCU_ERU0.U |= (1 << SRE_BIT_LSB_IDX);
}

void initCC60(void)
{
    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);

    // Modify Access to clear ENDINIT : clear와 동시에 lock 걸기
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) | (1 << LCK_BIT_LSB_IDX)) & ~(1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0); // wait until locked

    CCU60_CLC.U &= ~(1 << DISR_BIT_LSB_IDX);                    // enable CCY

    // Password Access to unlock SCU_WDTSCON0
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) & ~(1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) != 0);

    // Modify Access to set ENDINIT
    SCU_WDTCPU0_CON0.U = ((SCU_WDTCPU0_CON0.U ^ 0XFC) | (1 << LCK_BIT_LSB_IDX)) | (1 << ENDINIT_BIT_LSB_IDX);
    while ((SCU_WDTCPU0_CON0.U & (1 << LCK_BIT_LSB_IDX)) == 0); // wait until locked

    // CCU60 T12 configurations
    while ((CCU60_CLC.U & (1 << DISS_BIT_LSB_IDX)) != 0);       // wait until CCU60 module enabled

    CCU60_TCTR0.U &= ~(0x7 << T12CLK_BIT_LSB_IDX);              // f_T12 = f_CCU6 / prescaler
    CCU60_TCTR0.U |= 0x2 << T12CLK_BIT_LSB_IDX;                 // f_CCU6 = 50 MHz, prescaler = 1024
    CCU60_TCTR0.U |= 0x1 << T12PRE_BIT_LSB_IDX;                 // f_T12 = 48,828 Hz
    CCU60_TCTR0.U &= ~(0x1 << CTM_BIT_LSB_IDX);              // T12 auto reset when period match (PM) occur

    CCU60_T12PR.U = 24414 - 1;                                  // PM interrupt freq. = f_T12 / (T12PR + 1)
    CCU60_TCTR4.U |= 0x1 << T12STR_BIT_LSB_IDX;                 // load T12PR from shadow reg.

    CCU60_T12.U = 0;                                            // clear T12 counter reg.


    // CCU60_T12_PM interrupt setting
    CCU60_INP.U &= ~(0x3 << INPT12_BIT_LSB_IDX);                // service request output SR0 selected
    CCU60_IEN.U |= 0x1 << ENT12PM_BIT_LSB_IDX;                  // enable T12 PM interrupt


    //SRC Interrupt setting
    SRC_CCU6_CCU60_SR0.U &= ~(0xFF << SRPN_BIT_LSB_IDX);
    SRC_CCU6_CCU60_SR0.U |= (0x0B << SRPN_BIT_LSB_IDX);         // set priority 0x0B

    SRC_CCU6_CCU60_SR0.U &= ~(0x3 << TOS_BIT_LSB_IDX);          // CPU0 service T12 PM interrupt

    SRC_CCU6_CCU60_SR0.U |= (0x1 << SRE_BIT_LSB_IDX);           // SR0 enabled

    //CCU60 T12 counting start
    CCU60_TCTR4.U = 0x1 << T12RS_BIT_LSB_IDX;                   // T12 start counting
}

void initLED(void)
{
    P10_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);  // reset P10_IOCR0 PC1
    P10_IOCR0.U &= ~(0x1F << PC2_BIT_LSB_IDX);  // reset P10_IOCR0 PC1
    P10_IOCR0.U |= 0x10 << PC1_BIT_LSB_IDX;     // set P10.1 push-pull general output
    P10_IOCR0.U |= 0x10 << PC2_BIT_LSB_IDX;     // set P10.1 push-pull general output
}

void initButton(void){
    P02_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX);  // reset P02_IOCR0 PC1
    P02_IOCR0.U |= 0x02 << PC1_BIT_LSB_IDX;     // set P02.1 general input (pull-up connected)

    //P02_IOCR0.B.PC0 = 0x02;
    //P02_IOCR0.B.PC1 = 0x02;
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
    
    initERU();
    initCC60();
    initLED();
    initButton();

    //int start;
    while(1)
    {
    }
    return (1);
}
