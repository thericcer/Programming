#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 40
#define LOOP_COUNT 1000000000

void * long_loop(void * thread) {
    long i = 0;
    int thread_num = *((int *)thread);

    printf("Thread %d Starting.\n",  thread_num);

    for (i = 0; i < LOOP_COUNT; i++);
    for (i = 0; i < LOOP_COUNT; i++);
    for (i = 0; i < LOOP_COUNT; i++);
    for (i = 0; i < LOOP_COUNT; i++);

    printf("Thread %d Exiting.\n", thread_num);
    pthread_exit(NULL);
}

int main (int argc, char ** argv) {

    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr[NUM_THREADS];
    struct sched_param param[NUM_THREADS];

    int thread_num[NUM_THREADS];


    int rc;
    int i;

    for (i=0; i < NUM_THREADS; i++) {
        printf("Launching thread %d\n", i);

        param[i].sched_priority = NUM_THREADS - i;
        pthread_attr_init(&attr[i]);
        pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
        pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedparam(&attr[i], &param[i]);


        thread_num[i] = i;

        rc = pthread_create(&threads[i], &attr[i], long_loop, (void *)&thread_num[i]);
        if (rc) {
            printf("Error Launching thread %d!\n", i);
        }

    }

    pthread_exit(NULL);
}
