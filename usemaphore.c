#include "types.h"
#include "user.h"
#include "x86.h"

#define MAX_BSEM 128

struct semaphore {
	uint available;
	uint bid;
	uint waiting_threads[MAX_UTHREADS];
	uint waiting_thread_index;
};

struct counting_semaphore {
	uint s1;
	uint s2;
	uint value;
};

struct semaphore semaphores[MAX_BSEM];
uint bsemIndex = 0;
uint bidCounter = 0;

uint find_bsem(int bid) {
	uint i;
	for (i = 0; i < bsemIndex; i++) {
		if (semaphores[i].bid == bid) {
			return i;
		}
	}
	return -1;
}

int bsem_alloc() {
	if (bsemIndex >= MAX_BSEM) {
		printf(2, "No space to alloc semaphore\n");
		exit();
	}

	
	uint bid = bidCounter;
	bidCounter++;

	semaphores[bsemIndex].bid = bid;
	semaphores[bsemIndex].available = 1;
	semaphores[bsemIndex].waiting_thread_index = 0;
	memset(semaphores[bsemIndex].waiting_threads, -1, sizeof(semaphores[bsemIndex].waiting_threads));
	//printf(1, "Waiting threads: %d %d %d\n", semaphores[bsemIndex].waiting_threads[0], semaphores[bsemIndex].waiting_threads[1], semaphores[bsemIndex].waiting_threads[2]);

	bsemIndex++;

	return bid;
}

void bsem_free(int bid) {
	uint i = find_bsem(bid);
	if (i == -1) {
		printf(2, "bsem_free: failed to find semaphore %d\n", bid);
		exit();
	}

	printf(1, "Deleting semaphore %d\n", bid);
	uint j;
	for (j = i; j < bsemIndex - 1; j++) {
		semaphores[j] = semaphores[j + 1];
	}
}

void bsem_down(int bid) {
	alarm(0);
	uint i = find_bsem(bid);
	if (i == -1) {
		printf(2, "bsem_down: failed to find semaphore %d\n", bid);
		exit();
	}
	struct semaphore* b = &semaphores[i];

	if (b->available) {
		b->available = 0;
		alarm(UTHREAD_QUANTA);
	} else {
		uint j;
		for (j = 0; j < MAX_UTHREADS; j++) {
			if (b->waiting_threads[j] == -1) {
				break;
			}
		}
		if (j == MAX_UTHREADS) {
			printf(2, "Too many waiting threads!\n");
			exit();
		}
		//printf(1, "bsem_down: Added %d to waiting list\n", uthread_self());
		b->waiting_threads[j] = uthread_self();
		uthread_wait();
	}
}

void bsem_up(int bid) {
	alarm(0);
	//printf(1, "bsem_up: %d\n", uthread_self());
	uint i = find_bsem(bid);
	if (i == -1) {
		printf(2, "bsem_up: failed to find semaphore %d\n", bid);
		exit();
	}	

	struct semaphore* b = &semaphores[i];

	if (b->available) {
		printf(2, "bsem_up: Tried to up an available semaphore (%d)!\n", uthread_self());
		exit();
	}
		
	i = b->waiting_thread_index;
	uint threadsChecked = 0;
	while (threadsChecked < MAX_UTHREADS) {
		if (b->waiting_threads[i] != -1) {
			//printf(1, "bsem_up: Waking up %d\n", b->waiting_threads[i]);
			uthread_wakeup(b->waiting_threads[i]);
			b->waiting_threads[i] = -1;
			b->waiting_thread_index = (i+1) % MAX_UTHREADS;
			alarm(UTHREAD_QUANTA);
			return;
		}
		i = (i + 1) % MAX_UTHREADS;
		threadsChecked++;
	}

	alarm(UTHREAD_QUANTA);
	b->available = 1;
}

struct counting_semaphore* sem_alloc(uint value) {
	struct counting_semaphore* s = malloc(sizeof(struct counting_semaphore));
	s->value = value;
	s->s1 = bsem_alloc();
	s->s2 = bsem_alloc();
	if (value == 0) {
		bsem_down(s->s2);
	}
	return s;
}

void sem_free(struct counting_semaphore* s) {
	//printf(1, "Freeing counting_semaphore %x\n", (uint)s);
	free(s);
}

void down(struct counting_semaphore* sem) {
	bsem_down(sem->s2);
	bsem_down(sem->s1);
	sem->value--;
	if (sem->value > 0) {
		bsem_up(sem->s2);
	}
	bsem_up(sem->s1);
}

void up(struct counting_semaphore* sem) {
	bsem_down(sem->s1);
	sem->value++;
	if (sem->value == 1) {
		bsem_up(sem->s2);
	}
	bsem_up(sem->s1);
}