/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file main.c
 **
 ** \brief This sample demonstrates how to use EMB.
 **
 **   - 2021-04-16  CDT  first version for Device Driver Library of EMB.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
//#include "hc32f460_timer6.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define  Timer6x                (M4_TMR61)

//#define DAC_Enable

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
uint16_t u16Flag_EMB1_Braking;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void Timer6_UnderFlow_CallBack(void);
static void EMB1_CallBack(void);
static void Timer6_Config(void);
static void M4_CMP_Init(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/


/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  Timer6 underflow interrupt callback function
 **
 ** \param  None
 **
 ** \return None
 ******************************************************************************/
static void Timer6_UnderFlow_CallBack(void)
{
    static uint8_t i = 0u;

    if( 0u == i)
    {
        Timer6_SetGeneralCmpValue(Timer6x, Timer6GenCompareC, 0x3000u);
        i = 1u;
    }
    else
    {
        Timer6_SetGeneralCmpValue(Timer6x, Timer6GenCompareC, 0x6000u);
        i = 0u;
    }
}

/**
 *******************************************************************************
 ** \brief  EMB1 callback function
 **
 ** \param  None
 **
 ** \return None
 ******************************************************************************/
static void EMB1_CallBack(void)
{
    if(true == EMB_GetStatus(M4_EMB1, EMBFlagCmp))
    {
        BSP_LED_On(LED_RED);

        EMB_SwBrake(M4_EMB1, true);  //Software brake Enable, still shunt down PWM after Clear Port In Brake

        EMB_ClrStatus(M4_EMB1, EMBCmpFlagClr);  //Clear Port In Brake

        u16Flag_EMB1_Braking = 1u;
    }
}

/**
 *******************************************************************************
 ** \brief  Timer6 configure function
 **
 ** \param  None
 **
 ** \return None
 ******************************************************************************/
static void Timer6_Config(void)
{
    uint16_t                         u16Period;
    uint16_t                         u16Compare;
    stc_timer6_basecnt_cfg_t         stcTIM6BaseCntCfg;
    stc_timer6_port_output_cfg_t     stcTIM6PWMxCfg;
    stc_timer6_gcmp_buf_cfg_t        stcGCMPBufCfg;
    stc_irq_regi_conf_t              stcIrqRegiConf;
    stc_timer6_deadtime_cfg_t        stcDeadTimeCfg;

    MEM_ZERO_STRUCT(stcTIM6BaseCntCfg);
    MEM_ZERO_STRUCT(stcTIM6PWMxCfg);
    MEM_ZERO_STRUCT(stcGCMPBufCfg);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcDeadTimeCfg);

    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM61, Enable);

    PORT_SetFunc(PortE, Pin09, Func_Tim6, Disable);    //Timer61 PWMA
    PORT_SetFunc(PortE, Pin08, Func_Tim6, Disable);    //Timer61 PWMB


    stcTIM6BaseCntCfg.enCntMode   = Timer6CntTriangularModeA;           //Triangular wave mode
    stcTIM6BaseCntCfg.enCntDir    = Timer6CntDirUp;                     //Counter counting up
    stcTIM6BaseCntCfg.enCntClkDiv = Timer6PclkDiv1;                     //Count clock: pclk
    Timer6_Init(Timer6x, &stcTIM6BaseCntCfg);                           //timer6 PWM frequency, count mode and clk config

    u16Period = 0x8340u;
    Timer6_SetPeriod(Timer6x, Timer6PeriodA, u16Period);                //period set

    u16Compare = 0x3000u;
    Timer6_SetGeneralCmpValue(Timer6x, Timer6GenCompareA, u16Compare);  //Set General Compare RegisterA Value
    Timer6_SetGeneralCmpValue(Timer6x, Timer6GenCompareC, u16Compare);  //Set General Compare RegisterC Value as buffer register of GCMAR


    /*PWMA/PWMB output buf config*/
    stcGCMPBufCfg.bEnGcmpTransBuf = true;
    stcGCMPBufCfg.enGcmpBufTransType = Timer6GcmpPrdSingleBuf;          //Single buffer transfer
    Timer6_SetGeneralBuf(Timer6x, Timer6PWMA, &stcGCMPBufCfg);          //GCMAR buffer transfer set
    Timer6_SetGeneralBuf(Timer6x, Timer6PWMB, &stcGCMPBufCfg);          //GCMBR buffer transfer set


    stcTIM6PWMxCfg.enPortMode = Timer6ModeCompareOutput;    //Compare output function
    stcTIM6PWMxCfg.bOutEn     = true;                       //Output enable
    stcTIM6PWMxCfg.enPerc     = Timer6PWMxCompareKeep;      //PWMA port output keep former level when CNTER value match PERAR
    stcTIM6PWMxCfg.enCmpc     = Timer6PWMxCompareInv;       //PWMA port output inverse level when CNTER value match with GCMAR
    stcTIM6PWMxCfg.enStaStp   = Timer6PWMxStateSelSS;       //PWMA output status is decide by STACA STPCA when CNTER start and stop
    stcTIM6PWMxCfg.enStaOut   = Timer6PWMxPortOutLow;       //PWMA port output set low level when CNTER start
    stcTIM6PWMxCfg.enStpOut   = Timer6PWMxPortOutLow;       //PWMA port output set low level when CNTER stop
    stcTIM6PWMxCfg.enDisVal   = Timer6PWMxDisValLow;
    Timer6_PortOutputConfig(Timer6x, Timer6PWMA, &stcTIM6PWMxCfg);

    stcTIM6PWMxCfg.enPortMode = Timer6ModeCompareOutput;    //Compare output function
    stcTIM6PWMxCfg.bOutEn     = true;                       //Output enable
    stcTIM6PWMxCfg.enPerc     = Timer6PWMxCompareKeep;      //PWMB port output keep former level when CNTER value match PERAR
    stcTIM6PWMxCfg.enCmpc     = Timer6PWMxCompareInv;       //PWMB port output inverse level when CNTER value match with GCMBR
    stcTIM6PWMxCfg.enStaStp   = Timer6PWMxStateSelSS;       //PWMB output status is decide by STACB STPCB when CNTER start and stop
    stcTIM6PWMxCfg.enStaOut   = Timer6PWMxPortOutHigh;      //PWMB port output set high level when CNTER start
    stcTIM6PWMxCfg.enStpOut   = Timer6PWMxPortOutLow;       //PWMB port output set low level when CNTER stop
    stcTIM6PWMxCfg.enDisVal   = Timer6PWMxDisValLow;
    Timer6_PortOutputConfig(Timer6x, Timer6PWMB, &stcTIM6PWMxCfg);

    Timer6_SetDeadTimeValue(Timer6x, Timer6DeadTimUpAR, 3360u);     // Set dead time value (up count)
    //Timer6_SetDeadTimeValue(Timer6x, Timer6DeadTimDwnAR, 3360u);  // Set dead time value (down count)

    stcDeadTimeCfg.bEnDeadtime     = true;  //Enable Hardware DeadTime
    stcDeadTimeCfg.bEnDtBufUp      = false; //Disable buffer transfer
    stcDeadTimeCfg.bEnDtBufDwn     = false; //Disable buffer transfer
    stcDeadTimeCfg.bEnDtEqualUpDwn = true;  //Make the down count dead time value equal to the up count dead time setting
    Timer6_ConfigDeadTime(Timer6x, &stcDeadTimeCfg);        // Hardware dead time function config

    /*config interrupt*/
    /* Enable timer61 under flow interrupt */
    Timer6_ConfigIrq(Timer6x, Timer6INTENUDF, true);

    stcIrqRegiConf.enIRQn = Int002_IRQn;                    //Register INT_TMR61_GUDF Int to Vect.No.002
    stcIrqRegiConf.enIntSrc = INT_TMR61_GUDF;               //Select Event interrupt function
    stcIrqRegiConf.pfnCallback = &Timer6_UnderFlow_CallBack;//Callback function
    enIrqRegistration(&stcIrqRegiConf);                     //Registration IRQ

    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);            //Clear Pending
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_03);//Set priority
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);                   //Enable NVIC
}

/*******************************************************************************
 ** \brief CMP init function
 **
 ** \param [in]  None
 **
 ** \retval None
 **
 ******************************************************************************/
static void M4_CMP_Init(void)
{
    stc_cmp_init_t         stcCmpConfig;
    stc_irq_regi_conf_t    stcIrqRegiConf;
    stc_port_init_t        stcPortInit;
    stc_cmp_input_sel_t    stcCmpInput;
    stc_cmp_dac_init_t     stcDacInitCfg;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcCmpConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);
    MEM_ZERO_STRUCT(stcCmpInput);
    MEM_ZERO_STRUCT(stcDacInitCfg);

    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_CMP, Enable);

#ifdef DAC_Enable
    /* Set DAC */
    //DAC1 for CMP1(INM3); DAC2 for CMP2(INM3); DAC1 for CMP3(INM3) / DAC2 for CMP3(INM4)
    stcDacInitCfg.u8DacData = 0x80u;
    stcDacInitCfg.enCmpDacEN = Enable;
    CMP_DAC_Init(CmpDac1, &stcDacInitCfg);
    CMP_DAC_Init(CmpDac2, &stcDacInitCfg);
#endif

//CMP1
#if 1
    stcPortInit.enPinMode = Pin_Mode_Ana;
    /* Set PA0 as CMP1_INP1 input */
    PORT_Init(PortA, Pin00, &stcPortInit);
    /* Set PC3 as CMP1_INM2 input */
    PORT_Init(PortC, Pin03, &stcPortInit);

    /* Set PB12 as CH1  output */
    PORT_SetFunc(PortB, Pin12, Func_Vcout, Disable);

    /* mode set */
    stcCmpConfig.enCmpIntEN = Disable;            //intrrupt disable
    stcCmpConfig.enCmpInvEn = Disable;            //CMP INV sel for output
    stcCmpConfig.enCmpOutputEn = Enable;          //CMP Output enable
    stcCmpConfig.enCmpVcoutOutputEn = Enable;     //CMP output result enable
    stcCmpConfig.enEdgeSel = CmpRisingEdge;       //Fall edge active brake
    stcCmpConfig.enFltClkDiv = CmpFltPclk3Div32;  //PCLK3/32
    CMP_Init(M4_CMP1, &stcCmpConfig);

    //gpio set for cmp
    stcCmpInput.enInp4Sel = CmpInp4None;
    stcCmpInput.enInpSel = CmpInp1;
    stcCmpInput.enInmSel = CmpInm2;
  #ifdef DAC_Enable
     stcCmpInput.enInmSel = CmpInm3;
  #endif
    CMP_InputSel(M4_CMP1, &stcCmpInput);

  #if 0
    stcIrqRegiConf.enIntSrc = INT_ACMP1;          //Select CMP
    stcIrqRegiConf.enIRQn = Int112_IRQn;          //Register CMP
    stcIrqRegiConf.pfnCallback = &ACMP1_Callback; //Callback function
    enIrqRegistration(&stcIrqRegiConf);           //Registration IRQ


    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);  //Clear pending
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);//Set priority
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);       //Enable NVIC
  #endif

    CMP_Cmd(M4_CMP1,Enable);    //Enable CMP1
#endif


//CMP2
#if 0

    stcPortInit.enPinMode = Pin_Mode_Ana;
    /* Set PA6 as CMP2_INP3 input */
    PORT_Init(PortA, Pin06, &stcPortInit);
    /* Set PC4 as CMP2_INN2 input */
    PORT_Init(PortC, Pin04, &stcPortInit);

    /* Set PB13 as CH2  output */
    PORT_SetFunc(PortB, Pin13, Func_Vcout, Disable);

    /* mode set */
    stcCmpConfig.enCmpIntEN = Disable;            //intrrupt disable
    stcCmpConfig.enCmpInvEn = Disable;            //CMP INV sel for output
    stcCmpConfig.enCmpOutputEn = Enable;          //CMP Output enable
    stcCmpConfig.enCmpVcoutOutputEn = Enable;     //CMP output result enable
    stcCmpConfig.enEdgeSel = CmpRisingEdge;       //Fall edge active brake
    stcCmpConfig.enFltClkDiv = CmpFltPclk3Div32;  //PCLK3/32
    CMP_Init(M4_CMP2, &stcCmpConfig);

    stcCmpInput.enInpSel = CmpInp3;
    stcCmpInput.enInmSel = CmpInm2;
  #ifdef DAC_Enable
     stcCmpInput.enInmSel = CmpInm3;
  #endif
    CMP_InputSel(M4_CMP2, &stcCmpInput);

    CMP_Cmd(M4_CMP2,Enable);    //Enable CMP2

#endif


//CMP3
#if 0

    stcPortInit.enPinMode = Pin_Mode_Ana;
    /* Set PB1 as CMP3_INP2 input */
    PORT_Init(PortB, Pin01, &stcPortInit);
    /* Set PC5 as CMP3_INM2 input */
    PORT_Init(PortC, Pin05, &stcPortInit);

    /* Set PB14 as CH3  output */
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enPinSubFunc = Enable;
    PORT_Init(PortB,  Pin14, &stcPortInit);
    PORT_SetFunc(PortB, Pin14, Func_Vcout, Disable);

    /* mode set */
    stcCmpConfig.enCmpIntEN = Disable;            //intrrupt disable
    stcCmpConfig.enCmpInvEn = Disable;            //CMP INV sel for output
    stcCmpConfig.enCmpOutputEn = Enable;          //CMP Output enable
    stcCmpConfig.enCmpVcoutOutputEn = Enable;     //CMP output result enable
    stcCmpConfig.enEdgeSel = CmpRisingEdge;       //Fall edge active brake
    stcCmpConfig.enFltClkDiv = CmpFltPclk3Div32;  //PCLK3/32
    CMP_Init(M4_CMP3, &stcCmpConfig);

    stcCmpInput.enInpSel = CmpInp2;
    stcCmpInput.enInmSel = CmpInm2;
  #ifdef DAC_Enable
    stcCmpInput.enInmSel = CmpInm3;
  #endif
    CMP_InputSel(M4_CMP3, &stcCmpInput);

    CMP_Cmd(M4_CMP3, Enable);   //Enable CMP3
#endif

}

/**
 *******************************************************************************
 ** \brief  Main function of project
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 **
 ******************************************************************************/
int32_t main(void)
{
    stc_emb_ctrl_timer6_t   stcEMBConfigCR;
    stc_irq_regi_conf_t     stcIrqRegiConf;
    stc_port_init_t         stcPortInit;

    MEM_ZERO_STRUCT(stcEMBConfigCR);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);

    /* Initialize Clock */
    BSP_CLK_Init();

    /* Initialize LED port */
    BSP_LED_Init();

    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_EMB, Enable);

    PORT_SetFunc(PortA, Pin11, Func_Emb, Disable);      //PA11 EMB_IN1

    //PORT_SetFunc(PortB, Pin02, Func_Emb, Disable);    //PB02 EMB_IN1

    Timer6_Config();

    M4_CMP_Init();

    Ddl_Delay1ms(10ul);

    //EMB_ClrStatus(M4_EMB1, EMBCmpFlagClr);  //Clear Port In Brake

    stcEMBConfigCR.bEnCmp1Brake = true;
    EMB_Config_CR_Timer6(&stcEMBConfigCR);

    EMB_ClrStatus(M4_EMB1, EMBCmpFlagClr);  //Clear Port In Brake
    EMB_ConfigIrq(M4_EMB1, CMPBrkIrq, true);

    stcIrqRegiConf.enIRQn = Int005_IRQn;                    //Register INT_TMR61_GUDF Int to Vect.No.002
    stcIrqRegiConf.enIntSrc = INT_EMB_GR0;                  //Select Event interrupt function
    stcIrqRegiConf.pfnCallback = &EMB1_CallBack;            //Callback function
    enIrqRegistration(&stcIrqRegiConf);                     //Registration IRQ

    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);            //Clear Pending
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_01);//Set priority
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);                   //Enable NVIC


    /*start timer6*/
    Timer6_StartCount(Timer6x);

    while(1)
    {
        if(1u == u16Flag_EMB1_Braking)
        {
            //Add brake process code

            Ddl_Delay1ms(3000ul);  //only for demo using

            EMB_SwBrake(M4_EMB1, false); //Disable software brake, Enable PWM output

            BSP_LED_Off(LED_RED);
            u16Flag_EMB1_Braking = 0u;
        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
