#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct listItem listItem;
typedef struct LinkedList LinkedList;
typedef struct hashMapItem hashMapItem;
typedef struct HashMap HashMap;

struct listItem {
    void *data;
    listItem *next;
    listItem *prev;
};

struct LinkedList {
    listItem *first;
    listItem *last;
    int elements;
};

struct hashMapItem {
    char *key;
    void *data;
    void (*freeRoutine)(void*);
};

struct HashMap {
    LinkedList *items;
    int buckets;
    int elementCount;
};

void freeLinkedList(LinkedList *lis);
listItem *traverseList(int desiredIndex, int currentIndex, listItem *pointOfList);
void deleteItem(listItem *target);
void insertItem(listItem *new, listItem *prev, listItem *next);

unsigned long hash(unsigned char *str);
HashMap create_hashmap(int buckets);
LinkedList *grabHashMapLocation(HashMap *map, char *key);
void changeHashMap(HashMap *map, int extensionCount);
listItem *grabHashMapItem(HashMap *map, char *key);
void *searchHashMap(HashMap *map, char *key);
void addItemToMap(HashMap *map, void *item, char *key, void (*freeRoutine)(void*));
void deleteItemFromMap(HashMap *map, char *key);
void freeHashMap(HashMap map);

#endif