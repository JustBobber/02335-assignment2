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
    void *val;
    void *next;
} QueueMessage;

typedef struct {
    void *alarm;
    QueueMessage *q_msg;
} Queue;

AlarmQueue aq_create() {
    Queue *q = malloc(sizeof(Queue));
    return q;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (aq == NULL) return AQ_UNINIT;
    if (msg == NULL) return AQ_NULL_MSG;

    Queue* queue = aq;

    // alarm message
    if (k == AQ_ALARM) {
        if (queue->alarm != NULL) {
            return AQ_NO_ROOM;
        }
        queue->alarm = msg;
        return 0;
    }

    // normal message
    if (k == AQ_NORMAL) {
        QueueMessage *new_msg = malloc(sizeof(QueueMessage));
        if (queue->q_msg == NULL) {
            queue->q_msg = new_msg;
            new_msg->val = msg; // add msg to queue
        } else {
            QueueMessage *current = queue->q_msg;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_msg;
            new_msg->val = msg; // add msg to queue
        }
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

    // if there is alarm
    if (aq_alarms(queue) == 1) {
        *msg = queue->alarm;
        queue->alarm = NULL;
        return AQ_ALARM;
    }

    if (queue->q_msg != NULL) {
        *msg = queue->q_msg->val;
        if (queue->q_msg->next != NULL) {
            QueueMessage *next = queue->q_msg->next;
            free(queue->q_msg);
            queue->q_msg = next;
        } else {
            free(queue->q_msg);
            queue->q_msg = NULL; // For removing last element of queue
        }
        return AQ_NORMAL;
    }

    return AQ_NO_MSG;
}

int aq_size(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    Queue *queue = aq;
    int size = 0;
    
    QueueMessage *current = queue->q_msg;
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
