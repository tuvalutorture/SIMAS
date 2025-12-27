/* CMAS (C SIMAS "SIMple ASsembly") interpreter - by tuvalutorture  */

/* the automobile seatbelt was invented by John Lennon the CCXXVII  */
/* in 375 BC and 204 years later his child, John Bing the MCLXXVI   */
/* of Cornholio invented the windshield wiper in the year of our    */
/* lord 171 BC, but their inventions were lost to time in the year  */
/* 582 ACDC, and were only just now redicovered in the present day. */

/*                           Stuttering                             */
/*                          Cold and damp                           */
/*                 Steal the warm wind, tired friend                */
/*                          Times are gone                          */
/*                          For honest men                          */
/*                 Sometimes, far too long for snakes               */
/*                           In my shoes                            */
/*                          Walking sleep                           */
/*                   In my youth, I pray to keep                    */
/*                           Heaven send                            */
/*                            Hell away                             */
/*                   No one sings like you anymore                  */
/*    can the sun just fucking collapse into a black hole already   */ 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define STR 1
#define NUM 2
#define BOOL 3
#define LIST 4
#define IN 5

#define DEBUG_PRINTF if (debugMode) printf /* macro abuse at its finest */
#define DEBUG_PRINT(string) if (debugMode) puts(string)
/* the preprocessor is awesome */
#define ALLOC_DEBUGGING 0

int debugMode = 0;

typedef struct listItem listItem;
typedef struct LinkedList LinkedList;
typedef struct hashMapItem hashMapItem;
typedef struct HashMap HashMap;

typedef struct variable variable;
typedef struct instruction instruction;
typedef struct openFile openFile;
typedef struct operator operator;
typedef struct InstructionSet InstructionSet;
typedef struct command command;

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
    int maxElements;
    int elementCount;
};

struct command { 
    char *(*commandPointer)(instruction*, openFile*); 
};

union variableData { /* UNIONISE, MY CHILDREN! RISE AGAINST THE EMPLOYERS WHO TREAT YOU AS WAGE SLAVES! BECOME A UNION! FIGHT FOR WORKPLACE RIGHTS! GET YO 401(k)!!! */
    double num;
    char *str;
    int bool;
    void *etc;
}; 

struct variable {
    int type;
    union variableData data;
};

struct instruction {
    char *operation;
    int argumentCount;
    char **arguments;
    char *prefix;
};

struct openFile {
    char *path;
    instruction **instructions;
    HashMap variables;
    HashMap labels;
    HashMap lists; 
    int instructionCount;
    int programCounter;
};

struct operator {
    char *name;
    char *prefix;
    void (*functionPointer)(void*); /* guys i think this points or smth idk */ 
    int minArgs;
};

struct InstructionSet {
    operator *set;
    char **prefixes;
    int count;
    int prefixCount;
};

InstructionSet ValidInstructions;
HashMap ValidCommands;

/* preprocessor fuckery so that i can debug exactly where all my damn allocations are   */
/* set the debugging define to 0 if you don't want the cycle loss with this             */
#if ALLOC_DEBUGGING == 1

FILE *allocatorLog;
int allocationCounter = 0;

void *mallocate(size_t size) {
    allocationCounter += 1;
    fprintf(allocatorLog, "allocated %d bytes (allocation #%d)\r\n", (int)size, allocationCounter);
    DEBUG_PRINTF("allocated %d bytes (allocation #%d)\r\n", (int)size, allocationCounter);
    return malloc(size);
}

void *callocate(size_t count, size_t size) {
    allocationCounter += 1;
    fprintf(allocatorLog, "cleared & allocated %d bytes(allocation #%d)\r\n", (int)size * (int)count, allocationCounter);
    DEBUG_PRINTF("cleared & allocated %d bytes(allocation #%d)\r\n", (int)size * (int)count, allocationCounter);
    return calloc(count, size);
}

void *reallocate(void *block, size_t size) {
    allocationCounter += 1;
    fprintf(allocatorLog, "reallocated %d bytes(allocation #%d)\r\n", (int)size, allocationCounter);
    DEBUG_PRINTF("reallocated %d bytes(allocation #%d)\r\n", (int)size, allocationCounter);
    return realloc(block, size);
}

#else 

#define mallocate(size) malloc(size)
#define callocate(count, size) calloc(count, size)
#define reallocate(block, size) realloc(block, size)

#endif

void executeFile(openFile *current, int doFree); /* forward */
void setUpCommands();
void beginCommandLine(char *entryMsg, openFile *passed);
void addItemToMap(HashMap *map, void *item, char *key, void (*freeRoutine)(void*));

void dummy() { float f=0,*fp; fp=&f; printf("%f",*fp); } /* only needed for retarded systems like turbo c to trick it into bringing in float libs */

void toggleDebugMode() {
    debugMode = !debugMode;
    if (debugMode) { puts("debug mode enabled"); }
    else { puts("debug mode disabled"); }
}

char *strdup(const char *string) {
    int len = strlen(string) + 1;
    char *final = (char *)mallocate(len * sizeof(char));
    if (!final) return NULL;
    memcpy(final, string, len);
    return final;
}

unsigned long hash(unsigned char *str) { /* wonderful lil algorithm called djb2. written by someone much smarter than me. used in programming for ages. no fuckign clue how it works. */
    unsigned long hashbrick = 5381; int c;
    while ((c = *str++)) hashbrick = ((hashbrick << 5) + hashbrick) + c; /* hash * 33 + c */
    return hashbrick;
}

void freeAndPrint(char *allocated) { printf("%s", allocated); free(allocated); }

void freeInstruction(instruction *inst) {
    int i;
    if (inst == NULL) return;
    DEBUG_PRINTF("freeing %s instruction\n", inst->operation);
    free(inst->operation);
    for (i = 0; i < inst->argumentCount; i++) {
        DEBUG_PRINTF("freeing %s arg\n", inst->arguments[i]); free(inst->arguments[i]);
    }
    free(inst->arguments);
    if (inst->prefix != NULL) free(inst->prefix);
    free(inst);
}

void freeVariable(variable *var) { if (var->type == STR && var->data.str) { free(var->data.str); } free(var); }
void freeLinkedList(LinkedList *lis) { listItem *current = lis->first; while (current != NULL) { listItem *next = current->next; free(current); current = next; }}
void freeList(LinkedList *lis) { int i; listItem *current = lis->first; for (i = 0; i < lis->elements; i++) { listItem *next = current->next; freeVariable((variable *)current->data); current = next; } freeLinkedList(lis); free(lis); }
void freeOperator(operator op) { if (op.name != NULL) { DEBUG_PRINTF("freeing %s operator\n", op.name); free(op.name); } if (op.prefix != NULL) { DEBUG_PRINTF("freeing %s prefix\n", op.prefix); free(op.prefix); }}
void freeInstructionSet(InstructionSet isa) { int i; if (isa.set != NULL) { for (i = 0; i < isa.count; i++) { freeOperator(isa.set[i]); } free(isa.set); } if (isa.prefixes != NULL) { for (i = 0; i < isa.prefixCount; i++) { free(isa.prefixes[i]); } free(isa.prefixes); }}
void freeHashMap(HashMap map) {
    int i;
    if (map.items == NULL) return;
    for (i = 0; i < map.maxElements; i++) {
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
void freeFile(openFile file) {
    int i;
    DEBUG_PRINT("freeing instructions\n");
    if (file.instructions != NULL) { for (i = 0; i < file.instructionCount; i++) { freeInstruction(file.instructions[i]); } free(file.instructions); }
    DEBUG_PRINT("freeing vars\n");
    freeHashMap(file.variables);
    DEBUG_PRINT("freeing labels\n");
    freeHashMap(file.labels);
    DEBUG_PRINT("freeing lists\n");
    freeHashMap(file.lists);
    if (file.path != NULL) { free(file.path); }
}

void cleanFile(openFile *file) { /* cleans a file for re-execution */
    freeHashMap(file->variables); freeHashMap(file->lists); freeHashMap(file->labels);
    file->labels.items = NULL; file->lists.items = NULL; file->variables.items = NULL; file->programCounter = 0;
}

char *stripSemicolon(char *input) { int position; char *string = strdup(input); if (strlen(input) == 0) { return string; } position = (int)strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; return string; }
char *lowerize(char *input) { int i, len; char *string = strdup(input); if (strlen(input) == 0) { return string; } len = (int)strlen(string); for (i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); } return string; }
void lowerizeInPlace(char *string) { int i, len = (int)strlen(string); for (i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); }}
void stripSemicolonInPlace(char *string) { int i, len = (int)strlen(string); for (i = 0; i < len; i++) { if (string[i] == ';') { string[i] = '\0'; }}}

int findNumberArgs(char *instruction, InstructionSet isa) { int i; for (i = 0; i < isa.count; i++) { if (strcmp(isa.set[i].name, instruction) == 0) { return isa.set[i].minArgs; }} return -1; }

void cry(char *msg) { puts(msg); exit(2847172); }
void handleError(char *errorMsg, int errCode, int fatal, openFile *file) {
    if (fatal) {
        printf("Fatal error: %s of code %d\nPress 'enter' to quit...\n", errorMsg, errCode);
        freeFile(*file);
        freeInstructionSet(ValidInstructions);
        getchar();
        exit(errCode); /* theres no real errcodes but we're gonna pretend we do */
    } else {
        char string[512];
        sprintf(string, "A non-fatal error %d (%s) has occurred at line %d. \nYou are being entered into the SIMAS command line.\nType \"!help\" for a list of helpful commands.\n", errCode, errorMsg, file->programCounter);
        cleanFile(file);
        beginCommandLine(string, file);
    }
}

int trueOrFalse(char *string) {
    char *check = lowerize(string); int value = 0;
    char *check = lowerize(string); int value = 0;
    if (strcmp(check, "true") == 0) { value = 1; }
    else if (strcmp(check, "false") == 0) { value = 0; }
    free(check);
    return value;
}

void set_variable_value(variable *var, int type, char *value, double num, int bool) { /* too fucking lazy to pass in one at a time or wutever, so just pass in all of them manually, even if some are blank :3 */
    if (var->type == STR) free(var->data.str); 
    var->data.str = NULL;
    if (type == NUM) { var->data.num = num; }
    else if (type == BOOL) { var->data.bool = bool; }
    else if (type == STR) {
        if (!value) cry("No value passed in!\n");
        var->data.str = strdup(value);
        if (var->data.str == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    }
    var->type = type;
    DEBUG_PRINTF("\nvariable now has value %s, %f, %d\n", value, (float)num, bool);
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

instruction *add_instruction(char *inst, char *arguments[], char *prefix, int args) {
    int i; char *ins = stripSemicolon(inst);
    instruction *instruct = (instruction *)mallocate(sizeof(instruction));
    if (args >= 1) { instruct->arguments = (char **)mallocate(sizeof(char*) * args); for (i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }}
    else { instruct->arguments = NULL; }
    instruct->operation = strdup(ins); instruct->argumentCount = args;
    if (prefix != NULL) { instruct->prefix = strdup(prefix); }
    else { instruct->prefix = NULL; }
    DEBUG_PRINTF("added instruction %s of arg count %d\n", instruct->operation, instruct->argumentCount);
    free(ins);
    return instruct;
}

operator create_operator(char *name, char *prefix, void (*functionPointer)(void*), int minimumArguments) { /* CONSTRUCTORS!? OBJECTS!? IN MY C CODE!? WHAT THE FUCK IS THIS, JAVA!? */
    operator op;
    op.name = strdup(name);
    op.prefix = (prefix != NULL) ? strdup(prefix) : NULL; 
    op.functionPointer = functionPointer; 
    op.minArgs = minimumArguments;
    return op;
}

HashMap create_hashmap(int maxElements) { /* genuinely one of the only times java might be good for shit ngl */
    HashMap new;
    new.maxElements = maxElements;
    new.elementCount = 0;
    new.items = (LinkedList *)callocate(maxElements, sizeof(LinkedList));
    return new;
}

void addOperator(operator *op, InstructionSet *isa) {
    isa->set = (operator *)reallocate(isa->set, (isa->count + 1) * sizeof(operator));
    isa->set[isa->count] = *op;
    isa->count += 1;
}

void addOperatorSet(operator *ops, InstructionSet *isa, int count) {
    isa->set = (operator *)reallocate(isa->set, (isa->count + count) * sizeof(operator));
    memcpy(isa->set + isa->count, ops, count * sizeof(operator));
    isa->count += count;
}

void addPrefix(char *prefix, InstructionSet *isa) {
    isa->prefixes = (char **)reallocate(isa->prefixes, (isa->prefixCount + 1) * sizeof(char *));
    isa->prefixes[isa->prefixCount] = strdup(prefix);
    isa->prefixCount += 1;
}

void varcpy(variable *dest, variable *src) { 
    if (src->type == NUM) set_variable_value(dest, src->type, NULL, src->data.num, 0); 
    if (src->type == STR) set_variable_value(dest, src->type, src->data.str, 0.0, 0); 
    if (src->type == BOOL) set_variable_value(dest, src->type, NULL, 0.0, src->data.bool); 
}

int grabType(char *input) {
    char *type = lowerize(input);
    if (strcmp(type, "str") == 0) { free(type); return STR; }
    else if (strcmp(type, "num") == 0) { free(type); return NUM; }
    else if (strcmp(type, "bool") == 0) { free(type); return BOOL; }
    else if (strcmp(type, "in") == 0) { free(type); return IN; }
    else { free(type); return -1; }
}

void appendElementToList(LinkedList *li, variable var) {
    listItem *new = (listItem *)mallocate(sizeof(listItem));
    new->data = (variable *)callocate(1, sizeof(variable));
    new->prev = NULL; new->next = NULL;
    varcpy((variable *)new->data, &var);
    if (li->elements == 0) { li->first = new; }
    else insertItem(new, li->last, NULL);
    li->last = new; li->elements += 1;
}

void removeElementFromList(LinkedList *li, int element) {
    listItem *target;
    if (element < 1) { 
        target = li->first; 
        if (li->first->next != NULL) { 
            li->first = li->first->next; 
            li->first->prev = NULL;
        }
    } 
    else { target = traverseList(element - 1, 0, li->first); }
    if (target == li->last && li->last != li->first) li->last = target->prev;
    freeVariable((variable *)target->data);
    li->elements -= 1;
    deleteItem(target);
}

char *grabStringOfNumber(double num) {
    int i, len, zeroes = 0; char buffer[331], *final;
    sprintf(buffer, "%f", (float)num); /* gatta love that unsafety hehehe... it'll be fine */ 
    len = strlen(buffer);
    for (i = 1; i < 7; i++) { if (buffer[len - i] == '0') { zeroes++; buffer[len - i] = '\0'; } else break; } /* yoink trailing zeroes */
    if (zeroes == 6) { buffer[len - 7] = '\0'; } /* terminate it if it's just trailing zeroes */
    final = (char *)mallocate(strlen(buffer) * sizeof(char) + 1);
    strcpy(final, buffer);
    return final;
}

size_t grabLengthOfNumber(double num) { char *temp = grabStringOfNumber(num); size_t len = strlen(temp); free(temp); return len; }

size_t stringLenFromVar(variable var) {
    if (var.type == STR) { return strlen(var.data.str); }
    else if (var.type == BOOL) { return var.data.bool ? strlen("true") : strlen("false"); }
    else if (var.type == NUM) { return grabLengthOfNumber(var.data.num); }
    else { return 0; }
}

char *stringFromVar(variable var) {
    if (var.type == STR) { return strdup(var.data.str); }
    else if (var.type == BOOL) { return var.data.bool ? strdup("true") : strdup("false"); }
    else if (var.type == NUM) { return grabStringOfNumber(var.data.num); }
    else { return NULL; }
}

void changeHashMap(HashMap *map, int extensionCount) {
    int i, newMax = map->maxElements + extensionCount; HashMap temp; 
    if (extensionCount == 0 || map->maxElements + extensionCount < map->elementCount) return;
    temp = create_hashmap(newMax);
    for (i = 0; i < map->maxElements; i++) {
        listItem *loc = map->items[i].first;
        if (loc == NULL) { continue; } /* if there ain't a first element, there ain't no fuckin' data */
        while (loc != NULL) {
            hashMapItem *current = loc->data; listItem *next = loc->next;
            loc->next = NULL; loc->prev = NULL;
            addItemToMap(&temp, current->data, current->key, current->freeRoutine);
            free(current->key); free(current);
            free(loc); loc = next;
        }
    }
    map->maxElements = newMax;
    free(map->items);
    map->items = temp.items; map->maxElements = newMax;
}

void *searchHashMap(HashMap *map, char *key) {
    unsigned long index = hash((unsigned char *)key) % map->maxElements;
    listItem *current = map->items[index].first;
    hashMapItem *check;
    if (current == NULL) { DEBUG_PRINTF("aint shit here (%lu, %d)", hash((unsigned char *)key), (int)index); return NULL; }
    check = (hashMapItem *)current->data;
    while (strcmp(check->key, key)) {
        if (current == NULL) return NULL; /* it's not gonna be there */
        check = (hashMapItem *)current->data;
        current = current->next;
    } 
    return check->data;
}

void addItemToMap(HashMap *map, void *item, char *key, void (*freeRoutine)(void*)) {
    unsigned long index = hash((unsigned char *)key) % map->maxElements;
    listItem *prev, *dataLocation = map->items[index].first;
    if (searchHashMap(map, key) != NULL) return; 
    if (map->maxElements < map->elementCount + 1) {
        changeHashMap(map, 10); /* 10 just to give some headroom */
        addItemToMap(map, item, key, freeRoutine);
        return;
    }
    while (dataLocation != NULL) { prev = dataLocation; dataLocation = dataLocation->next; } 
    if (dataLocation == NULL) {
        hashMapItem *newItem;
        dataLocation = (listItem *)callocate(1, sizeof(listItem));
        dataLocation->data = (hashMapItem *)mallocate(sizeof(hashMapItem));
        newItem = (hashMapItem *)dataLocation->data;
        newItem->data = item; newItem->key = strdup(key); newItem->freeRoutine = freeRoutine;
        DEBUG_PRINTF("added item with key %s in index %lu\n", key, index);
    }
    if (map->items[index].first == NULL) { map->items[index].first = dataLocation; }
    else { prev->next = dataLocation; }
    map->elementCount += 1; 
}

void batchAddToMap(HashMap *map, void **items, char **keys, void (*freeRoutine)(void*), int itemCount) {
    int i;
    for (i = 0; i < itemCount; i++) {
        addItemToMap(map, items[i], keys[i], freeRoutine);
    }
}

void preprocessLabels(openFile *new) {
    int i;
    new->labels = create_hashmap(new->labels.maxElements); 
    for (i = 0; i < new->instructionCount; i++) {
        if (strcmp(new->instructions[i]->operation, "label") == 0) { 
            int *location = (int *)mallocate(sizeof(int));
            *location = i - 1;
            addItemToMap(&new->labels, location, new->instructions[i]->arguments[0], free);
        }
    }
}

char *grabUserInput(const int maxSize) {
    char *val = callocate(maxSize + 1, sizeof(char)); 
    if (!val) return NULL;
    if(!fgets(val, maxSize, stdin)) { free(val); return NULL; }
    val[strcspn(val, "\n")] = '\0'; /* compensate for the newline by fucking yeeting it out of existence */
    DEBUG_PRINT(val);
    return val;
}

void strip(char *string, char character) {
    int i, len = strlen(string);
    for (i = 0; i < len; i++) {
        if (string[i] == character) {
            memmove(string + i, string + 1 + i, len - i);
            len -= 1; i--;
        }
    }
} 

int isWhitespace(char check) {
    if (check == ' ' || check == '\t' || check == '\r' || check == '\n' || check == '\0') return 1;
    return 0;
}

char **stringSlicer(char *string, int *elementCount) { /* strtok? what the fuck is that? sounds dangerous, no thanks */
    int len = strlen(string), offset = 0, i, tokens = 0, currentToken = 0; char **arr = NULL;
    while (isWhitespace(string[offset])) { if (string[offset] == '\0') { return NULL; } offset += 1; } /* skip all beginning whitespace, and bail if it's a blank line */
    for (i = offset; i <= len; i++) {
        if (isWhitespace(string[i])) tokens += 1; /* if there's whitespace, it must be the end of a token */
        if (string[i] != '\0') { int prev = i; while (isWhitespace(string[i]) && i <= len) { i++; } if (prev != i) { i--; }} /* keep goin till we hit another real token, then rewind one back since i will increase agains */
    }
    if (!tokens) { DEBUG_PRINTF("\"%s\"\n", string); return NULL; }
    arr = (char **)callocate(tokens, sizeof(char *));
    DEBUG_PRINTF("token count: %d\n", tokens);
    while (offset != len && currentToken < tokens) {
        int tokenLen = 0;
        for (i = offset; i < len; i++) { if (isWhitespace(string[i])) { offset += 1; } else {break;}}
        for (i = offset; i < len; i++) { if (!isWhitespace(string[i])) { tokenLen += 1; } else {break;}}
        DEBUG_PRINTF("allocating for token %d\n", currentToken);
        arr[currentToken] = (char *)mallocate(tokenLen + 1);
        memcpy(arr[currentToken], string + offset, tokenLen);
        arr[currentToken][tokenLen] = '\0';
        offset += tokenLen + 1;
        DEBUG_PRINT(arr[currentToken]);
        currentToken += 1;
    }
    *elementCount = tokens;
    return arr;
}

instruction *parseInstructions(char *string, InstructionSet isa) {
    int i, argc = 0, index = 0, arrCount; 
    char *operation, *prefix = NULL, **tokenized = stringSlicer(string, &arrCount); 
    instruction *new; 
    lowerizeInPlace(tokenized[index]); 
    while (strcmp(tokenized[index], "please") == 0) { index += 1; lowerizeInPlace(tokenized[index]);  }
    for (i = 0; i < isa.prefixCount; i++) { if (strcmp(tokenized[index], isa.prefixes[i]) == 0) { prefix = tokenized[index]; index += 1; lowerizeInPlace(tokenized[index]); break; }}
    lowerizeInPlace(tokenized[index]); operation = tokenized[index]; index += 1;
    argc = arrCount - index; 
    new = add_instruction(operation, tokenized + index, prefix, argc);
    if (argc >= 1) { for (i = 0; i < argc; i++) { DEBUG_PRINTF("instruction %s has arg \"%s\"\n", operation, tokenized[index + i]); }}
    for (i = 0; i < arrCount; i++) { free(tokenized[i]); } free(tokenized); 
    return new;
}

char *unParseInstructions(instruction *inst) {
    int i; size_t size = strlen(inst->operation) + 1; char *final;
    for (i = 0; i < inst->argumentCount; i++) { size += strlen(inst->arguments[i]) + 1; }
    if (inst->prefix != NULL) { size += strlen(inst->prefix) + 1; }
    size += 2; /* nullterm and semicolon ofc */
    final = (char *)callocate(size, sizeof(char)); if (!final) return NULL;
    if (inst->prefix != NULL) { strcat(final, inst->prefix); strcat(final, " "); } 
    strcat(final, inst->operation); 
    for (i = 0; i < inst->argumentCount; i++) {
        strcat(final, " "); /* space between items ofc */
        strcat(final, inst->arguments[i]);
    }
    strcat(final, ";\0");
    return final;
}

int readFileToAndIncludingChar(FILE* file, char character) { /* verbose, much? */
    char currentChar = 0; int count = 0;
    while (currentChar != character) { 
        currentChar = (char)fgetc(file); 
        if (!feof(file)) { count += 1; } else {break;} 
        if (currentChar == '\n' || currentChar == '\r') { DEBUG_PRINT("\\n"); } 
        else { DEBUG_PRINTF("%c", currentChar); }} /* push the pointer forwards */
    return count;
}

openFile openSimasFile(const char path[]) {
    int i;
    FILE *file = fopen(path, "rb");
    openFile new;
    memset(&new, 0, sizeof(openFile));
    openFile new;
    memset(&new, 0, sizeof(openFile));

    if (file == NULL) { printf("failed to find a simas file!\n"); return new; }

    new.path = strdup(path);
    new.path = strdup(path);

    while (!feof(file)) {
        int size = readFileToAndIncludingChar(file, ';'); char *buffer;
        DEBUG_PRINTF("\n%d\n", size);
        DEBUG_PRINT("goin back for more\n"); 
        if (feof(file)) break;

        fseek(file, size * -1, SEEK_CUR);
        buffer = (char *)callocate(size + 1, sizeof(char)); 
        fread(buffer, sizeof(char), size, file);
        buffer[size] = '\0';

        if (strchr(buffer, '@') == NULL) {
            new.instructions = (instruction **)reallocate(new.instructions, sizeof(instruction *) * (new.instructionCount + 1));
            if (new.instructions == NULL) cry("welp, cant add more functions, guess its time to die now");
            new.instructions[new.instructionCount] = parseInstructions(buffer, ValidInstructions);
            if (strcmp(new.instructions[new.instructionCount]->operation, "label") == 0 && new.instructions[new.instructionCount]->prefix == NULL) new.labels.maxElements += 1;
            new.instructionCount += 1;
        }
        
        free(buffer);
    }
    if (debugMode) { for (i = 0; i < new.instructionCount; i++) { DEBUG_PRINTF("%d: %s\n", i, new.instructions[i]->operation); }}
    fclose(file);
    return new;
}

void beginCommandLine(char *entryMsg, openFile *passed) {
    puts(entryMsg);

    if (!ValidCommands.items) setUpCommands();

    while (1) {
        char *value, *temp; instruction *inst;
        printf("$ ");
        value = grabUserInput(256);
        if (!value) { cry("**FATAL ERROR**:\nUnable to allocate memory!\n"); }
        temp = stripSemicolon(value); strip(temp, ' ');
        if (strcmp(temp, "") == 0) {free(value); free(temp); continue;} /* blank check */
        free(temp);
        inst = parseInstructions(value, ValidInstructions);
        if (inst->operation[0] == '!') {
            command *cmd = ((command *)searchHashMap(&ValidCommands, inst->operation));
            if (cmd != NULL) {
                char *ret = cmd->commandPointer(inst, passed);
                if (ret == NULL) {
                    freeInstruction(inst);
                    free(value);
                    goto exit; /* chat is this BASIC? */
                }
                printf("%s", ret);
            } else { 
                printf("invalid command\n"); 
            }
        } else if (findNumberArgs(inst->operation, ValidInstructions) == -1 && strchr(inst->operation, '@') == NULL) {
            printf("invalid instruction\n"); freeInstruction(inst); free(value); continue;
        } else {
            if (strchr(value, ';') != NULL && inst->argumentCount >= findNumberArgs(inst->operation, ValidInstructions)) {
                passed->instructions = (instruction **)reallocate(passed->instructions, sizeof(instruction *) * (passed->instructionCount + 1));
                if (passed->instructions == NULL) { printf("**FATAL ERROR**:\nReallocation failed!\n"); free(value); break; }
                passed->instructions[passed->instructionCount] = add_instruction(inst->operation, inst->arguments, inst->prefix, inst->argumentCount);
                passed->instructionCount += 1;
                if (strcmp(inst->operation, "label") == 0 && inst->prefix == NULL) passed->labels.maxElements += 1;
                printf("ok\n");
            } else if (inst->argumentCount < findNumberArgs(inst->operation, ValidInstructions)) {
                printf("too little arguments for instruction\n");
            } else {
                printf("code must end with a semicolon\n");
            }
        }

        freeInstruction(inst);
        free(value);
    }

    exit: /* yes gotos are shit. no i can't double-break from a loop. */
    
    freeHashMap(ValidCommands);
    freeFile(*passed);
    freeInstructionSet(ValidInstructions);

    exit(0);
}

void convertLiteralNewLineToActualNewLine(char *string) {
    int i, sizeOf = strlen(string);
    for (i = 1; i < sizeOf; i++) { 
        if ((string[i] == 'n' || string[i] == 'r')&& string[i - 1] == '\\') { 
            string[i - 1] = '\n';
            memcpy(string + i, string + 1 + i, sizeOf - i);
            i -= 1; 
        }
    } 
}

char *joinStringsSentence(char **strings, int stringCount, int offset) {
    char *finalString = NULL; int i, sizeOf = 0;
    if (stringCount == 1) { finalString = strdup(strings[0]); return finalString; }
    for (i = offset; i < stringCount; i++) { sizeOf += strlen(strings[i]) + 1;}
    finalString = (char *)callocate(sizeOf + 1, sizeof(char)); if (finalString == NULL) cry("unable to string\nplease do not the string\n"); 
    for (i = offset; i < stringCount; i++) { strcat(finalString, strings[i]); if (i + 1 < stringCount) { strcat(finalString, " "); }} /* fuck yo optimization */
    convertLiteralNewLineToActualNewLine(finalString);
    return finalString;
}

int checkVarTruthiness(variable *var) {
    switch (var->type) {
        case NUM: if (var->data.num != 0.0) { return 1; } else return 0;
        case BOOL: if (var->data.bool != 0) { return 1; } else return 0;
        case STR: if (var->data.str != NULL) { return trueOrFalse(var->data.str); } else return 0;
        default: return 0;
    }
}

void convert(variable *var, int type) {
    DEBUG_PRINTF("\nConverted from type %d to type %d\n", var->type, type);
    if (var->type != type) {
        if (var->type == NUM) {
            if (type == BOOL) {
                if (var->data.num != 0.0) { var->data.bool = 1; }
                else { var->data.bool = 0; }
            } else if (type == STR) {
                var->data.str = grabStringOfNumber(var->data.num);
            }
        } else if (var->type == BOOL) {
            int truth = var->data.bool;
            if (type == NUM) { var->data.num = (double)truth; }
            else if (type == STR) {
                if (truth) var->data.str = strdup("true");
                if (!truth) var->data.str = strdup("false");
            }
        } else if (var->type == STR) {
            char *temp = strdup(var->data.str); if (var->data.str != NULL) { free(var->data.str); }
            if (type == BOOL) { var->data.bool = trueOrFalse(temp); }
            else if (type == NUM) { var->data.num = (double)atof(temp); }
            free(temp);
        }
        var->type = type;
    } 
    else if (var->type == type) { /* do nothing, as you cannot convert to the same type, fucknuts */ } 
    else { cry("invalid variable type.\n"); }
}

int areTwoVarsEqual(variable *var1, variable *var2) {
    if (var1->type != var2->type) { return 0; }
    if (var1->type == STR && strcmp(var1->data.str, var2->data.str) == 0) { return 1; }
    else if (var1->type == NUM && var1->data.num == var2->data.num) { return 1; }
    else if (var1->type == BOOL && var1->data.bool == var2->data.bool) { return 1; }
    return 0;
}

char *readFile(char path[]) {
    FILE *file = fopen(path, "r"); char *contents = NULL; long length;
    if (file == NULL) cry("cannot open le file!");
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file); 
    contents = (char *)callocate(length + 1, sizeof(char));
    fread(contents, 1, length, file);
    contents[length] = '\0';
    fclose(file);
    DEBUG_PRINT(contents);
    return contents;
}

void writeFile(char *path, char *value) {
    FILE *file = fopen(path, "w");
    if (file == NULL) { cry("failed to write to file!"); }
    fprintf(file, "%s", value);
    fclose(file);
}

void freeAndWrite(char *path, char *value) { writeFile(path, value); free(value); }

void setVar(variable *var, int type, char* value, double num, int bool) {
    char *val = NULL;
    if (type == IN) { /* let the user type whatever bullshit is on their minds */
        type = STR; val = grabUserInput(100);
    } else if (type == STR) { 
        val = strdup(value); 
    }
    set_variable_value(var, type, val, num, bool);
    if (val) free(val);
}

/* actual function code / helpers                                                       */
void negateBoolean(variable *var) { if (var->type == BOOL) { var->data.bool = !var->data.bool; } else { cry("NOT must be used on a bool!"); }}
void writeFromVar(variable *var, char *path) { char *variable = stringFromVar(*var); writeFile(path, variable); free(variable); } 
void labelJump(int *location, int *programCounter) { *programCounter = *location; }
void equalityCheckVarVsConst(HashMap *varMap, char **arguments, int flip) {
    variable *var1 = searchHashMap(varMap, arguments[1]), var2;
    int output = 0, type = grabType(arguments[0]); var2.type = type; var2.data.str = NULL;
    if (type == NUM) { var2.data.num = (double)atof(arguments[2]); }
    else if (type == STR) { var2.data.str = strdup(arguments[2]); }
    else if (type == BOOL) { var2.data.bool = trueOrFalse(arguments[2]); } 
    if (var1 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, &var2) : areTwoVarsEqual(var1, &var2);
    if (var1->type == STR && var1->data.str != NULL) free(var1->data.str);
    var1->type = BOOL;
    var1->data.bool = output;
    if (type == STR && var2.data.str != NULL) free(var2.data.str);
}
void equalityCheckVarVsVar(variable *var1, variable *var2, int flip) {
    int output = 0;
    if (var1 == NULL || var2 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, var2) : areTwoVarsEqual(var1, var2);
    if (var1->type == STR && var1->data.str != NULL) free(var1->data.str);
    var1->type = BOOL;
    var1->data.bool = output;
}

void jumpConditionally(int *location, variable *var, int *programCounter, int flip) {
    int allowed = checkVarTruthiness(var);
    if (flip) { allowed = !allowed; }
    if (allowed) labelJump(location, programCounter);
}

void standardMath(HashMap *varMap, char **arguments, char operation) {
    double op2 = 0;
    variable *var1 = searchHashMap(varMap, arguments[1]), *var2 = searchHashMap(varMap, arguments[2]); 
    if (!var1) { var1 = (variable *)mallocate(sizeof(variable)); addItemToMap(varMap, var1, arguments[1], (void (*)(void *))freeVariable); var1 = searchHashMap(varMap, arguments[1]); }
    if (var1->type != NUM) cry("You can only do math on a 'num' type variable!");
    if (var2 == NULL) { op2 = atof(arguments[2]); }
    else if (var2->type != NUM) cry("You can only do math on a 'num' type variable!");
    else { op2 = var2->data.num; }
    switch (operation) {
        case '+': var1->data.num += op2; break;
        case '-': var1->data.num -= op2; break;
        case '*': var1->data.num *= op2; break;
        case '/': if (op2 == 0.0) {cry("div by zero error\n");} else{ var1->data.num /= op2;} break; /* this is when we tell the user to eat shit and die, nerd */
        default: var1->data.num = 0;
    }
}

variable *createVarIfNotFound(HashMap *varMap, char *name) {
    variable *var = searchHashMap(varMap, name);
    if (!var) { var = (variable *)callocate(1, sizeof(variable)); addItemToMap(varMap, var, name, (void (*)(void *))freeVariable); var = searchHashMap(varMap, name); }
    return var; 
}

void variableSet(HashMap *varMap, char **arguments, int argumentCount) {
    int type = grabType(arguments[0]); char *concatenated = NULL;
    if (type == STR) concatenated = joinStringsSentence(arguments, argumentCount, 2);
    switch (type) {
        case IN: setVar(createVarIfNotFound(varMap, arguments[1]), type, NULL, 0, 0); break;
        case STR: setVar(createVarIfNotFound(varMap, arguments[1]), type, concatenated, 0.0, 0); break;
        case NUM: setVar(createVarIfNotFound(varMap, arguments[1]), type, NULL, (double)atof(arguments[2]), 0); break; 
        case BOOL: setVar(createVarIfNotFound(varMap, arguments[1]), type, NULL, 0.0, trueOrFalse(arguments[2])); break;
        default: cry("That's not a valid type!\n");
    }
    free(concatenated);
}
void grabTypeFromVar(variable check, variable *var) {
    switch (check.type) {
        case NUM: set_variable_value(var, STR, "num", 0.0, 0); break;
        case BOOL: set_variable_value(var, STR, "bool", 0.0, 0); break;
        case STR: set_variable_value(var, STR, "str", 0.0, 0); break;
        default: set_variable_value(var, STR, "none", 0.0, 0); break;
    }
}

void compareNums(HashMap *varMap, char **arguments, char operation) {
    variable *var1 = searchHashMap(varMap, arguments[1]);
    variable *var2 = searchHashMap(varMap, arguments[2]);
    double operand1 = 0; double operand2 = 0;
    if (var1 == NULL) { cry("No variable!"); }
    else if (var1->type != NUM) { cry("Operand must be of \"num\" type!\n"); }
    else { operand1 = var1->data.num; }
    if (var2 == NULL) { operand2 = (double)atof(arguments[2]); }
    else if (var2->type != NUM) { cry("Operand must be of \"num\" type!\n"); }
    else { operand2 = var2->data.num; }

    var1->type = BOOL;

    switch (operation) {
        case '>': var1->data.bool = (operand1 > operand2); break;
        case ']': var1->data.bool = (operand1 >= operand2); break;
        case '<': var1->data.bool = (operand1 < operand2); break;
        case '[': var1->data.bool = (operand1 <= operand2); break;
        default: var1->data.bool = 0; break;
    }
}

void compareBools(HashMap *varMap, char **arguments, char operation, char flip) {
    variable *var1 = searchHashMap(varMap, arguments[1]);
    variable *var2 = searchHashMap(varMap, arguments[2]);
    int operand1 = 0; int operand2 = 0;
    if (var1 == NULL) { cry("No variable!"); } /* can't save SHIT if you dont have a variable */
    else { operand1 = checkVarTruthiness(var1); }
    if (var2 == NULL) { if (grabType(arguments[0])) {operand2 = trueOrFalse(arguments[2]);} else { operand2 = atoi(arguments[2]); }}
    else { operand2 = checkVarTruthiness(var2); }
    if (var1->type == STR && var1->data.str != NULL) free(var1->data.str);
    var1->type = BOOL;
    switch (operation) {
        case '&': var1->data.bool = flip ? (operand1 && operand2) : !(operand1 && operand2); break;
        case '|': var1->data.bool = flip ? (operand1 || operand2) : !(operand1 || operand2); break;
        case '!': var1->data.bool = (operand1 != operand2); break;
        default: var1->data.bool = 0; break;
    }
}

char *formatList(LinkedList li) {
    char *final; int i; size_t bytes = 3; listItem *current = li.first;
    for (i = 0; i < li.elements; i++) {
        DEBUG_PRINTF("\n\n%d\n\n", i);
        bytes += stringLenFromVar(*(variable *)current->data) + 2; 
        if (((variable *)current->data)->type == STR) { bytes += 2; } /* "" */
        current = current->next;
    }
    final = (char *)mallocate(bytes); if (final == NULL) cry("List Formatting failed!");
    final[0] = '\0';

    strcat(final, "[");
    current = li.first;
    for (i = 0; i < li.elements; i++) {
        char *temp = stringFromVar(*((variable *)current->data));
        if (temp) {
            if (((variable *)current->data)->type == STR) strcat(final, "\"");
            strcat(final, temp);
            if (((variable *)current->data)->type == STR) strcat(final, "\"");
            free(temp);
            if (i + 1 != li.elements) strcat(final, ","); /* make sure no trailing comma is left */
        }
        current = current->next;
    }
    strcat(final, "]");
    return final;
}

void unFormatList(LinkedList *li, char *string) {
    int type, i, j, start = 0; 
    while (1) { if (string[start] == '[') { break; } start += 1; }
    for (i = start; i < (int)strlen(string); i++) {
        variable var; char *value, c = string[i]; int length = 0;
        if (c == ']') break;
        if (c == '[' || c == ',') continue;
        if (c == '"') { type = STR; continue; }
        if (type != STR) { if (isdigit(c)) { type = NUM; } else { type = BOOL; }}

        while ((c = string[i + length]) != ',' && (c = string[i + length]) != '"' && (c = string[i + length]) != '[' && (c = string[i + length]) != ']') { length += 1; DEBUG_PRINT(&c); }

        value = (char *)callocate(length + 1, sizeof(char));
        for (j = 0; j < length; j++) { value[j] = string[i + j]; }
        i += length; 
        value[length] = '\0';
        DEBUG_PRINT(value);

        var.type = type;
        if (type == NUM) { var.data.num = (double)atof(value); }
        else if (type == STR) { var.data.str = value; }
        else if (type == BOOL) { var.data.bool = trueOrFalse(value); }
        appendElementToList(li, var); type = 0; free(value);
    }
}

void loadList(HashMap *listMap, char *name, char *path) {
    LinkedList *li = searchHashMap(listMap, name), *new; char *temp = readFile(path);
    if (li != NULL) { freeList(li); }
    new = (LinkedList *)callocate(1, sizeof(LinkedList)); addItemToMap(listMap, new, name, (void (*)(void *))freeList); li = searchHashMap(listMap, name);
    unFormatList(li, temp); free(temp);
}

void listAppendConstant(LinkedList *li, char **arguments, int *argumentCount) {
    int type = grabType(arguments[1]);
    variable var; var.type = type;
    if (type == NUM) { var.data.num = (double)atof(arguments[2]); }
    else if (type == BOOL) { var.data.bool = trueOrFalse(arguments[2]); }
    else if (type == STR) { var.data.str = joinStringsSentence(arguments, *argumentCount, 2); }
    appendElementToList(li, var); if (type == STR && var.data.str) free(var.data.str);
}

variable create_variable_with_value(char *name, int type, char *value, double num, int bool) {
    variable var;
    switch (type) {
        case STR: set_variable_value(&var, type, value, 0.0, 0); break;
        case NUM: set_variable_value(&var, type, NULL, num, 0); break;
        case BOOL: set_variable_value(&var, type, NULL, 0.0, bool); break;
        default: cry("Invalid type!\n");
    }
    return var;
}

/* console i/o      */
void con_prints(openFile *file) { putc(' ', stdout); }
void con_println(openFile *file) { puts(""); }
void con_printv(openFile *file) { freeAndPrint(stringFromVar(*(variable *)searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]))); }
void con_printc(openFile *file) { freeAndPrint(joinStringsSentence(file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount, 0)); }
/*file i/o          */
void fio_read(openFile *file) { char *read = readFile(file->instructions[file->programCounter]->arguments[0]); set_variable_value(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1]), STR, read, 0.0, 0); free(read); }
void fio_write(openFile *file) { freeAndWrite(file->instructions[file->programCounter]->arguments[0], joinStringsSentence(file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount, 1)); }
void fio_writev(openFile *file) { writeFromVar(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), file->instructions[file->programCounter]->arguments[0]); }
/* misc             */
void etc_not(openFile *file) { negateBoolean(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]));  }
void etc_quit(openFile *file) { freeFile(*file); freeInstructionSet(ValidInstructions); exit(0); }
/* jumps            */
void jmp_jump(openFile *file) { labelJump(searchHashMap(&file->labels, file->instructions[file->programCounter]->arguments[0]), &file->programCounter); }
void jmp_jumpv(openFile *file) { jumpConditionally(searchHashMap(&file->labels, file->instructions[file->programCounter]->arguments[0]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), &file->programCounter, 0); }
void jmp_jumpnv(openFile *file) { jumpConditionally(searchHashMap(&file->labels, file->instructions[file->programCounter]->arguments[0]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), &file->programCounter, 1); }
/* math             */
void mat_add(openFile *file) { standardMath(&file->variables, file->instructions[file->programCounter]->arguments, '+'); }
void mat_sub(openFile *file) { standardMath(&file->variables, file->instructions[file->programCounter]->arguments, '-'); }
void mat_mul(openFile *file) { standardMath(&file->variables, file->instructions[file->programCounter]->arguments, '*'); }
void mat_div(openFile *file) { standardMath(&file->variables, file->instructions[file->programCounter]->arguments, '/'); }
/* variable ops     */
void var_set(openFile *file) { variableSet(&file->variables, file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }
void var_type(openFile *file) { grabTypeFromVar(*(variable *)searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]), createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1])); }
void var_conv(openFile *file) { convert(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]), grabType(file->instructions[file->programCounter]->arguments[1])); }
void var_copy(openFile *file) { variable *var = createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1]); varcpy(var, searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0])); } 
/* comparison       */
void cmp_gt(openFile *file) { compareNums(&file->variables, file->instructions[file->programCounter]->arguments, '>'); }
void cmp_gte(openFile *file) { compareNums(&file->variables, file->instructions[file->programCounter]->arguments, ']'); }
void cmp_st(openFile *file) { compareNums(&file->variables, file->instructions[file->programCounter]->arguments, '<'); }
void cmp_ste(openFile *file) { compareNums(&file->variables, file->instructions[file->programCounter]->arguments, '['); }
void cmp_eqv(openFile *file) { equalityCheckVarVsVar(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[2]), 0); }
void cmp_neqv(openFile *file) { equalityCheckVarVsVar(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[2]), 1); }
void cmp_eqc(openFile *file) { equalityCheckVarVsConst(&file->variables, file->instructions[file->programCounter]->arguments, 0); }
void cmp_neqc(openFile *file) { equalityCheckVarVsConst(&file->variables, file->instructions[file->programCounter]->arguments, 1); }
void cmp_and(openFile *file) { compareBools(&file->variables, file->instructions[file->programCounter]->arguments, '&', 0); }
void cmp_nand(openFile *file) { compareBools(&file->variables, file->instructions[file->programCounter]->arguments, '&', 1); }
void cmp_or(openFile *file) { compareBools(&file->variables, file->instructions[file->programCounter]->arguments, '|', 0); }
void cmp_nor(openFile *file) { compareBools(&file->variables, file->instructions[file->programCounter]->arguments, '|', 1); }
void cmp_xor(openFile *file) { compareBools(&file->variables, file->instructions[file->programCounter]->arguments, '!', 0); }
/* list ops         */
void lis_del(openFile *file) { LinkedList *li = searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]); if (atoi(file->instructions[file->programCounter]->arguments[1]) > li->elements) { handleError("invalid index", 92, 0, file); } else { removeElementFromList(li, atoi(file->instructions[file->programCounter]->arguments[1]) - 1); }}
void lis_appv(openFile *file) { appendElementToList(searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), *(variable *)searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[2])); }
void lis_show(openFile *file) { freeAndPrint(formatList(*(LinkedList *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))); }
void lis_new(openFile *file) { LinkedList *new = (LinkedList *)callocate(1, sizeof(LinkedList)); addItemToMap(&file->lists, new, file->instructions[file->programCounter]->arguments[0], (void (*)(void *))freeList); }
void lis_upv(openFile *file) { varcpy((variable *)traverseList(atoi(file->instructions[file->programCounter]->arguments[1]) - 1, 0, ((LinkedList *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))->first)->data, searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[3])); }
void lis_acc(openFile *file) { varcpy(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[2]), (variable *)traverseList(atoi(file->instructions[file->programCounter]->arguments[1]) - 1, 0, ((LinkedList *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))->first)->data); }
void lis_load(openFile *file) { loadList(&file->lists, file->instructions[file->programCounter]->arguments[0], file->instructions[file->programCounter]->arguments[1]); }
void lis_len(openFile *file) { set_variable_value(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1]), NUM, NULL, ((LinkedList *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))->elements, 0); }
void lis_dump(openFile *file) { freeAndWrite(file->instructions[file->programCounter]->arguments[1], formatList(*(LinkedList *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))); }
void lis_upc(openFile *file) { char **arguments = file->instructions[file->programCounter]->arguments; variable var = create_variable_with_value(NULL, grabType(arguments[2]), joinStringsSentence(arguments, file->instructions[file->programCounter]->argumentCount, 4), (double)atof(arguments[4]), trueOrFalse(arguments[3])); varcpy((variable *)traverseList(atoi(arguments[1]) - 1, 0, ((LinkedList *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))->first)->data, &var); }
void lis_appc(openFile *file) { listAppendConstant(searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments, &file->instructions[file->programCounter]->argumentCount); }

/* command functions for the CLI */
char *cmd_quit(instruction *inst, openFile *file) { return NULL; } /* this is what we call a pro gamer move */
char *cmd_clear(instruction *inst, openFile *file) { freeFile(*file); memset(file, 0, sizeof(openFile)); return ""; }
char *cmd_debug(instruction *inst, openFile *file) { debugMode = !debugMode; return "debugger toggled\n"; }
char *cmd_load(instruction *inst, openFile *file) {
    if (inst->argumentCount) {
        freeFile(*file);
        *file = openSimasFile(inst->arguments[0]);
        if (file->path) { return "loaded successfully\n"; }
    } else return "you need to specify a file\n";
    return "";
}
char *cmd_save(instruction *inst, openFile *file) {
    if (inst->argumentCount) {
        FILE* dest = fopen(inst->arguments[0], "wb");
        if (dest) {
            int i;
            for (i = 0; i < file->instructionCount; i++) {
                char *string = unParseInstructions(file->instructions[i]);
                fwrite(string, sizeof(char), strlen(string), dest);
                free(string); fputc('\n', dest);
            }
            fclose(dest); return "successfully saved!\n";
        } else return "unable to open file\n";
    } else return "you need to specify a file to save to \n";
}
char *cmd_dump(instruction *inst, openFile *file) {
    int i;
    for (i = 0; i < file->instructionCount; i++) {
        char *string = unParseInstructions(file->instructions[i]);
        printf("%d: %s\n", i + 1, string); free(string);
    } 
    return ""; /* dummy print */
}
char *cmd_edit(instruction *inst, openFile *file) {
    if (inst->argumentCount) {
        if (atoi(inst->arguments[0]) <= file->instructionCount && file->instructionCount && (atoi(inst->arguments[0]) - 1) >= 0) {
            char *out, *temporary, *old = unParseInstructions(file->instructions[atoi(inst->arguments[0]) - 1]);
            instruction *instruct;
            printf("Old instruction: %s\n", old); free(old);
            printf("Enter new instruction: ");
            out = grabUserInput(256); temporary = stripSemicolon(out); strip(temporary, ' ');
            instruct = parseInstructions(out, ValidInstructions);
            if (strcmp(temporary, "") != 0 && strchr(out, ';') && instruct->argumentCount >= findNumberArgs(instruct->operation, ValidInstructions)) { 
                freeInstruction(file->instructions[atoi(inst->arguments[0]) - 1]);
                file->instructions[atoi(inst->arguments[0]) - 1] = add_instruction(instruct->operation, instruct->arguments, instruct->prefix, instruct->argumentCount);
            } else if (strchr(out, ';') == NULL) {
                printf("code must end with a semicolon\n");
            } else if (instruct->argumentCount < findNumberArgs(instruct->operation, ValidInstructions)) {
                printf("too little arguments for instruction\n");
            }
            freeInstruction(instruct); free(temporary); free(out);
            return "";
        }

        else if (!file->instructionCount) return "no instructions\n";
        else if (atoi(inst->arguments[0]) >= file->instructionCount) return "invalid index\n";
    }
    else return "you need to specify an index\n";
    return "";
}
char *cmd_help(instruction *inst, openFile *file) {
    return(
        "CMAS Command List:\n"
        "!quit: Quits the CMAS command line.\n"
        "!clear: Resets the current program.\n"
        "!edit <index>: Edit the instruction at an index, starting from 1.\n"
        "!dump: Dumps the current program to terminal.\n"
        "!load <filename>: Loads a SIMAS file.\n"
        "!save <filename>: Saves the current SIMAS program to disk.\n"
        "!run: Executes the current SIMAS program.\n"
        "Please read the README.md for a list of all instructions and their operators.\n"
    );
}
char *cmd_run(instruction *inst, openFile *file) {
    if (file->instructionCount) {
        executeFile(file, 0); 
        cleanFile(file); return ""; 
    } else { return "no instructions to execute\n"; }
}

void executeInstruction(openFile *cur) { /* all of these are defined up here so this function can operate independently of any files */
    int i;
    DEBUG_PRINTF("\nExecuting instruction %s on line %d.\n", cur->instructions[cur->programCounter]->operation, cur->programCounter);
    if (strlen(cur->instructions[cur->programCounter]->operation) == 0) return;
    for (i = 0; i < ValidInstructions.count; i++) { 
        if (strcmp(ValidInstructions.set[i].name, cur->instructions[cur->programCounter]->operation) == 0) {
            if (ValidInstructions.set[i].prefix != NULL) {
                if (cur->instructions[cur->programCounter]->prefix == NULL) continue;
                if (strcmp(ValidInstructions.set[i].prefix, cur->instructions[cur->programCounter]->prefix) != 0) continue;
            }
            (ValidInstructions.set[i].functionPointer)(cur);
        }
    }
}

void executeFile(openFile *current, int doFree) {
    preprocessLabels(current); current->lists = create_hashmap(1); current->variables = create_hashmap(10); 
    for (current->programCounter = 0; current->programCounter < current->instructionCount; current->programCounter++) { executeInstruction(current); }
    if (doFree) freeFile(*current);
}

void setUpStdlib(void) {
    int count = 44;
    operator *ops = (operator *)mallocate(sizeof(operator) * count);
    addPrefix("list", &ValidInstructions);
    ops[ 0] = create_operator("print", NULL, (void(*)(void*))con_printv, 1); 
    ops[ 1] = create_operator("println", NULL, (void(*)(void*))con_println, 0);
    ops[ 2] = create_operator("prints", NULL, (void(*)(void*))con_prints, 0); 
    ops[ 3] = create_operator("printc", NULL, (void(*)(void*))con_printc, 1); 
    ops[ 4] = create_operator("read", NULL, (void(*)(void*))fio_read, 2); 
    ops[ 5] = create_operator("write", NULL, (void(*)(void*))fio_write, 2);
    ops[ 6] = create_operator("writev", NULL, (void(*)(void*))fio_writev, 2); 
    ops[ 7] = create_operator("not", NULL, (void(*)(void*))etc_not, 1);
    ops[ 8] = create_operator("quit", NULL, (void(*)(void*))etc_quit, 1);
    ops[ 9] = create_operator("add", NULL, (void(*)(void*))mat_add, 3);
    ops[10] = create_operator("sub", NULL, (void(*)(void*))mat_sub, 3);
    ops[11] = create_operator("mul", NULL, (void(*)(void*))mat_mul, 3);
    ops[12] = create_operator("div", NULL, (void(*)(void*))mat_div, 3);
    ops[13] = create_operator("set", NULL, (void(*)(void*))var_set, 2);
    ops[14] = create_operator("type", NULL, (void(*)(void*))var_type, 2);
    ops[15] = create_operator("conv", NULL, (void(*)(void*))var_conv, 2);
    ops[16] = create_operator("copy", NULL, (void(*)(void*))var_copy, 2);
    ops[17] = create_operator("gt", NULL, (void(*)(void*))cmp_gt, 3); 
    ops[18] = create_operator("gte", NULL, (void(*)(void*))cmp_gte, 3); 
    ops[19] = create_operator("st", NULL, (void(*)(void*))cmp_st, 3); 
    ops[20] = create_operator("ste", NULL, (void(*)(void*))cmp_ste, 3); 
    ops[21] = create_operator("eqv", NULL, (void(*)(void*))cmp_eqv, 3); 
    ops[22] = create_operator("neqv", NULL, (void(*)(void*))cmp_neqv, 3);
    ops[23] = create_operator("eqc", NULL, (void(*)(void*))cmp_eqc, 3); 
    ops[24] = create_operator("neqc", NULL, (void(*)(void*))cmp_neqc, 3);
    ops[25] = create_operator("and", NULL, (void(*)(void*))cmp_and, 3); 
    ops[26] = create_operator("nand", NULL, (void(*)(void*))cmp_nand, 3);
    ops[27] = create_operator("or", NULL, (void(*)(void*))cmp_or, 3); 
    ops[28] = create_operator("nor", NULL, (void(*)(void*))cmp_nor, 3);
    ops[29] = create_operator("xor", NULL, (void(*)(void*))cmp_xor, 3);
    ops[30] = create_operator("jump", NULL, (void(*)(void*))jmp_jump, 1);
    ops[31] = create_operator("jumpv", NULL, (void(*)(void*))jmp_jumpv, 2); 
    ops[32] = create_operator("jumpnv", NULL, (void(*)(void*))jmp_jumpnv, 2);
    ops[33] = create_operator("del", "list", (void(*)(void*))lis_del, 1); 
    ops[34] = create_operator("appv", "list", (void(*)(void*))lis_appv, 2); 
    ops[35] = create_operator("show", "list", (void(*)(void*))lis_show, 0); 
    ops[36] = create_operator("new", "list", (void(*)(void*))lis_new, 0); 
    ops[37] = create_operator("upv", "list", (void(*)(void*))lis_upv, 3); 
    ops[38] = create_operator("acc", "list", (void(*)(void*))lis_acc, 2); 
    ops[39] = create_operator("load", "list", (void(*)(void*))lis_load, 2); 
    ops[40] = create_operator("len", "list", (void(*)(void*))lis_len, 1); 
    ops[41] = create_operator("dump", "list", (void(*)(void*))lis_dump, 1); 
    ops[42] = create_operator("upc", "list", (void(*)(void*))lis_upc, 3); 
    ops[43] = create_operator("appc", "list", (void(*)(void*))lis_appc, 2); 
    addOperatorSet(ops, &ValidInstructions, count);
    free(ops);
}

void setUpCommands() {
    int count = 9, i;
    command **cmds = (command **)mallocate(count * sizeof(command *));
    for (i = 0; i < count; i++) cmds[i] = (command *)mallocate(sizeof(command));
    cmds[0]->commandPointer = cmd_quit;
    cmds[1]->commandPointer = cmd_run;
    cmds[2]->commandPointer = cmd_load;
    cmds[3]->commandPointer = cmd_save;
    cmds[4]->commandPointer = cmd_dump;
    cmds[5]->commandPointer = cmd_clear;
    cmds[6]->commandPointer = cmd_edit;
    cmds[7]->commandPointer = cmd_help;
    cmds[8]->commandPointer = cmd_debug;
    ValidCommands = create_hashmap(9); 
    addItemToMap(&ValidCommands, cmds[0], "!quit", free);
    addItemToMap(&ValidCommands, cmds[1], "!run", free);
    addItemToMap(&ValidCommands, cmds[2], "!load", free);
    addItemToMap(&ValidCommands, cmds[3], "!save", free);
    addItemToMap(&ValidCommands, cmds[4], "!dump", free);
    addItemToMap(&ValidCommands, cmds[5], "!clear", free);
    addItemToMap(&ValidCommands, cmds[6], "!edit", free);
    addItemToMap(&ValidCommands, cmds[7], "!help", free);
    addItemToMap(&ValidCommands, cmds[8], "!debug", free);
    free(cmds);
}

int main(int argc, const char * argv[]) {
    openFile new;
    memset(&new, 0, sizeof(openFile));
    #if ALLOC_DEBUGGING == 1
    allocatorLog = fopen("./allocs.log", "w");
    #endif

    setUpStdlib();

    if (argc == 2) { if (strcmp(argv[1], "-d") == 0) { toggleDebugMode(); }};

    if (argc >= 2 && !debugMode) { 
        if (argc == 3) { if (strcmp(argv[2], "-d") == 0) { toggleDebugMode(); }}
        new = openSimasFile(argv[1]);
        executeFile(&new, 1);
    } else { 
        beginCommandLine("CMAS (C Simple Assembly) Interpreter.\nWritten by tuvalutorture, Licensed under GNU GPLv3.\nUsing The SIMAS Programming Language, created by Turrnut.\nGitHub repo: https://github.com/tuvalutorture/simas \nType !help for a list of commands.\n", &new); 
    }

    freeInstructionSet(ValidInstructions);

    #if ALLOC_DEBUGGING == 1
    fclose(allocatorLog);
    #endif

    return 0;
}
/* we are the shinglefuckers of bong juice ltd. */