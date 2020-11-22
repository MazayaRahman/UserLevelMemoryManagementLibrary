#include "my_vm.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include<string.h>

int init = 0;

int* vBitMap;
int* pBitMap;
char* memory;
long numVP;
long numPP;
int offBits;
long ppCount;


int pgdrBits;
int pgtblBits;
int offset = 0XFFFFFFFF;

unsigned int PG_DIR_MASK = 0XFFFFFFFF;
unsigned int PG_TBL_MASK = 0XFFFFFFFF;

int totalRequests = 0;
int missRequests = 0;

pde_t* pgdir;
pthread_mutex_t lock;
/*
Function responsible for allocating and setting your physical memory 
*/
void SetPhysicalMem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating

    memory = malloc(sizeof(char)* MEMSIZE);


    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

    offBits = log2(PGSIZE); //offset bits based on pagesize
    offset = offset <<  offBits; //offset bits as an int
    offset = ~offset;

     
    numVP = MAX_MEMSIZE / PGSIZE; //# of virtual pages
    numPP = MEMSIZE / PGSIZE; //# of physical pages
    ppCount = numPP;

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
    pgdrBits = (32-offBits)/2; //10
    pgdir = malloc(sizeof(pde_t)*pow(2,pgdrBits));

    pgtblBits = 32-pgdrBits-offBits; //10
    PG_DIR_MASK = PG_DIR_MASK << (32-pgdrBits);


    unsigned int dirMaskFlip = ~PG_DIR_MASK;

    PG_TBL_MASK = PG_TBL_MASK << offBits;
    PG_TBL_MASK = dirMaskFlip & PG_TBL_MASK;


    //Initialize tlb structure
    tlb_store = malloc(sizeof(struct tlb));
    tlb_store->count = 0;

    //Initialize mutexes
    if(pthread_mutex_init(&lock, NULL) != 0){
      printf("mutex intialization failed\n");

    }


}

int
add_TLB(void *va, void *pa)
{
    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    if(tlb_store->count < TLB_SIZE){
        struct node* newEntry = malloc(sizeof(struct node));

        if(tlb_store->head == NULL){
            tlb_store->head = newEntry;
            tlb_store->tail = newEntry;
            newEntry->next = NULL;
        }else{
            newEntry->next = tlb_store->head;
            tlb_store->head = newEntry;
        }
	//lock and unlokc here
        newEntry->va = va;
        newEntry->pa = pa;

        tlb_store->count++;


    }
    else{
        struct node* ptr = tlb_store->head;
        while(ptr->next != tlb_store->tail){
            ptr = ptr->next;
        }

        tlb_store->tail->next = tlb_store->head;
        tlb_store->head = tlb_store->tail;
        tlb_store->tail = ptr;
        ptr->next = NULL;

        tlb_store->head->va = va;
        tlb_store->head->pa = pa;

    }
    return -1;
}

pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */

  void* vaToSearch =(void*)((int)va & (~offset));


    struct node* ptr = tlb_store->head;
    while(ptr != NULL){
        if(ptr->va == vaToSearch){
            //found
            totalRequests++;
            return ptr->pa;
        }
        ptr = ptr->next;
    }

    //not found, must add entry
    missRequests++;
    totalRequests++;
    return NULL;

}


void print_TLB_missrate(){
  double miss_rate = 0;
  //printf("total requests: %d\n", totalRequests);
  //printf("miss requests: %d\n", missRequests);
  miss_rate = (float)missRequests/(float)totalRequests;
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
    
    //FIRST LETS CHECK TLB
    if(tlb_store->count > 0){
        void* paAddr = check_TLB(va);
        if(paAddr != NULL){
	  void* offToAdd = (void*)((int)va & offset);
            paAddr = paAddr + (int)offToAdd; //adding offset bits to the current addr
            return paAddr;
        }
    }
    int pgDirIndex = ((int)va & PG_DIR_MASK); //HOW TO WORK WITH VOID* address
    if(pgdir[pgDirIndex] == NULL){
        return NULL;
    }else{
        pte_t* currTable = pgdir[pgDirIndex];
        int pgTblIndex = ((int)va & PG_TBL_MASK) >> offBits;
        void * currAddr = currTable[pgTblIndex];
        void* offToAdd = (void*)((int)va & offset);
        currAddr = currAddr + (int)offToAdd; //adding offset bits to the current addr
        //ADD TO TLB
        add_TLB(va, currAddr);
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

    int pgDirIndex = ((int)va & PG_DIR_MASK);
    if(pgdir[pgDirIndex] == NULL){
        //need to allocate a page table
        pgdir[pgDirIndex] = malloc(sizeof(pte_t)*pow(2,pgtblBits));
    }
    pte_t* currTable = pgdir[pgDirIndex];

    int pgTblIndex = ((unsigned int)va & PG_TBL_MASK) >> offBits;
    currTable[pgTblIndex] = pa;
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
   for(int i = 1; i < numVP; i++){
        if(vBitMap[i] == 0){
            if(count == 0) first = i;
            count++;
            if(count == num_pages){
                //your done
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
      void* virtAddr =(void*)((first)*PGSIZE);
        
        return virtAddr;
    }
    else{
      return NULL;
    }
   
}

void *get_next_avail_phys(int num_pages) {
    int i;
    for(i = 1; i < numPP; i++){
        if(pBitMap[i] == 0){
            break;
        }
    }

    // ith page is free
    void* physAddr = i*(PGSIZE) + memory;
    pBitMap[i] = 1; //now it's in use
    ppCount--;
    return physAddr;
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *myalloc(unsigned int num_bytes) {
    pthread_mutex_lock(&lock);
    //HINT: If the physical memory is not yet initialized, then allocate and initialize.
    if(init == 0){
        SetPhysicalMem();
        init = 1;
    }

   /* HINT: If the page directory is not initialized, then initialize the
   page directory. Next, using get_next_avail(), check if there are free pages. If
   free pages are available, set the bitmaps and map a new page. Note, you will 
   have to mark which physical pages are used. */

    
   float pagesNeeded = ceil((float)num_bytes/(float)PGSIZE); //TODO: it keeps rounding down, fix later

   void* currVA = get_next_avail((int)pagesNeeded);
   if(currVA == NULL){
     return NULL; //Not enough memory
   }
   //MAP to physical memory
   void * ptrva = currVA;
   if(ppCount < pagesNeeded){
      pthread_mutex_unlock(&lock);
      return NULL;
       
   }else{
       for(int i = 0; i < pagesNeeded; i++){
           void* currPA = get_next_avail_phys(1);
           //currVA = (int)currVA * i;
           ptrva=  currVA + (i * PGSIZE);

           //TRANSLATE VA TO GET INDEX..? Map it with PA (using pagemap()?)

           PageMap(pgdir, ptrva, currPA);

           int vBMIndex = (int)ptrva / PGSIZE;
           vBitMap[vBMIndex] = 1; //in use

           //ADD TO TLB
           check_TLB(ptrva);
           add_TLB(ptrva, currPA);     

       }
   }
   pthread_mutex_unlock(&lock);
    return currVA;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void myfree(void *va, int size) {
    pthread_mutex_lock(&lock);
    //Free the page table entries starting from this virtual address (va)
    // Also mark the pages free in the bitmap
    //Only free if the memory from "va" to va+size is valid

    if((int)va +size > MAX_MEMSIZE){
        printf("Invalid free\n");
        pthread_mutex_unlock(&lock);
        return;
    }

    double pagesNeeded = ceil((float)size/(float)PGSIZE); //TODO: it keeps rounding down, fix later
    void * ptrva = va;

    for(int i = 0; i < pagesNeeded; i++){
        ptrva=  va + (i * PGSIZE);

        int pgDirIndex = ((int)ptrva & PG_DIR_MASK);
        if(pgdir[pgDirIndex] == NULL){
            //nothing to free there..
            pthread_mutex_unlock(&lock);
            return;
        }
        pte_t* currTable = pgdir[pgDirIndex];
        int pgTblIndex = ((unsigned int)ptrva & PG_TBL_MASK) >> offBits;
        
        void* currpa = currTable[pgTblIndex];
        currTable[pgTblIndex] = NULL;

        int vBMIndex = (int)ptrva / PGSIZE;
        vBitMap[vBMIndex] = 0; //now free

        int pBMIndex = ((int)currpa - (int)memory)/PGSIZE;
        pBitMap[pBMIndex] = 0; //now free
        ppCount++;

    }

    pthread_mutex_unlock(&lock);


}


void PutVal(void *va, void *val, int size) {
    
    pthread_mutex_lock(&lock);
    void* vaEnds = va + size;
    int bytesToWrite = size;
    int numPages = 0;
    int totalToWrite = size;

    //Check if we start and end on the same page or diff pages, if diff pages, then bytesToWrite is from va to end of current page
    if((int)va / PGSIZE != (int)vaEnds / PGSIZE){
        int vInd = (int)va / PGSIZE; //get bitmap index of vitual page
	
	void* pgEnds =(void*)( (vInd)*PGSIZE + PGSIZE); //get addr of where the curr page ends
	//	int* temp = (vInd)*PGSIZE + PGSIZE;
	//void * pgEnds = &temp;
        bytesToWrite = pgEnds-va; //bytes from va to end of the curr page
    }

    int fullPages = size - bytesToWrite; //# of full pages we writing to

    if(fullPages > PGSIZE){
        numPages = fullPages/PGSIZE;
    }

    void * vaptr = va;
    void * valptr = val;
    for(int i = 0; i <= numPages; i++){
        void * phyAddr = Translate(pgdir, vaptr); //getting the physical addr
        memcpy(phyAddr, valptr, bytesToWrite);
        //va needs to be incremented by how much was written
        vaptr = vaptr + bytesToWrite;
        valptr = valptr + bytesToWrite;
        totalToWrite -= bytesToWrite;
        if(totalToWrite > PGSIZE){
            bytesToWrite = PGSIZE;
        }else{
            bytesToWrite = totalToWrite;
        }
        
    }
    pthread_mutex_unlock(&lock);
    

}

void GetVal(void *va, void *val, int size) {
  pthread_mutex_lock(&lock);
    void* vaEnds = va + size;
    int bytesToRead = size;
    int numPages = 0;
    int totalToRead = size;

    if((int)va / PGSIZE != (int)vaEnds / PGSIZE){
        int vInd = (int)va / PGSIZE;
        void* pgEnds =(void*)((vInd)*PGSIZE + PGSIZE);
        bytesToRead = pgEnds-va;
    }

    int fullPages = size - bytesToRead;

    if(fullPages > PGSIZE){
        numPages = fullPages/PGSIZE;
    }

    void * vaptr = va;
    void * valptr = val;
    for(int i = 0; i <= numPages; i++){
        void * phyAddr = Translate(pgdir, vaptr); //getting the physical addr
        memcpy(valptr, phyAddr, bytesToRead);
        //va needs to be incremented by how much was written
        vaptr = vaptr + bytesToRead;
        valptr = valptr + bytesToRead;
        totalToRead -= bytesToRead;
        if(totalToRead > PGSIZE){
            bytesToRead = PGSIZE;
        }else{
            bytesToRead = totalToRead;
        }
        
    }
    pthread_mutex_unlock(&lock);


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


   
    int zero = 0;
    int currAnsElement;
    int m1Element;
    int m2Element;
    int ansElement;
    for(int i =0; i< size; i++){
        for(int j =0; j<size; j++){
	  PutVal((void*)((unsigned int)answer) + ((i * size * sizeof(int))) + (j * sizeof(int)), &zero, sizeof(int));
            for(int k =0; k <size; k++){
	      GetVal((void*)((unsigned int)answer) + ((i * size * sizeof(int))) + (j * sizeof(int)), &currAnsElement, sizeof(int));

	      GetVal((void*)((unsigned int)mat1) + ((i * size * sizeof(int))) + (k * sizeof(int)), &m1Element, sizeof(int));

	      GetVal((void*)((unsigned int)mat2) + ((k * size * sizeof(int))) + (j * sizeof(int)), &m2Element, sizeof(int));
                ansElement = currAnsElement + (m1Element*m2Element);

                
                PutVal((void*)((unsigned int)answer) + ((i * size * sizeof(int))) + (j * sizeof(int)), &ansElement, sizeof(int));

            }
        }
    }
    
    
 
}
