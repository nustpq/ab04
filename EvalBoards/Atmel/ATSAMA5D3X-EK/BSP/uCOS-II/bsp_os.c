/*
*********************************************************************************************************
*
*                                    MICRIUM BOARD SUPPORT PACKAGE
*
*                          (c) Copyright 2003-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               This BSP is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                    MICRIUM BOARD SUPPORT PACKAGE
*                                  ATSAMA5D3X OS BOARD SUPORT PACKAGE
*
* Filename      : bsp_os.c
* Version       : V1.00
* Programmer(s) : JBL
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <lib_def.h>
#include  <cpu.h>

#include  <os_cpu.h>

#include  <bsp.h>
#include  <bsp_os.h>
#include  <bsp_int.h>



/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  BSP_SAMA5_PIT_REG         ((SAMA5_REG_PIT_PTR)(0xFFFFFE30u))

                                                                /* ---------- ADVANCED INTERRUPT CONTROLLER ----------- */
typedef  struct  sama5_reg_pit {
    CPU_REG32  PIT_MR;                                          /* Mode Register.                                       */
    CPU_REG32  PIT_SR;                                          /* Status Register.                                     */
    CPU_REG32  PIT_PIVR;                                        /* Periodic Interval Value Register.                    */
    CPU_REG32  PIT_PIIR;                                        /* Periodic Interval Image Register.                    */
} SAMA5_REG_PIT, *SAMA5_REG_PIT_PTR;


#define  SAMA5_REG_PMC_PCER     (*((CPU_REG32 *)(0xFFFFFC10)))

#if 0
/*
*********************************************************************************************************
*                                      BSP_OS_SemCreate()
*
* Description : Creates a sempahore to lock/unlock
*
* Argument(s) : p_sem        Pointer to a BSP_OS_SEM structure
*
*               sem_val      Initial value of the semaphore.
*
*               p_sem_name   Pointer to the semaphore name.
*
* Return(s)   : DEF_OK       if the semaphore was created.
*               DEF_FAIL     if the sempahore could not be created.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_OS_SemCreate (BSP_OS_SEM       *p_sem,
                               BSP_OS_SEM_VAL    sem_val,
                               CPU_CHAR         *p_sem_name)
{
    OS_ERR     err;


    OSSemCreate((OS_SEM    *)p_sem,
                (CPU_CHAR  *)p_sem_name,
                (OS_SEM_CTR )sem_val,
                (OS_ERR    *)&err);

    if (err != OS_ERR_NONE) {
        return (DEF_FAIL);
    }

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                     BSP_OS_SemWait()
*
* Description : Wait on a semaphore to become available
*
* Argument(s) : sem          sempahore handler
*
*               dly_ms       delay in miliseconds to wait on the semaphore
*
* Return(s)   : DEF_OK       if the semaphore was acquire
*               DEF_FAIL     if the sempahore could not be acquire
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_OS_SemWait (BSP_OS_SEM  *p_sem,
                             CPU_INT32U   dly_ms)
{
    OS_ERR      err;
    CPU_INT32U  dly_ticks;


    dly_ticks  = ((dly_ms * DEF_TIME_NBR_mS_PER_SEC) / OSCfg_TickRate_Hz);

    OSSemPend((OS_SEM *)p_sem,
              (OS_TICK )dly_ticks,
              (OS_OPT  )OS_OPT_PEND_BLOCKING,
              (CPU_TS  )0,
              (OS_ERR *)&err);

    if (err != OS_ERR_NONE) {
       return (DEF_FAIL);
    }

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                      BSP_OS_SemPost()
*
* Description : Post a semaphore
*
* Argument(s) : sem          Semaphore handler
*
* Return(s)   : DEF_OK     if the semaphore was posted.
*               DEF_FAIL      if the sempahore could not be posted.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_OS_SemPost (BSP_OS_SEM *p_sem)
{
    OS_ERR  err;


    OSSemPost((OS_SEM *)p_sem,
              (OS_OPT  )OS_OPT_POST_1,
              (OS_ERR *)&err);

    if (err != OS_ERR_NONE) {
        return (DEF_FAIL);
    }

    return (DEF_OK);
}
#endif

/*
*********************************************************************************************************
*                                       BSP_OS_TmrTickHandler()
*
* Description : Interrupt handler for the tick timer
*
* Argument(s) : cpu_id     Source core id
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_OS_TmrTickHandler()
{
    volatile CPU_INT32U val;

    val = BSP_SAMA5_PIT_REG->PIT_PIVR;
    OSTimeTick();
}


/*
 *********************************************************************************************************
 *                                            BSP_OS_TmrTickInit()
 *
 * Description : Initialize uC/OS-III's tick source
 *
 * Argument(s) : ticks_per_sec              Number of ticks per second.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : Application.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */

void BSP_OS_TmrTickInit(CPU_INT32U tick_rate)
{
    CPU_INT32U  val;


    BSP_IntVectSet ( 3u,
                     0u,
                    &BSP_OS_TmrTickHandler);

    BSP_IntSrcEn(3u);


    CPU_MB();

    val = 133000000 / 16 / tick_rate;

    BSP_SAMA5_PIT_REG->PIT_MR = DEF_BIT_24 | DEF_BIT_25 | val;
}


/*
 *********************************************************************************************************
 *                                          OS_CPU_ExceptHndlr()
 *
 * Description : Handle any exceptions.
 *
 * Argument(s) : except_id     ARM exception type:
 *
 *                                  OS_CPU_ARM_EXCEPT_RESET             0x00
 *                                  OS_CPU_ARM_EXCEPT_UNDEF_INSTR       0x01
 *                                  OS_CPU_ARM_EXCEPT_SWI               0x02
 *                                  OS_CPU_ARM_EXCEPT_PREFETCH_ABORT    0x03
 *                                  OS_CPU_ARM_EXCEPT_DATA_ABORT        0x04
 *                                  OS_CPU_ARM_EXCEPT_ADDR_ABORT        0x05
 *                                  OS_CPU_ARM_EXCEPT_IRQ               0x06
 *                                  OS_CPU_ARM_EXCEPT_FIQ               0x07
 *
 * Return(s)   : none.
 *
 * Caller(s)   : OS_CPU_ARM_EXCEPT_HANDLER(), which is declared in os_cpu_a.s.
 *
 * Note(s)     : (1) Only OS_CPU_ARM_EXCEPT_FIQ and OS_CPU_ARM_EXCEPT_IRQ exceptions handler are implemented.
 *                   For the rest of the exception a infinite loop is implemented for debuging pruposes. This behavior
 *                   should be replaced with another behavior (reboot, etc).
 *********************************************************************************************************
 */

void OS_CPU_ExceptHndlr(CPU_INT32U except_id) {

    switch (except_id) {
    case OS_CPU_ARM_EXCEPT_FIQ:
        BSP_IntHandler();
        break;

    case OS_CPU_ARM_EXCEPT_IRQ:
        BSP_IntHandler();
        break;

    case OS_CPU_ARM_EXCEPT_RESET:
        /* $$$$ Insert code to handle a Reset exception               */
         while(1);
    case OS_CPU_ARM_EXCEPT_UNDEF_INSTR:
        /* $$$$ Insert code to handle a Undefine Instruction exception */
          while(1);
    case OS_CPU_ARM_EXCEPT_SWI:
        /* $$$$ Insert code to handle a Software exception             */
          while(1);
    case OS_CPU_ARM_EXCEPT_PREFETCH_ABORT:
        /* $$$$ Insert code to handle a Prefetch Abort exception       */
          while(1);
    case OS_CPU_ARM_EXCEPT_DATA_ABORT:
        /* $$$$ Insert code to handle a Data Abort exception           */
          while(1);
    case OS_CPU_ARM_EXCEPT_ADDR_ABORT:
        /* $$$$ Insert code to handle a Address Abort exception        */
          while(1);
    default:

        while (DEF_TRUE) { /* Infinite loop on other exceptions. (see note #1)          */
            //CPU_WaitForEvent();
        }
    }
}
