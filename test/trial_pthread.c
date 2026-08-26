#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex;
pthread_cond_t condition;

int event = 0; // event

void* sender(void* arg){
    //generate event and alart
    pthread_mutex_lock(&mutex);
    event = 1;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* receiver(void* arg){
    //   wait for receiving event
    pthread_mutex_lock(&mutex);
    while (event = 0) {
        pthread_cond_wait(&condition, &mutex);
    }
    // process after received event
    printf("Event Received!/n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t sender_thread, receiver_thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);

    //creating thread
    pthread_create(&sender_thread, NULL, sender, NULL);
    pthread_create(&receiver_thread, NULL, receiver, NULL);

    //waiting thread finish
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);

    return (0);

}