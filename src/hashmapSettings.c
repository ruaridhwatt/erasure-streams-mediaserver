#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmapSettings.h"

unsigned long string_hash_function(void *data) {
	char *str = (char*) data;
	unsigned long hash = 5381;
	int i = 0;
	char c = str[i];
	i++;

	while (c != '\0') {
		hash = ((hash << 5) + hash) + c;
		c = str[i];
		i++;
	}

	return hash;
}

entry *create_entry(char *k, char *v) {
	entry *en = malloc(sizeof(entry));
	if (!en) {
		return NULL;
	}

	en->key_size = strlen(k) + 1;
	en->key = calloc(en->key_size, sizeof(char));
	if (!en->key) {
		free(en);
		return NULL;
	}
	strcpy(en->key, k);

	en->value = calloc(strlen(v) + 1, sizeof(char));
	if (!en->value) {
		free(en->key);
		free(en);
		return NULL;
	}
	strcpy(en->value, v);
	return en;
}

void entry_free_func(void *data) {
	entry *e = (entry *) data;
	free(e->key);
	free(e->value);
	free(e);
}
