#ifndef MY_MALLOC_H_INCLUDED
#define MY_MALLOC_H_INCLUDED

#include <stddef.h>

#define FIRST_FIT	0
#define NEXT_FIT	1
#define BEST_FIT	2

void myinit(int allocAlg);

void *mymalloc(size_t size);

void myfree(void *ptr);

void *myrealloc(void *ptr, size_t size);

void mycleanup(void);

/* Basic type for the double-link list. */
struct list_head {
	struct list_head *next, *prev;
};

/* Initialize a new list head. */
#define INIT_LIST_HEAD(ptr) \
	(ptr)->next = (ptr)->prev = (ptr)

#define LIST_HEAD_INIT(name) { &(name), &(name) }

/**
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/* Add new element at the head of the list. */
static inline void list_add(struct list_head *newp, struct list_head *head)
{
	head->next->prev = newp;
	newp->next = head->next;
	newp->prev = head;
	head->next = newp;
}

/* Add new element at the tail of the list. */
static inline void list_add_tail(struct list_head *newp, struct list_head *head)
{
	head->prev->next = newp;
	newp->next = head;
	newp->prev = head->prev;
	head->prev = newp;
}

/* Remove element from list. */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/* Remove element from list. */
static inline void list_del(struct list_head *elem)
{
	__list_del(elem->prev, elem->next);
}

/* Delete from list, add to another list as head. */
static inline void list_move(struct list_head *elem, struct list_head *head)
{
	__list_del(elem->prev, elem->next);
	list_add(elem, head);
}

/* Delete from list, add to another list as head. */
static inline void list_move_tail(struct list_head *elem, struct list_head *head)
{
	__list_del(elem->prev, elem->next);
	list_add_tail(elem, head);
}

/* Replace an old entry. */
static inline void list_replace(struct list_head *old, struct list_head *newp)
{
	newp->next = old->next;
	newp->prev = old->prev;
	newp->prev->next = newp;
	newp->next->prev = newp;
}

/* Get typed element from list at a given position. */
#define list_entry(ptr, type, member) \
	((type *) ((char *) (ptr) - offsetof(type, member)))

/* Get first entry from a list. */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/**
 * list_next_entry - get the next element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * list_prev_entry - get the prev element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

/* Iterate forward over the elements of the list. */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * Iterate forward over the elements list. The list elements can be
 * removed from the list while doing this.
 */
#define list_for_each_safe(pos, p, head) \
	for (pos = (head)->next, p = pos->next; \
		pos != (head); \
		pos = p, p = pos->next)

/* Iterate backward over the elements of the list. */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * Iterate backwards over the elements list. The list elements can be
 * removed from the list while doing this.
 */
#define list_for_each_prev_safe(pos, p, head) \
	for (pos = (head)->prev, p = pos->prev; \
		pos != (head); \
		pos = p, p = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member)		\
	for (pos = list_next_entry(pos, member);			\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member)			\
	for (; &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     &pos->member != (head);			\
	     pos = n, n = list_next_entry(n, member))

static inline int list_empty(const struct list_head *head)
{
	return head == head->next;
}

#endif /* MY_MALLOC_H_INCLUDED */
