#include <cpu.h>
#include <globals.h>
#include <graphics/lfb.h>
#include <symbols.h>
#include <sys/schedule.h>
#include <usr/cxx.h>
#include <usr/math.h>
#include <usr/string.h>
#include <usr/test.h>
#include <usr/timed.h>
#include <usr/uart.h>

// IRQ Information Structures
// static struct SysTimerInfo stime_0 = { .tick_rate = 5000000, .priority = 0, .arg = 0, .oneshot = 0, };
// static struct SysTimerInfo stime_1 = { .tick_rate = 700000, .priority = 0, .arg = 0, .oneshot = 0, };
// static struct SysTimerInfo stime_2 = { .tick_rate = 300000, .priority = 0, .arg = 0, .oneshot = 0, };
// static struct SysTimerInfo stime_3 = { .tick_rate = 10, .priority = 0, .arg = 0, .oneshot = 0, };
// static struct UartInfo UART_INFO = { .priority = 2, };
// static struct GPIOInfo gpinfo = { .pin = (1<<16 | 1<<12), .priority = 0, };

void main(void)
{
	// Runs the Core RTOS tests
	add_thread(test_super, 0, 4);
	// GPIO Tests
	//subscribe_irq(GPIO_BANK_1_IRQ, gptest, &gpinfo);
	// Delayed Execution Test
	//subscribe_irq(SYS_TIMER_3_IRQ, delaytest, &stime_3);

	// The following are examples of subscribing IRQs to specific callbacks
	//subscribe_irq(UART_IRQ, handle_data, &UART_INFO);
	//subscribe_irq(SYS_TIMER_0_IRQ, loopt, &stime_0);
	//subscribe_irq(SYS_TIMER_1_IRQ, loopt, &stime_1);
	//subscribe_irq(SYS_TIMER_2_IRQ, loopt, &stime_2);
	//subscribe_irq(SYS_TIMER_3_IRQ, loopt, &stime_3);
	//subscribe_irq(GPIO_BANK_1_IRQ, gptest, &gpinfo);

	// Adding custom callbacks at different priorities example
	//add_thread(loop, 0, 8);
	//add_thread(consumer, 0, 3);
	//add_thread(test_super, 0, 4);

	// C++ Execution Demo
	//cpp_demo(53);
}
