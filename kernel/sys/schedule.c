#include <cpu.h>
#include <globals.h>
#include <graphics/lfb.h>
#include <drivers/uart.h>
#include <lib/kmem.h>
#include <sys/schedule.h>
#include <util/mutex.h>

extern void kernel_usr_task_loop(void);

void init_scheduler(void)
{
	// Set rthread to usrloopthread - an infinitely running thread so that the pointer will never be null
	usrloopthread.pc = (void*)kernel_usr_task_loop;
	usrloopthread.sp = (void*)0x5FC8;
	*(unsigned long**)usrloopthread.sp = (unsigned long*)kernel_usr_task_loop;
	usrloopthread.sp_base = -1;
	usrloopthread.mptr = 0;
	usrloopthread.pid = -1;
	usrloopthread.priority = -1;
	usrloopthread.old_priority = -1;
	usrloopthread.status = THREAD_READY;
	usrloopthread.offset = -1;
	usrloopthread.highest_mutex = 0;
	scheduler.rthread = &usrloopthread;

	// Initialize Scheduling Queues
	for (unsigned long p = 0; p < PRIORITIES; p++) {
		// Ready Init
		scheduler.ready[p].start.value = 0;
		scheduler.ready[p].start.next = &scheduler.ready[p].end;
		scheduler.ready[p].start.entry_type = START_ENTRY;
		scheduler.ready[p].end.value = 0;
		scheduler.ready[p].end.next = &scheduler.ready[p].start;
		scheduler.ready[p].end.entry_type = END_ENTRY;
		// Mutex Wait Init
		scheduler.mwait[p].start.value = 0;
		scheduler.mwait[p].start.next = &scheduler.mwait[p].end;
		scheduler.mwait[p].start.entry_type = START_ENTRY;
		scheduler.mwait[p].end.value = 0;
		scheduler.mwait[p].end.next = &scheduler.mwait[p].start;
		scheduler.mwait[p].end.entry_type = END_ENTRY;
		// Signal Wait Init
		scheduler.swait[p].start.value = 0;
		scheduler.swait[p].start.next = &scheduler.swait[p].end;
		scheduler.swait[p].start.entry_type = START_ENTRY;
		scheduler.swait[p].end.value = 0;
		scheduler.swait[p].end.next = &scheduler.swait[p].start;
		scheduler.swait[p].end.entry_type = END_ENTRY;
	}

	// Initialize nextpid
	nextpid = FIRST_AVAIL_PID;

	// Initialize Threads - Stack Base and Offsets
	for (unsigned long i = 0; i < MAX_THREADS; i++) {
		struct Thread* t = &threads[i];
		t->offset = i;
		t->sp_base = 0x20000000 - STACK_SIZE*i;
		t->highest_mutex = 0;
		thread_entries[i].value = t;
		thread_entries[i].next = &thread_entries[(i+1)];
		thread_entries[i].entry_type = VALUE_ENTRY;
	}
	// Initialize the free queue
	scheduler.free_threads.start.value = 0;
	scheduler.free_threads.start.entry_type = START_ENTRY;
	scheduler.free_threads.end.value = 0;
	scheduler.free_threads.end.entry_type = END_ENTRY;
	scheduler.free_threads.start.next = &thread_entries[0];
	scheduler.free_threads.end.next = &thread_entries[MAX_THREADS-1];
	thread_entries[MAX_THREADS-1].next = &scheduler.free_threads.end;
}

void push_thread_to_queue(struct Thread* t, unsigned char type, unsigned char priority)
{
	struct Entry* entry = &thread_entries[t->offset];
	struct Queue* queue;
	if (type == THREAD_READY) {
		queue = &scheduler.ready[priority];
	} else if (type == THREAD_MWAIT) {
		queue = &scheduler.mwait[priority];
	} else if (type == THREAD_SWAIT) {
		queue = &scheduler.swait[priority];
	} else {
		return;
	}
	push_to_queue(entry, queue);
	//queue->end.next->next = entry;
	//queue->end.next = entry;
	//entry->next = &queue->end;
}

void prepend_thread_to_queue(struct Thread* t, unsigned char type, unsigned char priority)
{
	struct Entry* entry = &thread_entries[t->offset];
	struct Queue* queue;
	if (type == THREAD_READY) {
		queue = &scheduler.ready[priority];
	} else if (type == THREAD_MWAIT) {
		queue = &scheduler.mwait[priority];
	} else if (type == THREAD_SWAIT) {
		queue = &scheduler.swait[priority];
	} else {
		return;
	}
	prepend_to_queue(entry, queue);
}

struct Entry* pop_thread_from_queue(unsigned char type, unsigned char priority)
{
	struct Entry* entry = 0;
	struct Queue* queue;
	if (type == THREAD_READY) {
		queue = &scheduler.ready[priority];
	} else if (type == THREAD_MWAIT) {
		queue = &scheduler.mwait[priority];
	} else if (type == THREAD_SWAIT) {
		queue = &scheduler.swait[priority];
	} else {
		return entry;
	}
	return pop_from_queue(queue);
}

struct Entry* find_pid(unsigned long pid)
{
	for (unsigned char p = 0; p < PRIORITIES; p++) {
		struct Queue* queue;
		struct Entry* prev;
		struct Entry* entry;

		queue = &scheduler.ready[p];
		prev = &queue->start;
		entry = prev->next;
		while (entry->entry_type != END_ENTRY) {
			if (((struct Thread*)entry->value)->pid == pid)
				return prev;
			prev = entry;
			entry = entry->next;
		}

		queue = &scheduler.mwait[p];
		prev = &queue->start;
		entry = prev->next;
		while (entry->entry_type != END_ENTRY) {
			if (((struct Thread*)entry->value)->pid == pid)
				return prev;
			prev = entry;
			entry = entry->next;
		}

		queue = &scheduler.swait[p];
		prev = &queue->start;
		entry = prev->next;
		while (entry->entry_type != END_ENTRY) {
			if (((struct Thread*)entry->value)->pid == pid)
				return prev;
			prev = entry;
			entry = entry->next;
		}
	}
	return 0;
}

struct Entry* find_mutex_wait_next(void* m)
{
	for (unsigned char p = 0; p < PRIORITIES; p++) {
		struct Queue* queue = &scheduler.mwait[p];
		struct Entry* prev = &queue->start;
		struct Entry* entry = prev->next;
		while (entry->entry_type != END_ENTRY) {
			if (((struct Thread*)entry->value)->mptr == m)
				return prev;
			prev = entry;
			entry = entry->next;
		}
	}
	return 0;
}

struct Entry* find_signal_wait_next(void* s)
{
	for (unsigned char p = 0; p < PRIORITIES; p++) {
		struct Queue* queue = &scheduler.swait[p];
		struct Entry* prev = &queue->start;
		struct Entry* entry = prev->next;
		while (entry->entry_type != END_ENTRY) {
			if (((struct Thread*)entry->value)->mptr == s)
				return prev;
			prev = entry;
			entry = entry->next;
		}
	}
	return 0;
}

struct Entry* get_unused_thread(void)
{
	struct Queue* q = &scheduler.free_threads;
	// If we have no available free threads
	//  return null pointer
	if (q->start.next->entry_type == END_ENTRY)
		return 0;
	// Otherwise, get the next thread
	return pop_from_queue(q);
}

unsigned char find_duplicate(void* pc)
{
	for (unsigned char p = 0; p < PRIORITIES; p++) {
		struct Queue* queue = &scheduler.ready[p];
		struct Entry* entry = queue->start.next;
		while (entry->entry_type == VALUE_ENTRY) {
			if (((struct Thread*)entry->value)->pc == pc) {
				return 1;
			}
		}
	}
	return 0;
}

unsigned char add_thread_without_duplicate(void* pc, void* arg, unsigned char priority)
{
	if (!find_duplicate(pc)) {
		return add_thread(pc, arg, priority);
	}
	return 1;
}

unsigned char svc_add_thread(void* pc, void* arg, unsigned char priority)
{
	struct Entry* thread_entry = get_unused_thread();
	// The only point-of-failure is not having a thread available
	if (thread_entry == 0)
		return 1;
	struct Thread* thread = thread_entry->value;
	/// Thread Setup
	thread->pc = pc;
	unsigned long* argp = (void*)thread->sp_base;
	argp -= 13;
	*argp = (unsigned long)arg; // Set r0 to the argument
	argp -= 1;
	*(unsigned long**)argp = (unsigned long*)cleanup; // Set lr to the cleanup function
	thread->sp = argp;
	thread->status = THREAD_READY;
	thread->mptr = (void*)0;
	thread->pid = nextpid++;
	// Reset next pid on overflow
	if (nextpid < FIRST_AVAIL_PID) {
		nextpid = FIRST_AVAIL_PID;
	}
	// Cap Priority Level
	if (priority >= PRIORITIES)
		thread->priority = PRIORITIES - 1;
	else
		thread->priority = priority;
	// This thread is new
	thread->old_priority = -1;
	// Reserved for non-preemptible tasking
	thread->preempt = 0;
	/// Add Thread to Scheduler
	push_thread_to_queue(thread, THREAD_READY, thread->priority);
	return 0;
}

void uart_scheduler(void)
{
	uart_string("Scheduler Info\n==============\nCurrent\n");
	uart_hex((unsigned long)scheduler.rthread);
	uart_char(' ');
	kmemshow32((void*)scheduler.rthread, 9);
	unsigned long length;
	for(int p = 0; p < PRIORITIES; p++) {
		uart_string("Priority ");
		uart_10(p);
		uart_char('\n');
		struct Queue* queue;
		struct Entry* entry;

		queue = &scheduler.ready[p];
		uart_string("Ready Queue\n");
		entry = queue->start.next;
		length = 0;
		while (entry->entry_type != END_ENTRY) {
			uart_hex((unsigned long)entry->value);
			uart_char(' ');
			kmemshow32((void*)entry->value, 9);
			entry = entry->next;
			length++;
		}
		uart_hexn(length);

		queue = &scheduler.mwait[p];
		uart_string("Mutex Wait Queue\n");
		entry = queue->start.next;
		length = 0;
		while (entry->entry_type != END_ENTRY) {
			uart_hex((unsigned long)entry->value);
			uart_char(' ');
			kmemshow32((void*)entry->value, 9);
			entry = entry->next;
			length++;
		}
		uart_hexn(length);

		queue = &scheduler.swait[p];
		uart_string("Signal Wait Queue\n");
		entry = queue->start.next;
		length = 0;
		while (entry->entry_type != END_ENTRY) {
			uart_hex((unsigned long)entry->value);
			uart_char(' ');
			kmemshow32((void*)entry->value, 9);
			entry = entry->next;
			length++;
		}
		uart_hexn(length);
	}
	// Count number of free threads
	struct Queue* queue = &scheduler.free_threads;
	struct Entry* entry = queue->start.next;
	while (entry->entry_type != END_ENTRY) {
		entry = entry->next;
		length++;
	}
	uart_hexn(length);
	uart_string("==============\n");
}

struct Thread* next_thread(void)
{
	// Recurse through all priorities to try to find a ready thread
	for (int p = 0; p < PRIORITIES; p++) {
		struct Queue* rq = &scheduler.ready[p];
		if (rq->start.next->entry_type == END_ENTRY)
			continue;
		return rq->start.next->value;
	}
	// No thread found, use basic usrloopthread while waiting for new thread
	return &usrloopthread;
}

void c_cleanup(void)
{
	struct Thread* rt = scheduler.rthread;
	struct Entry* e = pop_thread_from_queue(THREAD_READY, rt->priority);
	// Add to free threads
	push_to_queue(e, &scheduler.free_threads);
}

void yield(void)
{
	struct Thread* rthread = scheduler.rthread;
	// usrloopthread should not be yielded
	if (rthread == &usrloopthread)
		return;
	// Put current thread at the end of its ready queue,
	//  thus any threads of the same priority can be run first
	unsigned char priority = rthread->priority;
	struct Entry* tq;
	// Remove from top of queue
	tq = pop_thread_from_queue(THREAD_READY, priority);
	if (tq != 0) {
		// Add to bottom of queue
		push_thread_to_queue(tq->value, THREAD_READY, priority);
	}
}

void sched_mutex_yield(void* m)
{
	struct Thread* rthread = scheduler.rthread;
	// usrloopthread should not be yielded
	if (rthread == &usrloopthread)
		return;
	unsigned char priority = rthread->priority;
	// Signify which lock this thread is waiting for
	rthread->mptr = m;
	struct Entry* rt;
	// Remove from top of running queue
	rt = pop_thread_from_queue(THREAD_READY, priority);
	if (rt != 0)
		// Push to bottom of wait queue
		push_thread_to_queue(rt->value, THREAD_MWAIT, priority);
	// Find the thread that has the mutex locked
	struct Mutex* mtex = m;
	struct Entry* mutex_next = find_pid(mtex->pid);
	if (mutex_next == 0)
		return;
	// The next thread is the one with the lock
	struct Entry* mutex_thread_entry = mutex_next->next;
	// Check if it is lower priority
	if (((struct Thread*)mutex_thread_entry->value)->priority > priority) {
		// Remove it from the old priority queue
		remove_next_from_queue(mutex_next);
		struct Thread* t = mutex_thread_entry->value;
		// Preserve the old priority
		if (t->old_priority == 0xFF)
			t->old_priority = t->priority;
		t->priority = priority;
		// Add it to the higher priority queue
		push_thread_to_queue(t, THREAD_READY, priority);
	}
}

void sched_semaphore_yield(void* s)
{
	struct Thread* rthread = scheduler.rthread;
	// usrloopthread should not be yielded
	if (rthread == &usrloopthread)
		return;
	unsigned char priority = rthread->priority;
	// Signify which lock this thread is waiting for
	rthread->mptr = s;
	struct Entry* rt;
	// Remove from top of running queue
	rt = pop_thread_from_queue(THREAD_READY, priority);
	if (rt != 0)
		// Push to bottom of wait queue
		push_thread_to_queue(rt->value, THREAD_SWAIT, priority);
}

unsigned long sched_mutex_resurrect(void* m)
{
	// Find any mutex to resurrect
	struct Entry* prev = find_mutex_wait_next(m);
	if (prev == 0)
		return 0;
	struct Entry* entry = prev->next;
	struct Thread* thread = entry->value;
	// Resurrect the thread
	thread->mptr = 0;
	// Remove from wait queue
	entry = remove_next_from_queue(prev);
	if (entry == 0)
		return 1;
	// Add to ready queue
	push_thread_to_queue(entry->value, THREAD_READY, ((struct Thread*)entry->value)->priority);
	// Demote current thread
	struct Thread* rthread = scheduler.rthread;
	unsigned long p = rthread->priority;
	unsigned long op = rthread->old_priority;
	// Restore the original priority level
	if (op != 0xFF) {
		struct Entry* tentry = pop_thread_from_queue(THREAD_READY, p);
		((struct Thread*)tentry->value)->priority = op;
		((struct Thread*)tentry->value)->old_priority = 0xFF;
		prepend_thread_to_queue(tentry->value, THREAD_READY, op);
	}
	return 1;
}

unsigned long sched_semaphore_resurrect(void* s, unsigned long count)
{
	unsigned long cnt = count;
	while (cnt) {
		// Find any signal/ semaphore to resurrect
		struct Entry* prev = find_signal_wait_next(s);
		if (prev == 0 && count == cnt)
			return 0;
		else if (prev == 0)
			return 1;
		struct Entry* entry = prev->next;
		struct Thread* thread = entry->value;
		// Resurrect the thread
		thread->mptr = 0;
		// Remove from wait queue
		entry = remove_next_from_queue(prev);
		if (entry == 0)
			return 1;
		// Add to ready queue
		push_thread_to_queue(entry->value, THREAD_READY, ((struct Thread*)entry->value)->priority);
		cnt--;
	}
	return 1;
}
