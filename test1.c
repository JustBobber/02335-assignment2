#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>

#include "aq.h"
#include "aux.h"

static AlarmQueue q;

void *alarm_producer(void *arg) {
    put_alarm(q, 0);
    put_alarm(q, 1);
    put_normal(q, 99);
    return 0;
}

void *normal_producer(void *arg) {
    put_normal(q, 0);
    put_normal(q, 1);
    put_normal(q, 2);
    return 0;
}

void *consumer(void *arg) {
    msleep(50);
    get(q);
    get(q);
    get(q);
    get(q);
    get(q);
    assert(get(q) == 99);
    return 0;
}

int main(int argc, char **argv) {
    q = aq_create();

    if (q == NULL) {
        printf("Alarm queue could not be created\n");
        exit(1);
    }

    pthread_t t1;
    pthread_t t2;
    pthread_t t3;

    void *res1;
    void *res2;
    void *res3;

    printf("----------------\n");

    /* Fork threads */
    pthread_create(&t1, NULL, alarm_producer, NULL);
    pthread_create(&t2, NULL, normal_producer, NULL);
    pthread_create(&t3, NULL, consumer, NULL);

    /* Join with all threads */
    pthread_join(t1, &res1);
    pthread_join(t2, &res2);
    pthread_join(t3, &res3);

    printf("----------------\n");
    printf("Threads terminated with %ld, %ld, %ld\n", (uintptr_t) res1, (uintptr_t) res2, (uintptr_t) res3);

    print_sizes(q);

    return 0;
}