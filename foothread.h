#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>	
#include <sys/shm.h>
#include <fcntl.h>

#define FOOTHREAD_DEFAULT_STACK_SIZE 2097152
#define FOOTHREAD_JOINABLE 1
#define FOOTHREAD_DETACHED 2
#define FOOTHREAD_MUTEX_INITIALIZER {.type = FOOTHREAD_DETACHED, .stack_size = FOOTHREAD_DEFAULT_STACK_SIZE}
#define FOOTHREAD_THREADS_MAX 100

typedef struct {
    int type;
    int stack_size;
} foothread_attr_t;

typedef struct foothread_t{
    pid_t tid;
    int is_joinable;
    int is_detached;
    int is_running;
    int is_finished;
    int is_joined;
    void *retval;
    pid_t par_tid;
    int semid;
    char *stack;
} foothread_t;

typedef struct {
    int semid;
    pid_t owner;
} foothread_mutex_t;

typedef struct {
    int count;
    int reached;
    int semaphore;
    int mutex_semaphore;
} foothread_barrier_t;

void foothread_create(foothread_t *, foothread_attr_t *, int (*) (void *), void *);
void foothread_exit();
void foothread_attr_setjointype(foothread_attr_t *, int );
void foothread_attr_setstacksize(foothread_attr_t *, int );
void foothread_mutex_init ( foothread_mutex_t * ) ;
void foothread_mutex_lock ( foothread_mutex_t * ) ;
void foothread_mutex_unlock ( foothread_mutex_t * ) ;
void foothread_mutex_destroy ( foothread_mutex_t * ) ;
void foothread_barrier_init ( foothread_barrier_t * , int ) ;
void foothread_barrier_wait ( foothread_barrier_t * ) ;
void foothread_barrier_destroy ( foothread_barrier_t * ) ;
