#include "types.h"
#include "user.h"

void handler(int signum) {
	printf(1, "OUR handler called: %d\n", signum);
}

void handler2(int signum) {
	printf(1, "A %d\n", signum);
	printf(1, "B");
}

int
main(int argc, char *argv[])
{
	int pid = getpid();
	sigsend(pid, 5);
	printf(1, "Finished sigsend\n");

	printf(1, "Called signal: %x\n", signal(6, handler));
	sigsend(pid, 6);
	sigsend(pid, 5);

	printf(1, "Called signal: %x\n", signal(3, handler2));
	sigsend(pid, 3);
	sleep(10);
	exit();
}