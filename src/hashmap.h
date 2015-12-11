
/*
 * Copyright (C) 2015 Ruaridh Watt
 */

#ifndef SRC_HASHMAP_H_
#define SRC_HASHMAP_H_

#include "llist.h"

typedef struct _entry {
	void *key;
	size_t key_size;
	void *value;
} entry;

typedef struct _hashmap {
	llist **llist_array; // array of entry lists.
	int array_size;
	int nr_entries;
	unsigned long (*hash_func)(void *);
	void (*entry_free_func)(void *);
} hashmap;

hashmap *hashmap_empty(int initialSize, unsigned long (*hashFunc)(void *), void (*entryFreeFunc)(void *));

int hashmap_put(entry *entry, hashmap *hashmap);

int hashmap_remove(void *key, size_t key_size, hashmap *hashmap);

entry *hashmap_get(void *key, size_t key_size, hashmap *hashmap);

void hashmap_free(hashmap *hashmap);

/* "private" methods */
llist *__get_list(void *key, size_t key_size, hashmap *hashmap, unsigned long *index);

element *__get_position(void *key, size_t key_size, llist *list);

entry *__get_entry(void *key, size_t key_size, hashmap *hashmap);

int __remove(void *key, size_t key_size, llist *list);

#endif /* SRC_HASHMAP_H_ */
