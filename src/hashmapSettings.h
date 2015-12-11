#include "llist.h"
#include "hashmap.h"
unsigned long string_hash_function(void *data);
entry *create_entry(char *k, char *v);
void entry_free_func(void *data);
