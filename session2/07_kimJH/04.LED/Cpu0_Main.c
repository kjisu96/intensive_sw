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

#define PC1_BIT_LSB_IDX 11
#define PC2_BIT_LSB_IDX 19
#define P1_BIT_LSB_IDX  1
#define P2_BIT_LSB_IDX  2


IfxCpu_syncEvent g_cpuSyncEvent = 0;

void initLED(void)
{
    P10_IOCR0.U &= ~(0x1F << PC1_BIT_LSB_IDX); //reset P10_IOCR0 PC1
    P10_IOCR0.U &= ~(0x1F << PC2_BIT_LSB_IDX); //reset P10_IOCR0 PC2

    P10_IOCR0.U |= 0x10 << PC1_BIT_LSB_IDX; //set P10.1 push-pull general output
    P10_IOCR0.U |= 0x10 << PC2_BIT_LSB_IDX; //set P10.2 push-pull general output
}

void initButton(void)
{
    P02_IOCR0.U &=~(0x1F << PC1_BIT_LSB_IDX);
    P02_IOCR0.U |= 0x02 << PC1_BIT_LSB_IDX;
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
    
    /*initLED();
    int Cnt_1;
    int Cnt_2;
    int Cnt_3;
    */

    while(1)
    {
        //GPIO OUT
        /*for(uint32 i=0; i<10000000; i++);
        //SW delay
        P10_OUT.U ^= 0x1 << P1_BIT_LSB_IDX;

        for(uint32 i=0; i<10000000; i++);
        P10_OUT.U ^= 0x1 << P2_BIT_LSB_IDX;

        //GIPO IN
        Cnt_1 = P02_IN.U & (0x1<< P1_BIT_LSB_IDX);
        for(int i = 0; i<100; i++);

        Cnt_2 = P02_IN.U & (0x1<< P1_BIT_LSB_IDX);
        for(int i = 0; i<100; i++);

        Cnt_3 = P02_IN.U & (0x1<< P1_BIT_LSB_IDX);
        for(int i = 0; i<100; i++);

        if(Cnt_1==0 & Cnt_2==0 & Cnt_3==0)
        {
            P10_OUT.U |= 0x1<<P1_BIT_LSB_IDX;
        }
        else
        {
            P10_OUT.U &= ~(0x1<<P1_BIT_LSB_IDX);
        }*/




    }
    return (1);
}
