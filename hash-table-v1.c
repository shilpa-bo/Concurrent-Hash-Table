#include "hash-table-base.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include <pthread.h>

static pthread_mutex_t hash_lock;

struct list_entry {
	const char *key;
	uint32_t value;
	SLIST_ENTRY(list_entry) pointers;
};

SLIST_HEAD(list_head, list_entry);

struct hash_table_entry { //each entry has a list head associated with it- maybe also a lock?
	struct list_head list_head;
};

struct hash_table_v1 {  //creates the hash table and an array of hash table entries: this is the hash tbale
	struct hash_table_entry entries[HASH_TABLE_CAPACITY];
};

struct hash_table_v1 *hash_table_v1_create()
{
	struct hash_table_v1 *hash_table = calloc(1, sizeof(struct hash_table_v1));
	assert(hash_table != NULL);
	if (pthread_mutex_init(&hash_lock, NULL)!=0){
		exit(EXIT_FAILURE);
	}
	for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
		struct hash_table_entry *entry = &hash_table->entries[i];
		SLIST_INIT(&entry->list_head);
	}
	return hash_table;
}

//calculates index using a hash function on the key
//returns the bucket corresponding (uses index to get bucket from hash table)
static struct hash_table_entry *get_hash_table_entry(struct hash_table_v1 *hash_table,
                                                     const char *key)
{
	assert(key != NULL);
	uint32_t index = bernstein_hash(key) % HASH_TABLE_CAPACITY;
	struct hash_table_entry *entry = &hash_table->entries[index];
	return entry;
}

//makes sure we have a valid key
//loop through the linked list of list entries starting at respective list head 
//if the entry exists (strcmp)m then return the entry, otherwise return null
static struct list_entry *get_list_entry(struct hash_table_v1 *hash_table,
                                         const char *key,
                                         struct list_head *list_head)
{
	assert(key != NULL);

	struct list_entry *entry = NULL;
	
	SLIST_FOREACH(entry, list_head, pointers) {
	  if (strcmp(entry->key, key) == 0) {
	    return entry;
	  }
	}
	return NULL;
}

bool hash_table_v1_contains(struct hash_table_v1 *hash_table,
                            const char *key)
{
	struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);
	struct list_head *list_head = &hash_table_entry->list_head;
	struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);
	return list_entry != NULL;
}

//ONLY CHANGE THIS
void hash_table_v1_add_entry(struct hash_table_v1 *hash_table,
                             const char *key,
                             uint32_t value)
{

	//LOCK HERE
	int lock_return_val = pthread_mutex_lock(&hash_lock);
	if (lock_return_val!=0){
		exit(lock_return_val);
	}
	//locating the bucket that we insert the element into 
	struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);
	struct list_head *list_head = &hash_table_entry->list_head; //pointing the list_head to the bucket we want to insert into
	
	struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);

	/* Update the value if it already exists */
	if (list_entry != NULL) {
		list_entry->value = value;
		int unlock_return_val = pthread_mutex_unlock(&hash_lock)!=0;
		if(unlock_return_val!=0){
			exit(unlock_return_val);
		}
		return;
	}

	list_entry = calloc(1, sizeof(struct list_entry));
	list_entry->key = key;
	list_entry->value = value;
	SLIST_INSERT_HEAD(list_head, list_entry, pointers);
	int unlock_return_val = pthread_mutex_unlock(&hash_lock)!=0;
	if(unlock_return_val!=0){
		exit(unlock_return_val);
	}


}

uint32_t hash_table_v1_get_value(struct hash_table_v1 *hash_table,
                                 const char *key)
{
	struct hash_table_entry *hash_table_entry = get_hash_table_entry(hash_table, key);
	struct list_head *list_head = &hash_table_entry->list_head;
	struct list_entry *list_entry = get_list_entry(hash_table, key, list_head);
	assert(list_entry != NULL);
	return list_entry->value;
}

void hash_table_v1_destroy(struct hash_table_v1 *hash_table)
{
	if (pthread_mutex_destroy(&hash_lock)!=0){ //don't really get how to know which exit calls to use
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < HASH_TABLE_CAPACITY; ++i) {
		struct hash_table_entry *entry = &hash_table->entries[i];
		struct list_head *list_head = &entry->list_head;
		struct list_entry *list_entry = NULL;
		while (!SLIST_EMPTY(list_head)) {
			list_entry = SLIST_FIRST(list_head);
			SLIST_REMOVE_HEAD(list_head, pointers);
			free(list_entry);
		}
	}
	free(hash_table);
	
}
