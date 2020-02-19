#ifndef _LIST_H
#define _LIST_H

struct list_head {
	struct list_head *next, *prev;
};

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

#define LIST_POISON1	((void *) 0x00100100)
#define LIST_POISON2	((void *) 0x00200200)

static inline void list_del(struct list_head *entry)
{
	__list_del_entry(entry);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member)				\
	container_of(ptr, type, member)

#define list_first_entry_or_null(ptr, type, member) ({		\
	struct list_head *head__ = (ptr);			\
	struct list_head *pos__  = head__->next;		\
	pos__ != head__ ? list_entry(pos__, type, member) : 	\
						NULL;})

#define list_first_entry(ptr, type, member)			\
	list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member)				\
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each_entry(pos, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member);\
	     &pos->member != (head);				\
	     pos = list_next_entry(pos, member))

#endif
