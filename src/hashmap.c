/*
 * rwc-hashmap - A hashmap in C.
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

#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

hashmap *hashmap_empty(int initialSize, unsigned long (*hashFunc)(void *), void (*entryFreeFunc)(void *)) {

	hashmap *hm = (hashmap *) malloc(sizeof(hashmap));
	if (hm == NULL) {
		return NULL;
	}
	hm->llist_array = (llist **) calloc(initialSize, sizeof(llist *));
	if (hm->llist_array == NULL) {
		return NULL;
	}
	hm->array_size = initialSize;
	hm->nr_entries = 0;
	hm->entry_free_func = entryFreeFunc;
	hm->hash_func = hashFunc;
	return hm;
}

int hashmap_put(entry *entry, hashmap *hashmap) {
	unsigned long index;
	llist *list = __get_list(entry->key, entry->key_size, hashmap, &index);
	if (!list) {
		hashmap->llist_array[index] = llist_empty(hashmap->entry_free_func);
		list = hashmap->llist_array[index];
		if (!list) {
			return EXIT_FAILURE;
		}
	} else if (__remove(entry->key, entry->key_size, list)) {
		hashmap->nr_entries--;
	}
	element *pos = llist_first(list);
	pos = llist_insert(pos, list, entry);
	if (!pos) {
		return EXIT_FAILURE;
	}
	hashmap->nr_entries++;
	return EXIT_SUCCESS;
}

entry *hashmap_get(void *key, size_t key_size, hashmap *hashmap) {
	entry *en = __get_entry(key, key_size, hashmap);
	if (!en) {
		return NULL;
	}
	return en->value;
}

int hashmap_remove(void *key, size_t key_size, hashmap *hashmap) {
	llist *list = __get_list(key, key_size, hashmap, NULL);
	if (!list) {
		return 0;
	}
	if (__remove(key, key_size, list)) {
		hashmap->nr_entries--;
		return 1;
	}
	return 0;
}

void hashmap_free(hashmap *hashmap) {
	int i;
	for (i = 0; i < hashmap->array_size; i++) {
		if (hashmap->llist_array[i] != NULL) {
			llist_free(hashmap->llist_array[i]);
		}
	}
	free(hashmap->llist_array);
	free(hashmap);
}

llist *__get_list(void *key, size_t key_size, hashmap *hashmap, unsigned long *index) {
	llist *list;
	if (index) {
		*index = hashmap->hash_func(key) % hashmap->array_size;
		list = hashmap->llist_array[*index];
	} else {
		unsigned long i = hashmap->hash_func(key) % hashmap->array_size;
		list = hashmap->llist_array[i];
	}
	return list;
}

element *__get_position(void *key, size_t key_size, llist *list) {
	if (!list) {
		return NULL;
	}
	element *pos = llist_first(list);
	entry *en = (entry *) llist_inspect(pos);
	while (en != NULL && (en->key_size != key_size || memcmp(key, en->key, key_size))) {
		pos = llist_next(pos);
		en = (entry *) llist_inspect(pos);
	}
	return pos;
}

entry *__get_entry(void *key, size_t key_size, hashmap *hashmap) {
	llist *list = __get_list(key, key_size, hashmap, NULL);
	if (!list) {
		return NULL;
	}
	element *pos = __get_position(key, key_size, list);
	return (entry *) llist_inspect(pos);
}

int __remove(void *key, size_t key_size, llist *list) {
	element *pos = __get_position(key, key_size, list);
	if (llist_isEnd(pos)) {
		return 0;
	}
	llist_remove(pos, list);
	return 1;
}
