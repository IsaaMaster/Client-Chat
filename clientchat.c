#include <stdio.h>  // Preprocessor directive
#include <string.h>
#include <sys/socket.h>  // Preprocessor directive
#include <sys/types.h>  // Preprocessor directive
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation failed\n");
        return 1;
    } else {
        printf("Socket created successfully\n");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(49153);
    server_addr.sin_addr.s_addr = inet_addr("10.115.12.240"); 
   
    printf("Connecting to %s:%d...\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));



    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection to server failed\n");
        return 1;
    } else {
        printf("Connected to server successfully\n");
    }

    //receive the welcome message from the server
    char recv_message[2048] = {0}; 
    recv(sock, recv_message, sizeof(recv_message) - 1, 0);
    printf("1: %s", recv_message);
    memset(recv_message, 0, sizeof(recv_message));

    // Send the username to the server
    printf("Enter your your username: ");
    char user[20];
    scanf("%s", user);
    
    char send_message[2048];
    snprintf(send_message, sizeof(send_message), "%s\n", user);
    send(sock, send_message, strlen(send_message), 0);
    memset(send_message, 0, sizeof(send_message));

    usleep(200000);
    /*
    char message[1024] = {0};
    printf("Enter your message: ");
    scanf("%s", message);
    snprintf(send_message, sizeof(send_message), "%s\n", message);
    send(sock, send_message, sizeof(send_message), 0); 
    memset(send_message, 0, sizeof(send_message));

    int bytes_received;
    while (send(sock, send_message, sizeof(send_message), 0) > 0) {
        printf("Enter your message: ");
        scanf("%s", message);
        snprintf(send_message, sizeof(send_message), "%s\n", message);
        send(sock, send_message, sizeof(send_message), 0);
        memset(send_message, 0, sizeof(send_message));

        while ((bytes_received = recv(sock, recv_message, sizeof(recv_message) - 1, 0)) > 0) {
            recv_message[bytes_received] = '\0';  // Ensure null termination
            printf("%s", recv_message);
        }
    }
    */

    //struct timeval timeout;
    //timeout.tv_sec = 3;
    //timeout.tv_usec = 0;
    //setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));


   // int bytes_received;
    while(1){
        // Read all available messages
       // while ((bytes_received = recv(sock, recv_message, sizeof(recv_message) - 1, 0)) > 0) {
        //    recv_message[bytes_received] = '\0'; // Null-terminate the string
        //    printf("%s", recv_message); // Print all received messages at once
        //}
               
        recv(sock, recv_message, sizeof(recv_message) - 1, 0);
        printf("> %s\n", recv_message);
        fflush(stdout);
        memset(recv_message, 0, sizeof(recv_message));

        char message[1024] = {0};
        printf("Enter your message> ");
        scanf(" %[^\n]", message);
        snprintf(send_message, sizeof(send_message), "%s\n", message);
        send(sock, send_message, strlen(send_message), 0);
        memset(send_message, 0, sizeof(send_message));
        
        usleep(100000);

       
    }

    
    return 0;  // Exit the program
}