#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
# define NUM_THREAD 4

struct data{
    int thread_num;
    int start;
    int end;
};


void* sum(void* inputs){
    struct data* struct_input = (struct data*) inputs;

    int thread_total = 0;
    for (int i = struct_input->start; i <= struct_input->end; ++i){
        thread_total += i;
    }
    
    printf("The sum for thread %d is: %d\n", (int) struct_input->thread_num, thread_total);
    return ((void*) thread_total);
}

int main(){
    
    pthread_t tid1, tid2, tid3, tid4;

    struct data *thread1_data = malloc(sizeof(struct data));
    struct data *thread2_data = malloc(sizeof(struct data));
    struct data *thread3_data = malloc(sizeof(struct data));
    struct data *thread4_data = malloc(sizeof(struct data));
    thread1_data->thread_num = 1;
    thread1_data->start = 0;
    thread1_data->end = 24;
    thread2_data->thread_num = 2;
    thread2_data->start = 25;
    thread2_data->end = 49;
    thread3_data->thread_num = 3;
    thread3_data->start = 50;
    thread3_data->end = 74;
    thread4_data->thread_num = 4;
    thread4_data->start = 75;
    thread4_data->end = 100;
    int total = 0;
    int* returned_sum;

    

   if(pthread_create(&tid1, NULL, sum, (void*) thread1_data) != 0){
       perror("Error.\n");
       exit(1);
   }
   if(pthread_create(&tid2, NULL, sum, (void*) thread2_data) != 0){
       perror("Error.\n");
       exit(1);
   }
   if(pthread_create(&tid3, NULL, sum, (void*) thread3_data) != 0){
       perror("Error.\n");
       exit(1);
   }
   if(pthread_create(&tid4, NULL, sum, (void*) thread4_data) != 0){
       perror("Error.\n");
       exit(1);
   }


    pthread_join(tid1, &returned_sum);
    printf("the returned sum is: %d\n", (int) returned_sum);
    total += (int) returned_sum;
    pthread_join(tid2, &returned_sum);
    printf("the returned sum is: %d\n", (int) returned_sum);
    total += (int) returned_sum;
    pthread_join(tid3, &returned_sum);
    printf("the returned sum is: %d\n", (int) returned_sum);
    total += (int) returned_sum;
    pthread_join(tid4, &returned_sum);
    printf("the returned sum is: %d\n", (int) returned_sum);
    total += (int) returned_sum;

    //pthread_join(tid, (void**)&returned_sum);
    printf("the returned sum is: %d\n", (int) returned_sum);

    printf("The total is: %d\n", total);

    return 0;
}



