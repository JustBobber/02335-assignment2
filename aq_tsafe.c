/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include <pthread.h>
#include <string.h>
#include "aq.h"
#include "stdlib.h"

#define SEND_SUCCESS 0

typedef struct {
    void *msg;
    int kind;
    struct QueueMessage *next;
} QueueMessage;

typedef struct {
    QueueMessage *head;
    int size;
    pthread_mutex_t lock;
    pthread_cond_t has_content_condition, has_alarm_condition;
} Queue;

AlarmQueue aq_create() {
    Queue *queue = (Queue*) malloc(sizeof(Queue));
    queue->head = NULL;
    queue->size = 0;
    pthread_mutex_init(&(queue->lock), 0);
    pthread_cond_init(&(queue->has_content_condition), 0);
    pthread_cond_init(&(queue->has_alarm_condition), 0);
    return queue;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (aq == NULL) return AQ_UNINIT;
    Queue* queue = (Queue*) aq;

    if (msg == NULL) return AQ_NULL_MSG;

    // if unknown message kind, return err code
    if (k != 0 && k != 1) return AQ_NOT_IMPL;

    pthread_mutex_lock(&(queue->lock)); // aquire lock no matter the message kind

    while (aq_alarms(queue) > 0) {
        pthread_cond_wait(&(queue->has_alarm_condition), &(queue->lock));
    }

    // initialize new message
    QueueMessage *new_msg = malloc(sizeof(QueueMessage));
    new_msg->next = NULL;
    new_msg->msg = msg;
    new_msg->kind = k;

    if (queue->head == NULL) {
        // if queue head is null, set the new message as the head
        queue->head = new_msg;
    } else {
        // ... otherwise find the tail and append the new message
        QueueMessage *current = queue->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_msg;
    }

    queue->size++;
    if (k == AQ_ALARM) pthread_cond_signal(&(queue->has_alarm_condition)); // signal queue has alarm
    pthread_cond_signal(&(queue->has_content_condition)); // signal queue has content
    pthread_mutex_unlock(&(queue->lock)); // release mutex
    return SEND_SUCCESS;
}

int aq_recv(AlarmQueue aq, void **msg) {
    if (aq == NULL) return AQ_UNINIT;
    Queue *queue = (Queue*) aq;
    
    if (msg == NULL) return AQ_NULL_MSG;

    pthread_mutex_lock(&(queue->lock));

    while (queue->size == 0) {
        pthread_cond_wait(&(queue->has_content_condition), &(queue->lock));
    }
    
    // if there is alarm
    if (aq_alarms(queue) > 0) {
        QueueMessage *previous = NULL;
        QueueMessage *current = queue->head;
        while (current->next != NULL && current->kind != AQ_ALARM) {
            previous = current;
            current = current->next;
        }

        // store alarm (which is current) in result pointer and fix linked list if necessary
        *msg = current;
        if (current->next != NULL && previous != NULL) previous->next = current->next;

        queue->size--;
        pthread_cond_signal(&(queue->has_alarm_condition));
        if (queue->size == 0) pthread_cond_signal(&(queue->has_content_condition));
        pthread_mutex_unlock(&(queue->lock));
        return AQ_ALARM;
    }
    
    // if there is a normal message
    if (queue->head != NULL) {
        *msg = queue->head->msg; // store normal message queue head in result pointer
        QueueMessage *recv_msg = queue->head; // the received msg that is to be freed after.
        if (queue->head->next != NULL) {
            // if there is another message in the normal message queue, move that to be the head of the queue
            QueueMessage *next = queue->head->next;
            queue->head = next;
        } else {
            queue->head = NULL; // ... otherwise remove the last message
        }
        queue->size--;
        free(recv_msg); // freeing the received message
        if (queue->size == 0) pthread_cond_signal(&(queue->has_content_condition));
        pthread_mutex_unlock(&(queue->lock));
        return AQ_NORMAL;
    }
    
    // no alarms, no normal messages. Release and return err code
    pthread_mutex_unlock(&(queue->lock));
    return AQ_NO_MSG;
}

int aq_size(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = (Queue*) aq;
    return queue->size;
}

int aq_alarms(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = (Queue*) aq;
    
    pthread_mutex_lock(&(queue->lock));
    
    if (queue->head == NULL) return 0;

    int alarms = 0;
    QueueMessage *current = queue->head;
    while (current->next != NULL) {
        if (current->kind == AQ_ALARM) {
            alarms++;
        }
        current = current->next;
    }
    if (current->kind == AQ_ALARM) alarms++;
    
    pthread_mutex_unlock(&(queue->lock));
    
    return alarms;
}