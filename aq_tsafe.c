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

typedef struct QueueMessage {
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
    
    // initialize new message
    QueueMessage *new_msg = malloc(sizeof(QueueMessage));
    new_msg->next = NULL;
    new_msg->msg = msg;
    new_msg->kind = k;
    
    while (k == AQ_ALARM && queue->head->kind == AQ_ALARM) {
        pthread_cond_wait(&(queue->has_alarm_condition), &(queue->lock));
    }
    if (queue->head == NULL) {
        // if queue head is null, set the new message as the head
        queue->head = new_msg;
    } 
    // Put the alarm front of queue
    else if (k == AQ_ALARM) {
        QueueMessage *temp = queue->head;
        queue->head = new_msg;
        queue->head->next = temp;
    } else {
        // ... otherwise find the tail and append the new message
        QueueMessage *current = queue->head;
        while (current->next != NULL) {
            current = (QueueMessage*) current->next;
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

    // this guard clause shouldn't ever run since the above cond_wait call should wait for there to be something in the queue
    // we'll leave this just for safety
    if (queue->head == NULL) {
        pthread_mutex_unlock(&(queue->lock));
        return AQ_NO_MSG;
    }

    *msg = queue->head->msg; // copy head to result
    QueueMessage *received_queue_msg = queue->head; // the received queue message that should be freed after
    MsgKind received_kind = received_queue_msg->kind;

    if (queue->head->next != NULL)  {
        // there is another message in the queue, move this to be the new queue head
        QueueMessage *new_head = (QueueMessage*) queue->head->next;
        queue->head = new_head;
    } else {
        // no more messages, set head to null
        queue->head = NULL;
    }

    queue->size--;
    free(received_queue_msg);
    if (received_queue_msg->kind == AQ_ALARM) pthread_cond_signal(&(queue->has_alarm_condition));
    if (queue->size == 0) pthread_cond_signal(&(queue->has_content_condition));
    pthread_mutex_unlock(&(queue->lock));
    return received_kind;
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
    if (queue->head == NULL) return 0;
    return queue->head->kind == AQ_ALARM ? 1 : 0;
}