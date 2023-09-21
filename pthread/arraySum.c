#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<string.h>
#include<pthread.h>
#include<time.h>


#define THREAD_NUM 10
#define endl printf("\n")

// Global Space
long long SIZE = 0;
long long EXE_SIZE = 0;
long long resSum = 0;
int size = 0;
pthread_mutex_t sum_update;
pthread_mutex_t entry;



// Structure of Argument Passing
typedef struct threadArg{
    void* arg_ptr[10];
}threadArg;


void udpateGlobal_var(int arrSize){
    SIZE = arrSize;
    EXE_SIZE = arrSize/THREAD_NUM;
    return;
}

// Function to convert number character to number from a file
int str_to_i(FILE* fptr){
    char ch = fgetc(fptr);
    int sum = 0;
    while(1){
        sum += ch - '0';
        fscanf(fptr,"%c",&ch);
        if(ch == ',' || ch == EOF)break;
        else sum*=10;
    }

    return sum;
}


// Reading File and assigning integer data to arr
int* readFile(FILE* fptr){

    int temp = str_to_i(fptr);
    int* arr = (int*)calloc(sizeof(int),temp);
    
    udpateGlobal_var(temp);

    for(int i = 0; i < temp; ++i){
        *(arr + i) = str_to_i(fptr);
    }

    return arr;
}


// Function to print array
void printArray(int* arr,int size){
    for(int i = 0; i < size; ++i){
        printf("%d ",arr[i]);
    }
    printf("\n");
    return;
}

// Update global variable shared variable sum
void updateSum(int x){
    pthread_mutex_lock(&sum_update);
    resSum += x;
    pthread_mutex_unlock(&sum_update);
    return;
}

// Thread function to get sum
void *arrSum(void* args){

    // Temp variable to store sum 
    int sum = 0;

    // Get Array Header
    threadArg* arg = (threadArg*)args;
    int* arr = (int*)arg->arg_ptr[0];  

    // Update Size
    pthread_mutex_lock(&entry);
    arr += size;
    size += EXE_SIZE;
    pthread_mutex_unlock(&entry);

    for(int i = 0; i < EXE_SIZE; ++i){
        sum += (*(arr + i));
    }

    updateSum(sum);
}

void sum_of_array(int* arr){
    long long sum1 = 0;
    for(int i = 0; i < SIZE; ++i){
        sum1 += (*(arr + i));
    }
    printf("Sum = %lld",sum1);endl;
    return;
}

int main(){
    FILE* fptr = fopen("file.txt","r+");
    int* arr = readFile(fptr);

    
    // For Timing init
    clock_t start,end;

    // For without threading
    start = clock();
    sum_of_array(arr);
    end = clock();
    double duration = ((double)end - start)/CLOCKS_PER_SEC;
    printf("Time taken: %f",duration);endl;


    // For threading
    start = clock();

    // init locks
    pthread_mutex_init(&sum_update,NULL);
    pthread_mutex_init(&entry,NULL);

    // thread init
    pthread_t threads[THREAD_NUM];

    // Thread Argument
    threadArg* args = (threadArg*)malloc(sizeof(threadArg));
    args->arg_ptr[0] = (void*)arr;


    // Create threads
    for(int i = 0; i < THREAD_NUM; ++i){
        pthread_create(&threads[i],NULL,arrSum,args);
    }

    // Join threads
    for(int i = 0; i < THREAD_NUM; ++i){
        pthread_join(threads[i],NULL);
    }

    // Printing Sum
    printf("Sum = %lld",resSum);endl;

    // Locks destroy
    pthread_mutex_destroy(&sum_update);
    pthread_mutex_destroy(&entry);

    end = clock();
    duration = ((double)end - start)/CLOCKS_PER_SEC;

    printf("Time Taken: %f",duration);

    // Free Memory
    free(arr);

    // Close File
    fclose(fptr);


    return 0;
}