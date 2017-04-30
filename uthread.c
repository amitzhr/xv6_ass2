#include "types.h"
#include "user.h"
#include "x86.h"

#define MAX_UTHREADS 64
#define UTHREAD_QUANTA 5

#define THREAD_STACK_SIZE 4096

typedef struct {
	uint tid;
	char stack[THREAD_STACK_SIZE];
	uint sp;
} TCB, *PTCB;

TCB* threads[MAX_UTHREADS];
uint threadsIndex = 0;

uint tidCounter = 0;

uint currentThread = 0;

PTCB find_thread_by_tid(int tid) {
	uint i;
	for (i = 0; i < MAX_UTHREADS; i++) {
		if (threads[i] && threads[i]->tid == tid)
			return threads[i];
	}
	return 0;
}

void uthread_yield() {
	sigsend(getpid(), SIGALRM);
}

void alarm_handler(int signum) {
	uthread_schedule();
}

void uthread_schedule() {
	printf(1, "uthread_schedule called\n");

	uint threadsChecked = 0;
	uint i = (currentThread + 1) % THREAD_STACK_SIZE;
	while (threadsChecked < MAX_UTHREADS) {
		if (threads[i])
			break;
		i = (i + 1) % MAX_UTHREADS;
		threadsChecked++;
	}

	if (threads[i] == 0) {
		printf(1, "No thread to run. Exiting process\n");
		exit();
	}

	printf(1, "Running thread %d\n", threads[i]->tid);

	if (i != currentThread) {
		uint current_esp;
		__asm__ ("movl %%esp, %0\n\t": "=r" (current_esp));
		uint* ptr = (uint*)(current_esp);
	    while (*ptr != OLD_TF_MAGIC) {
	      ptr++;
	    }

	    printf(1, "Guessed tf: %x\n", (ptr+1));
	    struct trapframe* tf = (struct trapframe*)(ptr + 1);

	    tf->esp = threads[i]->sp;
	    if (currentThread >= 0) 
	    	threads[currentThread]->sp = current_esp;

		currentThread = i;	
	}

	alarm(UTHREAD_QUANTA);
}

void uthread_exit() {
	printf(1, "uthread_exit called for %d\n", threads[currentThread]->tid);
	free(threads[currentThread]);
	threads[currentThread] = 0;

	uint j;
	for (j = currentThread; j < MAX_UTHREADS - 1; j++) {
		threads[j] = threads[j + 1];
	}

	currentThread = -1;

	uthread_yield();
}

int uthread_init() {
	if (threadsIndex != 0) {
		printf(2, "uthread_init called more than once!");
		return -1;
	}

	threads[threadsIndex] = malloc(sizeof(TCB));
	printf(1, "esp before: 0x%x\n", threads[threadsIndex]->sp);
	__asm__ ("movl %%esp, %0\n\t": "=r" (threads[threadsIndex]->sp));
	printf(1, "Saved esp: 0x%x\n", threads[threadsIndex]->sp);

	threads[threadsIndex]->tid = tidCounter;
	tidCounter++;

	threadsIndex++;

	signal(SIGALRM, alarm_handler);
	alarm(UTHREAD_QUANTA);
	return 0;
}

int uthread_create(void (*start_func)(void *), void* arg) {
	if (threadsIndex >= MAX_UTHREADS) {
		printf(2, "Too many threads! Exiting from uthread_create...");
		return -1;
	}

	PTCB tcb = malloc(sizeof(TCB));

	tcb->tid = tidCounter;
	tidCounter++;

	threads[threadsIndex] = tcb;
	threadsIndex++;

	tcb->sp = (uint)(tcb->sp + THREAD_STACK_SIZE);
	tcb->sp -= 4;
	*((uint*)tcb->sp) = (uint)arg;
	tcb->sp -= 4;
	*((uint*)tcb->sp) = (uint)start_func;

	return tcb->tid;
}

int uthread_self() {
	return (int)threads[currentThread]->tid;
}

int uthread_join(int tid) {
	while (find_thread_by_tid(tid) != 0)
		uthread_yield();
	return 0;
}

int uthread_sleep(int ticks) {
	uint start_time = uptime();
	printf(1, "Start time before: %d\n", start_time);
	while (uptime() - start_time < ticks) {
		printf(1, "Start time: %d\n", start_time);
		uthread_yield();
	}
	printf(1, "Sleep done\n");
	return 0;
}

