#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE_SIZE 4096

int main(int argc, char** argv)
{
	int *p = (int*)0xfdf007;

	//mmap(address-hint, size, protections, flags, fd, offset)
	void *rwpage = mmap(p, PAGE_SIZE, 
		PROT_READ | PROT_WRITE, 
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("Asked for %p, and got a page starting at %p\n",p, rwpage);

	int *intptr = (int*)rwpage;

	printf("Can I write to it...%d\n",*intptr = 532);
	printf("Can I write to it...%d\n",*(intptr + 1) = 1024);
        
	printf("Can I read from it...%d\n",*intptr);
	printf("Can I read from it...%d\n\n",*(intptr + 1));

	void *ropage = mmap(p, PAGE_SIZE, 
		PROT_READ, 
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	printf("Asked for %p, and got a page starting at %p\n",p, ropage);

	int *another = (int*)ropage;

	//printf("Can I write to it...%d\n",*another = 532);
	printf("Can I read from it...%d\n",*another);

	//and mmap can also be used to map files into memory!
	int fd = open("./test.txt",O_RDWR, S_IRUSR | S_IWUSR);
	struct stat sb;

	if (fstat(fd,&sb) == -1)
	{
		perror("couldn't get file size.\n");
	} else {
		printf("file size is %ld\n",sb.st_size);
	}
	char *filemappedpage = mmap(p, sb.st_size, 
		PROT_READ | PROT_WRITE, 
		MAP_SHARED, fd, 0);

	printf("Printing file, as an array of characters...\n\n");
	for (int i=0; i < sb.st_size; i++)
	{
		printf("%c",filemappedpage[i]);
		if (filemappedpage[i] == 't')
		{
			filemappedpage[i] = 'T';
		} 
	}
	printf("\n");
	//munmap(filemappedpage, sb.st_size);
	close(fd);

}
