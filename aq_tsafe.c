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
    pthread_cond_t has_content, has_alarm;
} Queue;

AlarmQueue aq_create() {
    Queue *queue = (Queue*) malloc(sizeof(Queue));
    queue->head = NULL;
    queue->size = 0;
    pthread_mutex_init(&(queue->lock), 0);
    pthread_cond_init(&(queue->has_content), 0);
    pthread_cond_init(&(queue->has_alarm), 0);
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

    pthread_mutex_lock(&(queue->lock)); // acquire lock no matter the message kind

    // if k is alarm then wait till there is no alarms in the queue.
    if (k == AQ_ALARM) {
        while (queue->head != NULL && queue->head->kind == AQ_ALARM) {
            pthread_cond_wait(&(queue->has_alarm), &(queue->lock));
        }
    }

    // initialize new message
    QueueMessage *new_msg = malloc(sizeof(QueueMessage));
    new_msg->next = NULL;
    new_msg->msg = msg;
    new_msg->kind = k;

    // place the new message correctly in the queue
    if (queue->head == NULL) {
        queue->head = new_msg;
    } else if (new_msg->kind == AQ_ALARM) {
        // alarms are inserted at head
        new_msg->next = queue->head;
        queue->head = new_msg;
    } else {
        // ... otherwise find the tail and append the new message
        QueueMessage *current = queue->head;
        while (current->next != NULL) {
            current = (QueueMessage*) current->next;
        }
        current->next = new_msg;
    }

    queue->size++;
    if (k == AQ_ALARM) {
        pthread_cond_signal(&(queue->has_alarm)); // signal queue has alarm
    }

    pthread_cond_signal(&(queue->has_content)); // signal queue has content
    pthread_mutex_unlock(&(queue->lock)); // release mutex
    return SEND_SUCCESS;
}

int aq_recv(AlarmQueue aq, void **msg) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = (Queue*) aq;

    if (msg == NULL) {
        return AQ_NULL_MSG;
    }
    pthread_mutex_lock(&(queue->lock));

    while (queue->size == 0) {
        pthread_cond_wait(&(queue->has_content), &(queue->lock));
    }

    int kind = queue->head->kind;
    *msg = queue->head->msg;

    // updating queue
    QueueMessage *message_to_free = queue->head;
    queue->head = queue->head->next;
    queue->size--;

    free(message_to_free);

    // setting the signals.
    if (kind == AQ_ALARM) {
        pthread_cond_signal(&(queue->has_alarm));
    }

    if (queue->head == NULL) {
        pthread_cond_signal(&(queue->has_content));
    }

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