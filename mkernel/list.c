/*
  _           _ 
   | . |   | |_|   Linked list functions
  \  | | | | |  
 
*/
#include <list.h>

/**
 *  Create linked list and return pointer
 *  to created list 'object'
 */
list_t* list_create(){
  list_t* lst = (list_t*) kmalloc(sizeof(list_t));
  lst->start = NULL;
  lst->count = 0;
  return lst;
}


/**
 *  Delete linked list
 *   also it's function delete all created list items
 */
void list_destroy(list_t* list){
  list_t* tmp;
  list_item_t* p,* l;
  l = p = list->start;
  for(;p;p=l) {
	l = (list_item_t*) p->next;
	kfree(p);
  }
  tmp = list;
  kfree(tmp);
}


/**
 *  Push data at the end of linked list
 *   data wrap by linked list item (list_item_t)
 *  return number of list items
 */
int list_push_back(list_t* lst,void* data){
  list_item_t* item,* p;
	
  if(!lst) return 0;
  item = (list_item_t*) kmalloc(sizeof(list_item_t));
  item->data = data;
  item->next = item->prev = NULL;

  if(!lst->count){
	lst->start = item;
  } else {
   /* go to the end */
	for(p=lst->start;p->next;p=(list_item_t*) p->next);
	p->next = (struct list_item_t*) item;
	item->prev = (struct list_item_t*) p;
	item->next = NULL;
  }
  
  return lst->count++;
}


/**
 *  Push data at the start of linked list
 *   data wrap by linked list item (list_item_t)
 *  return number of list items
 */
int list_push_front(list_t* lst,void* data){
  list_item_t* item,* p;
	
  if(!lst) return 0;
  item = (list_item_t*) kmalloc(sizeof(list_item_t));
  item->data = data;
  item->next = item->prev = NULL;

  if(!lst->count){
	lst->start = item;
  } else {
	p = lst->start;
	p->prev = (struct list_item_t*) item;
	item->next = (struct list_item_t*) p;
	lst->start = item;
  }
  
  return lst->count++;
}


/**
 * Initialize and return iterator
 */
list_item_t* list_iterator(list_t* list){
  list->iterator = list->start;
  return list->iterator;
}


/**
 * Is it iterator has next element ?
 */
int list_has_next(list_t* list){
  return (list->iterator->next!=NULL);
}


/**
 * Get next element in the list
 * list_iterator() must be called untill that
 */
list_item_t* list_next(list_t* list){
  list_item_t* p;
  p = list->iterator;
  list->iterator = (list_item_t*) list->iterator->next;
  return p;
}


/**
 * Remove item from the list
 */
void list_remove_item(list_t* lst,list_item_t* li){
  list_item_t* p, *n;

  if(!li) return;
  p = (list_item_t*) li->prev;
  n = (list_item_t*) li->next;
	
  if(!p) {
	lst->start = (list_item_t*) n;
	if(n) n->prev = NULL;
  }
  else {
	p->next = (struct list_item_t*) n;
	if(n) n->prev = (struct list_item_t*) p;
  }
  lst->count--;
}
