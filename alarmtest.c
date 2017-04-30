#include "types.h"
#include "user.h"

void handler(int signum) {
	printf(1, "ALARM handler called: %d\n", signum);
}

int
main(int argc, char *argv[])
{
	alarm(2);
	printf(1, "Finished sigalarm\n");

	sleep(5);

	signal(SIGALRM, handler);
	alarm(2);
	sleep(5);
	exit();
}