/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */


#include <pthread.h>
#include <string.h>

#include "aq.h"
#include "aq.h"
#include "stdlib.h"

typedef struct {
    void *val;
    void *next;
} NormalQueueMessage;

typedef struct {
    void *alarm;
    NormalQueueMessage *q_msg;
    pthread_mutex_t lock;
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

    // alarm message
    if (k == AQ_ALARM) {
        pthread_mutex_lock(&(queue->lock));
        if (queue->alarm != NULL) {
            pthread_cond_wait(&(queue->recvCondition), &(queue->lock));
        }
        queue->alarm = msg;
        pthread_cond_signal(&(queue->sendCondition));
        pthread_mutex_unlock(&(queue->lock));
        return 0;
    }

    // normal message
    if (k == AQ_NORMAL) {
        NormalQueueMessage *new_msg = malloc(sizeof(NormalQueueMessage));
        new_msg->next = NULL;
        if (queue->q_msg == NULL) {
            queue->q_msg = new_msg;
            new_msg->val = msg; // add msg to queue
        } else {
            NormalQueueMessage *current = queue->q_msg;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_msg;
            new_msg->val = msg; // add msg to queue
        }
        pthread_cond_signal(&(queue->sendCondition));
        return 0;
    }

    // unknown message kind
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

    if (aq_size(aq) == 0) {
        pthread_cond_wait(&(queue->sendCondition), &(queue->lock));
    }
    
    // if there is alarm
    if (aq_alarms(queue) == 1) {
        *msg = queue->alarm;
        queue->alarm = NULL;
        pthread_cond_signal(&(queue->recvCondition));
        pthread_mutex_unlock(&(queue->lock));
        return AQ_ALARM;
    }
    
    if (queue->q_msg != NULL) {
        *msg = queue->q_msg->val;
        if (queue->q_msg->next != NULL) {
            NormalQueueMessage *next = queue->q_msg->next;
            free(queue->q_msg);
            queue->q_msg = next;
            
        } else {
            free(queue->q_msg);
            queue->q_msg = NULL; // For removing last element of queue
        }
        pthread_mutex_unlock(&(queue->lock));
        return AQ_NORMAL;
    }
    
    pthread_mutex_unlock(&(queue->lock));
    return AQ_NO_MSG;
}

int aq_size(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = aq;
    int size = 0;
    
    NormalQueueMessage *current = queue->q_msg;
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
    Queue *queue = aq;
    return queue->alarm != NULL ? 1 : 0;
}