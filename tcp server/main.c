#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT     8080
#define MSG_SIZE 256

// ── framing ──────────────────────────────────────────

int send_msg(int fd, const char *msg, int len) {
    uint32_t net_len = htonl(len);
    write(fd, &net_len, 4);
    write(fd, msg, len);
    return 0;
}

int recv_msg(int fd, char *buf, int bufsize) {
    uint32_t net_len;
    if (read(fd, &net_len, 4) <= 0) return -1;

    int len = ntohl(net_len);
    if (len >= bufsize) return -1;

    int total = 0, n;
    while (total < len) {
        n = read(fd, buf + total, len - total);
        if (n <= 0) return -1;
        total += n;
    }
    buf[total] = '\0';
    return total;
}

// ── protocol ─────────────────────────────────────────

int handle_command(const char *msg, char *out) {
    char cmd[16], key[64], val[64];
    val[0] = '\0';

    sscanf(msg, "%15s %63s %63[^\n]", cmd, key, val);

    if (strcmp(cmd, "SET") == 0)
        return snprintf(out, MSG_SIZE, "OK — stored %s=%s", key, val);

    if (strcmp(cmd, "GET") == 0)
        return snprintf(out, MSG_SIZE, "VALUE for %s", key);

    return snprintf(out, MSG_SIZE, "ERR unknown command");
}

// ── client handler ────────────────────────────────────

void handle_client(int fd) {
    char msg[MSG_SIZE], resp[MSG_SIZE];

    while (1) {
        if (recv_msg(fd, msg, sizeof(msg)) <= 0) break;
        printf("fd=%d: %s\n", fd, msg);
        int len = handle_command(msg, resp);
        send_msg(fd, resp, len);
    }

    close(fd);
}

void *client_thread(void *arg) {
    int fd = *(int *)arg;
    free(arg);
    handle_client(fd);
    return NULL;
}

// ── accept loop ───────────────────────────────────────

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    bind(listen_fd,  (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 128);

    printf("server on port %d\n", PORT);

    while (1) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) continue;

        int *fd_ptr = malloc(sizeof(int));
        *fd_ptr = client_fd;

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, fd_ptr);
        pthread_detach(tid);
    }
}