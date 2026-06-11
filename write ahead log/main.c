#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_PATH "wal.log"

typedef struct {
    FILE            *file;
    uint64_t         next_lsn;
    pthread_mutex_t  lock;
} wal_t;

wal_t *wal_open() {
    wal_t *wal    = malloc(sizeof(wal_t));
    wal->next_lsn = 1;
    wal->file     = fopen(LOG_PATH, "a");
    pthread_mutex_init(&wal->lock, NULL);
    return wal;
}

void wal_write(wal_t *wal, const char *op, const char *key, const char *val) {
    pthread_mutex_lock(&wal->lock);

    fprintf(wal->file, "%llu|%llu|%s|%s|%s\n",
            (unsigned long long)wal->next_lsn++,
            (unsigned long long)time(NULL),
            op, key, val ? val : "");

    fflush(wal->file);          // stdio buffer -> OS
    fsync(fileno(wal->file));   // OS page cache -> disk

    pthread_mutex_unlock(&wal->lock);
}

void wal_recover() {
    FILE *f = fopen(LOG_PATH, "r");
    if (!f) return;

    char line[256];
    printf("--- replaying WAL ---\n");

    //In an actual system, this would replay to a data structure
    while (fgets(line, sizeof(line), f))
        printf("  %s", line);
    printf("--- done ---\n");

    fclose(f);
}

void *worker(void *arg) {
    wal_t *wal = (wal_t *)arg;
    wal_write(wal, "PUT",    "vm-1", "running");
    wal_write(wal, "PUT",    "vm-2", "stopped");
    wal_write(wal, "DELETE", "vm-2", NULL);
    return NULL;
}

int main() {
    wal_t *wal = wal_open();

    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker, wal);
    pthread_create(&t2, NULL, worker, wal);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    fclose(wal->file);
    free(wal);

    printf("\n=== restart ===\n");
    wal_recover();
    return 0;
}