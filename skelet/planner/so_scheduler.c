#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "../util/so_scheduler.h"
#include "../util/so_list.h"

typedef struct planner {
    PList threads;
    PList ready_threads;
    unsigned int thread_count;
    unsigned int thread_time;
    unsigned int event_nr;
}Planner, *PPlanner;

typedef struct param { 
    unsigned int prio;
    so_handler *func;
}Param, *PParam;

void *so_start_thread(void *data);
void decreaseTime();
void scheduler();

static PPlanner so_planner;
static PList removed = NULL;
static int init = 0;
static int no_threads = 1;
static int threads_ran = 0;
static int a = -10;
static sem_t sem;
static pthread_mutex_t lock;

int so_init(unsigned int time_quantum, unsigned int io)
{
    sem_init(&sem, 0, 1);
    pthread_mutex_init(&lock, NULL);
    if (so_planner == NULL) {
        so_planner = malloc(sizeof(Planner));
        so_planner->threads = NULL;
        so_planner->ready_threads = NULL;
        so_planner->thread_count = 0;
    } else
        return -1;

    if (io > SO_MAX_NUM_EVENTS)
        return -1;
    else
        so_planner->event_nr = io;

    if (time_quantum == 0)
        return -1;
    else
        so_planner->thread_time = time_quantum;
    if (init == 0) {
        init = 1;
    }
    return 0;
}

void so_end(void)
{
    sem_getvalue(&sem, &a);
    printf("Trying to run so_end init is %d, sem is %d\n", init, a);
    if (init == 1)
        sem_wait(&sem);
    printf("After wait.\n");
    if(so_planner != NULL) {
        if(so_planner->threads != NULL) {
            PList myhd = so_planner->threads;
                while (myhd != NULL) {
                    pthread_join(myhd->thread.tid, NULL);
                    myhd = myhd->next;
                }
            freeList(so_planner->threads);
            so_planner->threads = NULL;
        }
        if (so_planner->ready_threads != NULL) {
            // PList copy = so_planner->ready_threads, next;
            // while (copy != NULL) {
            //     next = copy->next;
            //     free(copy);
            //     copy = next;
            // }
            freeList(so_planner->ready_threads);
            so_planner->ready_threads = NULL;
        }
        free(so_planner);
    }
    if (init == 1) {
        init = 0;
    }
    if (removed != NULL)
        freeList(removed);
    so_planner = NULL;
    sem_destroy(&sem);
    pthread_mutex_destroy(&lock);
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
    decreaseTime();
    if (priority > SO_MAX_PRIO || func == NULL) {
        return INVALID_TID;
    }
    tid_t tid;
    Param *p = malloc(sizeof(Param));
    if (no_threads == 1) {
        sem_wait(&sem);
        no_threads = 0;
    }
    p->func = func;
    p->prio = priority;
    int ret = pthread_create(&tid, NULL, so_start_thread ,(void *) p);
    if (ret == 0) {
        return tid;
    }
    else {
        return INVALID_TID;
    }    
}

void *so_start_thread(void *data)
{
    Param x = *((Param *) data);
    Thread thread;
    thread.priority = x.prio;
    sem_init(&(thread.sem), 0, 0);
    thread.state = 0;
    thread.tid = pthread_self();
    thread.time_left = so_planner->thread_time;
    thread.waiting_for = -1;
    printf("%p\n",so_planner->ready_threads);
    so_planner->ready_threads = addT(so_planner->ready_threads, thread);
    //printList(so_planner->ready_threads);
    so_planner->threads = addT(so_planner->threads, thread);
    so_planner->thread_count++;
    scheduler();
    x.func(x.prio);
    free(data);

    if (so_planner->ready_threads != NULL) {
        printList(so_planner->ready_threads);
        PList copy = so_planner->ready_threads;
        pthread_mutex_lock(&lock);
        so_planner->ready_threads = removeHead(so_planner->ready_threads);
        pthread_mutex_unlock(&lock);
        if (removed == NULL) {
            removed = copy;
            removed->next = NULL;
        } else {
            copy->next = removed;
            removed = copy;
        }
        threads_ran++;
        printList(so_planner->ready_threads);
        if (so_planner->ready_threads != NULL) {
            sem_post(&(so_planner->ready_threads->thread.sem));
            printf("%ld was posted\n", so_planner->ready_threads->thread.tid);
        }
    }
    printf("Threads created %d : threads ran %d\n", so_planner->thread_count, threads_ran);
    if (so_planner->thread_count <= threads_ran)
        sem_post(&sem);
    pthread_exit(NULL);
}

void scheduler()
{
    if (so_planner->ready_threads == NULL) {
        return;
    }
    PThread pt = getT(so_planner->ready_threads, pthread_self());
    if(pt == NULL)
        return;
    printf("Current thread is: %ld, first thread is %ld,list is: ", pt->tid, so_planner->ready_threads->thread.tid);
    printList(so_planner->ready_threads);
    if (pt->tid == so_planner->ready_threads->thread.tid) {
        sem_post(&(pt->sem));
    }
    sem_getvalue(&(pt->sem), &a);
    printf("Thread %ld trying to wait | curr sem = %d\n", pt->tid, a);
    sem_wait(&(pt->sem));
    sem_getvalue(&(pt->sem), &a);
    printf("Thread %ld resumed | sem is %d\n", pt->tid, a);
}

int so_wait(unsigned int io)
{
    decreaseTime();
    return -1;
}

int so_signal(unsigned int io)
{
    decreaseTime();
    return -1;   
}

void so_exec(void)
{
    decreaseTime();
}

void decreaseTime() {
    PThread pt = getT(so_planner->ready_threads, pthread_self());
    if (pt != NULL) {
        pt->time_left--;
    }
}