#include <cpu.h>
#include <drivers/uart.h>
#include <util/mutex.h>
#include <util/lock.h>
#include <globals.h>

void mutex_init(void)
{
	for (unsigned long m = 0; m < MAX_MUTEXS; m++) {
		mutexs[m].pid = 0;
		mutexs[m].addr = 0;
		mutex_entries[m].value = &mutexs[m];
		mutex_entries[m].entry_type = VALUE_ENTRY;
		mutex_entries[m].next = &mutex_entries[m+1];
	}
	// Initialize Free Mutexs
	mutex_manager.free.start.value = 0;
	mutex_manager.free.start.next = &mutex_entries[0];
	mutex_manager.free.start.entry_type = START_ENTRY;
	mutex_manager.free.end.value = 0;
	mutex_manager.free.end.next = &mutex_entries[MAX_MUTEXS-1];
	mutex_entries[MAX_MUTEXS-1].next = &mutex_manager.free.end;
	mutex_manager.free.end.entry_type = END_ENTRY;
}

struct Mutex* create_mutex(void* addr)
{
	struct Entry* e = pop_from_queue(&mutex_manager.free);
	if (e == 0)
		return 0;
	struct Mutex* m = e->value;
	m->pid = 0;
	m->addr = addr;
	mutex_manager.used_mutexes++;
	return e->value;
}

unsigned char delete_mutex(struct Mutex* m)
{
	unsigned long spacing = (unsigned long)&mutexs[1] - (unsigned long)&mutexs[0];
	unsigned long difference = (unsigned long)m - (unsigned long)&mutexs[0];
	unsigned long index = difference/spacing;
	if (index >= MAX_MUTEXS)
		return 1;
	struct Entry* entry = &mutex_entries[index];
	// Add it to the free queue
	prepend_to_queue(entry, &mutex_manager.free);
	return 0;
}

void uart_mutexes(void)
{
	uart_hexn(mutex_manager.used_mutexes);
}

unsigned char lock_mutex(struct Mutex* m)
{
	struct Thread* rthread = scheduler.rthread;
	unsigned long mode = getmode() & 0x1F;
	if (mode == 0x10) {
		unsigned long spacing = (unsigned long)&mutexs[1] - (unsigned long)&mutexs[0];
		unsigned long difference = (unsigned long)m - (unsigned long)&mutexs[0];
		unsigned long index = difference/spacing;
		// If it is not a managed mutex, break away
		if (index >= MAX_MUTEXS)
			return 1;
		if ((unsigned char) index > rthread->highest_mutex)
			return 2;
		sys1(SYS_LOCK, m);
		return 0;
	}
	return 3;
}

void unlock_mutex(struct Mutex* m)
{
	unlock((struct Lock*)m);
}
