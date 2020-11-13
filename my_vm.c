#include "my_vm.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>

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
/*
Function responsible for allocating and setting your physical memory 
*/
void SetPhysicalMem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    printf("INITIALIZING LIBRARY\n");
    memory = malloc(sizeof(char)* MEMSIZE);
    printf("pm start at %p\n", memory);

    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

    offBits = log2(PGSIZE); //offset bits based on pagesize
    offset = offset <<  offBits; //offset bits as an int
    offset = ~offset;
    printf("the offset mask is %d\n", offset);
     
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
    printf("pgdir mask: %d\n", PG_DIR_MASK);

    unsigned int dirMaskFlip = ~PG_DIR_MASK;
    printf("initial mask: %d\n", PG_TBL_MASK);
    PG_TBL_MASK = PG_TBL_MASK << offBits;
    PG_TBL_MASK = dirMaskFlip & PG_TBL_MASK;
    printf("final masl: %d\n", PG_TBL_MASK);

    //Initialize tlb structure
    tlb_store = malloc(sizeof(struct tlb));
    tlb_store->count = 0;
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

        newEntry->va = va;
        newEntry->pa = pa;

        tlb_store->count++;

        //printf("Added va %p , pa %p to TLB\n", va, pa);
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

        //printf("Added va %p , pa %p to TLB\n", va, pa);

    }

    return -1;
}

pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */
    //TODO: EXCLUDE OFFBITS WHEN LOOKING?
    void* vaToSearch = (int)va & (~offset);


    struct node* ptr = tlb_store->head;
    while(ptr != NULL){
        if(ptr->va == vaToSearch){
            //found
            //printf("found mapping for %p in TLB\n", va);
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
  printf("total req: %d\n", totalRequests);
  printf("miss req: %d\n", missRequests);

    
  miss_rate = (float)missRequests/(float)totalRequests;
  printf("MISS RATE %f\n", miss_rate);


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
            void* offToAdd = (int)va & offset;
            //printf("offbits of va %d\n", offToAdd);
            paAddr = paAddr + (int)offToAdd; //adding offset bits to the current addr
            //printf("pa from TLB %p\n", paAddr);
            return paAddr;
        }
    }


    //printf("IN TRANSLATE\n");
    int pgDirIndex = ((int)va & PG_DIR_MASK); //HOW TO WORK WITH VOID* address
    //printf("dir index: %d\n", pgDirIndex);
    if(pgdir[pgDirIndex] == NULL){
        return NULL;
    }else{
        pte_t* currTable = pgdir[pgDirIndex];
        int pgTblIndex = ((int)va & PG_TBL_MASK) >> offBits;
        //printf("table index: %d\n", pgTblIndex);
        void * currAddr = currTable[pgTblIndex];
        void* offToAdd = (int)va & offset;
        //printf("offbits of va %d\n", offToAdd);
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
    printf("pgdir: %d\n", pgDirIndex);
    if(pgdir[pgDirIndex] == NULL){
        //need to allocate a page table
        pgdir[pgDirIndex] = malloc(sizeof(pte_t)*pow(2,pgtblBits));
    }
    pte_t* currTable = pgdir[pgDirIndex];

    int pgTblIndex = ((unsigned int)va & PG_TBL_MASK) >> offBits;
    printf("tblmask: %d, tblIndex: %d\n", PG_TBL_MASK, pgTblIndex);
    currTable[pgTblIndex] = pa;
    printf("pm mapped! pgdir: %d, pgtbl: %d with %p\n", pgDirIndex, pgTblIndex, currTable[pgTblIndex]);



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
   for(int i = 1; i < numVP; i++){
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
        void* virtAddr = (first)*PGSIZE;;
       
        
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
    printf("MYALLOC CALLED\n");
    //HINT: If the physical memory is not yet initialized, then allocate and initialize.
    if(init == 0){
        SetPhysicalMem();
        init = 1;
    }

   /* HINT: If the page directory is not initialized, then initialize the
   page directory. Next, using get_next_avail(), check if there are free pages. If
   free pages are available, set the bitmaps and map a new page. Note, you will 
   have to mark which physical pages are used. */
    printf("bytes wanted %d\n", num_bytes);

   float pagesNeeded = ceil((float)num_bytes/(float)PGSIZE); //TODO: it keeps rounding down, fix later
   printf("pages needed %f\n", pagesNeeded);

   void* currVA = get_next_avail((int)pagesNeeded);
   if(currVA == NULL){
       printf("Not enough memory\n");
   }else{
       printf("vm found! at %p\n", currVA);
   }

   //MAP to physical memory
   void * ptrva = currVA;
   if(ppCount < pagesNeeded){
       return NULL;
   }else{
       for(int i = 0; i < pagesNeeded; i++){
           void* currPA = get_next_avail_phys(1);
           printf("pm found! at %p\n", currPA);

           //currVA = (int)currVA * i;
           ptrva=  currVA + (i * PGSIZE);

           //TRANSLATE VA TO GET INDEX..? Map it with PA (using pagemap()?)

           PageMap(pgdir, ptrva, currPA);

           int vBMIndex = (int)ptrva / PGSIZE;
           printf("v bitmapindex to update: %d\n", vBMIndex);
           vBitMap[vBMIndex] = 1; //in use

           //ADD TO TLB
           check_TLB(ptrva);
           add_TLB(ptrva, currPA);     

       }
   }

    return currVA;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void myfree(void *va, int size) {

    //Free the page table entries starting from this virtual address (va)
    // Also mark the pages free in the bitmap
    //Only free if the memory from "va" to va+size is valid

    // 1. use size to see how many pages would need to be freed -> count
    // 2. use va to get pgdir index, and pg table index
    // 3. for count pages, get va and go to that entry in pg table and null it out
    // 4. 0 the corresponding entries in both bitmaps
    // 5. rmbr to increment ppCount when freeing phsysical pages.
    printf("VA to free %p\n", va);
    if((int)va +size > MAX_MEMSIZE){
        printf("Invalid free\n");
        return NULL;
    }

    printf("bytes to free %d", size);
    double pagesNeeded = ceil((float)size/(float)PGSIZE); //TODO: it keeps rounding down, fix later
    printf("pages to free %f\n", pagesNeeded);

    void * ptrva = va;

    for(int i = 0; i < pagesNeeded; i++){
        ptrva=  va + (i * PGSIZE);

        int pgDirIndex = ((int)ptrva & PG_DIR_MASK);
        printf("pgdir: %d\n", pgDirIndex);
        if(pgdir[pgDirIndex] == NULL){
            //nothing to free there..
            return NULL;
        }
        pte_t* currTable = pgdir[pgDirIndex];

        //int pgTblIndex = ((unsigned int)ptrva & PG_TBL_MASK);
        int pgTblIndex = ((unsigned int)ptrva & PG_TBL_MASK) >> offBits;
        printf("tblmask: %d, tblIndex: %d\n", PG_TBL_MASK, pgTblIndex);
        
        void* currpa = currTable[pgTblIndex];
        currTable[pgTblIndex] = NULL;
        //TODO: Do we also have to clear out the space in physical memory, or just let the user overwrite? No

        printf("pm unmapped! pgdir: %d, pgtbl: %d previously had %p\n", pgDirIndex, pgTblIndex, currpa);

        int vBMIndex = (int)ptrva / PGSIZE;
        printf("v bitmapindex to update: %d\n", vBMIndex);
        vBitMap[vBMIndex] = 0; //now free

        int pBMIndex = ((int)currpa - (int)memory)/PGSIZE;
        printf("p bitmapindex to update: %d\n", pBMIndex);
        pBitMap[pBMIndex] = 0; //now free
        ppCount++;

    }




}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void PutVal(void *va, void *val, int size) {
    //printf("PUTVAL\n");
    //printf("the va passed: %p\n", va);
    int v = (int)val;
    //printf("the val passed: %d\n", v);
    //printf("the size passed: %d\n", size);
    int numPages = 1;
    if(size > PGSIZE){
     numPages = ceil((float)size/(float)PGSIZE);;
     }
     void * vaptr = va;
     void * valptr = val;
    for(int i = 0; i< numPages; i++){
        vaptr = va +(PGSIZE* i);
        valptr = val+(PGSIZE * i); //CONFIRM THIS??
        void * phyAddr = Translate(pgdir, vaptr); //getting the physical addr
        //printf("phys addr translated to %p\n", phyAddr);
       if(size > PGSIZE){
        memcpy(phyAddr, valptr, PGSIZE);
        }else{
        memcpy(phyAddr, valptr, size);
        //printf("num is %d\n", *(int*)phyAddr);
        }
     }



    //first check if the size is larger than the pageSize.
    //if it then see how many pages that is.
    //use translate to map each page from VA to PA.



    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/
      // 1. Locate the physical page.


}


/*Given a virtual address, this function copies the contents of the page to val*/
void GetVal(void *va, void *val, int size) {
int numPages = 1;
 if(size > PGSIZE){
  numPages = ceil((float)size/(float)PGSIZE);
 }
 for(int i =0; i<numPages; i++){
    void * vaptr =va + (i * PGSIZE);
    void * valptr = val + (i*PGSIZE);
    void * phyAddr = Translate(pgdir, vaptr);
    if(size > PGSIZE){
     memcpy(valptr, phyAddr, PGSIZE);
    }else{
    memcpy(valptr, phyAddr, size);
    }


 }


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


    int x;
    printf("matrix 1\n");
    for(int r =0; r< size; r++){
        for(int s =0; s<size;s++){
            //GetVal(mat1+(r*size+s), &x, sizeof(int));
            GetVal((unsigned int)mat1 + ((r * size * sizeof(int))) + (s * sizeof(int)), &x, sizeof(int));
            printf("%d ", x);
	    }
	    printf("\n");
    }

    printf("matrix 2\n");
    for(int r =0; r< size; r++){
        for(int s =0; s<size;s++){
            //GetVal(mat2+(r*size+s), &x, sizeof(int));
            GetVal((unsigned int)mat2 + ((r * size * sizeof(int))) + (s * sizeof(int)), &x, sizeof(int));
            printf("%d ", x);
	    }
	    printf("\n");
    }
    int zero = 0;
    int currAnsElement;
    int m1Element;
    int m2Element;
    int ansElement;
    for(int i =0; i< size; i++){
        for(int j =0; j<size; j++){
  
            //PutVal(answer+(i*size + j), &zero, sizeof(int));
            PutVal((unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int)), &zero, sizeof(int));
            for(int k =0; k <size; k++){
                //GetVal(answer+(i*size + j), &currAnsElement, sizeof(int));
                GetVal((unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int)), &currAnsElement, sizeof(int));
                //printf("c %d ", currAnsElement);
                //GetVal(mat1+(i*size + k), &m1Element, sizeof(int));
                GetVal((unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int)), &m1Element, sizeof(int));
                
                //GetVal(mat2+(k*size + j), &m2Element, sizeof(int));
                GetVal((unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int)), &m2Element, sizeof(int));
                ansElement = currAnsElement + (m1Element*m2Element);

                //PutVal(answer+(i*size + j), &ansElement, sizeof(int));
                PutVal((unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int)), &ansElement, sizeof(int));

            }
        }
    }

    printf("ans matrix\n");
    for(int r =0; r< size; r++){
        for(int s =0; s<size;s++){
            //GetVal(answer+(r*size+s), &x, sizeof(int));
            GetVal((unsigned int)answer + ((r * size * sizeof(int))) + (s * sizeof(int)), &x, sizeof(int));
            printf("%d ", x);
	    }
	    printf("\n");
    }
 
}
