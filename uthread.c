#include "types.h"
#include "user.h"
#include "x86.h"

#define MAX_UTHREADS 64

#define THREAD_STACK_SIZE 4096

typedef struct {
	uint tid;
	char stack[THREAD_STACK_SIZE];
	struct trapframe tf;
	int new;
} TCB, *PTCB;

TCB* threads[MAX_UTHREADS];
uint threadsIndex = 0;

uint tidCounter = 0;

int currentThread = 0;

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

struct trapframe* find_old_tf() {
	uint current_esp;
	__asm__ ("movl %%esp, %0\n\t": "=r" (current_esp));
	uint* ptr = (uint*)(current_esp);
    while (*ptr != OLD_TF_MAGIC) {
      ptr++;
    }

    return (struct trapframe*)(ptr + 1);
}

void uthread_schedule() {
	//printf(1, "uthread_schedule called\n");

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

	//printf(1, "Running thread %d\n", threads[i]->tid);

	if (i != currentThread) {
		struct trapframe* tf = find_old_tf();

	    if (currentThread >= 0) {
	    	threads[currentThread]->tf = *tf;
	    }

	    if (threads[i]->new) {
	    	tf->esp = threads[i]->tf.esp;
	    	tf->eip = threads[i]->tf.eip;
	    	tf->ebp = threads[i]->tf.ebp;
	    	threads[i]->new = 0;
	    } else {
			//printf(1, "TF Before: %x %x %x %x %x %x %x %x\n", tf->eip, tf->ebp, tf->esp, tf->cs, tf->ds, tf->gs, tf->fs, tf->ss);
	    	*tf = threads[i]->tf;
			//printf(1, "TF After: %x %x %x %x %x %x %x %x\n", tf->eip, tf->ebp, tf->esp, tf->cs, tf->ds, tf->gs, tf->fs, tf->ss);
	    }

		currentThread = i;	
	}

	alarm(UTHREAD_QUANTA);
}

void uthread_exit() {
	//printf(1, "uthread_exit called for %d\n", threads[currentThread]->tid);
	free(threads[currentThread]);
	threads[currentThread] = 0;

	uint j;
	for (j = currentThread; j < MAX_UTHREADS - 1; j++) {
		threads[j] = threads[j + 1];
	}

	currentThread = -1;
	threadsIndex--;

	uthread_yield();
}

int uthread_init() {
	if (threadsIndex != 0) {
		printf(2, "uthread_init called more than once!\n");
		exit();
		return -1;
	}

	threads[threadsIndex] = malloc(sizeof(TCB));

	threads[threadsIndex]->tid = tidCounter;
	tidCounter++;

	threadsIndex++;

	signal(SIGALRM, alarm_handler);
	alarm(UTHREAD_QUANTA);
	return 0;
}

int uthread_create(void (*start_func)(void *), void* arg) {
	if (threadsIndex >= MAX_UTHREADS) {
		printf(2, "Too many threads! Exiting from uthread_create...\n");
		return -1;
	}

	PTCB tcb = malloc(sizeof(TCB));

	tcb->tid = tidCounter;
	tidCounter++;

	threads[threadsIndex] = tcb;
	threadsIndex++;

	tcb->tf.esp = (uint)(tcb->stack + THREAD_STACK_SIZE);
	tcb->tf.esp -= 4;
	*((uint*)tcb->tf.esp) = (uint)arg;
	tcb->tf.esp -= 4;
	*((uint*)tcb->tf.esp) = (uint)uthread_exit;
	tcb->tf.ebp = tcb->tf.esp;
	tcb->tf.eip = (uint)start_func;
	tcb->new = 1;

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
	//printf(1, "Thread %d sleeping for %d ticks\n", threads[currentThread]->tid, ticks);
	while (uptime() - start_time < ticks) {
		uthread_yield();
	}
	//printf(1, "%d Sleep done\n", threads[currentThread]->tid);
	return 0;
}

