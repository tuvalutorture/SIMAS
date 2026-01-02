#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#include "strings.h"

/* linketh listeth  */
void freeLinkedList(LinkedList *lis) { 
    listItem *current = lis->first; 
    while (current != NULL) { listItem *next = current->next; free(current); current = next; }
}

listItem *traverseList(int desiredIndex, int currentIndex, listItem *pointOfList) {
    listItem *currentPointer = pointOfList; 
    if (currentIndex == desiredIndex) return currentPointer; /* bail early */
    while (currentIndex != desiredIndex && currentPointer != NULL) {
        currentPointer = (currentIndex < desiredIndex) ? currentPointer->next : currentPointer->prev;
        currentIndex += (currentIndex < desiredIndex) ? 1 : -1;
    }
    return currentPointer;
}

void deleteItem(listItem *target) { /* you GOTTA free the data inside it first, mind you */
    listItem *nextTemp = target->next, *prevTemp = target->prev;
    free(target); 
    if (prevTemp != NULL) prevTemp->next = nextTemp;
    if (nextTemp != NULL) nextTemp->prev = prevTemp;
} 

void insertItem(listItem *new, listItem *prev, listItem *next) { /* sandwiches it betwixt two elements */
    if (prev != NULL) prev->next = new;
    if (next != NULL) next->prev = new;
    new->prev = prev; new->next = next;
}

/* hatchmap     */
unsigned long hash(unsigned char *str) { /* wonderful lil algorithm called djb2. written by someone much smarter than me. used in programming for ages. no fuckign clue how it works. */
    unsigned long brickofhash = 5381; int c;
    while ((c = *str++)) brickofhash = ((brickofhash << 5) + brickofhash) + c; /* hash * 33 + c */
    return brickofhash;
}

HashMap create_hashmap(int buckets) { /* CONSTRUCTORS!? OBJECTS!? IN MY C CODE!? WHAT THE FUCK IS THIS, JAVA!? */
    HashMap new;
    new.buckets = buckets;
    new.elementCount = 0;
    new.items = (LinkedList *)calloc(buckets, sizeof(LinkedList));
    return new;
}

LinkedList *grabHashMapLocation(HashMap *map, char *key) {
    unsigned long index = hash((unsigned char *)key) % map->buckets;
    DEBUG_PRINTF("index: %lu", index);
    return &map->items[index];
}

void changeHashMap(HashMap *map, int extensionCount) {
    int i, newMax = map->buckets + extensionCount; HashMap temp; 
    if (extensionCount == 0) return;
    temp = create_hashmap(newMax);
    for (i = 0; i < map->buckets; i++) {
        listItem *loc = map->items[i].first;
        if (loc == NULL) { continue; } /* if there ain't a first element, there ain't no fuckin' data */
        while (loc != NULL) {
            hashMapItem *current = loc->data; listItem *next = loc->next;
            LinkedList *newSpot = grabHashMapLocation(&temp, current->key);
            loc->next = NULL; loc->prev = NULL;
            if (newSpot->first == NULL) newSpot->first = loc; 
            else { insertItem(loc, newSpot->last, NULL); }
            newSpot->last = loc;
            loc = next;
        }
    }
    map->buckets = newMax;
    free(map->items);
    map->items = temp.items; map->buckets = newMax;
}

listItem *grabHashMapItem(HashMap *map, char *key) {
    listItem *current = grabHashMapLocation(map, key)->first;
    while (1) { 
        hashMapItem *check;
        if (current == NULL) return NULL;
        check = (hashMapItem *)current->data; 
        if (check->key != NULL && !strcmp(check->key, key)) { DEBUG_PRINTF("found %s key\n", check->key); return current;}
        current = current->next; 
    } 
}

void *searchHashMap(HashMap *map, char *key) {
    listItem *loc = grabHashMapItem(map, key);
    if (loc != NULL) return ((hashMapItem *)loc->data)->data;
    else return NULL;
}

void addItemToMap(HashMap *map, void *item, char *key, void (*freeRoutine)(void*)) {
    LinkedList *location = grabHashMapLocation(map, key); listItem *newListItem; hashMapItem *newItem;
    if ((float)map->elementCount / (float)map->buckets > 0.75) { /* sex i3 piston - bringal */
        changeHashMap(map, map->buckets);  
        addItemToMap(map, item, key, freeRoutine); /* ooooo recursion */
        return; /* nvm no recursive functions here */
    }
    newListItem = (listItem *)calloc(1, sizeof(listItem));
    newItem = (hashMapItem *)malloc(sizeof(hashMapItem));
    newItem->data = item; newItem->key = stroustrup(key); newItem->freeRoutine = freeRoutine;
    newListItem->data = newItem;
    DEBUG_PRINTF("added item with key %s\n", key);
    if (location->first == NULL) { location->first = newListItem; }
    else { insertItem(newListItem, location->last, NULL); }
    location->last = newListItem;
    map->elementCount += 1; 
}

void deleteItemFromMap(HashMap *map, char *key) {
    listItem *nuked = grabHashMapItem(map, key); LinkedList *point = grabHashMapLocation(map, key);
    DEBUG_PRINT(((hashMapItem *)nuked->data)->key); free(((hashMapItem *)nuked->data)->key);
    if (((hashMapItem *)nuked->data)->freeRoutine != NULL) ((hashMapItem *)nuked->data)->freeRoutine(((hashMapItem *)nuked->data)->data);
    free(nuked->data); 
    if (point->first == nuked) point->first = nuked->next != NULL ? nuked->next : NULL;
    if (point->last == nuked) point->last = nuked->prev != NULL ? nuked->prev : NULL;
    deleteItem(nuked); 
    map->elementCount -= 1;
}

void freeHashMap(HashMap map) { /* Noli manere in memoria - Saevam iram et dolorem - Ferum terribile fatum - Ille iterum veniet */
    int i;
    if (map.items == NULL) return;
    for (i = 0; i < map.buckets; i++) {
        listItem *current = map.items[i].first;
        if (current == NULL) continue;
        while (current != NULL) {
            hashMapItem *item = (hashMapItem *)current->data;
            DEBUG_PRINTF("freeing %s key\n", item->key);
            free(item->key); 
            if (item->freeRoutine) item->freeRoutine(item->data); 
            free(item);
            current = current->next;
        }
        freeLinkedList(&map.items[i]); 
    }
    free(map.items);
}