;
;********************************************************************************************************
;                                    EXCEPTION VECTORS & STARTUP CODE
;
; File      : cstartup.s
; For       : Cortex-A5
; Toolchain : IAR EWARM V5.10 and higher
;********************************************************************************************************
;
	
;********************************************************************************************************
;                                           MACROS AND DEFINIITIONS
;********************************************************************************************************

                                ; Mode, correspords to bits 0-5 in CPSR
MODE_BITS   DEFINE  0x1F        ; Bit mask for mode bits in CPSR
USR_MODE    DEFINE  0x10        ; User mode
FIQ_MODE    DEFINE  0x11        ; Fast Interrupt Request mode
IRQ_MODE    DEFINE  0x12        ; Interrupt Request mode
SVC_MODE    DEFINE  0x13        ; Supervisor mode
ABT_MODE    DEFINE  0x17        ; Abort mode
UND_MODE    DEFINE  0x1B        ; Undefined Instruction mode
SYS_MODE    DEFINE  0x1F        ; System mode

                                ; MMU C1
CTRL_C1_M    DEFINE  0x0001
CTRL_C1_A    DEFINE  0x0002
CTRL_C1_C    DEFINE  0x0004
CTRL_C1_W    DEFINE  0x0008
CTRL_C1_S    DEFINE  0x0100
CTRL_C1_R    DEFINE  0x0200
CTRL_C1_Z    DEFINE  0x0800
CTRL_C1_I    DEFINE  0x1000
CTRL_C1_V    DEFINE  0x2000
CTRL_C1_RR   DEFINE  0x4000

                               ; Auxiliary control register C1
AUX_C1_FW    DEFINE  0x0001
AUX_C1_SMP   DEFINE  0x0040

                               ; Level 1 page table descriptor templates
TTB_ENTRY_SUPERSEC_DEV  DEFINE   0x50C06
TTB_ENTRY_SUPERSEC_NORM DEFINE   0x55C06

;********************************************************************************************************
;                                            ARM EXCEPTION VECTORS
;********************************************************************************************************

    SECTION .intvec:CODE:NOROOT(2)
    PUBLIC  __vector
    PUBLIC  __iar_program_start

    IMPORT  OS_CPU_ARM_ExceptUndefInstrHndlr
    IMPORT  OS_CPU_ARM_ExceptSwiHndlr
    IMPORT  OS_CPU_ARM_ExceptPrefetchAbortHndlr
    IMPORT  OS_CPU_ARM_ExceptDataAbortHndlr
    IMPORT  OS_CPU_ARM_ExceptIrqHndlr
    IMPORT  OS_CPU_ARM_ExceptFiqHndlr

    IMPORT BSP_DCacheInvalidateAll

    ARM

__vector:
    LDR	    PC, [PC,#24]    ; Absolute jump can reach 4 GByte
    LDR     PC, [PC,#24]    ; Branch to undef_handler
    LDR     PC, [PC,#24]    ; Branch to swi_handler
    LDR     PC, [PC,#24]    ; Branch to prefetch_handler
    LDR     PC, [PC,#24]    ; Branch to data_handler
__vector_0x14:
    DC32    0               ; Reserved
    LDR	    PC, [PC,#24]    ; Branch to irq_handler
    LDR	    PC, [PC,#24]    ; Branch to fiq_handler


    DC32    __iar_program_start
    DC32    OS_CPU_ARM_ExceptUndefInstrHndlr
    DC32    OS_CPU_ARM_ExceptSwiHndlr
    DC32    OS_CPU_ARM_ExceptPrefetchAbortHndlr
    DC32    OS_CPU_ARM_ExceptDataAbortHndlr
    DC32    0
    DC32    OS_CPU_ARM_ExceptIrqHndlr
    DC32    OS_CPU_ARM_ExceptFiqHndlr


;********************************************************************************************************
;                                   LOW-LEVEL INITIALIZATION
;********************************************************************************************************

    SECTION FIQ_STACK:DATA:NOROOT(3)
    SECTION IRQ_STACK:DATA:NOROOT(3)
    SECTION SYS_STACK:DATA:NOROOT(3)
    SECTION ABT_STACK:DATA:NOROOT(3)
    SECTION UND_STACK:DATA:NOROOT(3)
    SECTION CSTACK:DATA:NOROOT(3)
    SECTION MMU_TT:DATA:NOROOT(3)
    SECTION text:CODE:NOROOT(2)
    REQUIRE __vector
    EXTERN  ?main
    PUBLIC  __iar_program_start
    EXTERN  lowlevel_init

__iar_program_start:

;********************************************************************************************************
;                                    STACK POINTER INITIALIZATION
;********************************************************************************************************

    MRC     p15, 0, r0, c1, c0, 0               ; Read control register
    BIC     r0, r0, #CTRL_C1_M                  ; Disable MMU
    BIC     r0, r0, #CTRL_C1_C                  ; Disable data cache
    BIC     r0, r0, #CTRL_C1_I                  ; Disable instruction cache
    MCR     p15, 0, r0, c1, c0, 0               ; Write control register

    MRS     r0,cpsr                             ; Original PSR value
    BIC     r0,r0,#MODE_BITS                    ; Clear the mode bits
    ORR     r0,r0,#SYS_MODE                     ; Set SVC mode bits
    MSR     cpsr_c,r0                           ; Change the mode
    LDR     sp,=SFE(SYS_STACK)                  ; End of SYS_STACK

    BIC     r0,r0,#MODE_BITS                    ; Clear the mode bits
    ORR     r0,r0,#UND_MODE                     ; Set UND mode bits
    MSR     cpsr_c,r0                           ; Change the mode
    LDR     sp,=SFE(UND_STACK)                  ; End of UND_STACK

    BIC     r0,r0,#MODE_BITS                    ; Clear the mode bits
    ORR     r0,r0,#ABT_MODE                     ; Set ABT mode bits
    MSR     cpsr_c,r0                           ; Change the mode
    LDR     sp,=SFE(ABT_STACK)                  ; End of ABT_STACK

    BIC     r0,r0,#MODE_BITS                    ; Clear the mode bits
    ORR     r0,r0,#FIQ_MODE                     ; Set FIQ mode bits
    MSR     cpsr_c,r0                           ; Change the mode
    LDR     sp,=SFE(FIQ_STACK)                  ; End of FIQ_STACK

    BIC     r0,r0,#MODE_BITS                    ; Clear the mode bits
    ORR     r0,r0,#IRQ_MODE                     ; Set IRQ mode bits
    MSR     cpsr_c,r0                           ; Change the mode
    LDR     sp,=SFE(IRQ_STACK)                  ; End of IRQ_STACK

    BIC     r0,r0,#MODE_BITS                    ; Clear the mode bits
    ORR     r0,r0,#SVC_MODE                     ; Set System mode bits
    MSR     cpsr_c,r0                           ; Change the mode
    LDR     sp,=SFE(CSTACK)                     ; End of CSTACK


;********************************************************************************************************
;                                   ADDITIONAL INITIALIZATION
;********************************************************************************************************

                                                ; -------------------- SET INT VECS LOC ------------------- ;
    LDR     R0,=SFB(.intvec)                    ; Start of .intvec
    MCR     P15, 0,  R0, C12, C0, 0             ; Write exception vecs loc
    MCR     P15, 0,  R0, C12, C0, 1             ; Write exception vecs loc

    MRC     p15, 0, R0, c1, c1, 0


    MOV     r0, #0x0
    MCR     p15, 0, r0, c7, c5, 6               ; Invalidate branch predictor

    MOV     r0, #0x0
    MCR     p15, 0, r0, c8, c7, 0               ; Invalidate TLB

    MOV     r0, #0
    MCR     p15, 0, r0, c7, c5, 0               ; Invalidate instruction cache

    IMPORT BSP_DCacheInvalidateAll              ; Invalidate data cache
    BL BSP_DCacheInvalidateAll

                                                ; Set domain access
    MRC     p15, 0, r0, c3, c0, 0
    LDR     r0, =0x55555555
    MCR     p15, 0, r0, c3, c0, 0


                                                ; Setup page table.
    LDR     r0,=SFE(MMU_TT)                     ; Load page table base address


                                                ; Init the whole page table as dev memory by default
    MOV     r4, #0x00000000
    MOV     r3, r0
    ADD     r3, r3, #0x0

TTbl_Dev_Loop1
    MOV32   r1, #TTB_ENTRY_SUPERSEC_DEV
    ADD     r1, r1, r4
    MOV     r5, #16
TTbl_Dev_Loop2
    STR     r1, [r3], #4
    SUBS    r5, r5, #1
    BNE     TTbl_Dev_Loop2
    ADD     r4, r4, #0x1000000
    CMP     r4, #0x0
    BNE     TTbl_Dev_Loop1



                                                ; Map DDR RAM as normal memory
    MOV     r4, #0x20000000
    MOV     r3, r0
    ADD     r3, r3, #0x800

TTbl_RAM_Loop1
    MOV32   r1, #TTB_ENTRY_SUPERSEC_NORM
    ADD     r1, r1, r4
    MOV     r5, #16
TTbl_RAM_Loop2
    STR     r1, [r3], #4
    SUBS    r5, r5, #1
    BNE     TTbl_RAM_Loop2
    ADD     r4, r4, #0x1000000
    CMP     r4, #0x30000000
    BNE     TTbl_RAM_Loop1

                                                     ; Set L1 page table location
    MCR     p15, 0, r0, c2, c0, 0

    DSB

    MRC     p15,  0, r0, c1, c0, 0                   ; Read  control register
    ORR     r0,  r0, #CTRL_C1_M                      ; Enable MMU, See note #3
    MCR     p15,  0, r0, c1, c0, 0                   ; Write control register

    DSB

                                                     ; Set access permission for VFP
    MRC     p15, 0, r0, c1, c0, 2
    ORR     r0, r0, #0xf00000
    MCR     p15, 0, r0, c1, c0, 2
    ISB

                                                     ; Start the VFP engine
    MOV     r0, #0x40000000
    VMSR    FPEXC, r0

                                                     ; Join SMP
    MRC     p15, 0, r0, c1, c0, 1
    ORR     r0, r0, #AUX_C1_SMP
    MCR     p15, 0, r0, c1, c0, 1


;********************************************************************************************************
;                           CONTINUE TO ?main FOR ADDITIONAL INITIALIZATION
;********************************************************************************************************

    LDR     r0,=?main
    BX      r0

    END
