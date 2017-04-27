#include "types.h"
#include "user.h"

void handler(int signum) {
	printf(1, "OUR handler called: %d\n", signum);
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
	exit();
}