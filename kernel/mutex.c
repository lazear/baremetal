/*
mutex.c
*/ 

#include <types.h>
#include <mutex.h>

void spin_lock (mutex* m) {
	while(m->lock);
		preempt();
	m->lock = 1;
	printf("Mutex (%x) locked by PID %d\n", m, getpid());
}

void spin_unlock(mutex* m) {
	if (m->lock) {
		m->lock = 0;
	//	k_sched_state(1);
		printf("Mutex (%x) unlocked by PID %d\n", m, getpid());
	}
}

