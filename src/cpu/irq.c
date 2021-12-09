#include "../cpu/irq.h"
#include "../sys/core.h"
#include "../sys/timer.h"
#include "../drivers/uart.a.h"
#include "../drivers/uart.h"
#include "../util/time.h"

extern void disable_irq(void);
extern void enable_irq(void);

void c_irq_handler(void) {
	disable_irq();
	unsigned long source = load32(CORE0_IRQ_SOURCE);
	if (source & (1 << 8)) {
		if(load32(IRQ_PENDING2) & (1 << 25)) {
			if(load32(UART0_MIS) & (1<<4)) {
				unsigned long data = load32(UART0_DR);
				// Tab to toggle timer
				if(data == 0x14) {
					unsigned long timer_status;
					asm volatile("mrc p15, 0, %0, c14, c3, 1" : "=r"(timer_status));
					if(timer_status == 0) {
						cntfrq = read_cntfrq();
						write_cntv_tval(cntfrq);
						enable_cntv();
						uart_string((char*)"\033[?25l\033[s\033[4;1H\033[0KTimer: \033[92mEnabled\033[0m\033[u\033[?25h");
					} else {
						disable_cntv();
						uart_string((char*)"\033[?25l\033[s\033[4;1H\033[0KTimer: \033[91mDisabled\033[0m\033[u\033[?25h");
					}
				} else {
					if (data == 0x0D) {
						//uart_char(0x0a);
						uart_char(data);
						uart_string("\033[?25l> \033[0K\033[?25h");
					} else if (data == 0x08) {
						uart_char((unsigned char)data);
						uart_char(0x20);
						uart_char((unsigned char)data);
					} else {
						uart_char((unsigned char)data);
					}
				}
				enable_irq();
				return;
			}
		}
	} else if (source & (1 << 3)) {
		c_timer();
		enable_irq();
	}
	return;
}