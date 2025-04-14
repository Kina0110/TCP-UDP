#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 9999
#define BUF_SIZE 1024
#define LOG_FILE "echo.log"

void error_handling(const char *message);

int main() {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    char buffer[BUF_SIZE];
    FILE *log_file;

    // Create a UDP socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        error_handling("Socket creation failed");
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error_handling("Bind failed");
    }

    printf("Log server running on port %d\n", PORT);

    while (1) {
        client_addr_size = sizeof(client_addr);

        // Receive log message
        int str_len = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_addr_size);
        if (str_len > 0) {
            buffer[str_len] = '\0';  // Null-terminate the received string

            // Open the log file in append mode
            log_file = fopen(LOG_FILE, "a");
            if (!log_file) {
                perror("Error opening log file");
                continue;
            }

            // Get the timestamp
            time_t now = time(NULL);
            struct tm *time_info = localtime(&now);
            char timestamp[64];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time_info);

            // Write the log entry
            fprintf(log_file, "%s \"%s\" was received from %s\n", timestamp, buffer, inet_ntoa(client_addr.sin_addr));
            fflush(log_file);

            // Print the log entry to the console for debugging
            printf("Log entry: %s \"%s\" was received from %s\n", timestamp, buffer, inet_ntoa(client_addr.sin_addr));

            fclose(log_file);  // Close the log file
        }
    }

    close(sock);  // Close the socket
    return 0;
}

void error_handling(const char *message) {
    perror(message);
    exit(1);
}
