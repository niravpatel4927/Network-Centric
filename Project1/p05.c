#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/uio.h>   
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){

    // inputs are in the form main, string to be searched, and substrings to search for
    // check to see if enough arguments are provided
    // must have at least 3 inputs in order for code to work

    if (argc < 3) {
        perror("Invalid number of inputs");
        exit(-1);
    }


    // check to see if flag is present 
    int flag = 0; 

    // we can see how many substrings need to be found, because after the first two arguments, 
    // everything else will be a substring so we can get the total number of substrings to be searched
    int number_substring = 0;
    if ((strcmp("--systemcalls", argv[1])) == 0){
        flag = 1; 
        number_substring = argc - 3;
    } else {
        flag = 0;
        // flag isn't true, meaning no flag is present so there are only 2 initial arguments to ignore
        number_substring = argc-2; 
    }
    
    if (argc < 4 && flag){
        perror("Invalid number of inputs");
        exit(-1);
    }
    
    if (flag){  // if you want to implement this with systemcalls
        int fd = open(argv[2], 0, 0); // because O_RDONLY returned errors, that argument was replaced with 0
        if (fd < 0){ // check to see if file exists
            perror("Invalid file");
            exit(-1);
        }
        int count = 0; // initialize count

        for (int i = 0; i < number_substring; ++i){ // want to loop through all substrings
            char* target_substring = argv[i + 3]; // i + 3 because the first substring starts at the 4th position

            for (int j = 0; j < strlen(target_substring); ++j){ // make all the characters in the substring lowercase
               target_substring[j] = tolower(target_substring[j]);
            }

            lseek(fd, 0, SEEK_SET); // reset file pointer to beginning of the file

            char *target_word = (char *) calloc(strlen(target_substring) + 1, sizeof(char)); // allocate memory for the target word
            // the target_word will get changed each iteration, but each target_word should only be the same length as the substring

            for (int j = 0; ; ++j){
                int sz = read(fd, target_word, strlen(target_substring)); // load length of substring into target_word
                target_substring[sz] = '\0'; // add cushion at the end
                if (strlen(target_substring) != strlen(target_word)){
                    break;
                } // if we reach the end of the target_word, and there are no more characters so that the length of the target_word 
                 // is not the same as the substring, then break

                for (int j = 0; j < strlen(target_word); ++j){ // lower target_word lowercase
                    target_word[j] = tolower(target_word[j]);
                }
                if (strcmp(target_substring, target_word) == 0){ // they are equal to each other
                    count++;
                } 
                lseek(fd, j + 1, SEEK_SET); // reset file pointer to correct position
            }
            
            printf("%d\n", count);
            count = 0; // reset count for increment
            free(target_word);
            
            
           
        }
        
    } else { // if no flag has been given
         // open file where string to be searched is located
        FILE* fp = fopen(argv[1], "r+");
        // if file doesn't exist, then you can't search for substrings in NULL
        if (fp == NULL) {
            printf("Error: %s \n", strerror);
            exit(-1);
        }
        
        // initialize
        int count = 0;
        size_t size = 0;
        size_t len = 0;
        char *line = NULL;


        // initialize an array for total number of substrings to be found
        char* buffer[number_substring];

        // loop through total number of substrings found
        for (int i = 0; i < number_substring; ++i){
            // store substring within a variable so original input isn't changed
            char* target_substring = argv[i + 2];
            for (int j = 0; j < strlen(target_substring); ++j){
                // make each substring lowercase in order to make them case-insensitive
                target_substring[j] = tolower(target_substring[j]);
            }
            // store the lowercased substrings into the buffer
            buffer[i] = target_substring;
        }

        
        for (int i = 0; i < number_substring; ++i){
            while ((len = getline(&line, &size, fp)) != -1){
                char* substring = buffer[i];
                //printf("%s\n", substring);
                const char* search_word = line;
                //printf("%s\n", search_word);
                //fflush(stdout);
                while((search_word = strstr(search_word, substring)) != NULL){
                    count++;
                    search_word++;
                    //printf("%s", search_word);
                    //fflush(stdout);
                }
            }
            printf("%d\n", count);
            count = 0;
        }
        free(line);
        fclose(fp);
        return(0);
    }
   
    
}


