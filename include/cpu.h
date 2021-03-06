#ifndef CPU_H
#define CPU_H

extern void _start(void);

static inline unsigned long getmode(void)
{
	unsigned long mode;
	asm volatile ("mrs %0, cpsr" : "=r"(mode));
	return mode;
}

static inline void setsvc(void)
{
	asm volatile ( "cps #0x13" );
}

static inline void setmode(unsigned long mode)
{
	asm volatile ("msr cpsr_c, %0" :: "r"(mode));
}

static inline void* getsysstack(void)
{
	void* sp;
	asm volatile (
	"mrs r0, cpsr\n"
	"bic r1, r0, #0x1F\n"
	"orr r1, r1, #0x1F\n"
	"msr cpsr_c, r1\n"
	"mov %0, sp\n"
	"msr cpsr_c, r0"
	: "=r"(sp));
	return sp;
}

static inline void* getsvcstack(void)
{
	void* sp;
	asm volatile (
	"mrs r0, cpsr\n"
	"bic r1, r0, #0x1F\n"
	"orr r1, r1, #0x13\n"
	"msr cpsr_c, r1\n"
	"mov %0, sp\n"
	"msr cpsr_c, r0"
	: "=r"(sp));
	return sp;
}

static inline void* getfiqstack(void)
{
	void* sp;
	asm volatile (
	"mrs r0, cpsr\n"
	"bic r1, r0, #0x1F\n"
	"orr r1, r1, #0x11\n"
	"msr cpsr_c, r1\n"
	"mov %0, sp\n"
	"msr cpsr_c, r0"
	: "=r"(sp));
	return sp;
}

static inline void* getirqstack(void)
{
	void* sp;
	asm volatile (
	"mrs r0, cpsr\n"
	"bic r1, r0, #0x1F\n"
	"orr r1, r1, #0x12\n"
	"msr cpsr_c, r1\n"
	"mov %0, sp\n"
	"msr cpsr_c, r0"
	: "=r"(sp));
	return sp;
}

#define syscall_h_quote(text)	#text
#define syscall_h_expand_and_quote(text)	syscall_h_quote(text)

#define sys0(sys_n) asm volatile("svc #" syscall_h_expand_and_quote(sys_n) ::: "r0", "r1", "r2", "r3");
#define sys0_32(sys_n,addr) asm volatile("svc #" syscall_h_expand_and_quote(sys_n) "\nmov r2, %0\nstr r0, [r2]" ::"r"(addr): "r0", "r1", "r2", "r3", "memory");
#define sys0_64(sys_n,addr) asm volatile("svc #" syscall_h_expand_and_quote(sys_n) "\nmov r2, %0\nstr r1, [r2]\nstr r0, [r2, #4]" ::"r"(addr): "r0", "r1", "r2", "r3", "memory");
//#define sys1(sys_n,arg0) asm volatile("svc #" syscall_h_expand_and_quote(sys_n) ::[r0]"r"(arg0): "r0", "r1", "r2", "r3");
#define sys1(sys_n,arg0) {\
		register long r0 asm ("r0") = (long) (arg0); \
		asm volatile("svc #" syscall_h_expand_and_quote(sys_n) ::"r"(r0): "memory"); \
		}

#define SYS_YIELD        0x00000
#define SYS_TIME         0x00001
#define SYS_SCHED        0x00002
#define SYS_YIELD_HIGH   0x00002
#define SYS_ADD_THREAD   0x00003
#define SYS_LOCK         0x00004
#define SYS_UNLOCK       0x00005
#define SYS_SEMAPHORE_P  0x00006
#define SYS_SEMAPHORE_V  0x00007
#define SYS_SEMAPHORE_IV 0x00008
#define SYS_TIME_2       0x00009
#define SYS_ENABLE_CNTV  0x0000A
#define SYS_DISABLE_CNTV 0x0000B

#endif
