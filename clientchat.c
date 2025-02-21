#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>


// Set the terminal to "raw mode", so that input is not buffered and we
// can read one character at a time. 
void set_raw_mode(int fd) {
    struct termios oldt, newt;
    tcgetattr(fd, &oldt);  // Get the current terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode (buffering) and echo
    newt.c_cc[VMIN] = 1;  // Minimum number of characters to read (1 character at a time)
    newt.c_cc[VTIME] = 0;  // No timeout for input
    tcsetattr(fd, TCSANOW, &newt);  // Apply the new settings
}

// Reset the terminal to normal mode. See comments above. 
void reset_terminal(int fd) {
    struct termios oldt;
    tcgetattr(fd, &oldt);
    oldt.c_lflag |= (ICANON | ECHO);  // Restore canonical mode (buffering) and echo
    tcsetattr(fd, TCSANOW, &oldt);
}

// Connects to the server at the specified IP address and port.
int connect_to_server(int sock, int port, char* ip) {
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
        return 0; 
    }
}

// Sends a message to the server. Clears the message buffer after sending.
int sendMessage(int sock, char* message) {
    send(sock, message, strlen(message), 0);
    memset(message, 0, sizeof(message));
    return 0;
}

// Adds a character to a string.
int addChar(char* message, char ch) {
    int len = strlen(message);
    if (len < sizeof(message) - 1) {
        message[len] = ch;
        message[len + 1] = '\0';  // Null-terminate the string
    }
    return 0;
}


int main() {
    char send_message[2048];
    char recv_message[2048] = {0}; 
    int PORT = 49153;
    char IP[] =  "10.115.12.240"; 
    
    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation failed\n");
        return 1;
    } else {
        printf("Socket created successfully\n");
    }

    connect_to_server(sock, PORT, IP);

    //receive the welcome message from the server
    recv(sock, recv_message, sizeof(recv_message) - 1, 0);
    printf("%s", recv_message);
    memset(recv_message, 0, sizeof(recv_message));

    // Send the username to the server
    printf("Enter your your username: ");
    char user[20];
    scanf("%s", user);
    snprintf(send_message, sizeof(send_message), "%s\n", user);
    sendMessage(sock, send_message);

    // Set terminal to raw mode
    set_raw_mode(STDIN_FILENO);
    
    fd_set read_fds;

    while (1) {  
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);  

        int max_fd = sock > STDIN_FILENO ? sock : STDIN_FILENO;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        // Check for messages from the server
        if (FD_ISSET(sock, &read_fds)) {
            int bytes_received = recv(sock, recv_message, sizeof(recv_message) - 1, 0);
            if (bytes_received > 0) {
                recv_message[bytes_received] = '\0';
                // Only clear the previous line if the previous message was not the username of the user. 
                // This is to prevent the username from being cleared when the message is only partially received.
                if(recv_message[strlen(recv_message) - 1] == '>'){
                    printf("\r\033[K");
                }
                printf("%s", recv_message);
                // Prints the user's last send_message out if the user did not get a chance to 
                // send it before a message was received from the server.
                if(recv_message[strlen(recv_message) - 1] != '>'){
                    printf("%s", send_message);
                    fflush(stdout);
                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Handle user input
            char ch;
            int num_read = read(STDIN_FILENO, &ch, 1);  // Read one character at a time

            if (num_read > 0) {
                // When the enter key is pressed, send the message to the server
                if (ch == '\n') {
                    addChar(send_message, '\n');
                    sendMessage(sock, send_message);
                // When the backspace key is pressed, remove the last character from the message
                } else if (ch == 127) {
                    // Handle backspace
                    int len = strlen(send_message);
                    if (len > 0) {
                       send_message[len - 1] = '\0';  // Remove last character
                       printf("\b \b");  // Erase the last character from the screen
                       fflush(stdout);  // Ensure the character is erased immediately
                    }
                // Otherwise add the character to the message
                } else {
                    addChar(send_message, ch);
                    // Display the character on the screen
                    printf("%c", ch);
                    fflush(stdout);  // Ensure the character is displayed immediately
                }
            }       
        }
    }

    reset_terminal(STDIN_FILENO); // Reset terminal to normal mode. 
    return 0;  
}