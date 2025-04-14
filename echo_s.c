#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <netinet/in.h>

#define LOG_SERVER_PORT 9999
#define BUF_SIZE 1024

void handle_client(int client_sock, struct sockaddr_in client_addr, int is_udp, int log_sock);
void send_log(const char *client_ip, const char *message, int log_sock);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int tcp_sock, udp_sock, log_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUF_SIZE];

    // Create TCP socket
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) {
        perror("TCP socket creation failed");
        exit(1);
    }

    // Create UDP socket
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("UDP socket creation failed");
        close(tcp_sock);
        exit(1);
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind TCP and UDP sockets
    if (bind(tcp_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ||
        bind(udp_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(tcp_sock);
        close(udp_sock);
        exit(1);
    }

    // Listen on TCP socket
    if (listen(tcp_sock, 5) < 0) {
        perror("Listen failed");
        close(tcp_sock);
        close(udp_sock);
        exit(1);
    }

    // Setup log server connection
    log_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (log_sock < 0) {
        perror("Log server socket creation failed");
        close(tcp_sock);
        close(udp_sock);
        exit(1);
    }

    printf("Echo server running on port %d\n", port);

    fd_set read_fds;
    int max_fd = tcp_sock > udp_sock ? tcp_sock : udp_sock;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(tcp_sock, &read_fds);
        FD_SET(udp_sock, &read_fds);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(tcp_sock, &read_fds)) {
                client_len = sizeof(client_addr);
                int client_sock = accept(tcp_sock, (struct sockaddr *)&client_addr, &client_len);
                if (client_sock < 0) {
                    perror("TCP client accept failed");
                    continue;
                }
                if (fork() == 0) {
                    close(tcp_sock);
                    handle_client(client_sock, client_addr, 0, log_sock);
                    exit(0);
                }
                close(client_sock);
            }

            if (FD_ISSET(udp_sock, &read_fds)) {
                client_len = sizeof(client_addr);
                int recv_len = recvfrom(udp_sock, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
                if (recv_len > 0) {
                    buffer[recv_len] = '\0';
                    printf("UDP: Received '%s' from %s\n", buffer, inet_ntoa(client_addr.sin_addr));
                    sendto(udp_sock, buffer, recv_len, 0, (struct sockaddr *)&client_addr, client_len);
                    send_log(inet_ntoa(client_addr.sin_addr), buffer, log_sock);
                }
            }
        }
    }

    close(tcp_sock);
    close(udp_sock);
    close(log_sock);
    return 0;
}

void handle_client(int client_sock, struct sockaddr_in client_addr, int is_udp, int log_sock) {
    char buffer[BUF_SIZE];
    int recv_len;
    while ((recv_len = recv(client_sock, buffer, BUF_SIZE, 0)) > 0) {
        buffer[recv_len] = '\0';
        printf("TCP: Received '%s' from %s\n", buffer, inet_ntoa(client_addr.sin_addr));
        send(client_sock, buffer, recv_len, 0);
        send_log(inet_ntoa(client_addr.sin_addr), buffer, log_sock);
    }
    close(client_sock);
}

void send_log(const char *client_ip, const char *message, int log_sock) {
    struct sockaddr_in log_server_addr;
    memset(&log_server_addr, 0, sizeof(log_server_addr));
    log_server_addr.sin_family = AF_INET;
    log_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    log_server_addr.sin_port = htons(LOG_SERVER_PORT);

    char log_message[BUF_SIZE];
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    strftime(log_message, BUF_SIZE, "%Y-%m-%d %H:%M:%S", time_info);
    snprintf(log_message + strlen(log_message), BUF_SIZE - strlen(log_message), " '%s' was received from %s", message, client_ip);

    sendto(log_sock, log_message, strlen(log_message), 0, (struct sockaddr *)&log_server_addr, sizeof(log_server_addr));
}
