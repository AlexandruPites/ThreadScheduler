#include "so_scheduler.h"
#include <semaphore.h>

#define STATE_NEW 0
#define STATE_READY 1
#define STATE_RUNNING 2
#define STATE_WAITING 3
#define STATE_TERMINATED 4

typedef struct thread {
    tid_t tid;
    unsigned int state;
    int waiting_for;
    unsigned int priority;
    int time_left;
    sem_t sem;
}Thread, *PThread;

typedef struct list {
    struct list *next;
    Thread thread;
}List, *PList;

PList addT(PList head, Thread thread);
PThread getT(PList head, tid_t tid);
PList getL(PList head, tid_t tid);
int removeT(PList head, Thread thread);
void freeList(PList head);
void printList(PList head);
PList removeHead(PList head);