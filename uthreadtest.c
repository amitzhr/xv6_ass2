#include "types.h"
#include "user.h"

void test(void* arg) {
	printf(1, "Test called: %d, %d\n", uthread_self(), (uint)arg);
	//uthread_exit();
}

int main(int argc, char *argv[])
{
	int threads[5], i;
	uthread_init();

	uthread_join(uthread_create(test, 0));

	for (i = 0; i < 5; i++) {
		threads[i] = uthread_create(test, (void*)i);
	}

	for (i = 0; i < 5; i++) {
		uthread_join(threads[i]);
	}
	
	exit();
	return 0;
}