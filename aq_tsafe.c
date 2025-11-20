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
<<<<<<< Updated upstream
=======
    int alarm_count;
>>>>>>> Stashed changes
    pthread_mutex_t lock;
    pthread_cond_t has_content, has_no_alarm;
} Queue;


AlarmQueue aq_create() {
    Queue *queue = (Queue*) malloc(sizeof(Queue));
    queue->head = NULL;
    queue->size = 0;
<<<<<<< Updated upstream
=======
    queue->alarm_count = 0;
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
    pthread_mutex_lock(&(queue->lock)); // acquire lock no matter the message kind

    // if k is alarm then wait till there is no alarms in the queue.
    if (k == AQ_ALARM) {
        while (queue->head != NULL && queue->head->kind == AQ_ALARM) {
            pthread_cond_wait(&(queue->has_no_alarm), &(queue->lock));
        }
    } 

=======
>>>>>>> Stashed changes
    // initialize new message
    QueueMessage *new_msg = malloc(sizeof(QueueMessage));
    new_msg->next = NULL;
    new_msg->msg = msg;
    new_msg->kind = k;

    pthread_mutex_lock(&(queue->lock)); // acquire lock no matter the message kind

    while (k == AQ_ALARM && queue->alarm_count > 0) {
        pthread_cond_wait(&(queue->has_alarm), &(queue->lock));
    }

    // place the new message correctly in the queue
    if (queue->head == NULL) {
        queue->head = new_msg;
    } else if (new_msg->kind == AQ_ALARM) {
        // alarms are inserted at head
        new_msg->next = queue->head;
        queue->head = new_msg;
        queue->alarm_count++;
    } else {
        // ... otherwise find the tail and append the new message
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
        pthread_cond_wait(&(queue->has_content), &(queue->lock));
    }

    // updating queue
    QueueMessage *message_to_free = queue->head;
    *msg = message_to_free->msg;
    queue->head = queue->head->next;

    // setting the signals.
<<<<<<< Updated upstream
    if (kind == AQ_ALARM) {
        pthread_cond_signal(&(queue->has_no_alarm));
=======
    if (message_to_free->kind == AQ_ALARM) {
        queue->alarm_count--;
        pthread_cond_signal(&(queue->has_alarm));
>>>>>>> Stashed changes
    }
<<<<<<< Updated upstream
    
    queue->size--;
    int kind = message_to_free->kind;
    free(message_to_free);

    pthread_mutex_unlock(&(queue->lock));
    return kind;
=======

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
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    pthread_mutex_lock(&(queue->lock));
    if (queue->head == NULL) {
        pthread_mutex_unlock(&(queue->lock));
        return 0;
    }
    MsgKind alarms = queue->head->kind == AQ_ALARM ? 1 : 0;
    pthread_mutex_unlock(&(queue->lock));

    return alarms;
=======
    if (queue->head == NULL) return 0;
    return queue->head->kind == AQ_ALARM ? 1 : 0;
>>>>>>> Stashed changes
}