#include <stdio.h>
#include <stdlib.h>

#include "mymalloc.h"

int main(void)
{
	int alloc_alg = FIRST_FIT;
	void *a, *b, *c;
	size_t size;

//	if (argc > 1)
//		alloc_alg = atoi(argv[1]);

	printf("Enter allocation strategy:\n");
	printf("0: FIRST_FIT\n1: NEXT_FIT\n2:BEST_FIT\n");
	scanf("%d", &alloc_alg);
	if (alloc_alg != 0 && alloc_alg != 1 && alloc_alg != 2) {
		printf("You entered wrong choice\n");
		return -1;
	}

	myinit(alloc_alg);

	size = 10;
	a = mymalloc(size);
	printf("Allocated memory block of: %p of size %zu\n",
	       a, size);

	size = 800;
	b = mymalloc(size);
	printf("Allocated memory block of: %p of size %zu\n",
	       b, size);

	size = 100;
	c = mymalloc(size);
	printf("Allocated memory block of: %p of size %zu\n",
	       c, size);

	printf("Re-Allocated memory block: %p from size %zu to size 256\n",
	       c, size);
	size = 256;
	c = myrealloc(c, size);
	printf("Re-Allocated memory block of: %p of size %zu\n",
	       c, size);

	printf("Freed memory block: %p\n", a);
	myfree((void *)&a);
	printf("Freed memory block: %p\n", b);
	myfree((void *)&b);
	printf("Freed memory block: %p\n", c);
	myfree((void *)&c);

	mycleanup();
	return 0;
}
