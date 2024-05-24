#include <stdio.h>
#include <stdlib.h>

#include "../util/so_list.h"

PList init(PList list, Thread pt)
{
    PList node = malloc(sizeof(List));
    node->thread = pt;
    node->next = NULL;
    return node;
}

PList addT(PList head, Thread pt)
{
    if (head == NULL) {
        head = init(head, pt);
        return head;
    } else {
        PList prev = NULL, node = init(head, pt);
        PList copy = head;
        while (head != NULL && node->thread.priority <= head->thread.priority) {
            prev = head;
            head = head->next;
        }
        if (prev != NULL) {
            prev->next = node;
            node->next = head;
            head = copy;
        } else {
            node->next = head;
            head = node;
        }
        return head;
    }
}

void printList(PList head)
{
    if (head == NULL)
        printf("EMpty\n");
    else {
        while (head != NULL) {
            printf("%ld : %d ", head->thread.tid, head->thread.priority);
            head = head->next;
        }
        printf("\n");
    }
}

void freeList(PList head)
{
    if (head == NULL)
        printf("List empty.\n");
    else {
        PList copy;
        while (head != NULL) {
            copy = head->next;
            sem_destroy(&(head->thread.sem));
            free(head);
            head = copy;
        }
    }
}

PThread getT(PList head, tid_t tid)
{
    if (head == NULL)
        printf("List empty or element not found.\n");
    else {
        while (head != NULL) {
            if (head->thread.tid == tid)
                return &(head->thread);
            head = head->next;
        }
    }
    return NULL;
}

PList removeHead(PList head)
{
    if (head == NULL) {
        printf("List empty.\n");
        return NULL;
    } else {
        head = head->next;
        return head;
    }
}