// Name: Caleb Priester

#define _GNU_SOURCE

void __attribute__ ((constructor)) libraryInit(void);
void *innermalloc(int lNum, int size);

#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define PAGE_SIZE 4096
#define PAGE_HEADER 36
#define PART_HEADER 8

void *lists[10];

int fd;

void libraryInit(void) {
   fd = open("/dev/zero", O_RDWR);
}

void *malloc(size_t size) {

   if(size == 0) return NULL;
   if(size == 1) size = 2;

   //Find out which list will be used to allocate data.
   int listNum = (int) ceil(log2(size));

   //Call another method that indicates the specified list and size.
   switch(listNum) {
   case 1:  return innermalloc(0,   2);   break;
   case 2:  return innermalloc(1,   4);   break;
   case 3:  return innermalloc(2,   8);   break;
   case 4:  return innermalloc(3,  16);   break;
   case 5:  return innermalloc(4,  32);   break;
   case 6:  return innermalloc(5,  64);   break;
   case 7:  return innermalloc(6, 128);   break;
   case 8:  return innermalloc(7, 256);   break;
   case 9:  return innermalloc(8, 512);   break;
   case 10: return innermalloc(9, 1024);
   }

   void *r = mmap(NULL, size + PAGE_HEADER + PART_HEADER, 
      PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, fd, 0);

   *((int *)r) = size;
   *((int *)(r + 4)) = 0;
   *((int *)(r + 8)) = size;
   *((void **)(r + 12)) = NULL;
   *((void **)(r + 20)) = NULL;
   *((void **)(r + 28)) = NULL;
   *((void **)(r + PAGE_HEADER)) = r;
   return r + PAGE_HEADER + PART_HEADER;

}

void *innermalloc(int lNum, int size) {

   int numParts = (PAGE_SIZE - PAGE_HEADER) / (PART_HEADER + size);

   //Traverse list until a page with an open slot is found or until the end
   //  of the list.
   void *page = lists[lNum];
   while(page != NULL && *((int *)(page)) == 0) {
      page = *((void **)(page+12));
   }

   //If there was either no space on any pages or no pages initialized,
   //  map a page and prepend it to the page list.
   if(page == NULL) {
      page = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
         MAP_PRIVATE | MAP_ANONYMOUS, fd, 0);

      //Set current number of free segments.
      *((int *)(page)) = numParts;

      //Set maximum number of free segments.
      *((int *)(page+4)) = numParts;

      //Set size into header.
      *((int *)(page+8)) = size;

      //Set the new page's next page pointer to the head of the list and
      //  set the head of the list's previous page pointer to the new page.
      *((void **)(page+12)) = lists[lNum];
      if(lists[lNum] != NULL) *((void **)(lists[lNum]+20)) = page;

      //Set the new page's previous page pointer to NULL.
      *((void **)(page+20)) = NULL;

      //Set the head of the free list to the first segment on the new page.
      *((void **)(page+28)) = page + PAGE_HEADER;

      //Put all parts into the free list and terminate with NULL pointer.
      void *temp = *((void **)(page+28));
      int i;
      for(i = 0; i < numParts - 1; i++) {
         *((void **)temp) = temp + PART_HEADER + size;
	 temp = *((void **)temp);
      }
      *((void **)temp) = NULL;

      lists[lNum] = page;
   }

   //Go to the first free part.
   void *part = *((void **)(page+28));
//-------------------------------------------------------------------------
   assert(part != NULL); *((void **)(page+28)) = *((void **)part);
//-------------------------------------------------------------------------

   //Set the pointer stored in the part header to the current page
   //  (this is used in free).
   *((void **)part) = page;

   //Decrement the number of available parts on the page.
   *((int *)(page)) = *((int *)(page)) - 1;

   return part+PART_HEADER;


}

void *calloc(size_t nmemb, size_t size) {

   //Mallocs space for the number of objects "nmemb" of size "size."
   void *r = malloc(nmemb*size);

   //Sets the memory at the pointer to 0 bits.
   memset(r, 0, (nmemb*size));

   return r;
}

void *realloc(void *ptr, size_t size) {

   int curSize;

   //If the ptr is NULL, realloc is essentially malloc.
   if(ptr == NULL) return malloc(size);

   //If the size is 0, realloc is essentially free.
   if(size == 0) {
      free(ptr);
      return NULL;
   }
   
   //Calculate the current size of the object.
   void *part = ptr - PART_HEADER;
//-------------------------------------------------------------------------
   assert(part != NULL); void *page = *((void **)part);
//-------------------------------------------------------------------------
   
   curSize = *((int *)(page+8));
   

   //Allocate memory of the specified size, copy over as much of the data as
   //  will fit into the new pointer r, and then free ptr.
   void *r = malloc(size);
   if(size > curSize) memcpy(r, ptr, curSize);
   else               memcpy(r, ptr, size);

   free(ptr);

   return r;
}

void free(void *ptr) {
   if(ptr == NULL) return;

   //Grabs the part holding the space to be freed and
   //  the page holding that part.
   void *part = ptr - PART_HEADER;
   void *page = *((void **)part);

   if(*((int *)(page+4)) == 0) {
      munmap(page, *((int *)(page)) + PAGE_HEADER + PART_HEADER);
      return;
   }

   //Identify the size of the items on the page.
   int size = *((int *)(page+8));
   if(size == 1) size = 2;

   //Prepend the part to the head of the free segments list.
   *((void **)part) = *((void **)(page+28));
   *((void **)(page+28)) = part;

   //Increment the number of free segments counter.
   *((int *)(page)) = *((int *)page) + 1;

   //Unmap the page in the case that it is empty.
   if(*((int *)(page)) == *((int *)(page+4))) {
      void *prevPage = *((void **)(page+20));
      void *nextPage = *((void **)(page+12));

      if(prevPage != NULL) *((void **)(prevPage+12)) = nextPage;

      //If the previous page does not exist, then this page was the head and
      //  the corresponding list pointer must be updated to the new head.
      else {
         int listNum = (int) ceil(log2(size));
         switch(listNum) {
	    case 1:  lists[0] =  nextPage;   break;
	    case 2:  lists[1] =  nextPage;   break;
	    case 3:  lists[2] =  nextPage;   break;
	    case 4:  lists[3] =  nextPage;   break;
	    case 5:  lists[4] =  nextPage;   break;
	    case 6:  lists[5] =  nextPage;   break;
	    case 7:  lists[6] =  nextPage;   break;
	    case 8:  lists[7] =  nextPage;   break;
	    case 9:  lists[8] =  nextPage;   break;
	    case 10: lists[9] =  nextPage;
	 }
      }
      if(nextPage != NULL) *((void **)(nextPage+20)) = prevPage;

      munmap(page, PAGE_SIZE);
   }

   return;
}
