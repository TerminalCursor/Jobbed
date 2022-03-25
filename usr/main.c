#include <globals.h>
#include <graphics/lfb.h>
#include <symbols.h>
#include <sys/schedule.h>
#include <usr/string.h>
#include <usr/timed.h>
#include <usr/uart.h>

static struct SysTimerInfo stime_0 = {
	.tick_rate = 5000000,
	.priority = 0,
	.arg = 0,
};

static struct SysTimerInfo stime_1 = {
	.tick_rate = 700000,
	.priority = 0,
	.arg = 0,
};

static struct SysTimerInfo stime_2 = {
	.tick_rate = 300000,
	.priority = 0,
	.arg = 0,
};

static struct SysTimerInfo stime_3 = {
	.tick_rate = 70000,
	.priority = 0,
	.arg = 0,
};

static struct UartInfo UART_INFO = {
	.priority = 2,
};

static struct GPIOInfo gpinfo = {
	.pin = (1<<16),
	.priority = 0,
};

void gptest(void)
{
	static unsigned long count = 0;
	unsigned long gplev0 = *GPLEV0;
	static char str[14];
	draw_hex32(0, 30, gplev0);
	char* start = ulong_to_string(count++, str);
	draw_string(0, 31, start);
}

void main(void)
{
	subscribe_irq(UART_IRQ, handle_data, &UART_INFO);
	subscribe_irq(SYS_TIMER_0_IRQ, loopt, &stime_0);
	subscribe_irq(SYS_TIMER_1_IRQ, loopt, &stime_1);
	subscribe_irq(SYS_TIMER_2_IRQ, loopt, &stime_2);
	subscribe_irq(SYS_TIMER_3_IRQ, loopt, &stime_3);
	subscribe_irq(GPIO_BANK_1_IRQ, gptest, &gpinfo);
	add_thread(loop, 0, 8);
	add_thread(consumer, 0, 3);
}