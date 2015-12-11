/*
 Copyright (C) 2015 Ruaridh Watt
 ============================================================================
 Name        : llist.h
 Author      : Ruaridh Watt dv12rwt
 Created     : 5th September 2014
 Description : An implementation of a linked list.
 ============================================================================
 */

#ifndef LLIST_H_
#define LLIST_H_

#include <stdbool.h>

typedef struct _element {
	void *data;
	struct _element *next;
} element;

typedef struct _llist {
	element *head;
	void (*free_func)(void *);
} llist;

/*
 * Function: llist_empty
 * ---------------------
 * Creates an empty linked list.
 *
 * Returns: A pointer to an empty linked list.
 */
llist *llist_empty(void (*free_func)(void *));

/*
 * Function: llist_isEmpty
 * -----------------------
 * Checks if a list is empty.
 *
 * list:    A pointer to a linked list.
 *
 * Returns: true if the list is empty, otherwise false.
 */
bool llist_isEmpty(llist *list);

/*
 * Function: llist_first
 * -----------------------
 * Returns a pointer to the first element in the list.
 *
 * list:    A pointer to a linked list.
 *
 * Returns: A pointer to the list's first element struct.
 */
element *llist_first(llist *list);

/*
 * Function: llist_insert
 * -----------------------
 * Inserts data into the linked list at the specified position.
 *
 * pos:     A pointer specifying the position of insertion.
 *
 * list:    The list where the data should be stored.
 *
 * data:    A pointer to the data to be stored.
 *
 * Returns: A pointer to an element, which when inspected will return the data
 *          inserted.
 */
element *llist_insert(element *pos, llist *list, void *data);

/*
 * Function: llist_inspect
 * -----------------------
 * Inspects the data for the specified position.
 *
 * pos:     A pointer specifying the position to be inspected.
 *
 * Returns: A pointer to the data stored for the given position.
 */
void *llist_inspect(element *pos);

/*
 * Function: llist_next
 * -----------------------
 * Returns the next position in the list.
 *
 * pos:     A pointer to the current element.
 *
 * Returns: A pointer to the next element in the list.
 */
element *llist_next(element *pos);

/*
 * Function: llist_isEnd
 * -----------------------
 * Checks if the given position is the end of the list.
 *
 * pos:     An element pointer.
 *
 * Returns: True if the element pointer is the end of the list, otherwise
 *          false. OBS! Inspecting the end element will return NULL.
 */
bool llist_isEnd(element *pos);

/*
 * Function: llist_remove
 * -----------------------
 * Removes an element from the list.
 *
 * pos:     A pointer to the element to remove.
 *
 * dataFreeFunc: A pointer to a function which frees the memory allocated for
 *               the data.
 *
 * Returns: A pointer to the element that now occupies the position removed.
 */
element *llist_remove(element *pos, llist *list);

/*
 * Function: llist_free
 * -----------------------
 * Frees the memory allocated for the list. The dataFreeFunction is performed
 * on every entry in the list.
 *
 * plist: A pointer to the list to be freed.
 *
 * Returns: A pointer to the function to be used to free each data entry.
 */
void llist_free(llist *list);

#endif /* LLIST_H_ */
