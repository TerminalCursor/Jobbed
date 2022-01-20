#include <cpu/irq.h>
#include <drivers/uart.h>
#include <graphics/drawer.h>
#include <graphics/lfb.h>
#include <lib/mem.h>
#include <lib/q.h>
#include <lib/strings.h>
#include <symbols.h>
#include <sys/core.h>
#include <sys/kernel.h>
#include <sys/power.h>
#include <sys/schedule.h>
#include <sys/timer.h>
#include <util/mutex.h>
#include <util/status.h>
#include <util/time.h>

#define SYS_CORE_C
#ifndef VERSION
char* os_info_v = "?";
#else
char* os_info_v = VERSION;
#endif

void testlocal(void);

// Initialize IRQs
void sysinit(void)
{
	// Clear System Globals
	*(unsigned long*)exe_cnt_m.addr = 0;
	exe_cnt_m.pid = NULL_PID;
	cmdidx = 0;
	for(int i = 0; i < 2048; i++)
		cmd[i] = 0;
	///...

	// Route GPU interrupts to Core 0
	store32(0x00, GPU_INTERRUPTS_ROUTING);

	// Mask Overrun of UART0
	store32(1<<4, UART0_IMSC);
	// Enable UART GPU IRQ
	store32(1<<25, IRQ_ENABLE2);
	// Enable Timer
	// As an IRQ
	store32(1<<0, IRQ_BASIC_ENABLE);
	// Get the frequency
	cntfrq = read_cntfrq();
	// Clear cntv interrupt and set next 1 second timer
	write_cntv_tval(cntfrq);
	//// Route timer to core0 irq
	//routing_core0cntv_to_core0irq();
	// Route timer to core0 fiq
	routing_core0cntv_to_core0fiq();
	// Enable timer
	enablecntv();

	// Graphics Initialize
	lfb_init();
	lfb_showpicture();

	// Start Scheduler
	init_scheduler();

	// Enable IRQ & FIQ
	enableirq();
	enablefiq();

	//add_thread(testlocal, 0);
	//add_thread(testlocal, 1);
	//add_thread(testlocal, 3);
}

struct Mutex testm = {.addr = (void*)0xDEADBEEF, .pid = NULL_PID};
void testlocal1(void)
{
	unsigned long a = 5;
	struct Thread* t = scheduler.rthread_ll->data;
	uart_string("vRan Thread ");
	uart_10(t->data.pid);
	uart_string(" Pri. ");
	uart_10(t->data.priority);
	uart_string(" ...\n");
	add_thread(testlocal, 0);
	schedule();
	a += t->data.pid;
	uart_10(a);
	uart_string(" Done!\n");
}

void testlocal(void)
{
	struct Thread* t = scheduler.rthread_ll->data;
	uart_string("Ran Thread ");
	uart_10(t->data.pid);
	uart_string(" Pri. ");
	uart_10(t->data.priority);
	uart_string(" ...\n");
	//delay(0x80000000);
	if (t->data.pid == 5) {
		add_thread(testlocal1, 1);
		schedule();
	}
	if (t->data.pid == 3) {
		yield();
		yield();
		yield();
		yield();
		yield();
		yield();
		yield();
		// Example
		/*
			while (uart_tx_full) {
				t->data.status = THREAD_WAITING;
				schedule();
			} // Will wait until uart_tx is not full
		*/
	}
	uart_string("Done! ");
	uart_10(t->data.pid);
	uart_char('\n');
}
