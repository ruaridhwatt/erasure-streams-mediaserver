/*
* llist - a linked list in C.
* Copyright (C) 2015 Ruaridh Watt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*
 ============================================================================
 Name        : llist.c
 Author      : Ruaridh Watt dv12rwt
 Created     : 5th September 2014
 Description : An implementation of a linked list. See llist.h for
               documentation on individual functions.
               List uses an empty head element.
               Inspect returns data from from the element next to the position
               pointer.
               Inspecting the end element will return NULL.
 ============================================================================
 */

#include "llist.h"
#include <stdio.h>
#include <stdlib.h>

llist *llist_empty(void (*dataFreeFunc)(void *)) {
	llist *list = malloc(sizeof(llist));
	if (list == NULL) {
		return NULL;
	}
	list->head = malloc(sizeof(element));
	if (list->head == NULL) {
		return NULL;
	}
	list->head->next = NULL;
	list->head->data = NULL;
	list->free_func = dataFreeFunc;
	return list;
}

bool llist_isEmpty(llist *list) {
	return list == NULL || list->head->next == NULL;
}

element *llist_first(llist *list) {
	return list->head;
}

element *llist_insert(element *pos, llist *list, void *data) {
	element *newElement = malloc(sizeof(element));
	if (newElement == NULL) {
		return NULL;
	}
	newElement->data = data;
	newElement->next = pos->next;
	pos->next = newElement;
	return pos;
}

void *llist_inspect(element *pos) {
	if (llist_isEnd(pos)) {
		return NULL;
	}
	return pos->next->data;
}

element *llist_next(element *pos) {
	return pos->next;
}

bool llist_isEnd(element *pos) {
	return pos->next == NULL;
}

element *llist_remove(element *pos, llist *list) {
	element *el = pos->next;
	if (el != NULL) {
		pos->next = el->next;
		if (list->free_func != NULL) {
			list->free_func(el->data);
		}
		free(el);
	}
	return pos;
}

void llist_free(llist *list) {
	element *pos = llist_first(list);
	while (!llist_isEmpty(list)) {
		pos = llist_remove(pos, list);
	}
	free(list->head);
	free(list);
}
