.macro init_core coreid
	// set vector address.
	ldr r0, =vector
	mcr p15, 0, r0, c12, c0, 0
	cps #0x12 // Setup sp in IRQ mode.
	ldr sp, =irq_stack_core\coreid
	cps #0x11 // Setup sp in FIQ mode.
	ldr sp, =fiq_stack_core\coreid
	cps #0x1B // Setup sp in UNDEF mode.
	ldr sp, =undefined_stack_core\coreid
	cps #0x17 // Setup sp in ABORT mode.
	ldr sp, =data_stack_core\coreid
	cps #0x1f // Setup sp in USR/SYS mode.
	ldr sp, =sys_stack_core\coreid
	cps #0x13 // Setup sp in SVC mode.
	ldr sp, =svc_stack_core\coreid
.endm

.macro preserve_ctx
	cps #0x1f // Sys mode
	// Store Usr regs
	push {r0-r12}
	push {lr}
	ldr r3, =scheduler // struct Scheduler
	ldr r2, [r3, #0] // struct Thread* rthread
	str sp, [r2, #4] // svc_lr -> void* sp
	cps #0x13 // Svc mode
	mrs r1, spsr
	str r1, [r2, #0xc] // preserve cpsr
	str lr, [r2, #0] // svc_lr -> void* pc
.endm

.macro restore_ctx
	ldr r3, =scheduler // struct Scheduler
	ldr r2, [r3, #0] // struct Thread* rthread
	ldr lr, [r2, #0] // void* pc -> lr_svc
	ldr r0, [r2, #4] // void* sp -> r0
	ldr r1, [r2, #0xc] // restore cpsr
	msr spsr_f, r1
	cps #0x1f // Sys mode
	mov sp, r0 // Set stack pointer
	// Restore Usr regs
	pop {lr}
	pop {r0-r12}
	cps #0x13 // Svc mode
.endm

.macro core_stacks coreid
	.space 4096
undefined_stack_core\coreid:
	.space 4096
svc_stack_core\coreid:
	.space 4096
data_stack_core\coreid:
	.space 4096
irq_stack_core\coreid:
	.space 4096
fiq_stack_core\coreid:
	.space 4096
sys_stack_core\coreid:
.endm

.macro core_task coreid
__l_\@:	clrex
	ldr r0, =mbox_core\coreid
	ldrex r1, [r0]
	cmp r1, #0
	wfeeq
	beq __l_\@
	clrex
	mov r0, #\coreid
	blx r1
	ldr r0, =mbox_core\coreid
	mov r1, #0
	str r1, [r0]
	b __l_\@
.endm

.macro assign_ctask label, coreid
	ldr r0, =mbox_core\coreid
	ldr r1, =\label
__l_\@:	ldr r2, [r0]
	cmp r2, #0
	bne __l_\@
	str r1, [r0]
	sev
.endm
