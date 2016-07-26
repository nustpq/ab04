;
;********************************************************************************************************
;                                          CACHE UTILITY FUNCTIONS
;
; File      : bsp_cache.s
; For       : Cortex A5
; Toolchain : IAR
;
; Notes : none.
;********************************************************************************************************
;

;********************************************************************************************************
;                                           MACROS AND DEFINIITIONS
;********************************************************************************************************

                                ; MMU C1
CTRL_C1_M    EQU  0x0001
CTRL_C1_A    EQU  0x0002
CTRL_C1_C    EQU  0x0004
CTRL_C1_W    EQU  0x0008
CTRL_C1_S    EQU  0x0100
CTRL_C1_R    EQU  0x0200
CTRL_C1_Z    EQU  0x0800
CTRL_C1_I    EQU  0x1000
CTRL_C1_RR   EQU  0x4000


    PRESERVE8

    RSEG CODE:CODE:NOROOT(2)
    CODE32


;********************************************************************************************************
;                                      BSP_DCacheInvalidateAll()
;
; Description : Invalidate L1 data cache. Used during initialisation sequence.
;
; Prototypes  : void  BSP_DCacheInvalidateAll (void)
;
; Argument(s) : none.
;********************************************************************************************************

    EXPORT BSP_DCacheInvalidateAll

BSP_DCacheInvalidateAll

                                    ; Invalidate L1 data cache
  MOVW    r0, #0x1FE                ; Load set index
BSP_DCacheInvalidateAll_loop_1
  MOV     r1, #0x00000003           ; Load number of ways
BSP_DCacheInvalidateAll_loop_2
  MOV     r2, r1, LSL #30
  ADD     r2, r2, r0, LSL #4
  MCR     p15, 0, r2, c7, c6, 2
  SUBS    r1, r1, #1
  BGE     BSP_DCacheInvalidateAll_loop_2
  SUBS    r0, r0, #1
  BGE     BSP_DCacheInvalidateAll_loop_1
  DSB

  BX      lr


;********************************************************************************************************
;                                           BSP_CachesEn()
;
; Description : Enable L1 Data and Instruction cache.
;
; Prototypes  : void  BSP_CachesEn (void)
;
; Argument(s) : None
;********************************************************************************************************

    EXPORT BSP_CachesEn

BSP_CachesEn

    MRC     p15, 0, r0, c1, c0, 1
    ORR     r0, r0, #0xE
    MCR     p15, 0, r0, c1, c0, 1

    MRC     p15, 0, r0, c1, c0, 0
    ORR     r0, r0, #CTRL_C1_C
    ORR     r0, r0, #CTRL_C1_I
    MCR     p15, 0, r0, c1, c0, 0
    DSB
    ISB

    BX      lr


;********************************************************************************************************
;                                      INVALIDATE DATA CACHE RANGE
;
; Description : Invalidate a range of data cache by MVA.
;
; Prototypes  : void  BSP_DCache_InvalidateRange  (void       *p_mem,
;                                                  CPU_SIZE_T  range);
;
; Argument(s) : p_mem    Start address of the region to invalidate.
;
;               range    Size of the region to invalidate in bytes.
;
; Note(s)     : (1) p_mem value not aligned to 32 bytes will be truncated to the next lowest aligned
;                   address.
;
;               (2) range value not a multiple of 32 will be the next multiple of 32 that includes
;                   the start address and the end of the specified range.
;********************************************************************************************************

    EXPORT BSP_DCache_InvalidateRange

BSP_DCache_InvalidateRange
    MOV r3, r0
    ADD r1, r1, r0
    BIC r3, r3, #31


    MOV r3, r0
    BIC r3, r3, #31
Invalidate_RangeL1
    MCR p15,0, r3, c7, c6, 1
    ADD r3, r3, #32
    CMP r3, r1
    BLT Invalidate_RangeL1
    DSB
    BX LR


;********************************************************************************************************
;                                       FLUSH DATA CACHE RANGE
;
; Description : Flush (clean) a range of data cache by MVA.
;
; Prototypes  : void  BSP_DCache_FlushRange  (void       *p_mem,
;                                             CPU_SIZE_T  range);
;
; Argument(s) : p_mem    Start address of the region to flush.
;
;               range    Size of the region to invalidate in bytes.
;
; Note(s)     : (1) p_mem value not aligned to 32 bytes will be truncated to the next lowest aligned
;                   address.
;
;               (2) range value not a multiple of 32 will be the next multiple of 32 that includes
;                   the start address and the end of the specified range.
;********************************************************************************************************

    EXPORT BSP_DCache_FlushRange

BSP_DCache_FlushRange
    DSB
    MOV r3, r0
    ADD r1, r1, r0
    BIC r3, r3, #31

Flush_RangeL1
    MCR p15, 0, r3, c7, c14, 1
    ADD r3, r3, #32
    CMP r3, r1
    BLT Flush_RangeL1
    DSB

    BX lr


    END



