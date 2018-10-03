#ifndef LIST_H
#define LIST_H

#define YES 1
#define NO 0

typedef struct list_t list_t;

struct list_t {
	unsigned int size;
	unsigned int capacity;
	void** _data;
};

list_t* list_create(unsigned int capacity);
void list_resize(list_t* list, unsigned int capacity);
void* list_get(list_t* list, unsigned int index);
void list_push(list_t* list, const void* element);
int list_remove(list_t* list, const void* element);
void list_removeat(list_t* list, unsigned int index);
int list_find(list_t* list, const void* element);
void list_clear(list_t* list, unsigned int freedata);
void list_free(list_t* list, unsigned int freedata);

#endif
