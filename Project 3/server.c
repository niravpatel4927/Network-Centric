#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define SIZE 1024

int main(int argc, char** argv){
    struct sockaddr_in address;
    int serverfd, client;
    int c_read;
    int addrlen = sizeof(address);
    char buffer[SIZE] = {0};
    int pid;

    int port = atoi(argv[1]);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ // socket creation
        perror("Socket Creation Failed,\n");
        exit(1);
    }

    if (bind(serverfd, (struct sockaddr *)&address, addrlen) < 0){ // bind to port
        perror("Bind failed.\n");
        exit(1);
    }
    printf("Waiting for Client...\n");
    
    if (listen(serverfd, SIZE) < 0){ // block while waiting for client to connect
        perror("Listen Failed.\n");
        exit(1);
    }
    char response[]="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<HTML><BODY>I wish I grew 3 more inches.</BODY></HTML>\r\n";
    while(1){
        if ((client = accept(serverfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
            perror("New Socket Creation Failed.\n");
            exit(1);
        } 
        if ((pid = fork()) == 0){
            close(serverfd);
            c_read = read(client, buffer, sizeof(buffer));
            printf("The message is: %s.\n", buffer);
            buffer[c_read] = '\0';
            write(client, response, strlen(response));
            printf("Response msg sent\n");
            exit(0);
        }
        close(client); // parent closes connected socket
        printf("after close socket\n");
        
        
    }
    
}