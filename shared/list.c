#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

list_t* list_create(unsigned int capacity)
{
	list_t* l = malloc(sizeof(list_t));

	l->_data = malloc(sizeof(void*) * capacity);

	l->size = 0;
	l->capacity = capacity;

	return l;
}

void list_resize(list_t* l, unsigned int capacity)
{
	void** data = (void**)realloc(l->_data, sizeof(void*) * capacity);
	if (data)
	{
		l->_data = data;
		l->capacity = capacity;
	}
}

void* list_get(list_t* l, unsigned int index)
{
	assert(index < l->size);
	return l->_data[index];
}

void list_push(list_t* l, const void* element)
{
	if (l->size == l->capacity)
		list_resize(l, l->capacity * 2);
		
	l->_data[l->size++] = (void*)element;
}

int list_remove(list_t* l, const void* element)
{
	int index = list_find(l, element);
	if (index > -1)
	{
		list_removeat(l, index);
		return 1;
	}
	
	return 0;
}

void list_removeat(list_t* l, unsigned int index)
{
	assert(index < l->size);

	l->_data[index] = NULL;

	for (int i = index; i < l->size - 1; i++)
	{
		l->_data[i] = l->_data[i + 1];
	}

	l->size--;
	if (l->size > 0 && l->size == l->capacity / 4)
	{
		list_resize(l, l->capacity / 2);
	}
}

int list_find(list_t* l, const void* element)
{
	for (int i = 0; i < l->size; i++)
		if (element == l->_data[i])
			return i;

	return -1;
}

void list_clear(list_t* list, unsigned int freedata)
{
	if (freedata == YES)
	{
		for (int i = 0; i < list->size; i++)
			free(list->_data[i]);
	}

	list->size = 0;
	list_resize(list, 10);
}

void list_free(list_t* l, unsigned int freedata)
{
	if (freedata == YES)
	{
		for (int i = 0; i < l->size; i++)
			free(l->_data[i]);
	}

	free(l->_data);
	free(l);
}
