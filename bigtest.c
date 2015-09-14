#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void *allocs[10000];

int main() {
   int ct = 0;
   int choice;
   int i;
   int j, a, b;
   void *temp;
   int num;
   int size;

   srand(time(NULL));

   for(i = 0; i < 10000; i++) {
      choice = rand()%4;
      num = rand()%500;
      size = rand()%1500;

      if(choice == 0) {
         allocs[ct] = malloc(size);
	 ct++;
      }
      else if(choice == 1) {
         allocs[ct] = calloc(num, size);
	 ct++;
      }
      else if(choice == 2) {
         if(ct != 0) {
	    ct--;
	    free(allocs[ct]);
	 }
	 else free(NULL);
      }
      else if(choice == 3) {
         allocs[ct-1] = realloc(allocs[ct-1], size);
      }
      if(ct != 0) {
         for(j = 0; j < 10000; j++) {
            a = rand()%ct;
	    b = rand()%ct;
   
	    temp = allocs[a];
	    allocs[a] = allocs[b];
	    allocs[b] = temp;
         }
      }
   }
   
   
   return 1;
}
