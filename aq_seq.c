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
    int kind;
    struct QueueMessage *next;
} QueueMessage;

typedef struct {
    QueueMessage *head;
    int size;
} Queue;

AlarmQueue aq_create() {
    Queue *q = malloc(sizeof(Queue));
    return q;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (aq == NULL) return AQ_UNINIT;
    if (msg == NULL) return AQ_NULL_MSG;
    
    if (k == AQ_ALARM && aq_alarms(aq) == 1) {
        return AQ_NO_ROOM;
    }

    Queue* queue = aq;

    QueueMessage *new_msg = malloc(sizeof(QueueMessage));
    if (queue->head == NULL) {
        queue->head = new_msg;
        new_msg->msg = msg; // add msg to queue
        new_msg->kind = k; // set type of msg
        queue->size++; // update size
    } else {
        QueueMessage *current = queue->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_msg;
        new_msg->msg = msg; // add msg to queue
        new_msg->kind = k; // set type of msg
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
        QueueMessage *current = queue->head;
        QueueMessage *prev = NULL;
        while (current->next != NULL && current->kind != AQ_ALARM) {
            prev = current;
            current = current->next;
        }
        if (current->next != NULL && prev != NULL) {
            prev->next = current->next;
        }

        *msg = current->msg;
        queue->size--;
        return AQ_ALARM;
    }

    if (queue->head != NULL) {
        *msg = queue->head->msg;
        if (queue->head->next != NULL) {
            QueueMessage *next = queue->head->next;
            free(queue->head);
            queue->head = next;
        } else {
            free(queue->head);
            queue->head = NULL; // For removing last element of queue
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

    QueueMessage *current = queue->head;
    if (current == NULL) {
        return 0;
    }
    int alarms = 0;
    if (current->kind == AQ_ALARM) {
        alarms++;
    }
    while (current->next != NULL) {
        current = current->next;
        if (current->kind == AQ_ALARM) {
            alarms++;
        }
    }

    return alarms;
}
