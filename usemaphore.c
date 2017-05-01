#include "types.h"
#include "user.h"
#include "x86.h"

#define MAX_BSEM 128

struct semaphore {
	uint available;
	uint bid;
};

struct semaphore semaphores[MAX_BSEM];
uint bsemIndex = 0;
uint bidCounter = 0;

int find_bsem(int bid) {
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
	uint i = find_bsem(bid);
	if (i == -1) {
		printf(2, "bsem_down: failed to find semaphore %d\n", bid);
		exit();
	}

	struct semaphore* b = &semaphores[i];
	while (!b->available)
		uthread_yield();

	alarm(0);
	if (b->available) {
		b->available = 0;
		alarm(UTHREAD_QUANTA);
	} else {
		alarm(UTHREAD_QUANTA);
		bsem_down(bid);
	}
}

void bsem_up(int bid) {
	uint i = find_bsem(bid);
	if (i == -1) {
		printf(2, "bsem_up: failed to find semaphore %d\n", bid);
		exit();
	}	

	if (semaphores[i].available) {
		printf(2, "bsem_up: Tried to up an available semaphore!\n");
		exit();
	}
		
	semaphores[i].available = 1;
}