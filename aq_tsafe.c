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
    void *next;
} NormalQueueMessage;

typedef struct {
    void *alarm;
    NormalQueueMessage *queue_msg;
    pthread_mutex_t lock;
    // send condition is broadcasted when a message is sent, recv condition is broadcasted when a message is consumed
    pthread_cond_t sendCondition, recvCondition;
} Queue;

AlarmQueue aq_create() {
    Queue *q = malloc(sizeof(Queue));
    pthread_mutex_init(&(q->lock), 0);
    pthread_cond_init(&(q->sendCondition), 0);
    pthread_cond_init(&(q->recvCondition), 0);
    return q;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (aq == NULL) return AQ_UNINIT;
    if (msg == NULL) return AQ_NULL_MSG;

    Queue* queue = aq;
    pthread_mutex_lock(&(queue->lock)); // aquire lock no matter the message kind

    // alarm message
    if (k == AQ_ALARM) {
        // while an alarm already exists, we wait until a recvCondition tells us that a new
        // message has been received. Then, we check again if an alarm already exists.
        while (queue->alarm != NULL) {
            pthread_cond_wait(&(queue->recvCondition), &(queue->lock));
        }
        queue->alarm = msg; // set alarm in queue
        pthread_cond_broadcast(&(queue->sendCondition)); // signal every thread waiting for sendCondition that a message has been sent
        pthread_mutex_unlock(&(queue->lock)); // release mutex
        return SEND_SUCCESS;
    }

    // normal message
    if (k == AQ_NORMAL) {
        // initialize new message
        NormalQueueMessage *new_msg = malloc(sizeof(NormalQueueMessage));
        new_msg->next = NULL;
        new_msg->msg = msg;

        if (queue->queue_msg == NULL) {
            // if message list head is null, set the new message as the head
            queue->queue_msg = new_msg;
        } else {
            // ... otherwise find the tail and append the new message
            NormalQueueMessage *current = queue->queue_msg;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_msg;
        }

        pthread_cond_broadcast(&(queue->sendCondition)); // signal every thread waiting for sendCondition that a message has been sent
        pthread_mutex_unlock(&(queue->lock)); // release mutex
        return SEND_SUCCESS;
    }

    // unknown message kind, release and return err code
    pthread_mutex_unlock(&(queue->lock));
    return AQ_NOT_IMPL;
}

int aq_recv(AlarmQueue aq, void * *msg) {
    if (aq == NULL) return AQ_UNINIT;
    if (msg == NULL) return AQ_NULL_MSG;
    if (*msg != NULL) {
        free(*msg);
        *msg = NULL;
    }

    Queue *queue = aq;

    pthread_mutex_lock(&(queue->lock));

    while (aq_size(aq) == 0) {
        pthread_cond_wait(&(queue->sendCondition), &(queue->lock));
    }
    
    // if there is alarm
    if (aq_alarms(queue) == 1) {
        *msg = queue->alarm;
        queue->alarm = NULL;
        pthread_cond_broadcast(&(queue->recvCondition)); // signal every thread waiting for recvCondition that a message has been consumed
        pthread_mutex_unlock(&(queue->lock));
        return AQ_ALARM;
    }
    
    if (queue->queue_msg != NULL) {
        *msg = queue->queue_msg->msg;
        if (queue->queue_msg->next != NULL) {
            NormalQueueMessage *next = queue->queue_msg->next;
            free(queue->queue_msg);
            queue->queue_msg = next;
            
        } else {
            free(queue->queue_msg);
            queue->queue_msg = NULL; // For removing last element of queue
        }
        pthread_mutex_unlock(&(queue->lock));
        return AQ_NORMAL;
    }
    
    pthread_mutex_unlock(&(queue->lock));
    return AQ_NO_MSG;
}

int aq_size(AlarmQueue aq) {
    // this is only ever called from aq_recv, which has mutex protection, so this doesn't need mutex handling for itself
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = aq;
    int size = 0;
    
    NormalQueueMessage *current = queue->queue_msg;
    if (current == NULL) {
        return size + aq_alarms(queue);
    }
    do {
        size++;
        current = current->next;
    } while (current != NULL);
    return size + aq_alarms(queue);
}

int aq_alarms(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    // no mutex needed since we only access the queue once
    Queue *queue = aq;
    return queue->alarm != NULL ? 1 : 0;
}