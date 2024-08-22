#include "foothread.h"
#define mutno 10045
int semno = 1046;
int semm;
int seminit = 1;

foothread_t global_foothread[FOOTHREAD_THREADS_MAX];
int global_foothread_count = 0;
int mutex;
int init = 1;

void foothread_create(foothread_t *thread,foothread_attr_t *attr, int (* start_routine) (void *) ,void *arg)
{
    mutex = semget(mutno,1,0666 | IPC_CREAT);
    if(init){
        semctl(mutex,0,SETVAL,1);
        init = 0;
    }
    struct sembuf vop;
    vop.sem_num = 0;
	vop.sem_flg = 0;
	vop.sem_op = 1 ;
    semop(mutex,&vop,1);
    if(attr == NULL){
        char *stack = (char *)malloc(sizeof(char) * FOOTHREAD_DEFAULT_STACK_SIZE);
        if(stack == NULL){
            perror("Error in allocating stack\n");
            exit(1);
        }
        int tid = clone(start_routine, stack + FOOTHREAD_DEFAULT_STACK_SIZE, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD, arg);
        thread->tid = tid;
        thread->is_joinable = 0;
        thread->is_detached = 1;
        thread->is_running = 1;
        thread->is_finished = 0;
        thread->semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        thread->is_joined = 0;
        thread->par_tid = gettid();
        thread->stack = stack;
        global_foothread[global_foothread_count] = *thread;
        global_foothread_count++;
        semctl(thread->semid, 0, SETVAL, 0);
    }
    else if( attr->type == FOOTHREAD_DETACHED){
        char *stack = (char *)malloc(sizeof(char) * attr->stack_size);
        if(stack == NULL){
            perror("Error in allocating stack\n");
            exit(1);
        }
        int tid = clone(start_routine, stack + attr->stack_size, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD, arg);
        thread->tid = tid;
        thread->is_joinable = 0;
        thread->is_detached = 1;
        thread->is_running = 1;
        thread->is_finished = 0;
        thread->is_joined = 0;
        thread->par_tid = gettid();
        thread->semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        global_foothread[global_foothread_count] = *thread;
        global_foothread_count++;
        semctl(thread->semid, 0, SETVAL, 0);
    }
    else if( attr->type == FOOTHREAD_JOINABLE){
        char *stack = (char *)malloc(sizeof(char) * attr->stack_size);
        if(stack == NULL){
            perror("Error in allocating stack\n");
            exit(1);
        }
        int tid = clone(start_routine, stack + attr->stack_size, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD, arg);
        thread->tid = tid;
        thread->is_joinable = 1;
        thread->is_detached = 0;
        thread->is_running = 1;
        thread->is_finished = 0;
        thread->is_joined = 0;
        thread->par_tid = gettid();
        thread->semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        global_foothread[global_foothread_count] = *thread;
        global_foothread_count++;
        semctl(thread->semid, 0, SETVAL, 0);
    }
    //display global_foothread
    //int i = global_foothread_count - 1;
    //printf("tid: %d, par_tid: %d, is_joinable: %d, is_detached: %d, is_running: %d, is_finished: %d, is_joined: %d, semid: %d\n", global_foothread[i].tid, global_foothread[i].par_tid, global_foothread[i].is_joinable, global_foothread[i].is_detached, global_foothread[i].is_running, global_foothread[i].is_finished, global_foothread[i].is_joined,global_foothread[i].semid);
    semop(mutex,&vop,1);
}

void foothread_attr_setjointype(foothread_attr_t *attr, int type)
{
    attr->type = type;
}

void foothread_attr_setstacksize(foothread_attr_t *attr, int stack_size)
{
    attr->stack_size = stack_size;
}

void foothread_exit()
{
    //printf("Exiting ");
    mutex = semget(mutno,1,0666 | IPC_CREAT);
    struct sembuf pop,vop;
    //char *stack;
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
    int i;
    pid_t tid = gettid();
    //printf("tid: %d\n", tid);
    semop(mutex,&pop,1);
    for(i = 0; i < global_foothread_count; i++){
        if(global_foothread[i].par_tid == tid && global_foothread[i].is_joinable){
            semop(mutex, &vop, 1);
            //printf("%d waiting for %d\n",global_foothread[i].par_tid,global_foothread[i].tid);
            semop(global_foothread[i].semid, &pop, 1);
            semop(mutex, &pop, 1);
            global_foothread[i].is_joined = 1;
            semctl(global_foothread[i].semid, 0, IPC_RMID, 0);
        }
    }
    for(i = 0; i < global_foothread_count; i++){
        if(global_foothread[i].tid == tid){
            global_foothread[i].is_running = 0;
            global_foothread[i].is_finished = 1;
            semop(global_foothread[i].semid, &vop, 1);
            if(global_foothread[i].is_detached){
                semctl(global_foothread[i].semid, 0, IPC_RMID, 0);
            }
            //stack = global_foothread[i].stack;
            break;
        }
    }
    //display global_foothread
    /*for(i = 0; i < global_foothread_count; i++){
        printf("tid: %d, par_tid: %d, is_joinable: %d, is_detached: %d, is_running: %d, is_finished: %d, is_joined: %d, semid: %d\n", global_foothread[i].tid, global_foothread[i].par_tid, global_foothread[i].is_joinable, global_foothread[i].is_detached, global_foothread[i].is_running, global_foothread[i].is_finished, global_foothread[i].is_joined,global_foothread[i].semid);
   }*/
   semop(mutex,&vop,1);
}

void foothread_mutex_init ( foothread_mutex_t *mutex )
{
    struct sembuf pop,vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ; vop.sem_op = 1 ;
    if(seminit){
        semm = semget(ftok(".",semno), 1, 0666 | IPC_CREAT);
        semno++;
        semctl(semm, 0, SETVAL, 1);
        seminit = 0;
    }
    semop(semm, &pop, 1);
    mutex->semid = semget(ftok(".",semno), 1, 0666 | IPC_CREAT);
    semno++;
    semctl(mutex->semid, 0, SETVAL, 1);
    mutex->owner = -1;
    semop(semm, &vop, 1);
}

void foothread_mutex_lock ( foothread_mutex_t *mutex )
{
    struct sembuf pop,vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ; vop.sem_op = 1 ;
    semop(mutex->semid, &pop, 1);
    mutex->owner = gettid();
}

void foothread_mutex_unlock ( foothread_mutex_t *mutex )
{
    struct sembuf vop;
    vop.sem_num = 0;
    vop.sem_flg = 0;
    vop.sem_op = 1 ;
    if(mutex->owner != gettid()){
        //printf("Error: Not owner of mutex\n");
        //exit(1);
    }
    else{
    semop(mutex->semid, &vop, 1);
    mutex->owner = -1;
    }
}

void foothread_mutex_destroy ( foothread_mutex_t *mutex )
{
    semctl(mutex->semid, 0, IPC_RMID, 0);
}

void foothread_barrier_init ( foothread_barrier_t *barrier , int count ){
    struct sembuf pop,vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ; vop.sem_op = 1 ;
    if(seminit){
        semm = semget(ftok(".",semno), 1, 0666 | IPC_CREAT);
        semno++;
        semctl(semm, 0, SETVAL, 1);
        seminit = 0;
    }
    semop(semm, &pop, 1);
    barrier->count = count;
    barrier->reached = 0;
    barrier->semaphore = semget(ftok(".",semno), 1, 0666 | IPC_CREAT);
    semno++;
    semctl(barrier->semaphore, 0, SETVAL, 0);
    barrier->mutex_semaphore = semget(ftok(".",semno), 1, 0666 | IPC_CREAT);
    semno++;
    semctl(barrier->mutex_semaphore, 0, SETVAL, 1);
    semop(semm, &vop, 1);
}

void foothread_barrier_wait ( foothread_barrier_t *barrier ){
    struct sembuf pop,vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ; vop.sem_op = 1 ;
    semop(barrier->mutex_semaphore, &pop, 1);
    barrier->reached++;
    if(barrier->reached == barrier->count){
        semop(barrier->mutex_semaphore, &vop, 1);
        vop.sem_op = barrier->count;
        semop(barrier->semaphore, &vop, 1);
        barrier->reached = 0;
        semop(barrier->semaphore, &pop, 1);
    }
    else{
        semop(barrier->mutex_semaphore, &vop, 1);
        semop(barrier->semaphore, &pop, 1);
    }
}

void foothread_barrier_destroy ( foothread_barrier_t *barrier ){
    semctl(barrier->semaphore, 0, IPC_RMID, 0);
    semctl(barrier->mutex_semaphore, 0, IPC_RMID, 0);
}

