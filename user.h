struct stat;
struct rtcdate;

typedef void (*sighandler_t)(int);

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
sighandler_t signal(int signum, sighandler_t handler);
int sigsend(int pid, int signum);
int alarm(int);

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
void* malloc(uint);

// uthread.c
#define MAX_UTHREADS 64
int uthread_init();
int uthread_create(void (*start_func)(void *), void* arg);
void uthread_schedule();
void uthread_exit();
int uthread_self();
int uthread_join(int tid);
int uthread_sleep(int ticks);
void uthread_yield();
void uthread_wait();
void uthread_wakeup(int tid);

// usemaphore.c
int bsem_alloc();
void bsem_free(int);
void bsem_down(int);
void bsem_up(int);
struct counting_semaphore;
struct counting_semaphore* sem_alloc(uint value);
void sem_free(struct counting_semaphore* s);
void down(struct counting_semaphore* sem);
void up(struct counting_semaphore* sem);


#define SIGALRM 14

#define OLD_TF_MAGIC 0xDEADBEEF

#define UTHREAD_QUANTA 5