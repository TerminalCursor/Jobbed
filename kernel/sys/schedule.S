.section ".text"
.globl schedule

.include "macros.inc"

// Assumption: Enter in SVC mode
schedule:
	preserve_ctx
	ldr r1, =irqlr
	ldr r0, [r1]
	cmp r0, #0
	beq 1f
	// Replace LR with IRQ's LR
	ldr r3, =scheduler
	ldr r2, [r3, #0] // struct Thread* rthread
	str r0, [r2, #0] // svc_lr -> void* pc
	// Clear IRQ's LR
	mov r0, #0
	str r0, [r1]
1:
	bl next_thread // Thread* next -> r0
	ldr r3, =scheduler
	str r0, [r3, #0] // next -> rthread
	restore_ctx
	subs pc, lr, #0

.globl cleanup
cleanup:
	bl c_cleanup
	// usrloop -> rthread
	ldr r3, =scheduler
	ldr r2, =usrloopthread
	str r2, [r3, #0]
	ldr sp, [r2, #4]
	ldmfd sp!,{lr}
	ldmfd sp!,{r0-r12}
	ldr lr, =kernel_usr_task_loop
	// svc sched
	svc #2
.globl kernel_usr_task_loop
kernel_usr_task_loop:
	wfe
	b kernel_usr_task_loop

.globl add_thread
add_thread:
	mrs r3, cpsr
	and r3, #0x1F
	cmp r3, #0x10
	beq 1f
	b svc_add_thread
1:	svc #3
	bx lr
