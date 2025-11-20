/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include <pthread.h>
#include "aq.h"
#include "stdlib.h"

#define SEND_SUCCESS 0

typedef struct QueueMessage {
    void *msg;
    MsgKind kind;
    struct QueueMessage *next;
} QueueMessage;

typedef struct {
    QueueMessage *head;
    int size;
    int alarm_count;
    pthread_mutex_t lock;
    pthread_cond_t has_content, has_no_alarm;
} Queue;


AlarmQueue aq_create() {
    Queue *queue = (Queue*) malloc(sizeof(Queue));
    queue->head = NULL;
    queue->size = 0;
    queue->alarm_count = 0;
    pthread_mutex_init(&(queue->lock), 0);
    pthread_cond_init(&(queue->has_content), 0);
    pthread_cond_init(&(queue->has_no_alarm), 0);
    return queue;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }

    if (msg == NULL) {
        return AQ_NULL_MSG;
    }

    // if unknown message kind, return err code
    if (k != 0 && k != 1) {
        return AQ_NOT_IMPL;
    }
    
    Queue* queue = (Queue*) aq;

    // initialize new message
    QueueMessage *new_msg = malloc(sizeof(QueueMessage));
    new_msg->next = NULL;
    new_msg->msg = msg;
    new_msg->kind = k;

    pthread_mutex_lock(&(queue->lock)); // acquire lock no matter the message kind

    // if kind is alarm, wait
    while (k == AQ_ALARM && queue->alarm_count > 0) {
        pthread_cond_wait(&(queue->has_no_alarm), &(queue->lock));
    }

    // insert message into queue
    if (queue->head == NULL) {
        // if the queue is empty, insert at head
        queue->head = new_msg;
    } else if (new_msg->kind == AQ_ALARM) {
        // if the queue is not empty and the incoming kind is alarm, insert at heat and shift down
        new_msg->next = queue->head;
        queue->head = new_msg;
        queue->alarm_count++;
    } else {
        // if the queue is not empty and the incoming kind is normal, insert at tail
        QueueMessage *current = queue->head;
        while (current->next != NULL) {
            current = (QueueMessage*) current->next;
        }
        current->next = new_msg;
    }

    queue->size++;
    pthread_cond_signal(&(queue->has_content)); // signal queue has content
    pthread_mutex_unlock(&(queue->lock)); // release mutex
    return SEND_SUCCESS;
}

int aq_recv(AlarmQueue aq, void **msg) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }

    if (msg == NULL) {
        return AQ_NULL_MSG;
    }

    Queue *queue = (Queue*) aq;

    pthread_mutex_lock(&(queue->lock));

    while (queue->size == 0) {
        // queue is empty, wait
        pthread_cond_wait(&(queue->has_content), &(queue->lock));
    }

    // store result in result pointer and update queue head
    QueueMessage *message_to_free = queue->head;
    *msg = message_to_free->msg;
    queue->head = queue->head->next;

    if (message_to_free->kind == AQ_ALARM) {
        // received is alarm, decrement alarm count and signal alarm
        queue->alarm_count--;
        pthread_cond_signal(&(queue->has_no_alarm));
    }
    
    // decrement size no matter what
    queue->size--;
    int kind = message_to_free->kind;
    free(message_to_free);

    pthread_mutex_unlock(&(queue->lock));
    return kind;
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
    if (queue->head == NULL) {
        pthread_mutex_unlock(&(queue->lock));
        return 0;
    }
    MsgKind alarms = queue->head->kind == AQ_ALARM ? 1 : 0;
    pthread_mutex_unlock(&(queue->lock));

    return alarms;
}