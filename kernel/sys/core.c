#include <cpu/irq.h>
#include <cpu.h>
#include <drivers/uart.h>
#include <globals.h>
#include <graphics/lfb.h>
#include <lib/kmem.h>
#include <lib/mmu.h>
#include <lib/strings.h>
#include <symbols.h>
#include <sys/core.h>
#include <sys/power.h>
#include <sys/schedule.h>
#include <usr/main.h>
#include <util/mutex.h>
#include <util/status.h>
#include <util/time.h>

// Initialize IRQs
void sysinit(void)
{
	// Initialize System Globals
	//stimeh = *(unsigned long*)SYS_TIMER_CHI;
	//stimel = *(unsigned long*)SYS_TIMER_CLO;
	//*(unsigned long*) SYS_TIMER_C0 = 2000000 + stimeh; // 2 second trigger
	uart_init();
	///...

	// Route GPU interrupts to Core 0
	store32(0x00, GPU_INTERRUPTS_ROUTING);

	//// Mask Overrun of UART0
	//store32(1<<4, UART0_IMSC);
	//// Enable UART GPU IRQ
	//store32(1<<25, IRQ_ENABLE2);
	//// Enable Timer
	////// Get the frequency
	//cntfrq = read_cntfrq();
	//// Clear cntv interrupt and set next 1 second timer
	//write_cntv_tval(cntfrq);
	//// Route timer to core0 fiq
	//routing_core0cntv_to_core0fiq();
	//// Enable timer
	//enablecntv();
	//// Enable system timer
	//store32(SYS_TIMER_SC_M0, IRQ_ENABLE1);

	// Graphics Initialize
	lfb_init();
	lfb_showpicture();

	// Initialize Memory Management Unit
	mmu_init();
	
	// Initialize Mutex Manager
	mutex_init();

	// Start Scheduler
	init_scheduler();
	add_thread(main, 0, 0);
}
