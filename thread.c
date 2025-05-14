#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Constants
#define TOTAL_LINES 1000 
#define THREAD_COUNT 4
#define LINES_PER_THREAD (TOTAL_LINES / THREAD_COUNT)

char* logLines[TOTAL_LINES];

typedef struct {
    int getCount;
    int postCount;
    int status200;
    int status404;
    int status500;
} Stats;

Stats totalStats = {0};
pthread_mutex_t lock;

// Funtion each thread will execute
void* analyzeLogs(void* arg) {
    int index = *(int*)arg;
    int threadNum = index / LINES_PER_THREAD;
    free(arg);

    printf("Thread %d starting. Handling lines %d to %d\n", threadNum, index, index + LINES_PER_THREAD - 1);

    Stats local = {0};

    for (int i = index; i < index + LINES_PER_THREAD; i++) {
        if ((i - index) % 50 == 0) {
            printf("Thread %d processed line %d\n", threadNum, i);
        }        
        if (strstr(logLines[i], "GET")) local.getCount++;
        if (strstr(logLines[i], "POST")) local.postCount++;
        if (strstr(logLines[i], "200")) local.status200++;
        if (strstr(logLines[i], "404")) local.status404++;
        if (strstr(logLines[i], "500")) local.status500++;

        usleep(1000); 
    }

    printf("Thread %d: waiting for lock\n", threadNum);
    pthread_mutex_lock(&lock);
    printf("Thread %d: acquired lock\n", threadNum);

    totalStats.getCount += local.getCount;
    totalStats.postCount += local.postCount;
    totalStats.status200 += local.status200;
    totalStats.status404 += local.status404;
    totalStats.status500 += local.status500;

    pthread_mutex_unlock(&lock);

    printf("Thread %d finished.\n", threadNum);
    return NULL;
}

// Funtion for generating dummy logs
void generateFakeLogs() {
    const char* methods[] = {"GET", "POST"};
    const char* statuses[] = {"200", "404", "500"};

    for (int i = 0; i < TOTAL_LINES; i++) {
        logLines[i] = malloc(100);
        // Storing string in char buffer
        sprintf(logLines[i], "%s /index.html HTTP/1.1 %s",
                methods[rand() % 2], statuses[rand() % 3]);
    }
}

int main() {
    pthread_mutex_init(&lock, NULL);

    generateFakeLogs();

    pthread_t threads[THREAD_COUNT];

    // Creating thread
    for (int i = 0; i < THREAD_COUNT; i++) {
        int* startIndex = malloc(sizeof(int));
        *startIndex = i * LINES_PER_THREAD;
        pthread_create(&threads[i], NULL, analyzeLogs, startIndex);
    }

    // Waiting for each thread to complete
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final result:\n");
    printf("GET requests: %d\n", totalStats.getCount);
    printf("POST requests: %d\n", totalStats.postCount);
    printf("200 OK: %d\n", totalStats.status200);
    printf("404 Not Found: %d\n", totalStats.status404);
    printf("500 Server Error: %d\n", totalStats.status500);

    pthread_mutex_destroy(&lock);
    for (int i = 0; i < TOTAL_LINES; i++) {
        free(logLines[i]);
    }

    return 0;
}
