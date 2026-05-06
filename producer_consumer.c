#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5

char buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
int count = 0;
int done = 0;

pthread_mutex_t mutex;
pthread_cond_t not_full;
pthread_cond_t not_empty;

void *producer(void *arg)
{
    FILE *fp = fopen("message.txt", "r");

    if (fp == NULL) {
        printf("ERROR: can't open message.txt!\n");
        exit(1);
    }

    int ch;

    while ((ch = fgetc(fp)) != EOF) {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[in] = (char)ch;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }

    fclose(fp);

    pthread_mutex_lock(&mutex);
    done = 1;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void *consumer(void *arg)
{
    char ch;

    while (1) {
        pthread_mutex_lock(&mutex);

        while (count == 0 && !done) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        if (count == 0 && done) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        ch = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        putchar(ch);
        fflush(stdout);
    }

    pthread_exit(NULL);
}

int main()
{
    pthread_t prod_thread, cons_thread;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}
