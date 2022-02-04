struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int clone(void (*func_ptr)(void *), void *stack, int flags, void *arg);
int join(int tid);
int tkill(int tid);
int gettid(void);
int getppid(void);
int getcwdinum(void);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);


// For Kernel Threads
typedef struct THREAD{
    int tid;
    void* stack;
}THREAD;

THREAD pthread_create(void (*fcn)(void *), void *arg, int flags);
int pthread_join(THREAD t);
int get_pid(void);
int get_tid(void);
int pthread_kill(int tid);
int thread_getcwdinum(void);