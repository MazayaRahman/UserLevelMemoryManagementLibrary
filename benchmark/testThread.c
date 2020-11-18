#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../my_vm.h"

#define SIZE1 5
#define SIZE2 3

void * testThreadOne(void* arg){
    printf("Allocating three arrays of 400 bytes\n");
    void *a = myalloc(100*4);
    int old_a = (int)a;
    void *b = myalloc(100*4);
    void *c = myalloc(100*4);
    int x = 1;
    int y, z;
    int i =0, j=0;
    int address_a = 0, address_b = 0;
    int address_c = 0;

    printf("Addresses of the allocations: %x, %x, %x\n", (int)a, (int)b, (int)c);

    printf("Storing integers to generate a SIZE1xSIZE1 matrix\n");
    for (i = 0; i < SIZE1; i++) {
        for (j = 0; j < SIZE1; j++) {
            address_a = (unsigned int)a + ((i * SIZE1 * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE1 * sizeof(int))) + (j * sizeof(int));
            //printf("addr_a is %d\n", address_a);
            //printf("addr_b is %d\n", address_b);
            PutVal((void *)address_a, &x, sizeof(int));
            PutVal((void *)address_b, &x, sizeof(int));
            printf("%d ", x);
            
        }
        printf("\n");
    } 

    printf("Fetching matrix elements stored in the arrays\n");

    for (i = 0; i < SIZE1; i++) {
        for (j = 0; j < SIZE1; j++) {
            address_a = (unsigned int)a + ((i * SIZE1 * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE1 * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_a, &y, sizeof(int));
            GetVal( (void *)address_b, &z, sizeof(int));
            printf("%d ", y);
        }
        printf("\n");
    } 

    printf("Performing matrix multiplication with itself!\n");
    MatMult(a, b, SIZE1, c);


    printf("\n");
    printf("printing ans from test.c\n");

    for (i = 0; i < SIZE1; i++) {
        for (j = 0; j < SIZE1; j++) {
            address_c = (unsigned int)c + ((i * SIZE1 * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_c, &y, sizeof(int));
            printf("%d ", y);
        }
        printf("\n");
    }
    printf("Freeing the allocations!\n");
    myfree(a, 100*4);
    myfree(b, 100*4);
    myfree(c, 100*4);
}

void * testThreadTwo(void* arg){
    printf("Allocating three arrays of 400 bytes\n");
    void *a = myalloc(100*4);
    int old_a = (int)a;
    void *b = myalloc(100*4);
    void *c = myalloc(100*4);
    int x = 1;
    int y, z;
    int i =0, j=0;
    int address_a = 0, address_b = 0;
    int address_c = 0;

    printf("Addresses of the allocations: %x, %x, %x\n", (int)a, (int)b, (int)c);

    printf("Storing integers to generate a SIZE2xSIZE2 matrix\n");
    for (i = 0; i < SIZE2; i++) {
        for (j = 0; j < SIZE2; j++) {
            address_a = (unsigned int)a + ((i * SIZE2 * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE2 * sizeof(int))) + (j * sizeof(int));
            //printf("addr_a is %d\n", address_a);
            //printf("addr_b is %d\n", address_b);
            PutVal((void *)address_a, &x, sizeof(int));
            PutVal((void *)address_b, &x, sizeof(int));
            printf("%d ", x);
            
        }
        printf("\n");
    } 

    printf("Fetching matrix elements stored in the arrays\n");

    for (i = 0; i < SIZE2; i++) {
        for (j = 0; j < SIZE2; j++) {
            address_a = (unsigned int)a + ((i * SIZE2 * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE2 * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_a, &y, sizeof(int));
            GetVal( (void *)address_b, &z, sizeof(int));
            printf("%d ", y);
        }
        printf("\n");
    } 

    printf("Performing matrix multiplication with itself!\n");
    MatMult(a, b, SIZE2, c);


    printf("\n");
    printf("printing ans from test.c\n");

    for (i = 0; i < SIZE2; i++) {
        for (j = 0; j < SIZE2; j++) {
            address_c = (unsigned int)c + ((i * SIZE2 * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_c, &y, sizeof(int));
            printf("%d ", y);
        }
        printf("\n");
    }
    printf("Freeing the allocations!\n");
    myfree(a, 100*4);
    myfree(b, 100*4);
    myfree(c, 100*4);
}

int main() {

    pthread_t thread_one;
	pthread_t thread_two;

    pthread_create(&thread_one, NULL, &testThreadOne, NULL);	
	pthread_create(&thread_two, NULL, &testThreadTwo, NULL);

    pthread_join(thread_one, NULL);
    pthread_join(thread_two, NULL);

    return 1;
}