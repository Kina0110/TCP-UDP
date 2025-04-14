#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(const char *message);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Server_IP> <Port> <TCP/UDP>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int is_udp = (strcmp(argv[3], "UDP") == 0);

    int sock;
    struct sockaddr_in server_addr;
    char message[BUF_SIZE];
    int str_len;

    if (is_udp) {
        sock = socket(PF_INET, SOCK_DGRAM, 0);
    } else {
        sock = socket(PF_INET, SOCK_STREAM, 0);
    }

    if (sock == -1) {
        error_handling("Socket creation failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    if (!is_udp) {
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
            error_handling("Connection failed");
        }
    }

    while (1) {
        printf("Enter message (q to quit): ");
        fgets(message, BUF_SIZE, stdin);

        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n")) {
            break;
        }

        if (is_udp) {
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            str_len = recvfrom(sock, message, BUF_SIZE, 0, NULL, NULL);
        } else {
            write(sock, message, strlen(message));
            str_len = read(sock, message, BUF_SIZE - 1);
        }

        if (str_len == -1) {
            error_handling("Read failed");
        }

        message[str_len] = '\0';
        printf("Received: %s\n", message);
    }

    close(sock);
    return 0;
}

void error_handling(const char *message) {
    perror(message);
    exit(1);
}
