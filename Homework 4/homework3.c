#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <unistd.h> 
#include <math.h> 
#include <string.h> 
#define NUM_THREADS 5 
int x=0; 
int sum=0; 
int exponent=3; 
pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
/*Thread Routine*/ 
void * thread_function() { 
    
    int counter=0; 
    int result=0; 
    
    for(; (!pthread_mutex_lock(&thread_mutex) && x<1000000); x++, pthread_mutex_unlock(&thread_mutex)) { 
        result = sqrt(x); 
        result*= rand(); 

        result = pow(result, exponent); 
        sum += result; 
        counter++;  

    } 
    
    pthread_mutex_unlock(&thread_mutex);
    printf("This thread computed %u iterations and\n", counter); 
    printf("the last results was %u.\n\n", result); 
        
    return NULL; 
} 
 
 /*main routine*/ 
int main() {                                
    pthread_t tid[NUM_THREADS]; 
    int index; 
    for(index=0; index<NUM_THREADS; index++){ 
        pthread_create(&tid[index], NULL, thread_function, NULL);
    }
    for(index=0; index<NUM_THREADS; index++){  
        pthread_join(tid[index], NULL); 
    }
    printf("Sum %u\n", sum); 
    return 0;       
} 
