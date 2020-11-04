#include "my_vm.h"
#include "math.h"
#include <stdio.h>

int init = 0;

int* vBitMap;
int* pBitMap;
char* memory;
long numVP;
long numPP;
double offBits;

int PG_DIR_MASK = 255;
int PG_TBL_MASK = 255;

pde_t* pgdir;
/*
Function responsible for allocating and setting your physical memory 
*/
void SetPhysicalMem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    printf("INITIALIZING LIBRARY\n");
    memory = malloc(sizeof(char)* MEMSIZE);

    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

    offBits = log2(PGSIZE); //offset bits based on pagesize
    numVP = MAX_MEMSIZE / PGSIZE; //# of virtual pages
    numPP = MEMSIZE / PGSIZE; //# of physical pages

    vBitMap = (int*) malloc(sizeof(int) * numVP); //initialize virtual bitmap
    for(int i = 0; i < numVP; i++){
        vBitMap[i] = 0;
    }

    pBitMap = (int*) malloc(sizeof(int) * numPP); //initialize physical bitmap
    for(int i = 0; i < numVP; i++){
        pBitMap[i] = 0;
    }

    //initialize pg dir
    //how many entries in pgdir? = 2^pgdrBits
    int pgdrBits = (32-offBits)/2; //10
    pgdir = malloc(sizeof(pgdir)*pow(2,pgdrBits));

    int pgtblBits = 32-pgdrBits-offBits; //10
    PG_DIR_MASK = PG_DIR_MASK << (32-pgdrBits);
    PG_TBL_MASK = PG_TBL_MASK << (32-pgtblBits);
    PG_TBL_MASK = PG_TBL_MASK >> pgdrBits;
}

int
add_TLB(void *va, void *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}

pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */

}


void print_TLB_missrate(){
  double miss_rate = 0;


  fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}


/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t * Translate(pde_t *pgdir, void *va) {
    //HINT: Get the Page directory index (1st level) Then get the
    //2nd-level-page table index using the virtual address.  Using the page
    //directory index and page table index get the physical address

    int pgDirIndex = ((int)va & PG_DIR_MASK); //HOW TO WORK WITH VOID* address
    if(pgdir[pgDirIndex] == NULL){
        return NULL;
    }else{
        pte_t* currTable = pgdir[pgDirIndex];
        int pgTblIndex = ((int)va & PG_TBL_MASK);
        pte_t* currAddr = currTable[pgTblIndex];
        //ADD OFFSET?
        return currAddr;
    }



    //If translation not successfull
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
PageMap(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to Translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    //Use virtual address bitmap to find the next free page
    //go through virtual bitmap to find pages
   int found = 0;
   int count = 0;
   int first = 0;
    printf("Looking for pages, pages needed: %d, numVP: %d\n", num_pages, numVP);
   for(int i = 0; i < numVP; i++){
        if(vBitMap[i] == 0){
            if(count == 0) first = i;
            count++;
            if(count == num_pages){
                //your done
                printf("pages found in vm\n");
                found = 1;
                break;
            }
        }else{
            count = 0;
        }
    }

    if(found = 1){
        //need to return the first va
        //first is ur vpn
        void* virtAddr = (first+1)*PGSIZE;
        return virtAddr;
    }
    else{
        return NULL;
    }
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *myalloc(unsigned int num_bytes) {

    //HINT: If the physical memory is not yet initialized, then allocate and initialize.
    if(init == 0){
        SetPhysicalMem();
        init = 1;
    }

   /* HINT: If the page directory is not initialized, then initialize the
   page directory. Next, using get_next_avail(), check if there are free pages. If
   free pages are available, set the bitmaps and map a new page. Note, you will 
   have to mark which physical pages are used. */
    printf("bytes wanted %d", num_bytes);
   double pagesNeeded = ceil(num_bytes/PGSIZE)+1; //TODO: it keeps rounding down, fix later
   printf("pages needed %f\n", pagesNeeded);

   void* currVA = get_next_avail((int)pagesNeeded);
   if(currVA == NULL){
       printf("Not enough memory\n");
   }else{
       printf("vm found! at %p\n", currVA);
   }

   //MAP to physical memory

   

    return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void myfree(void *va, int size) {

    //Free the page table entries starting from this virtual address (va)
    // Also mark the pages free in the bitmap
    //Only free if the memory from "va" to va+size is valid
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void PutVal(void *va, void *val, int size) {

    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/

}


/*Given a virtual address, this function copies the contents of the page to val*/
void GetVal(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    "val" address. Assume you can access "val" directly by derefencing them.
    If you are implementing TLB,  always check first the presence of translation
    in TLB before proceeding forward */


}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void MatMult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
    matrix accessed. Similar to the code in test.c, you will use GetVal() to
    load each element and perform multiplication. Take a look at test.c! In addition to 
    getting the values from two matrices, you will perform multiplication and 
    store the result to the "answer array"*/

       
}
