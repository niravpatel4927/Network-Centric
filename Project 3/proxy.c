/*
 * proxy.c - A Simple Sequential Web proxy
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 2
 * Student Name: Nirav Patel
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#define SIZE 1024
pthread_mutex_t FILE_MUTEX = PTHREAD_MUTEX_INITIALIZER;

/*
 * Function prototypes
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

struct data{ // created a struct for the thread to share data to
    int browser_socket;
    struct sockaddr_in address;
};

void *thread_handle(void *vargp){
    struct data* thread_data = (struct data*) vargp;
    char buffer[SIZE] = {0};
    int listenfd = read(thread_data->browser_socket, buffer, SIZE); // read from browser
    printf("The message from the client is:\n%s\n", buffer);
    printf("\n\n");

    char ip_address[SIZE]; // initialize array to store ip address
    int serverPort = 80; // initialize so if no port given, will default to 80
    int buffer_length = strlen(buffer);
    ip_get(buffer, ip_address, &serverPort, buffer_length);
    //printf("%s\n", ip_address);
    //printf("%d\n", serverPort);

    // create a proxy client
    struct sockaddr_in client_address;
    int clientfd;
    int clientread;
    int clientlen = sizeof(client_address);
    char buffer2[SIZE] = {0};

    memset(&client_address, '0', clientlen);

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(serverPort);

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ // socket creation
        perror("Socket Creation Error.\n");
        exit(1);
    }

    if (inet_pton(AF_INET, ip_address, &client_address.sin_addr) <= 0){
        printf("Invalid address.\n");
        exit(1);
    }

    if (connect(clientfd, (struct sockaddr *)&client_address, clientlen) < 0){
        printf("connect failed.\n");
        exit(1);
    }
    write(clientfd, buffer, buffer_length); // wrote message to  actual server
    printf("Browser response sent to server.\n");
    clientread = read(clientfd, buffer2, SIZE); // reading from actual server
    buffer2[clientread] = '\0';
    printf("The server sent back: %s.\n", buffer2);

    write(thread_data->browser_socket, buffer2, strlen(buffer2)); // write to browser client

    // write to log file 
    FILE* fp;
    pthread_mutex_lock(&FILE_MUTEX); // lock before open to ensure that not more than one thread is reading to it at the same time
    if ((fp = fopen("proxy.log", "w")) < 0){
        perror("Failed to open.\n");
        exit(1);
    }

    char log_string[SIZE];
    format_log_entry(log_string, &thread_data->address, ip_address, 0);
    int response_size = strlen(buffer2);
    fputs(log_string, fp);
    fputs(" ", fp);
    char temp_str[SIZE];
    sprintf(temp_str, "%d", response_size); // temp_string contains length of response
    fputs(temp_str, fp);
    fclose(fp);
    pthread_mutex_unlock(&FILE_MUTEX); // unlock file
    exit(0);
}
/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{

    /* Check arguments */
    if (argc != 3) {
	    fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	    exit(0);
    }

    // get port from input argument
    int port = atoi(argv[1]);   // get port number
    char* method = argv[2]; // thread or no_thread
    
    printf("The port is: %d\n", port);
    
    if (strcmp(method,"no_thread") == 0){
        processes(port);
    } else {
        threads(port);
    }
}


void threads(int port){
    // creating proxy server
    struct sockaddr_in address;
    int serverfd, listenfd;
    int addrlen = sizeof(address);
    pthread_t tid;

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

    while(1){
        printf("Waiting for a client to connect…\n");
        int* browser_socket = malloc(sizeof(int));
        
        if ((*browser_socket = accept(serverfd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        struct data *thread_data = malloc(sizeof(struct data)); // allocate space for thread_data
        thread_data->browser_socket = *browser_socket;
        thread_data->address = address;

        pthread_create(&tid, NULL, thread_handle, (void*) thread_data);
        pthread_detach(tid);
    }
}





void processes(int port){
     // creating proxy server
    struct sockaddr_in address;
    int serverfd, listenfd, browser_socket;
    int addrlen = sizeof(address);
    char buffer[SIZE] = {0};
    int pid;

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

    while(1){
        if ((browser_socket = accept(serverfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
            perror("New Socket Creation Failed.\n");
            exit(1);
        } 
        if ((pid = fork()) == 0){ // child
            close(serverfd);
            listenfd = read(browser_socket, buffer, SIZE); // read from browser
            printf("The message from the client is:\n%s\n", buffer);
            printf("\n\n");

            char ip_address[SIZE]; // initialize array to store ip address
            int serverPort = 80; // initialize so if no port given, will default to 80
            int buffer_length = strlen(buffer);
            ip_get(buffer, ip_address, &serverPort, buffer_length);
            printf("%s\n", ip_address);
            printf("%d\n", serverPort);

            // create a proxy client
            struct sockaddr_in client_address;
            int clientfd;
            int clientread;
            int clientlen = sizeof(client_address);
            char buffer2[SIZE] = {0};

            memset(&client_address, '0', clientlen);

            client_address.sin_family = AF_INET;
            client_address.sin_port = htons(serverPort);

            if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ // socket creation
                perror("Socket Creation Error.\n");
                exit(1);
            }

            if (inet_pton(AF_INET, ip_address, &client_address.sin_addr) <= 0){
                printf("Invalid address.\n");
                exit(1);
            }

            if (connect(clientfd, (struct sockaddr *)&client_address, clientlen) < 0){
                printf("connect failed.\n");
                exit(1);
            }
            write(clientfd, buffer, buffer_length); // wrote message to  actual server
            printf("Browser response sent to server.\n");
            clientread = read(clientfd, buffer2, SIZE); // reading from actual server
            buffer2[clientread] = '\0';
            printf("The server sent back: %s.\n", buffer2);

            write(browser_socket, buffer2, strlen(buffer2)); // write to browser client

            // write to log file 
            int fd;
            struct flock fl;
            fl.l_type = F_WRLCK;
            printf("Locking the file now.\n");
            if (fcntl(fd, F_SETLK, &fl) == -1) { //set non-blocking block
                if (errno == EACCES || errno == EAGAIN) {
                    printf("Already locked by another process\n");
                /* We can't get the lock at the moment */
                } else {
                    perror("Handle unexpected error");
                }
            } else {
                printf ("Lock was granted.\n");
            }
            if ((fd = open("proxy.log", O_RDWR)) < 0){
                perror("Failed to open.\n");
                exit(1);
            }

            char log_string[SIZE];
            format_log_entry(log_string, &address, ip_address, 0);
            int response_size = strlen(buffer2);
            write(fd, log_string, strlen(log_string));
            write(fd, " ", strlen(" "));
            char temp_str[SIZE];
            sprintf(temp_str, "%d", response_size); // temp_string contains length of response
            write(fd, temp_str, strlen(temp_str));
            close(fd);
            fl.l_type = F_UNLCK;
            printf("Unlocking file.\n");
            if(fcntl(fd, F_SETLK, &fl) == -1){
                perror("Handle unexpected error");
            } else {
                printf("Unlock was granted");
            }
            exit(0);
        }
        close(browser_socket);
        printf("After socket has been closed\n");
    }
}

void ip_get(char* buffer, char* ip_address, int* serverPort, int* buffer_length){
    
    int flag = 0;
    
    
    char* temp = buffer;
    // code to extract ip_address and port number
    for(int i = 0; i < buffer_length; ++i){
        char c = temp[0];
        ++temp;
        if (c != '\n'){ // continue until 2nd line
            continue;
        } else {
            temp += 6; // move temp pointer to start of ip_address
            c = temp[0]; // set character to beginning of next string
            flag = 1; 
        }
        if (flag){
            int j = 0;
            while(c != '\r'){
                ip_address[j] = c; // copy character into the ip_address
                ++j; //increment index of ip_address   
                ++i; // increment i to get next character
                ++temp; // increment pointer to next character
                c = temp[0];
                if (c == ':'){ // if the 
                    ++temp; // increment pointer to next character
                    c = temp[0];
                    *serverPort = atoi(temp); // convert the string of the port to int
                    break;
                }
                
            }
            ip_address[j] = '\0'; // put in terminating character
            break;
        }
    }

    printf("The ip address is: '%s'\n", ip_address);
    printf("The port number is: %d\n", *serverPort);

}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}


