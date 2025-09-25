/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include "aq.h"
#include "stdlib.h"

typedef struct {
    void *val;
    void *next;
} NormalQueueMessage;

typedef struct {
    void *alarm;
    NormalQueueMessage *q_msg;
} Queue;

AlarmQueue aq_create() {
    Queue *q = malloc(sizeof(Queue));
    return q;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    return AQ_NOT_IMPL;
}

int aq_recv(AlarmQueue aq, void * *msg) {
    return AQ_NOT_IMPL;
}

int aq_size(AlarmQueue aq) {
    return 0;
}

int aq_alarms(AlarmQueue aq) {
    return 0;
}
