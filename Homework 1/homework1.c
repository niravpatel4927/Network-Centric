#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

int main(){
    // create a pipe for communication
    int p[2];
    int sum = 0;
    pid_t pid;
    int status;

    if (pipe(p) < 0){
        perror("Failed to create pipe");
        exit(1);
    }

    if ((pid = fork()) < 0){
        perror("Failed to create process");
        exit(1);
    }

    if (pid == 0){ // child process
        for (int i = 1; i < 51; ++i){
            sum = sum + i;
        }

        write(p[1], &sum, sizeof(int));
    } else {
        waitpid(-1, &status, 0); // wait for the child to end and send the sum to the parent
        read(p[0], &sum, sizeof(int));
        close(p[1]);
        for (int i = 51; i <= 100; ++i){
            sum = sum + i;
        }
        printf("The total is: %d", sum);

    }
    return 0;
}