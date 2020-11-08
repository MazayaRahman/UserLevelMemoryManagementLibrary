#include "my_vm.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
<<<<<<< HEAD
//#include <string.h>

=======
#include <string.h>
>>>>>>> 80783fbdafbb3ab17b656a95ad252f96ae565f35
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
<<<<<<< HEAD
int offset = 0XFFFFFFFF;

=======
int offset = 0XFFFFFFFFF;
int address_a =0, address_b =0;
>>>>>>> 80783fbdafbb3ab17b656a95ad252f96ae565f35
unsigned int PG_DIR_MASK = 0XFFFFFFFF;
unsigned int PG_TBL_MASK = 0XFFFFFFFF;

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
    
    //PG_TBL_MASK = PG_TBL_MASK << (32-pgtblBits);
    
    //PG_TBL_MASK = (PG_TBL_MASK >> pgdrBits) & (PG_DIR_MASK >> pgdrBits);
    //printf("shift left: %d\n", PG_TBL_MASK);
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
    
  // printf("IN TRANSLATE\n");
    int pgDirIndex = ((int)va & PG_DIR_MASK); //HOW TO WORK WITH VOID* address
    //printf("dir index: %d\n", pgDirIndex);
    if(pgdir[pgDirIndex] == NULL){
        return NULL;
    }else{
        pte_t* currTable = pgdir[pgDirIndex];
        int pgTblIndex = ((int)va & PG_TBL_MASK) >> offBits;
	// printf("table index: %d\n", pgTblIndex);
        void * currAddr = currTable[pgTblIndex];
        void* offToAdd = (int)va & offset;
	// printf("offbits of va %d\n", offToAdd);
        currAddr = currAddr + (int)offToAdd; //adding offset bits to the current addr
	//	printf("added offbits to curradd\n");
	
        return currAddr;
    }


    printf("Translation not successfull\n");
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

        int pgTblIndex = ((unsigned int)ptrva & PG_TBL_MASK);
        printf("tblmask: %d, tblIndex: %d\n", PG_TBL_MASK, pgTblIndex);
        
        void* currpa = currTable[pgTblIndex];
        currTable[pgTblIndex] = NULL;
        //TODO: Do we also have to clear out the space in physical memory, or just let the user overwrite?

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
  // printf("the va passed: %p\n", va);
    int v = (int)val;
    //printf("the val passed: %d\n", v);
    // printf("the size passed: %d\n", size);
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
	//  printf("phys addr translated to %p\n", phyAddr);
       if(size > PGSIZE){
        memcpy(phyAddr, valptr, PGSIZE);
        }else{
	 //printf("Before memcpy\n");
        memcpy(phyAddr, valptr, size);
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
  // printf("Inside getval function\n");
int numPages = 1;
 if(size > PGSIZE){
  numPages = ceil((float)size/(float)PGSIZE);
  //printf("No.of pages%d\n",numPages);
 }
 for(int i =0; i<numPages; i++){
    void * vaptr =va + (i * PGSIZE);
    void * valptr = val + (i*PGSIZE);
    // printf("Before translate in getval\n");
    void * phyAddr = Translate(pgdir, vaptr);
    // printf("After translate in getval\n");
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
int element;
int element1;
<<<<<<< HEAD
int e;
=======
void *  e;
>>>>>>> 80783fbdafbb3ab17b656a95ad252f96ae565f35
int ans;
void MatMult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
    matrix accessed. Similar to the code in test.c, you will use GetVal() to
    load each element and perform multiplication. Take a look at test.c! In addition to 
    getting the values from two matrices, you will perform multiplication and 
    store the result to the "answer array"*/

<<<<<<< HEAD
printf("Inside the matmul function\n");
  for(int i =0; i< size; i++){
     for(int j =0; j<size; j++){
  
       // PutVal(0, answer, sizeof(int));
       for(int k =0; k <size; k++){
	 printf("First getval\n");
	 GetVal(answer+(i*size +j),&e, sizeof(int));
	 GetVal(mat1+(i*size + k), &element, sizeof(int));
	 printf("FIRST VALUE RECORDED %d\n", element);
	GetVal(mat2+(k*size +j), &element1, sizeof(int));
	 
	ans = e + (element*element1);
	 printf("first mat mul %d\n", ans);
	PutVal(answer+(i*size+j), &ans, sizeof(int));
	 // memcpy(answer+(i*size+j), ans, sizeof(int));
=======


  printf("Inside the matmul function\n");
  for(int i =0; i< size; i++){
     for(int j =0; j<size; j++){
       //PutVal(0, answer, sizeof(int));
       for(int k =0; k <size; k++){
	 GetVal(answer+(i*size +j),(int) &e, sizeof(int));
	 GetVal(mat1+(i*size + k), &element, sizeof(int));
	 printf("FIRST VALUE RECORDED %d\n", element);
	 GetVal(mat2 +(k*size +j), &element1, sizeof(int));
	 
	 ans = e + (element*element1);
	 //printf("first mat mul %d\n", ans);
	  PutVal(answer+(i*size+j), ans, sizeof(int));
>>>>>>> 80783fbdafbb3ab17b656a95ad252f96ae565f35
       }
       
     }
    
  }
<<<<<<< HEAD

  printf("THE ANSWER MATRIX\n");

  for(int r =0; r< size; r++){
    for(int s =0; s<size;s++){
      printf("%d ", answer+(r*size+s));
	}
	printf("\n");
    }

  /*
  //printf(" mat mul %d\n", ans);
     printf("Printing the copied mat1\n");
=======
   printf(" mat mul %d\n", ans);
     printf("Printing the copied array\n");
>>>>>>> 80783fbdafbb3ab17b656a95ad252f96ae565f35
  for(int k =0; k <size; k++){
   for(int l =0; l <size; l++){
     printf("%d ", element+(k*size+l));
   }
   printf("\n");
  }
<<<<<<< HEAD
  printf("Printing themat2\n");
=======

>>>>>>> 80783fbdafbb3ab17b656a95ad252f96ae565f35
  for(int r =0; r<size;r++){
    for(int s=0;s<size;s++){
      printf("%d ", element1+(r*size+s));
    }
    printf("\n");
<<<<<<< HEAD
  } 
  */      
=======
  }
  
  /*
  for(int k = 0; k<size; k++){
    for(int l = 0; l<size; l++){
      for(int m =0; m<size;m++){
	*(answer+(k*size+l)) = *(element+(k*size+m)) * *(element1+(m*size+l));
      }
    }
  }

  printf("Testing the multiplication product\n");

  for(int r =0; r<size;r++){
    for(int s =0; s<size;s++){
      printf("%d ", answer+(r*size+s));
    }
    printf("\n");
  }
  */

>>>>>>> 80783fbdafbb3ab17b656a95ad252f96ae565f35
}
