#include "types.h"
#include "user.h"

#define MAX_QUEUE_VALUE 5
#define QUEUE_SIZE 3

uint queue[QUEUE_SIZE];
uint producerIndex = 0;
uint consumerIndex = 0;
uint producerValue = 1;

struct counting_semaphore* empty;
struct counting_semaphore* full;
uint mutex;

void producer_handler(void* param) {
	while (producerValue <= MAX_QUEUE_VALUE) {
		down(empty);
		bsem_down(mutex);
		printf(1, "Produced %d\n", producerValue);
		queue[producerIndex] = producerValue;
		producerIndex = (producerIndex + 1) % QUEUE_SIZE;
		producerValue++;
		bsem_up(mutex);
		up(full);
	}
}

void consumer_handler(void* param) {
	while (1) {
		down(full);
		bsem_down(mutex);
		uint item = queue[consumerIndex];
		printf(1, "Consumed %d\n", item);
		consumerIndex = (consumerIndex + 1) % QUEUE_SIZE;
		bsem_up(mutex);
		up(empty);
		uthread_sleep(item);
		printf(1, "Thread %d slept for %d ticks.\n", uthread_self(), item);
		if (item >= MAX_QUEUE_VALUE)
			break;
	}
}

int main(int argc, char *argv[])
{
	uthread_init();

	empty = sem_alloc(QUEUE_SIZE);
	full = sem_alloc(0);
	mutex = bsem_alloc();

	uint consumers[3];
	uint producer = uthread_create(producer_handler, 0);

	uint i = 0;
	for (i = 0; i < 3; i++) {
		consumers[i] = uthread_create(consumer_handler, 0);
	}

	for (i = 0; i < 3; i++) {
		uthread_join(consumers[i]);
	}
	uthread_join(producer);
	exit();
	return 0;
}