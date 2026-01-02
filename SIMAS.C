/* CMAS (C SIMAS "SIMple ASsembly") interpreter - by tuvalutorture  */

/* the automobile seatbelt was invented by John Lennon the CCXXVII  */
/* in 375 BC and 204 years later his child, John Bing the MCLXXVI   */
/* of Cornholio invented the windshield wiper in the year of our    */
/* lord 171 BC, but their inventions were lost to time in the year  */
/* 582 ACDC, and were only just now redicovered in the present day. */

/*                          In my eyes                              */
/*                          Indisposed                              */
/*                    In disguises no one knows                     */
/*                         Hides the face                           */
/*                         Lies the snake                           */
/*                   And the sun in my disgrace                     */
/*                          Boiling heat                            */
/*                         Summer stench                            */
/*                Neath the black, the sky looks dead               */
/*                          Call my name                            */
/*                        Through the cream                         */
/*                  And I'll hear you scream again                  */
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
int commandPrompt = 0;

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

typedef union variableData variableData;
typedef struct command command;
typedef struct variable variable;
typedef struct instruction instruction;
typedef struct function function;
typedef struct list list; 
typedef struct openFile openFile;
typedef struct operator operator;
typedef struct InstructionSet InstructionSet;

struct command { /* spoingus my beloved */
    char *(*commandPointer)(instruction*, openFile*); 
};

union variableData { /* UNIONISE, MY CHILDREN! RISE AGAINST THE EMPLOYERS WHO TREAT YOU AS WAGE SLAVES! BECOME A UNION! FIGHT FOR WORKPLACE RIGHTS! GET YO 401(k)!!! */
    double num;
    char *str;
    int boolean;
    void *etc;
}; 

struct variable {
    int *type, isPtr;
    variableData *data;
};

struct instruction {
    char **arguments;
    char *operation;
    char *prefix;
    int argumentCount;
};

struct function {
    int start, end;
    int parameterCount;
};

struct list { /* keeps them lookup times objectively speedy as fawk */
    int *elements, isAlias;
    variable **variables;
}; 

struct openFile {
    char *path;
    instruction **instructions;
    HashMap variables;
    HashMap labels;
    HashMap lists; 
    HashMap functions;
    int instructionCount;
    int programCounter;
};

struct operator {
    void (*functionPointer)(void*); /* guys i think this points or smth idk */ 
    int minArgs;
};

struct InstructionSet {
    HashMap operators;
    HashMap prefixes;
};

InstructionSet ValidInstructions;
HashMap ValidCommands;

HashMap garbage;

/* preprocessor fuckery so that i can debug exactly where all my damn allocations are   */
/* set the debugging define to 0 if you don't want the cycle loss with this             */
#if ALLOC_DEBUGGING == 1

FILE *allocatorLog;
int mallocationCounter = 0, callocationCounter = 0, reallocationCounter = 0;
size_t mallocationBytes = 0, callocationBytes = 0, reallocationBytes = 0;

void *mallocate(size_t size) {
    mallocationCounter += 1; mallocationBytes += size;
    fprintf(allocatorLog, "allocated %d bytes (allocation #%d)\r\n", (int)size, mallocationCounter);
    DEBUG_PRINTF("allocated %d bytes (allocation #%d)\r\n", (int)size, mallocationCounter);
    return malloc(size);
}

void *callocate(size_t count, size_t size) {
    callocationCounter += 1; callocationBytes += count * size;
    fprintf(allocatorLog, "cleared & allocated %d bytes(allocation #%d)\r\n", (int)size * (int)count, callocationCounter);
    DEBUG_PRINTF("cleared & allocated %d bytes(allocation #%d)\r\n", (int)size * (int)count, callocationCounter);
    return calloc(count, size);
}

void *reallocate(void *block, size_t size) {
    reallocationCounter += 1; reallocationBytes += size;
    fprintf(allocatorLog, "reallocated %d bytes(allocation #%d)\r\n", (int)size, reallocationCounter);
    DEBUG_PRINTF("reallocated %d bytes(allocation #%d)\r\n", (int)size, reallocationCounter);
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
void executeInstruction(openFile *cur);

void dummy() { float f=0,*fp; fp=&f; printf("%f",*fp); } /* only needed for retarded systems like turbo c to trick it into bringing in float libs */

void cry(char *msg) { puts(msg); exit(2847172); }

void snadmwithc() { /* sandwich hrhehehheheheheheheheheeeehheheherhehehehhehhehhehhhehhehhehehehhehhehhehhehehehehhehheheheehhehehhehehnehehehehe */
    debugMode = !debugMode;
    if (debugMode) { puts("debug mode enabled"); }
    else { puts("debug mode disabled"); }
}

char *stroustrup(const char *string) {
    int len = strlen(string) + 1;
    char *final = (char *)mallocate(len * sizeof(char));
    if (!final) return NULL;
    memcpy(final, string, len);
    return final;
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

variable *create_variable(void) { /* dumb allocation wrapper */
    variable *new = (variable *)callocate(1, sizeof(variable));
    new->data = (variableData *)callocate(1, sizeof(variableData));
    new->type = (int *)callocate(1, sizeof(int));
    new->isPtr = 0;
    return new;
}

unsigned long hash(unsigned char *str) { /* wonderful lil algorithm called djb2. written by someone much smarter than me. used in programming for ages. no fuckign clue how it works. */
    unsigned long brickofhash = 5381; int c;
    while ((c = *str++)) brickofhash = ((brickofhash << 5) + brickofhash) + c; /* hash * 33 + c */
    return brickofhash;
}

HashMap create_hashmap(int buckets) { /* CONSTRUCTORS!? OBJECTS!? IN MY C CODE!? WHAT THE FUCK IS THIS, JAVA!? */
    HashMap new;
    new.buckets = buckets;
    new.elementCount = 0;
    new.items = (LinkedList *)callocate(buckets, sizeof(LinkedList));
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
    newListItem = (listItem *)callocate(1, sizeof(listItem));
    newItem = (hashMapItem *)mallocate(sizeof(hashMapItem));
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

void freeAndPrint(char *allocated) { if (allocated != NULL) { printf("%s", allocated); free(allocated); }}

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

void freeVariable(variable *var) { if (var->isPtr == 0) { if (*var->type == STR && var->data->str != NULL) { free(var->data->str); } free(var->data); free(var->type); } free(var); }
void freeLinkedList(LinkedList *lis) { listItem *current = lis->first; while (current != NULL) { listItem *next = current->next; free(current); current = next; }}
void freeList(list *lis) { if (lis->isAlias == 0) { int i; for (i = 0; i < *lis->elements; i++) { freeVariable(lis->variables[i]); } free(lis->variables); free(lis->elements); } free(lis); }
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
void freeInstructionSet(InstructionSet *isa) { freeHashMap(isa->operators); freeHashMap(isa->prefixes); }
void cleanFile(openFile *file) { /* cleans a file for re-execution */
    freeHashMap(file->variables); freeHashMap(file->lists); freeHashMap(file->labels); freeHashMap(file->functions);
    file->labels.items = NULL; file->lists.items = NULL; file->variables.items = NULL; file->functions.items = NULL; file->programCounter = 0;
}
void freeFile(openFile file) {
    int i;
    DEBUG_PRINT("freeing instructions\n");
    if (file.instructions != NULL) { for (i = 0; i < file.instructionCount; i++) { freeInstruction(file.instructions[i]); } free(file.instructions); }
    cleanFile(&file);
    if (file.path != NULL) { free(file.path); }
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

void handleError(char *errorMsg, int errCode, int fatal, openFile *file) { /* more often than not, you will likely cause sOME sort of minor memory leak when this is called, as not everything has been properly cleaned up. */
    char *badInstruction = unParseInstructions(file->instructions[file->programCounter]);
    if (fatal) { /* veni, veni, venias; ne me mori facias */
        printf("Fatal error: %s of code %d:\n%s\nPress 'enter' to quit...\n", errorMsg, errCode, badInstruction);
        freeFile(*file);
        freeInstructionSet(&ValidInstructions);
        getchar();
        free(badInstruction);
        exit(errCode); /* theres no real errcodes but we're gonna pretend we do */
    } else {
        char string[512];
        sprintf(string, "A non-fatal error %d (%s) has occurred on this line: \n%s\nYou are being entered into the SIMAS command line.\nType \"!help\" for a list of helpful commands.\n", errCode, errorMsg, badInstruction);
        cleanFile(file);
        free(badInstruction);
        if (!commandPrompt) { beginCommandLine(string, file); }
        else { puts(string); commandPrompt = 2; }
    }
}

char *stripSemicolon(char *input) { int position; char *string = stroustrup(input); if (strlen(input) == 0) { return string; } position = (int)strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; return string; }
char *lowerize(char *input) { int i, len; char *string = stroustrup(input); if (strlen(input) == 0) { return string; } len = (int)strlen(string); for (i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); } return string; }
void lowerizeInPlace(char *string) { int i, len = (int)strlen(string); for (i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); }}
void stripSemicolonInPlace(char *string) { int i, len = (int)strlen(string); for (i = 0; i < len; i++) { if (string[i] == ';') { string[i] = '\0'; }}}

void formatEscapes(char *string) {
    int i, sizeOf = strlen(string);
    for (i = 0; i < sizeOf - 1; i++) { 
        if (string[i] == '\\') {
            char insertion = '\0'; 
            switch(tolower((unsigned char)string[i + 1])) {
                case 'r': insertion = '\r'; break;
                case 'n': insertion = '\n'; break;
                case 't': insertion = '\t'; break;
                case '\\': insertion = '\\'; break;
                default: continue; /* just in case there's no actual escape sequence */
            }
            string[i] = insertion;
            memmove(string + i + 1, string + 2 + i, (sizeOf - i - 1));
            string[sizeOf] = '\0'; 
            sizeOf -= 1;
        }
    } 
}

char *joinStringsSentence(char **strings, int stringCount, int offset) {
    char *finalString = NULL; int i, sizeOf = 0;
    if (stringCount == 1) { finalString = stroustrup(strings[0]); formatEscapes(finalString); return finalString; }
    for (i = offset; i < stringCount; i++) { sizeOf += strlen(strings[i]) + 1;}
    finalString = (char *)callocate(sizeOf + 1, sizeof(char)); if (finalString == NULL) cry("unable to string\nplease do not the string\n"); 
    for (i = offset; i < stringCount; i++) { strcat(finalString, strings[i]); if (i + 1 < stringCount) { strcat(finalString, " "); }} /* fuck yo optimization */
    formatEscapes(finalString);
    return finalString;
}

char *buildStringFromInstruction(instruction *inst) {
    char *joined;
    if (inst->prefix != NULL) { char **temp; temp = (char **)mallocate(sizeof(char *) * 2); temp[0] = inst->prefix; temp[1] = inst->operation; joined = joinStringsSentence(temp, 2, 0); free(temp); }
    else { joined = stroustrup(inst->operation); }
    return joined;
}

int findNumberArgs(instruction *inst, InstructionSet isa) { 
    char *temp = buildStringFromInstruction(inst);
    operator *search = ((operator *)searchHashMap(&isa.operators, temp));
    free(temp); 
    if (search != NULL) { return search->minArgs; } return -1; 
}

int trueOrFalse(char *string) {
    char *check = lowerize(string); int value = 0;
    if (strcmp(check, "true") == 0) { value = 1; }
    else if (strcmp(check, "false") == 0) { value = 0; }
    free(check);
    return value;
}

void set_variable_value(variable *var, int type, char *value, double num, int bool) { /* too fucking lazy to pass in one at a time or wutever, so just pass in all of them manually, even if some are blank :3 */
    if (*var->type == STR && var->data->str) free(var->data->str); 
    var->data->str = NULL;
    if (type == NUM) { var->data->num = num; }
    else if (type == BOOL) { var->data->boolean = bool; }
    else if (type == STR) {
        if (!value) cry("No value passed in!\n");
        var->data->str = stroustrup(value);
        if (var->data->str == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    }
    *var->type = type;
    DEBUG_PRINTF("\nvariable now has value %s, %f, %d\n", value, (float)num, bool);
}

instruction *add_instruction(char *inst, char *arguments[], char *prefix, int args) {
    int i; char *ins = stripSemicolon(inst);
    instruction *instruct = (instruction *)mallocate(sizeof(instruction));
    if (args >= 1) { instruct->arguments = (char **)mallocate(sizeof(char*) * args); for (i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }}
    else { instruct->arguments = NULL; }
    instruct->operation = stroustrup(ins); instruct->argumentCount = args;
    if (prefix != NULL) { instruct->prefix = stroustrup(prefix); }
    else { instruct->prefix = NULL; }
    DEBUG_PRINTF("added instruction %s of arg count %d\n", instruct->operation, instruct->argumentCount);
    free(ins);
    return instruct;
}

void addOperator(char *name, char *prefix, void (*functionPointer)(void*), int minimumArguments) { 
    operator *op = (operator *)mallocate(sizeof(operator)); char *joined;
    op->functionPointer = functionPointer; 
    op->minArgs = minimumArguments;
    if (prefix != NULL) { char **temp; temp = (char **)mallocate(sizeof(char *) * 2); temp[0] = prefix; temp[1] = name; joined = joinStringsSentence(temp, 2, 0); free(temp); }
    else { joined = stroustrup(name); }
    addItemToMap(&ValidInstructions.operators, op, joined, free);
    free(joined);
}

void addCommand(char *name, char *(*commandPointer)(instruction*, openFile*)) { 
    command *cmd = (command *)mallocate(sizeof(command));
    cmd->commandPointer = commandPointer;
    addItemToMap(&ValidCommands, cmd, name, free);
}

void varcpy(variable *dest, variable *src) { 
    if (*src->type == NUM) set_variable_value(dest, *src->type, NULL, src->data->num, 0); 
    if (*src->type == STR) set_variable_value(dest, *src->type, src->data->str, 0.0, 0); 
    if (*src->type == BOOL) set_variable_value(dest, *src->type, NULL, 0.0, src->data->boolean); 
}

void listcpy(list *dest, list *src) {
    int i;
    if (dest->variables != NULL) { for (i = 0; i < *dest->elements; i++) { freeVariable(dest->variables[i]); } free(dest->variables); }
    *dest->elements = *src->elements; 
    dest->variables = (variable **)mallocate(*src->elements * sizeof(variable *)); 
    for (i = 0; i < *src->elements; i++) {
        dest->variables[i] = create_variable();
        varcpy(dest->variables[i], src->variables[i]);
    }
}

int grabType(char *input) {
    char *type = lowerize(input);
    if (strcmp(type, "str") == 0) { free(type); return STR; }
    else if (strcmp(type, "num") == 0) { free(type); return NUM; }
    else if (strcmp(type, "bool") == 0) { free(type); return BOOL; }
    else if (strcmp(type, "in") == 0) { free(type); return IN; }
    else { free(type); return -1; }
}

void appendElementToList(list *li, variable *var) {
    variable *new = create_variable(), **tmp;
    varcpy((variable *)new, var);
    tmp = (variable **)reallocate(li->variables, (*li->elements + 1) * sizeof(variable *)); /* grow that bitch */
    if (tmp) { li->variables = tmp; } else { cry("reallocation failed!\n"); }
    li->variables[*li->elements] = new; *li->elements += 1;
}

void removeElementFromList(list *li, int element) {
    variable **tmp; size_t reallocSize = (*li->elements - 1) * sizeof(variable *);
    freeVariable(li->variables[element]);
    memmove(li->variables + element, li->variables + element + 1, (*li->elements - element - 1) * sizeof(variable *));
    if (reallocSize == 0) reallocSize = sizeof(variable *);
    tmp = (variable **)reallocate(li->variables, reallocSize); /* shrink that bitch */
    if (tmp) { li->variables = tmp; } else { cry("reallocation failed!\n"); }
    *li->elements -= 1;
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
    if (*var.type == STR) { return strlen(var.data->str); }
    else if (*var.type == BOOL) { return var.data->boolean ? strlen("true") : strlen("false"); }
    else if (*var.type == NUM) { return grabLengthOfNumber(var.data->num); }
    else { return 0; }
}

/* ze horsemen of type feckery */
double coerceStringToNum(char *string) {
    char *converted = lowerize(string); double val = 0.0f;
    if (!converted) return 0.0f;
    /* coerce from booleans */
    if (strcmp(converted, "true") == 0) { val = 1.0f; } 
    else if (strcmp(converted, "false") == 0) { val = 0.0f; }
    /* and then give up and use atof */
    else { val = atof(converted); }
    free(converted); return val;
}

int coerceStringToBool(char *string) {
    char *converted = lowerize(string); int val = 0;
    if (!converted) return 0;
    if (strcmp(converted, "true") == 0) { val = 1; } 
    else if (strcmp(converted, "false") == 0) { val = 0.; }
    else { val = atoi(converted) ? 1 : 0; } /* ternary fuckery to set to 1 if true */
    free(converted); return val;
}

char *stringFromVar(variable *var) { /* this already does the job of coercing strings over */
    if (*var->type == STR) { return stroustrup(var->data->str); }
    else if (*var->type == BOOL) { return var->data->boolean ? stroustrup("true") : stroustrup("false"); }
    else if (*var->type == NUM) { return grabStringOfNumber(var->data->num); }
    else { return NULL; }
}

double numFromVar(variable *src) {
    if (src == NULL) return 0.0f;
    switch (*src->type) {
        case NUM: return src->data->num;
        case STR: return coerceStringToNum(src->data->str);
        case BOOL: return (double)src->data->boolean;
        default: return 0.0f;
    }
}

int boolFromVar(variable *var) {
    switch (*var->type) {
        case NUM: return var->data->num ? 1 : 0;
        case BOOL: return var->data->boolean; 
        case STR: return coerceStringToBool(var->data->str); 
        default: return 0;
    }
}

void preprocessLabels(openFile *new) {
    int i;
    new->labels = create_hashmap(new->labels.buckets); 
    for (i = 0; i < new->instructionCount; i++) {
        if (strcmp(new->instructions[i]->operation, "label") == 0) { 
            int *location = (int *)mallocate(sizeof(int));
            *location = i - 1;
            if (new->instructions[i]->arguments[0][0] == '$') { free(location); new->programCounter = i; handleError("name is reserved", 99, 0, new); }
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
    if (isspace(check) || check == '\0') return 1; /* same thang but also checks nullterms */
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
    if (searchHashMap(&isa.prefixes, tokenized[index]) != NULL) { prefix = tokenized[index]; index += 1; lowerizeInPlace(tokenized[index]); }
    lowerizeInPlace(tokenized[index]); operation = tokenized[index]; index += 1;
    argc = arrCount - index; 
    new = add_instruction(operation, tokenized + index, prefix, argc);
    if (argc >= 1) { for (i = 0; i < argc; i++) { DEBUG_PRINTF("instruction %s has arg \"%s\"\n", operation, tokenized[index + i]); }}
    for (i = 0; i < arrCount; i++) { free(tokenized[i]); } free(tokenized); 
    return new;
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

    if (file == NULL) { printf("failed to find a simas file!\n"); return new; }

    new.path = stroustrup(path);

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
            if (strcmp(new.instructions[new.instructionCount]->operation, "label") == 0 && new.instructions[new.instructionCount]->prefix == NULL) new.labels.buckets += 1;
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
        commandPrompt = 1; printf("$ ");
        value = grabUserInput(256);
        if (!value) { handleError("Unable to allocate memory\n", 10, 1, passed); }
        temp = stripSemicolon(value); strip(temp, ' '); 
        if (strcmp(temp, "") == 0) {free(value); free(temp); continue;} /* blank check */
        free(temp);
        inst = parseInstructions(value, ValidInstructions);
        if (inst->operation[0] == '!') {
            command *cmd = ((command *)searchHashMap(&ValidCommands, inst->operation));
            if (cmd != NULL) {
                char *ret = cmd->commandPointer(inst, passed);
                if (ret == NULL) { free(value); break; }
                printf("%s", ret);
            } else { 
                printf("invalid command\n"); 
            }
        } else if (findNumberArgs(inst, ValidInstructions) == -1 && strchr(inst->operation, '@') == NULL) {
            printf("invalid instruction\n");
        } else {
            if (strchr(value, ';') != NULL && inst->argumentCount >= findNumberArgs(inst, ValidInstructions)) {
                passed->instructions = (instruction **)reallocate(passed->instructions, sizeof(instruction *) * (passed->instructionCount + 1));
                if (passed->instructions == NULL) { free(value); handleError("Reallocation of memory failed\n", 11, 1, passed); break; }
                passed->instructions[passed->instructionCount] = add_instruction(inst->operation, inst->arguments, inst->prefix, inst->argumentCount);
                passed->instructionCount += 1;
                if (strcmp(inst->operation, "label") == 0 && inst->prefix == NULL) passed->labels.buckets += 1;
                printf("ok\n");
            } else if (inst->argumentCount < findNumberArgs(inst, ValidInstructions)) {
                printf("too little arguments for instruction\n");
            } else {
                printf("code must end with a semicolon\n");
            }
        }

        freeInstruction(inst); free(value);
    }
    
    freeHashMap(ValidCommands);
    freeFile(*passed);
    freeInstructionSet(&ValidInstructions);

    exit(0);
}

/* actual function code / helpers                                                       */
void convert(variable *var, int type) {
    DEBUG_PRINTF("\nConverted from type %d to type %d\n", *var->type, type);
    if (*var->type != type) {
        if (*var->type == NUM) {
            if (type == BOOL) {
                if (var->data->num != 0.0) { var->data->boolean = 1; }
                else { var->data->boolean = 0; }
            } else if (type == STR) {
                var->data->str = grabStringOfNumber(var->data->num);
            }
        } else if (*var->type == BOOL) {
            int truth = var->data->boolean;
            if (type == NUM) { var->data->num = (double)truth; }
            else if (type == STR) {
                if (truth) var->data->str = stroustrup("true");
                if (!truth) var->data->str = stroustrup("false");
            }
        } else if (*var->type == STR) {
            char *temp = stroustrup(var->data->str); if (var->data->str != NULL) { free(var->data->str); }
            if (type == BOOL) { var->data->boolean = trueOrFalse(temp); }
            else if (type == NUM) { var->data->num = atof(temp); }
            free(temp);
        }
        *var->type = type;
    } 
    else if (*var->type == type) { /* do nothing, as you cannot convert to the same type, fucknuts */ } 
    else { cry("invalid variable type.\n"); }
}

int areTwoVarsEqual(variable *var1, variable *var2) {
    if (*var1->type == STR) { 
        char *temp = stringFromVar(var2); int ret = 0; 
        if (temp == NULL) { return 0; } if (var1->data->str == NULL) { free(temp); return 0; }
        ret = strcmp(var1->data->str, temp) ? 0 : 1; /* it shall only return true IF the comparison is true, which for strcmp is zero, oddly enoguh */
        free(temp);
        return ret; 
    }
    else if (*var1->type == NUM && var1->data->num == numFromVar(var2)) { return 1; }
    else if (*var1->type == BOOL && var1->data->boolean == boolFromVar(var2)) { return 1; }
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
        val = stroustrup(value); 
    }
    set_variable_value(var, type, val, num, bool);
    if (val) free(val);
}

void negateBoolean(variable *var) { int result = !boolFromVar(var); set_variable_value(var, BOOL, NULL, 0, result); }
void writeFromVar(variable *var, char *path) { char *variable = stringFromVar(var); writeFile(path, variable); free(variable); } 
void labelJump(int *location, int *programCounter) { *programCounter = *location; }
void equalityCheckVarVsConst(HashMap *varMap, char **arguments, int flip) {
    variable *var1 = searchHashMap(varMap, arguments[1]), *var2 = create_variable();
    int output = 0, type = grabType(arguments[0]); *var2->type = type; var2->data->str = NULL;
    if (type == NUM) { var2->data->num = atof(arguments[2]); }
    else if (type == STR) { var2->data->str = stroustrup(arguments[2]); }
    else if (type == BOOL) { var2->data->boolean = trueOrFalse(arguments[2]); } 
    if (var1 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, var2) : areTwoVarsEqual(var1, var2);
    if (*var1->type == STR && var1->data->str != NULL) free(var1->data->str);
    *var1->type = BOOL;
    var1->data->boolean = output;
    freeVariable(var2);
}

void equalityCheckVarVsVar(variable *var1, variable *var2, int flip) {
    int output = 0;
    if (var1 == NULL || var2 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, var2) : areTwoVarsEqual(var1, var2);
    if (*var1->type == STR && var1->data->str != NULL) free(var1->data->str);
    *var1->type = BOOL;
    var1->data->boolean = output;
}

void jumpConditionally(int *location, variable *var, int *programCounter, int flip) {
    int allowed = boolFromVar(var);
    if (flip) { allowed = !allowed; }
    if (allowed) labelJump(location, programCounter);
}

variable *createVarIfNotFound(HashMap *varMap, char *name) {
    variable *var = searchHashMap(varMap, name);
    if (!var) { var = create_variable(); addItemToMap(varMap, var, name, (void (*)(void *))freeVariable); }
    return var; 
}

void standardMath(openFile *current, char **arguments, char operation) {
    double op1 = 0, op2 = 0, result = 0; int returnType = grabType(arguments[0]); char *numStr = NULL;
    variable *var1 = searchHashMap(&current->variables, arguments[1]), *var2 = searchHashMap(&current->variables, arguments[2]); 
    if (var1 == NULL) { var1 = create_variable(); addItemToMap(&current->variables, var1, arguments[1], (void(*)(void *))freeVariable); *var1->type = NUM; }
    op1 = numFromVar(var1);
    if (var2 == NULL) { op2 = coerceStringToNum(arguments[2]); }
    else { op2 = numFromVar(var2); }
    DEBUG_PRINTF("%f %c %f\n", op1, operation, op2);
    switch (operation) {
        case '+': result = op1 + op2; break;
        case '-': result = op1 - op2; break;
        case '*': result = op1 * op2; break;
        case '/': if (op2 == 0.0) { handleError("div by zero", 49, 0, current); } else{ result = op1 / op2; } break;
        default: op1 = 0;
    }
    DEBUG_PRINTF("%f\n", result);
    if (returnType == STR) numStr = grabStringOfNumber(result);
    set_variable_value(var1, returnType, numStr, result, (result ? 1 : 0));
    if (numStr) free(numStr);
}

void variableSet(openFile *current, char **arguments, int argumentCount) {
    int type = grabType(arguments[0]); char *concatenated = NULL;
    if (arguments[1][0] == '$') handleError("name is reserved", 99, 0, current);
    if (type == STR) concatenated = joinStringsSentence(arguments, argumentCount, 2);
    switch (type) {
        case IN: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, NULL, 0, 0); break;
        case STR: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, concatenated, 0.0, 0); break;
        case NUM: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, NULL, atof(arguments[2]), 0); break; 
        case BOOL: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, NULL, 0.0, trueOrFalse(arguments[2])); break;
        default: handleError("invalid type specification", 30, 0, current);
    }
    free(concatenated);
}
void grabTypeFromVar(variable check, variable *var) {
    switch (*check.type) {
        case NUM: set_variable_value(var, STR, "num", 0.0, 0); break;
        case BOOL: set_variable_value(var, STR, "bool", 0.0, 0); break;
        case STR: set_variable_value(var, STR, "str", 0.0, 0); break;
        default: set_variable_value(var, STR, "none", 0.0, 0); break;
    }
}

void compareNums(HashMap *varMap, char **arguments, char operation) {
    variable *var1 = searchHashMap(varMap, arguments[1]), *var2 = searchHashMap(varMap, arguments[2]);
    double operand1 = 0, operand2 = 0; int result = 0, returnType = grabType(arguments[0]); char *boolStr;
    if (var1 == NULL) { var1 = create_variable(); addItemToMap(varMap, var1, arguments[1], (void(*)(void *))freeVariable); *var1->type = NUM; }
    operand1 = numFromVar(var1); 
    if (var2 == NULL) { operand2 = atof(arguments[2]); }
    else { operand2 = numFromVar(var2); }
    switch (operation) {
        case '>': result = (operand1 > operand2); break;
        case ']': result = (operand1 >= operand2); break;
        case '<': result = (operand1 < operand2); break;
        case '[': result = (operand1 <= operand2); break;
        default: result = 0; break;
    }
    if (returnType == STR) boolStr = result ? "true" : "false"; /* it's just gonna get strdup'd anyways */
    set_variable_value(var1, returnType, boolStr, (double)result, result);
}

void compareBools(HashMap *varMap, char **arguments, char operation, char flip) {
    variable *var1 = searchHashMap(varMap, arguments[1]), *var2 = searchHashMap(varMap, arguments[2]);
    int operand1 = 0, operand2 = 0, result = 0, returnType = grabType(arguments[0]); char *boolStr = NULL;
    if (var1 == NULL) { var1 = create_variable(); addItemToMap(varMap, var1, arguments[1], (void(*)(void *))freeVariable); *var1->type = BOOL; } 
    operand1 = boolFromVar(var1); 
    if (var2 == NULL) { operand2 = coerceStringToBool(arguments[2]); }
    else { operand2 = boolFromVar(var2); }
    switch (operation) {
        case '&': result = (operand1 && operand2); break;
        case '|': result = (operand1 || operand2); break;
        case '!': result = (operand1 != operand2); break;
        default: result = 0; break;
    }
    if (flip) result = !result;
    if (returnType == STR) boolStr = result ? "true" : "false";
    set_variable_value(var1, returnType, boolStr, (double)result, result);
}

char *formatList(list li) {
    char *final; int i; size_t bytes = 3;
    for (i = 0; i < *li.elements; i++) {
        bytes += stringLenFromVar(*li.variables[i]) + 2; 
        if (*li.variables[i]->type == STR) { bytes += 2; } /* quotes */
    }
    final = (char *)malloc(bytes); if (final == NULL) cry("List Formatting failed!");
    final[0] = '\0';

    strcat(final, "[");
    for (i = 0; i < *li.elements; i++) {
        char *temp = stringFromVar(li.variables[i]);
        if (temp) {
            if (*li.variables[i]->type == STR) strcat(final, "\"");
            strcat(final, temp);
            if (*li.variables[i]->type == STR) strcat(final, "\"");
            free(temp);
            if (i + 1 != *li.elements) strcat(final, ","); /* make sure no trailing comma is left */
        }
    }
    strcat(final, "]");
    return final;
}

void unFormatList(list *li, char *string) {
    int type = 0, i, j, start = 0; variable *var = create_variable();
    while (1) { if (string[start] == '[') { break; } start += 1; }
    for (i = start; i < (int)strlen(string); i++) {
        char *value, c = string[i]; int length = 0;
        if (c == ']') break; 
        if (c == '[' || c == ',') continue;
        if (c == '"') { type = STR; continue; }
        if (*var->type == STR && var->data->str != NULL) { free(var->data->str); }
        if (type != STR) { if (isdigit(c)) { type = NUM; } else { type = BOOL; }}

        while ((c = string[i + length]) != ',' && (c = string[i + length]) != '"' && (c = string[i + length]) != '[' && (c = string[i + length]) != ']') { length += 1; DEBUG_PRINT(&c); }

        value = (char *)callocate(length + 1, sizeof(char));
        for (j = 0; j < length; j++) { value[j] = string[i + j]; }
        i += length; 
        value[length] = '\0';
        DEBUG_PRINT(value);

        *var->type = type;
        if (type == NUM) { var->data->num = atof(value); }
        else if (type == STR) { var->data->str = stroustrup(value); }
        else if (type == BOOL) { var->data->boolean = trueOrFalse(value); }
        appendElementToList(li, var); type = 0; free(value);
    }
    freeVariable(var);
}

void loadList(HashMap *listMap, char *name, char *path) {
    list *new = searchHashMap(listMap, name); char *temp = readFile(path);
    if (new != NULL) { int i; for (i = 0; i < *new->elements; i++) { freeVariable(new->variables[i]); } *new->elements = 0; new->variables = (variable **)realloc(new->variables, sizeof(variable *)); } /* to preserve it for aliases */
    else { new = (list *)callocate(1, sizeof(list)); addItemToMap(listMap, new, name, (void (*)(void *))freeList); }
    unFormatList(new, temp); free(temp);
}

variable *indexList(openFile *current, list *li, char *indexArg) {
    int index; variable *src = searchHashMap(&current->variables, indexArg);
    if (src != NULL) { index = numFromVar(src); } else { index = atoi(indexArg); }
    if (index > *li->elements || index < 1) handleError("invalid list index", 20, 0, current);
    return (li->variables[index - 1]);
}

void listAppendConstant(list *li, char **arguments, int argumentCount) {
    int type = grabType(arguments[1]);
    variable *var = create_variable(); *var->type = type;
    if (type == NUM) { var->data->num = coerceStringToNum(arguments[2]); }
    else if (type == BOOL) { var->data->boolean = coerceStringToBool(arguments[2]); }
    else if (type == STR) { var->data->str = joinStringsSentence(arguments, argumentCount, 2); }
    appendElementToList(li, var); freeVariable(var);
}

void listUpdateConstant(openFile *current, list *li, char **arguments, int argumentCount) {
    char *sentence = joinStringsSentence(arguments, argumentCount, 3); 
    variable *var = create_variable();
    set_variable_value(var, grabType(arguments[2]), sentence, atof(arguments[3]), trueOrFalse(arguments[3]));  
    free(sentence); varcpy(indexList(current, li, arguments[1]), var); freeVariable(var);
}

void setPointer(openFile *current, variable *src, char *name) {
    variable *new;
    if (!src) handleError("invalid pointer target", 19, 0, current);
    if (name[0] == '$') handleError("name is reserved", 99, 0, current);
    new = searchHashMap(&current->variables, name);
    if (new != NULL && !new->isPtr) { handleError("cannot convert a variable to pointer", 36, 0, current); }
    else if (new == NULL) new = (variable *)callocate(1, sizeof(variable)); 
    new->data = src->data; new->type = src->type;
    new->isPtr = 1; addItemToMap(&current->variables, new, name, (void(*)(void *))freeVariable);
}

void setAlias(openFile *current, list *src, char *name) {
    list *new;
    if (!src) handleError("invalid list target", 27, 0, current);
    if (name[0] == '$') handleError("name is reserved", 99, 0, current);
    new = searchHashMap(&current->lists, name);
    if (new != NULL && !new->isAlias) { handleError("cannot convert a list to alias", 37, 0, current); }
    else if (new == NULL) new = (list *)callocate(1, sizeof(list)); 
    new->variables = src->variables; new->elements = src->elements;
    new->isAlias = 1; addItemToMap(&current->lists, new, name, (void(*)(void *))freeList);
}

void registerFunction(openFile *caller, char **arguments, int argumentCount) {
    int i; function *new = (function *)mallocate(sizeof(function)); 
    if (arguments[0][0] == '$') handleError("name is reserved", 99, 0, caller);
    if (argumentCount < 2) { free(new); handleError("too little arguments", 57, 0, caller); }
    new->parameterCount = atoi(arguments[1]);
    new->start = caller->programCounter; 
    for (i = caller->programCounter; i < caller->instructionCount; i++) { if (strcmp(caller->instructions[i]->operation, "end") == 0) { new->end = i; break; }}
    if (i == new->start) { free(new); handleError("no end to function", 84, 0, caller); }
    addItemToMap(&caller->functions, new, arguments[0], free); caller->programCounter = new->end;
}

void executeFunction(openFile *caller, char **arguments, int argumentCount) { /* monolith */
    int returnSpot = caller->programCounter, i, varCount = 0, listCount = 0, varIndex = 0, listIndex = 0;  /* ahhhh yes, we love ANSI "no mixed code with declarations!" C. */
    function *func = searchHashMap(&caller->functions, arguments[0]);
    /* we need to ensure that even in functions in function, it will retain the data till we ACTUALLY need to be rid of it                                          */
    /* this, however, comes at the cost of performance. however, this performance penalty is made up for in the lack of need to modify other aspects of the code.   */
    char **varNames, **listNames, *types, *funcName = arguments[0]; variable **varPtrs; list **listPtrs;
    if (func == NULL) handleError("invalid function", 38, 0, caller);
    DEBUG_PRINT(funcName);
    if (argumentCount < (func->parameterCount * 2) + 1) handleError("too little arguments", 57, 0, caller);
    if (func->parameterCount > 0) {
        char **tempNames = (char **)mallocate(sizeof(char *) * func->parameterCount);
        types = (char *)mallocate(sizeof(char) * func->parameterCount);
        for (i = 0; i < func->parameterCount; i++) {
            char *varName, *temp, varType = tolower(arguments[i * 2 + 1][0]);
            temp = grabStringOfNumber((double)i + 1.0f);
            varName = (char *)callocate(strlen(temp) + 2, sizeof(char));
            varName[0] = '$'; strcat(varName, temp);
            free(temp); tempNames[i] = varName; types[i] = varType;
            if (varType == 'l' || varType == 'a') { listCount += 1;}
            else if (varType == 'v' || varType == 's' || varType == 'b' || varType == 'n' || varType == 'p') { varCount += 1; }
            else { free(tempNames); handleError("invalid type specification", 30, 0, caller); }
        }    
        if (listCount > 0) { listNames = (char **)mallocate(sizeof(char *) * listCount); listPtrs = (list **)callocate(listCount, sizeof(list *)); } 
        if (varCount > 0) { varNames = (char **)mallocate(sizeof(char *) * varCount); varPtrs = (variable **)callocate(varCount, sizeof(variable *)); }
        for (i = 0; i < func->parameterCount; i++) { 
            if (types[i] == 'l' || types[i] == 'a') { 
                list *src = searchHashMap(&caller->lists, arguments[i * 2 + 2]), *test, *li = (list *)callocate(1, sizeof(list));
                listNames[listIndex] = tempNames[i]; 
                if (src == NULL) { free(li); free(tempNames); free(listNames); if (varNames) { free(varNames); } handleError("list expected", 26, 0, caller); }
                if (types[i] == 'l') { listcpy(li, src); }
                else { li->variables = src->variables; li->isAlias = 1; li->elements = src->elements; }
                listPtrs[listIndex] = li;
                test = searchHashMap(&caller->lists, listNames[listIndex]);
                if (test != NULL) { deleteItemFromMap(&caller->lists, listNames[listIndex]); } 
                addItemToMap(&caller->lists, li, listNames[listIndex], NULL); 
                listIndex += 1;
            } else { /* since we check types earlier it's safe to assume any non-'l' type will be a var */
                variable *newVar = create_variable(), *old, *test;
                varNames[varIndex] = tempNames[i]; varPtrs[varIndex] = newVar;
                switch (types[i]) {
                    case 'v': old = searchHashMap(&caller->variables, arguments[i * 2 + 2]); if (old == NULL) { free(tempNames); freeVariable(newVar); if (listNames) { free(listNames); } free(varNames); handleError("var expected", 26, 0, caller); } varcpy(newVar, old); break;
                    case 'n': *newVar->type = NUM; newVar->data->num = atof(arguments[i * 2 + 2]); break;
                    case 's': *newVar->type = STR; newVar->data->str = stroustrup(arguments[i * 2 + 2]); break;
                    case 'b': *newVar->type = BOOL; newVar->data->boolean = trueOrFalse(arguments[i * 2 + 2]); break;
                    case 'p': old = searchHashMap(&caller->variables, arguments[i * 2 + 2]); if (old == NULL) { free(tempNames); freeVariable(newVar); if (listNames) { free(listNames); } free(varNames); handleError("var expected", 26, 0, caller); } free(newVar->data); free(newVar->type); newVar->data = old->data; newVar->type = old->type; newVar->isPtr = 1; break;
                }
                test = searchHashMap(&caller->variables, varNames[varIndex]);
                if (test != NULL) { /* there shouldn't be any other $i vars */
                    if (test->data->str != NULL && *test->type == STR) free(test->data->str);
                    deleteItemFromMap(&caller->variables, varNames[varIndex]); 
                }
                addItemToMap(&caller->variables, newVar, varNames[varIndex], NULL);
                varIndex += 1;
            }
        }
        free(tempNames); free(types);
    }
    caller->programCounter = func->start + 1;
    while (strcmp(caller->instructions[caller->programCounter]->operation, "ret")) {
        for (i = 0; i < varCount; i++) { if (searchHashMap(&caller->variables, varNames[i]) == NULL) { addItemToMap(&caller->variables, varPtrs[i], varNames[i], NULL); }} /* re-adds any missing variables, should another function have prematurely deleted/overwritten it */
        for (i = 0; i < listCount; i++) { if (searchHashMap(&caller->lists, listNames[i]) == NULL) { addItemToMap(&caller->lists, listPtrs[i], listNames[i], NULL); }}
        executeInstruction(caller); caller->programCounter += 1;
        if (caller->programCounter == func->end) caller->programCounter = func->start + 2;
    }
    arguments = caller->instructions[caller->programCounter]->arguments; argumentCount = caller->instructions[caller->programCounter]->argumentCount; /* grab the current arguments */
    if (argumentCount >= 2) {
        char *retName = (char *)callocate(strlen(funcName) + 2, sizeof(char)), returnType = tolower(arguments[0][0]); 
        retName[0] = '$'; strcat(retName, funcName);
        if (returnType != 'l') {
            variable *returnedVar = create_variable(), *old;
            switch (returnType) {
                case 'v': old = searchHashMap(&caller->variables, arguments[1]); if (old == NULL) { freeVariable(returnedVar); handleError("var expected", 229, 0, caller); } varcpy(returnedVar, old); break;
                case 'n': *returnedVar->type = NUM; returnedVar->data->num = (double)atof(arguments[1]); break;
                case 's': *returnedVar->type = STR; returnedVar->data->str = stroustrup(arguments[1]); break;
                case 'b': *returnedVar->type = BOOL; returnedVar->data->boolean = trueOrFalse(arguments[1]); break;
                default: freeVariable(returnedVar); handleError("invalid type specification", 30, 0, caller); break;
            }
            old = searchHashMap(&caller->variables, retName); 
            if (old) { freeVariable(old); old = returnedVar; }
            else { addItemToMap(&caller->variables, returnedVar, retName, (void(*)(void *))freeVariable); };
        } else {
            list *src = searchHashMap(&caller->lists, arguments[1]), *returned = (list *)calloc(1, sizeof(list)), *old;
            if (src == NULL) { free(returned); handleError("list expected", 26, 0, caller); }
            listcpy(returned, src);
            old = searchHashMap(&caller->lists, retName);
            if (old) { freeList(old); old = returned; }
            else { addItemToMap(&caller->lists, returned, retName, (void(*)(void *))freeList); }
        }
        free(retName);
    }
    for (i = 0; i < varCount; i++) { deleteItemFromMap(&caller->variables, varNames[i]); free(varNames[i]); freeVariable(varPtrs[i]); }
    for (i = 0; i < listCount; i++) { deleteItemFromMap(&caller->lists, listNames[i]); free(listNames[i]); freeList(listPtrs[i]); }
    if (varCount > 0) { free(varNames); free(varPtrs); } if (listCount > 0) { free(listNames); free(listPtrs); }
    caller->programCounter = returnSpot; /* resume execution */
}

/* console i/o      */
void con_prints(openFile *file) { putc(' ', stdout); }
void con_println(openFile *file) { puts(""); }
void con_printv(openFile *file) { freeAndPrint(stringFromVar((variable *)searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]))); }
void con_printc(openFile *file) { freeAndPrint(joinStringsSentence(file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount, 0)); }
/*file i/o          */
void fio_read(openFile *file) { char *read = readFile(file->instructions[file->programCounter]->arguments[0]); set_variable_value(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1]), STR, read, 0.0, 0); free(read); }
void fio_write(openFile *file) { freeAndWrite(file->instructions[file->programCounter]->arguments[0], joinStringsSentence(file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount, 1)); }
void fio_writev(openFile *file) { writeFromVar(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), file->instructions[file->programCounter]->arguments[0]); }
/* misc             */
void etc_not(openFile *file) { negateBoolean(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]));  }
void etc_quit(openFile *file) { if (!commandPrompt) { freeFile(*file); freeInstructionSet(&ValidInstructions); exit(0); } else { cleanFile(file); commandPrompt = 2; }} /* 2 signifies it wants to ENTER the cmd prompt */
/* jumps            */
void jmp_jump(openFile *file) { labelJump(searchHashMap(&file->labels, file->instructions[file->programCounter]->arguments[0]), &file->programCounter); }
void jmp_jumpv(openFile *file) { jumpConditionally(searchHashMap(&file->labels, file->instructions[file->programCounter]->arguments[0]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), &file->programCounter, 0); }
void jmp_jumpnv(openFile *file) { jumpConditionally(searchHashMap(&file->labels, file->instructions[file->programCounter]->arguments[0]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]), &file->programCounter, 1); }
/* math             */
void mat_add(openFile *file) { standardMath(file, file->instructions[file->programCounter]->arguments, '+'); }
void mat_sub(openFile *file) { standardMath(file, file->instructions[file->programCounter]->arguments, '-'); }
void mat_mul(openFile *file) { standardMath(file, file->instructions[file->programCounter]->arguments, '*'); }
void mat_div(openFile *file) { standardMath(file, file->instructions[file->programCounter]->arguments, '/'); }
/* variable ops     */
void var_set(openFile *file) { variableSet(file, file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }
void var_type(openFile *file) { grabTypeFromVar(*(variable *)searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]), createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1])); }
void var_conv(openFile *file) { convert(searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]), grabType(file->instructions[file->programCounter]->arguments[1])); }
void var_copy(openFile *file) { if (file->instructions[file->programCounter]->arguments[1][0] == '$') { handleError("name is reserved", 99, 0, file); } varcpy(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0])); } 
void var_ptr(openFile *file) { if (file->instructions[file->programCounter]->arguments[0][0] == '$') { handleError("cannot create pointer to reserved variable", 94, 0, file); } setPointer(file, (variable *)searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments[1]); }
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
void lis_del(openFile *file) { list *li = searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]); int index; variable *src = searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[1]); if (src != NULL) { index = numFromVar(src); } else { index = atoi(file->instructions[file->programCounter]->arguments[1]); } if (index > *li->elements) { handleError("invalid index", 92, 0, file); } else { removeElementFromList(li, index - 1); }}
void lis_appv(openFile *file) { appendElementToList(searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), (variable *)searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[2])); }
void lis_show(openFile *file) { freeAndPrint(formatList(*(list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))); }
void lis_new(openFile *file) { list *new = (list *)callocate(1, sizeof(list)); if (file->instructions[file->programCounter]->arguments[0][0] == '$') { free(new); handleError("name is reserved", 99, 0, file); } new->elements = (int *)callocate(1, sizeof(int)); addItemToMap(&file->lists, new, file->instructions[file->programCounter]->arguments[0], (void (*)(void *))freeList); }
void lis_upv(openFile *file) { varcpy(indexList(file, (list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments[1]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[3])); }
void lis_acc(openFile *file) { varcpy(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[2]), indexList(file, (list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments[1])); }
void lis_load(openFile *file) { loadList(&file->lists, file->instructions[file->programCounter]->arguments[0], file->instructions[file->programCounter]->arguments[1]); }
void lis_len(openFile *file) { set_variable_value(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1]), NUM, NULL, *((list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))->elements, 0); }
void lis_dump(openFile *file) { freeAndWrite(file->instructions[file->programCounter]->arguments[1], formatList(*(list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))); }
void lis_upc(openFile *file) { listUpdateConstant(file, ((list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0])), file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); } 
void lis_appc(openFile *file) { listAppendConstant(searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }
void lis_copy(openFile *file) { list *li = searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[1]); if (!li) { li = (list *)callocate(1, sizeof(list)); li->elements = (int *)callocate(1, sizeof(int)); addItemToMap(&file->lists, li, file->instructions[file->programCounter]->arguments[1], (void (*)(void *))freeList); } listcpy(li, searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0])); }
void lis_alias(openFile *file) { if (file->instructions[file->programCounter]->arguments[0][0] == '$') { handleError("cannot create alias to reserved list", 95, 0, file); } setAlias(file, (list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments[1]); }
/* function ops     */
void fun_fun(openFile *file) { registerFunction(file, file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }
void fun_call(openFile *file) { executeFunction(file, file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }

/* command functions for the CLI */
char *cmd_quit(instruction *inst, openFile *file) { freeInstruction(inst); return NULL; } /* this is what we call a pro gamer move */
char *cmd_clear(instruction *inst, openFile *file) { freeFile(*file); memset(file, 0, sizeof(openFile)); return ""; }
char *cmd_debug(instruction *inst, openFile *file) { snadmwithc(); return ""; }
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
            if (strcmp(temporary, "") != 0 && strchr(out, ';') && instruct->argumentCount >= findNumberArgs(instruct, ValidInstructions)) { 
                freeInstruction(file->instructions[atoi(inst->arguments[0]) - 1]);
                file->instructions[atoi(inst->arguments[0]) - 1] = add_instruction(instruct->operation, instruct->arguments, instruct->prefix, instruct->argumentCount);
            } else if (strchr(out, ';') == NULL) {
                printf("code must end with a semicolon\n");
            } else if (instruct->argumentCount < findNumberArgs(instruct, ValidInstructions)) {
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
    operator *found; char *string;
    if (strlen(cur->instructions[cur->programCounter]->operation) == 0) return;
    string = buildStringFromInstruction(cur->instructions[cur->programCounter]);
    DEBUG_PRINTF("\nExecuting instruction %s on line %d.\n", string, cur->programCounter);
    found = searchHashMap(&ValidInstructions.operators, string); free(string);
    if (found != NULL && found->functionPointer != NULL) { (found->functionPointer)(cur); }
}

void executeFile(openFile *current, int doFree) {
    preprocessLabels(current); current->lists = create_hashmap(1); current->variables = create_hashmap(10); current->functions = create_hashmap(1);
    for (current->programCounter = 0; current->programCounter < current->instructionCount; current->programCounter++) { executeInstruction(current); if (commandPrompt == 2) { break; }}
    if (doFree) freeFile(*current);
}

void setUpStdlib(void) {
    ValidInstructions.operators = create_hashmap(53); ValidInstructions.prefixes = create_hashmap(1);
    addItemToMap(&ValidInstructions.prefixes, "list", "list", NULL);
    addOperator("label", NULL, NULL, 1); /* no-op */
    addOperator("end", NULL, NULL, 1); /* no-op */
    addOperator("ret", NULL, NULL, 0); /* no-op */
    addOperator("print", NULL, (void(*)(void*))con_printv, 1); 
    addOperator("println", NULL, (void(*)(void*))con_println, 0);
    addOperator("prints", NULL, (void(*)(void*))con_prints, 0); 
    addOperator("printc", NULL, (void(*)(void*))con_printc, 1); 
    addOperator("read", NULL, (void(*)(void*))fio_read, 2); 
    addOperator("write", NULL, (void(*)(void*))fio_write, 2);
    addOperator("writev", NULL, (void(*)(void*))fio_writev, 2); 
    addOperator("not", NULL, (void(*)(void*))etc_not, 1);
    addOperator("quit", NULL, (void(*)(void*))etc_quit, 0);
    addOperator("add", NULL, (void(*)(void*))mat_add, 3);
    addOperator("sub", NULL, (void(*)(void*))mat_sub, 3);
    addOperator("mul", NULL, (void(*)(void*))mat_mul, 3);
    addOperator("div", NULL, (void(*)(void*))mat_div, 3);
    addOperator("set", NULL, (void(*)(void*))var_set, 2);
    addOperator("type", NULL, (void(*)(void*))var_type, 2);
    addOperator("conv", NULL, (void(*)(void*))var_conv, 2);
    addOperator("copy", NULL, (void(*)(void*))var_copy, 2);
    addOperator("ptr", NULL, (void(*)(void*))var_ptr, 2);
    addOperator("gt", NULL, (void(*)(void*))cmp_gt, 3); 
    addOperator("gte", NULL, (void(*)(void*))cmp_gte, 3); 
    addOperator("st", NULL, (void(*)(void*))cmp_st, 3); 
    addOperator("ste", NULL, (void(*)(void*))cmp_ste, 3); 
    addOperator("eqv", NULL, (void(*)(void*))cmp_eqv, 3); 
    addOperator("neqv", NULL, (void(*)(void*))cmp_neqv, 3);
    addOperator("eqc", NULL, (void(*)(void*))cmp_eqc, 3); 
    addOperator("neqc", NULL, (void(*)(void*))cmp_neqc, 3);
    addOperator("and", NULL, (void(*)(void*))cmp_and, 3); 
    addOperator("nand", NULL, (void(*)(void*))cmp_nand, 3);
    addOperator("or", NULL, (void(*)(void*))cmp_or, 3); 
    addOperator("nor", NULL, (void(*)(void*))cmp_nor, 3);
    addOperator("xor", NULL, (void(*)(void*))cmp_xor, 3);
    addOperator("jump", NULL, (void(*)(void*))jmp_jump, 1);
    addOperator("jumpv", NULL, (void(*)(void*))jmp_jumpv, 2); 
    addOperator("jumpnv", NULL, (void(*)(void*))jmp_jumpnv, 2);
    addOperator("del", "list", (void(*)(void*))lis_del, 2); 
    addOperator("appv", "list", (void(*)(void*))lis_appv, 3); 
    addOperator("show", "list", (void(*)(void*))lis_show, 1); 
    addOperator("new", "list", (void(*)(void*))lis_new, 1); 
    addOperator("upv", "list", (void(*)(void*))lis_upv, 4); 
    addOperator("acc", "list", (void(*)(void*))lis_acc, 3); 
    addOperator("load", "list", (void(*)(void*))lis_load, 2); 
    addOperator("len", "list", (void(*)(void*))lis_len, 2); 
    addOperator("dump", "list", (void(*)(void*))lis_dump, 2); 
    addOperator("upc", "list", (void(*)(void*))lis_upc, 4); 
    addOperator("appc", "list", (void(*)(void*))lis_appc, 3); 
    addOperator("copy", "list", (void(*)(void*))lis_copy, 2); 
    addOperator("alias", "list", (void(*)(void*))lis_alias, 2);
    addOperator("copyl", NULL, (void(*)(void*))lis_copy, 2); 
    addOperator("fun", NULL, (void(*)(void *))fun_fun, 2);
    addOperator("call", NULL, (void(*)(void *))fun_call, 2);
}

void setUpCommands() {
    ValidCommands = create_hashmap(9); 
    addCommand("!quit", cmd_quit);
    addCommand("!run", cmd_run);
    addCommand("!load", cmd_load);
    addCommand("!save", cmd_save);
    addCommand("!dump", cmd_dump);
    addCommand("!clear", cmd_clear);
    addCommand("!edit", cmd_edit);
    addCommand("!help", cmd_help);
    addCommand("!debug", cmd_debug);
}

int main(int argc, const char * argv[]) {
    openFile new;
    memset(&new, 0, sizeof(openFile));
    #if ALLOC_DEBUGGING == 1
    allocatorLog = fopen("./allocs.log", "w");
    #endif

    setUpStdlib();

    if (argc >= 2) { 
        int i, maxIterations; /* support chainloading because it makes testing WAYYYY easier */
        if (strcmp(argv[argc - 1], "-d") == 0) { maxIterations = argc - 1; snadmwithc(); }
        else { maxIterations = argc; }
        for (i = 1; i < maxIterations; i++) {
            new = openSimasFile(argv[i]);
            executeFile(&new, 1);
        }
    } else { 
        beginCommandLine("CMAS (C Simple Assembly) Interpreter.\nWritten by tuvalutorture, Licensed under GNU GPLv3.\nUsing The SIMAS Programming Language, created by Turrnut.\nGitHub repo: https://github.com/tuvalutorture/simas \nType !help for a list of commands.\n", &new); 
    }

    freeInstructionSet(&ValidInstructions);

    #if ALLOC_DEBUGGING == 1
    fclose(allocatorLog);
    DEBUG_PRINTF("%d M's, %d C's, %d R's.\n%lu bytes M'd, %lu bytes C'd, %lu bytes R'd.\n", mallocationCounter, callocationCounter, reallocationCounter, mallocationBytes, callocationBytes, reallocationBytes);
    #endif

    return 0;
}
/* we are the phosphorylated constituents of bong juice ltd. */