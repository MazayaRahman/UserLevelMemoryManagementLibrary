#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../my_vm.h"

#define SIZE 25

int main() {


    void *a = myalloc(100*4);
    int old_a = (int)a;
    void *b = myalloc(100*4);
    void *c = myalloc(100*4);
    int x = 1;
    int y, z;
    int i =0, j=0;
    int address_a = 0, address_b = 0;
    int address_c = 0;




    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            address_a = (unsigned int)a + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
           
            PutVal((void *)address_a, &x, sizeof(int));
            PutVal((void *)address_b, &x, sizeof(int));
	    //  printf("%d ", x);
            
        }
	// printf("\n");
    } 

    // printf("Fetching matrix elements stored in the arrays\n");

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            address_a = (unsigned int)a + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_a, &y, sizeof(int));
            GetVal( (void *)address_b, &z, sizeof(int));
            //printf("%d ", y);
        }
        //printf("\n");
    } 

   
       MatMult(a, b, SIZE, c);

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            address_c = (unsigned int)c + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_c, &y, sizeof(int));
	    //   printf("%d ", y);
        }
	// printf("\n");
    }

    myfree(a, 100*4);
    myfree(b, 100*4);
    myfree(c, 100*4);
    
    print_TLB_missrate();
    /*
    a = myalloc(100*4);
    if ((int)a == old_a)
        printf("free function works\n");
    else
        printf("free function does not work\n");
    */
    // print_TLB_missrate();

    return 0;
}
