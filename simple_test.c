#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define NUMBUFS 15
int bufsizes[NUMBUFS] = {1,3,7,14,65,36,700,12,15,64,1024,1024,1024,1024,5000};

int main()
{

	uint8_t *bufs[NUMBUFS];
        int i;
        int b;

	void * firstbreak = sbrk(0);

	free(NULL); //just for kicks

	for (i=0; i < NUMBUFS; i++)
	{
		//allocate the next block
		bufs[i] = malloc(bufsizes[i]);
		assert(bufs[i] != NULL); //should never return NULL

		//write some data into the buffer
		memset(bufs[i], i, bufsizes[i]);
	}

	for (i=0; i < NUMBUFS; i++)
	{
		//check whether or not the memory is still intact
		for (b=0; b < bufsizes[i]; b++)
		{
			assert (bufs[i][b] == i);
		}

		free(bufs[i]);
	}

	void * lastbreak = sbrk(0);

	//verify that the program break never moved up.
	assert (firstbreak == lastbreak);

	return 0;
}
