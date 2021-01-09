#ifndef __LIST_H
#define __LIST_H

#include <types.h>
#include <kmalloc.h>

typedef struct _list_item_t {
  struct list_item_t* prev;
  struct list_item_t* next;
  void*  data;
} list_item_t;


typedef struct _list_t {
  list_item_t* start;
  list_item_t* iterator;
  uint16_t     count;
} list_t;

list_t*  list_create();
void list_destroy(list_t* lst);

int  list_push_back(list_t* lst,void* data);
int  list_push_front(list_t* lst,void* data);

list_item_t* list_iterator(list_t* list);
int          list_has_next(list_t* list);
list_item_t* list_next(list_t* list);
void         list_remove_item(list_t* lst,list_item_t* item);

/* Defining macros for passing the linked list */
#define foreach(x,y) for(x=y->start;x;x=(list_item_t*)x->next)

#endif

