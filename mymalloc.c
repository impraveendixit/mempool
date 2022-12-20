#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mymalloc.h"

#define BLOCK_SIZE	8
/* 1 MB HEAP size */
#define BLOCK_COUNT	((1024 * 1024) / BLOCK_SIZE)

/* Individual memory block */
struct mem_block {
	/* List pointer */
	struct list_head list;
	/* Memory pointed by the block */
	void *mem_ptr;
	/* Number of blocks */
	size_t count;
	/* Allocation status */
	unsigned int is_free;
};

static struct mem_block *create_mem_block(size_t count)
{
	struct mem_block *mb = malloc(sizeof(struct mem_block));
	if (mb == NULL) {
		printf("Failed to allocate memory block\n");
		return NULL;
	}
	INIT_LIST_HEAD(&mb->list);
	mb->mem_ptr = NULL;
	mb->count = count;
	mb->is_free = 1;
	return mb;
}

static void destroy_mem_block(struct mem_block *mb)
{
	if (mb) {
		mb->mem_ptr = NULL;
		free(mb);
		mb = NULL;
	}
}

static void create_free_list(struct list_head *head, int count)
{
	int nr = count / 8;

	/* Create free list */
	for (int i = 0; i < nr; i++) {
		struct mem_block *mb = create_mem_block(nr);
		if (mb == NULL)
			continue;
		list_add_tail(&mb->list, head);
	}
}

static void destroy_free_list(struct list_head *head)
{
	if (list_empty(head))
		return;

	struct mem_block *mb, *tmp = NULL;
	list_for_each_entry_safe(mb, tmp, head, list) {
		list_del(&mb->list);
		destroy_mem_block(mb);
	}
}

struct mem_pool {
	/* Free list pool */
	struct list_head free_list;
	/* List of memory allocated, freed */
	struct list_head mem_block_list;
	/* For use in next fit algorithm */
	struct list_head *next_block;
	/* Size of memory pool */
	size_t count;
	/* Memory pointer */
	void *mem_ptr;
	/* Allocation algorithm type */
	int alloc_alg;
};

static struct mem_pool *create_mem_pool(int alloc_alg, size_t count)
{
	struct mem_block *mb = NULL;
	struct mem_pool *mp = malloc(sizeof(struct mem_pool));
	if (mp == NULL) {
		printf("Memory allocation failure.\n");
		goto exit;
	}

	/* Initialize mem block list */
	INIT_LIST_HEAD(&mp->mem_block_list);
	mp->next_block = &mp->mem_block_list;

	/* Initialize free list */
	INIT_LIST_HEAD(&mp->free_list);

	mp->count = count;
	mp->alloc_alg = alloc_alg;

	mp->mem_ptr = malloc(count * BLOCK_SIZE);
	if (mp->mem_ptr == NULL) {
		printf("Failed to allocate memory for mem pool\n");
		goto exit_free;
	} else {
//		printf("Allocated memory pool of size %zu from %p address\n",
//		       count * BLOCK_SIZE, mp->mem_ptr);
	}

	/* Create free list */
	create_free_list(&mp->free_list, count);

	/* Initialize First block list */
	mb = create_mem_block(count);
	if (mb == NULL) {
		printf("failed to create mem block\n");
		goto exit_free_pool;
	}
	mb->mem_ptr = mp->mem_ptr;
	mb->count = mp->count;
	mb->is_free = 1;
	list_add_tail(&mb->list, &mp->mem_block_list);
	return mp;

 exit_free_pool:
	free(mp->mem_ptr);
 exit_free:
	free(mp);
 exit:
	return NULL;
}

/*static void print_mem_blocks(struct mem_pool *mp)
{
	struct mem_block *mb = NULL;
	list_for_each_entry(mb, &mp->mem_block_list, list) {
		printf("Mem block: %p of size: %zu, status: %d\n",
		       mb->mem_ptr, mb->count * BLOCK_SIZE, mb->is_free);
	}
}
*/

static void destroy_mem_pool(struct mem_pool *mp)
{
	if (mp) {
		destroy_free_list(&mp->free_list);

		struct mem_block *mb, *tmp = NULL;
		list_for_each_entry_safe(mb, tmp, &mp->mem_block_list, list) {
			list_del(&mb->list);
			destroy_mem_block(mb);
		}
		if (mp->mem_ptr) {
			free(mp->mem_ptr);
			mp->mem_ptr = NULL;
		}
		free(mp);
		mp = NULL;
	}
}

static struct mem_block *find_block_first_fit(struct mem_pool *mp, size_t count)
{
	struct mem_block *mb = NULL;

	list_for_each_entry(mb, &mp->mem_block_list, list) {
		if (mb->is_free && mb->count >= count) {
			mb->is_free = 0;
			__list_add(&mb->list, mb->list.prev, mb->list.next);
			return mb;
		}
	}
	return NULL;
}

static struct mem_block *find_block_best_fit(struct mem_pool *mp, size_t count)
{
	struct mem_block *min = NULL;
	struct mem_block *mb = NULL;

	list_for_each_entry(mb, &mp->mem_block_list, list) {
		if ((mb->is_free && mb->count >= count) &&
			(min == NULL || mb->count < min->count))
			min = mb;
	}
	if (min != NULL) {
		min->is_free = 0;
		__list_add(&min->list, min->list.prev, min->list.next);
	}
	return min;
}

static struct mem_block *find_block_next_fit(struct mem_pool *mp, size_t count)
{
	struct list_head *iter = mp->next_block;

	list_for_each(iter, &mp->mem_block_list) {
		struct mem_block *mb = list_entry(iter, struct mem_block, list);
		if (mb->is_free && mb->count >= count) {
			mb->is_free = 0;
			__list_add(&mb->list, mb->list.prev, mb->list.next);
			mp->next_block = &mb->list;
			return mb;
		}
	}
	return NULL;
}

static struct mem_pool *pool = NULL;

void myinit(int allocAlg)
{
	pool = create_mem_pool(allocAlg, BLOCK_COUNT);
}

void *mymalloc(size_t size)
{
	struct mem_block *mb = NULL;
	size_t count = (size / (float)BLOCK_SIZE + 0.5);

	if (count <= 0) {
		printf("Invalid size requested\n");
		return NULL;

	}
	if (pool == NULL) {
		printf("Invalid memory pool\n");
		return NULL;
	}

//	printf("Before allocation:\n");
//	print_mem_blocks(pool);

	switch (pool->alloc_alg) {
	case FIRST_FIT:
		mb = find_block_first_fit(pool, count);
		break;
	case BEST_FIT:
		mb = find_block_best_fit(pool, count);
		break;
	case NEXT_FIT:
		mb = find_block_next_fit(pool, count);
		break;
	default:
		printf("Wrong memory allocation choice\n");
		return NULL;
	}

	if(mb != NULL && mb->count > count) {
		/* Get empty list from free list */
		struct mem_block *rem = list_first_entry(&pool->free_list,
						 struct mem_block, list);
		rem->count = mb->count - count;
		mb->count = count;
		rem->mem_ptr = mb->mem_ptr + count * BLOCK_SIZE;
		list_move_tail(&rem->list, &pool->mem_block_list);
		pool->next_block = &rem->list;
		return mb->mem_ptr;
	}
	return NULL;
}

void myfree(void *ptr)
{
	if (pool == NULL) {
		printf("Invalid memory pool\n");
		return;
	}

	struct mem_block *mb = NULL;
	list_for_each_entry(mb, &pool->mem_block_list, list) {
		if (mb->mem_ptr == *(void **)ptr)
			break;
	}
	mb->is_free = 1;
	*(void **)ptr = NULL;

	struct mem_block *tmp = list_next_entry(mb, list);
	if (tmp != NULL && tmp->is_free) {
		mb->count += tmp->count;
		/* Move from block list to free list */
		list_move_tail(&tmp->list, &pool->free_list);
	}

	tmp = list_prev_entry(mb, list);
	if (tmp != NULL && tmp->is_free) {
		tmp->count += mb->count;
		/* Move from block list to free list */
		list_move_tail(&mb->list, &pool->free_list);
	}
}

void *myrealloc(void *ptr, size_t size)
{
	if (pool == NULL) {
		printf("Invalid memory pool\n");
		return NULL;
	}

	if (ptr == NULL)
		return mymalloc(size);

	/* Find size of block */
	struct mem_block *mb = NULL;
	list_for_each_entry(mb, &pool->mem_block_list, list) {
		if (mb->mem_ptr == ptr)
			break;
	}

	void *new = mymalloc(size);
	if (new == NULL)
		return NULL;

	memcpy(new, ptr, mb->count * BLOCK_SIZE);
	myfree((void *)&ptr);
	return new;
}

void mycleanup(void)
{
	destroy_mem_pool(pool);
}
