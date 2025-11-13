/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include <string.h>

#include "aq.h"
#include "stdlib.h"

typedef struct {
    void *msg;
    int msg_kind;
    struct QueueMessage *next;
} QueueMessage;

typedef struct {
    QueueMessage *queue_head;
    int size;
} Queue;

AlarmQueue aq_create() {
    Queue *q = malloc(sizeof(Queue));
    return q;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (aq == NULL) return AQ_UNINIT;
    if (msg == NULL) return AQ_NULL_MSG;

    Queue* queue = aq;

    QueueMessage *new_msg = malloc(sizeof(QueueMessage));
    if (queue->queue_head == NULL) {
        queue->queue_head = new_msg;
        new_msg->msg = msg; // add msg to queue
        new_msg->msg_kind = k; // set type of msg
        queue->size++; // update size
    } else {
        QueueMessage *current = queue->queue_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_msg;
        new_msg->msg = msg; // add msg to queue
        new_msg->msg_kind = k; // set type of msg
        queue->size++; // update size
    }
    return 0;
    

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

    // if there is alarm
    if (aq_alarms(queue) == 1) {
        QueueMessage *current = queue->queue_head;
        while (current->next != NULL && current->msg_kind != AQ_ALARM) {
            current = current->next;
        }
        *msg = current->msg;
        queue->size--;
        return AQ_ALARM;
    }

    if (queue->queue_head != NULL) {
        *msg = queue->queue_head->msg;
        if (queue->queue_head->next != NULL) {
            QueueMessage *next = queue->queue_head->next;
            free(queue->queue_head);
            queue->queue_head = next;
        } else {
            free(queue->queue_head);
            queue->queue_head = NULL; // For removing last element of queue
        }
        queue->size--;
        return AQ_NORMAL;
    }

    return AQ_NO_MSG;
}

int aq_size(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = aq;
    return queue->size;
}

int aq_alarms(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = aq;

    QueueMessage *current = queue->queue_head;
    int has_alarm = 0;
    while (current->next != NULL && current->msg_kind != AQ_ALARM) {
        current = current->next;
    }

    if (current->msg_kind == AQ_ALARM) {
        has_alarm = 1;
    }

    return has_alarm;
}
