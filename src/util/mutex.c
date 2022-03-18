#include <util/mutex.h>
#include <globals.h>

void mutex_init(void)
{
	for (unsigned long m = 0; m < MAX_MUTEXS; m++) {
		mutexs[m].pid = 0;
		mutexs[m].addr = 0;
		mutex_entries[m].value = &mutexs[m];
		mutex_entries[m].entry_type = VALUE_ENTRY;
		mutex_entries[m].next = &mutex_entries[(m+1)%MAX_MUTEXS];
	}
	// Initialize Free Mutexs
	mutex_manager.free.start.value = 0;
	mutex_manager.free.start.next = &mutex_entries[0];
	mutex_manager.free.start.entry_type = START_ENTRY;
	mutex_manager.free.end.value = 0;
	mutex_manager.free.end.next = &mutex_entries[MAX_MUTEXS-1];
	mutex_entries[MAX_MUTEXS-1].next = &mutex_manager.free.end;
	mutex_manager.free.end.entry_type = END_ENTRY;
	// Initialize In-use Mutexs
	mutex_manager.used.start.value = 0;
	mutex_manager.used.start.next = &mutex_manager.used.end;
	mutex_manager.used.start.entry_type = START_ENTRY;
	mutex_manager.used.end.value = 0;
	mutex_manager.used.end.next = &mutex_manager.used.start;
	mutex_manager.used.end.entry_type = END_ENTRY;
}

struct Mutex* create_mutex(void* addr)
{
	struct Entry* e = pop_from_queue(&mutex_manager.free);
	if (e == 0)
		return 0;
	struct Mutex* m = e->value;
	m->pid = 0;
	m->addr = addr;
	push_to_queue(e, &mutex_manager.used);
	return e->value;
}

unsigned char delete_mutex(struct Mutex* m)
{
	struct Entry* entry = find_value(m, &mutex_manager.used);
	if (entry == 0)
		return 1;
	// Remove it from the queue
	remove_next_from_queue(entry);
	// Add it to the free queue
	prepend_to_queue(entry, &mutex_manager.free);
	return 0;
}
