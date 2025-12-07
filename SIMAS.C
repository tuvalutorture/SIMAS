/* CMAS (C SIMAS "SIMple ASsembly") interpreter - by tuvalutorture  */
/* pull on my lever it's, my guilty pleasure yes,                   */
/* the gods of olympus have abandonded me                           */

/* trivia time!!! did you know:                                     */
/* "G3" was a term used by apple to refer to the PowerPC 750 series */
/* CPU back in the late 90's / early 2000's when they produced      */
/* computers using the PPC 750 processor, using "G3" as a marketing */
/* term. PowerPC is a cpu architecture based on the IBM "POWER"     */
/* Architecture, and was formed as a result of Apple, IBM, and      */
/* Motorola creating a new architecture, due to the Motorola 68k    */
/* series line slowly becoming a dead-end for Apple Computers       */
/* due to its inabilities to outperform Intel and other x86 chips.  */
/* "G4" and "G5" would later be used for subsequent Apple computers */
/* featuring later PowerPC processors, with G4 referring to the     */
/* 7400/8400 families of CPU (which were G3-based 32-bit CPUs       */
/* featuring AltiVec, or "Velocity Engine"), and G5 referred to the */
/* 7500/8500 series CPUs, and were a new 64-bit architecture, as    */
/* well as were the first PowerPC processors to feature Dual-core   */
/* capabilities without requiring 2 separate CPUs. Additionally,    */
/* PowerPC would see success outside of Apple Computers, such as    */
/* the G3 would see use in platforms such as the Nintendo GameCube, */
/* or a G5-based chip being used in the Xbox 360. However, due to   */
/* rampant heat issues and unscalability of the PPC architecture    */
/* as a whole, Apple Computers would choose to ditch PowerPC        */
/* in favor of Intel's x86 offerings, which would also put it       */
/* on par with other standard home computers at the time, which     */
/* typically ran x86 processors with Windows. This switch to x86    */
/* had the unintended consequence of enabling users to run Windows  */
/* natively alongside their Mac OS X installation. At first, this   */
/* was not officially supported by Apple, but Apple later chose     */
/* to add support in Mac OS X 10.5, and is still present in the OS, */
/* but is notably missing from M-series Macintoshes, as the ARM     */
/* chips in Macs are incompatible with Windows, despite an official */
/* ARM port of Windows existing. However, the reason it cannot be   */
/* natively installed is because has no drivers nor bootloader      */
/* compatibility for the M-series Macs, as newer Macintoshes have   */
/* a more specialised boot process.                                 */

/* the automobile seatbelt was invented by John Lennon the CCXXVII  */
/* in 375 BC and 204 years later his child, John Bing the MCLXXVI   */
/* of Cornholio invented the windshield wiper in the year of our    */
/* lord 171 BC, but their inventions were lost to time in the year  */
/* 582 ACDC, and were only just now redicovered in the present day. */

/* ok ok ok you're here for code, so here's code:                   */

#define _CRT_SECURE_NO_WARNINGS // to make windows shut the everliving fuck up about deprecated functions if compiling on msvc

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define STR 1
#define NUM 2
#define BOOL 3
#define LIST 4
#define IN 5

#define DEBUG_PRINTF if (debugMode) printf // macro abuse at its finest
#define DEBUG_PRINT(string) DEBUG_PRINTF("%s", string) // macro abuse ALSO at its finest hour

int debugMode = 0;
const char poem[] = "life is like a door\nnever trust a cat\nbecause the moon can't swim\n\nbut they live in your house\neven though they don't like breathing in\ndead oxygen that's out of warranty\n\nwhen the gods turn to glass\nyou'll be drinking lager out of urns\nand eating peanut butter with mud\n\nbananas wear socks in the basement\nbecause time can't tie its own shoes\nand the dead spiders are unionising\n\nand a microwave is just a haunted suitcase\nhenceforth gravity owes me twenty bucks\nbecause the couch is plotting against the fridge\n\nwhen pickles dream in binary\nthe mountain dew solidifies\ninto a 2007 toyota corolla\n";

typedef struct {
    char *name;
    int type;
    union { // UNIONISE, MY CHILDREN! RISE AGAINST THE EMPLOYERS WHO TREAT YOU AS WAGE SLAVES! BECOME A UNION! FIGHT FOR WORKPLACE RIGHTS! GET YO 401(k)!!!
        double num;
        char *str;
        int bool;
        void *etc;
    }; 
} variable;

typedef struct {
    char *name;
    int location;
} label;

typedef struct {
    char *name;
    variable *variables;
    int elements;
} list; 

typedef struct {
    char *operation;
    int argumentCount;
    char **arguments;
    char *prefix;
} instruction;

typedef struct {
    char *path;
    instruction **instructions;
    variable *variables;
    label *labels;
    list *lists; 
    void *functions; // VERY hacky workaround to avoid circular typedefs 
    int instructionCount;
    int variableCount;
    int labelCount;
    int listCount;
    int functionCount;
    int programCounter;
} openFile;

typedef struct {
    label location;
    openFile *caller;
    openFile *parent;
    variable *passedVariables;
    char *paramTypes;
    int paramCount;
    int numVars;
    int callLocation;
    int endLocation;
} function;

typedef struct {
    char *name;
    char *prefix;
    void (*functionPointer)(openFile*); // guys i think this points or smth idk
    int minArgs;
} operator;

typedef struct {
    operator *set;
    char **prefixes;
    int count;
    int prefixCount;
} InstructionSet;

InstructionSet ValidInstructions;
openFile **currentOpenFiles = NULL; // array of pointers to currently open files
openFile *currentExecution = NULL;
function **functionStack = NULL; // i think stack is appropriate here
int functionsDeep = 0;
int openFileCount = 0;

void executeFile(openFile *current); // quick forward decl

char *strdup(const char *string) {
    int len = strlen(string) + 1;
    char *final = (char *)malloc(len * sizeof(char));
    if (!final) return NULL;
    memcpy(final, string, len);
    return final;
}

void freeAndPrint(char *allocated) { printf("%s", allocated); free(allocated); fflush(stdout); }

void freeInstruction(instruction *inst) {
    if (inst == NULL) return;
    free(inst->operation);
    for (int i = 0; i < inst->argumentCount; i++) {
        free(inst->arguments[i]);
    }
    free(inst->arguments);
    if (inst->prefix != NULL) free(inst->prefix);
    free(inst);
}

void freeVariable(variable var) { if (var.name) { free(var.name); } if (var.type == STR && var.str) { free(var.str); }}
void freeLabel(label l) { free(l.name); }
void freeList(list lis) { free(lis.name); for (int i = 0; i < lis.elements; i++) { freeVariable(lis.variables[i]); } free(lis.variables); lis.variables = NULL; }
void freeOperator(operator op) { if (op.name != NULL) { free(op.name); } if (op.prefix != NULL) { free(op.prefix); }}
void freeInstructionSet(InstructionSet isa) { if (isa.set != NULL) { for (int i = 0; i < isa.count; i++) { freeOperator(isa.set[i]); } free(isa.set); } if (isa.prefixes != NULL) { for (int i = 0; i < isa.prefixCount; i++) { free(isa.prefixes[i]); } free(isa.prefixes); }}
void freeFunction(function func) { if (func.paramTypes != NULL) { free(func.paramTypes); } if (func.location.name != NULL) { free(func.location.name); } if (func.passedVariables != NULL) { for (int i = 0; i < func.paramCount; i++) { freeVariable(func.passedVariables[i]); } free(func.passedVariables); }}
void freeFile(openFile file) {
    if (file.instructions != NULL) { for (int i = 0; i < file.instructionCount; i++) { freeInstruction(file.instructions[i]); } free(file.instructions); }
    if (file.variables != NULL) { for (int i = 0; i < file.variableCount; i++) { freeVariable(file.variables[i]); } free(file.variables); }
    if (file.labels != NULL) { for (int i = 0; i < file.labelCount; i++) { freeLabel(file.labels[i]); } free(file.labels); }
    if (file.lists != NULL) { for (int i = 0; i < file.listCount; i++) { freeList(file.lists[i]); } free(file.lists); }
    if (file.functions != NULL) { for (int i = 0; i < file.functionCount; i++) { freeFunction(((function *)file.functions)[i]); } free(file.functions); }
    if (file.path != NULL) { free(file.path); }
}

char *stripSemicolon(char *input) { char *string = strdup(input); if (strlen(input) == 0) { return string; } int position = (int)strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; return string; }
char *lowerise(char *input) { char *string = strdup(input); if (strlen(input) == 0) { return string; } int len = (int)strlen(string); for (int i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); } return string; }
void loweriseInPlace(char *string) { int len = (int)strlen(string); for (int i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); }} // mutates the string to save a couple of cycles
void stripSemicolonInPlace(char *string) { int len = (int)strlen(string); for (int i = 0; i < len; i++) { if (string[i] == ';') { string[i] = '\0'; }}}

int findNumberArgs(char *instruction, InstructionSet isa) { for (int i = 0; i < isa.count; i++) { if (strcmp(isa.set[i].name, instruction) == 0) { return isa.set[i].minArgs; }} return -1; }

void cry(char sob[]) { printf("%s", sob); exit((int)2384708919); } // this is how i feel trying to debug this

int trueOrFalse(char *string) {
    char *check = lowerise(string); int value = 0;
    if (strcmp(check, "true") == 0) { value = 1; }
    else if (strcmp(check, "false") == 0) { value = 0; }
    free(check);
    return value;
}

void set_variable_value(variable *var, int type, char *value, double num, int bool) { // too fucking lazy to pass in one at a time or wutever, so just pass in all of them manually, even if some are blank :3
    if (var->type == STR) free(var->str); 
    var->str = NULL;
    if (type == NUM) { var->num = num; }
    else if (type == BOOL) { var->bool = bool; }
    else if (type == STR) {
        if (!value) cry("No value passed in!\n");
        var->str = strdup(value);
        if (var->str == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    }
    var->type = type;
    DEBUG_PRINTF("\nvariable %s now has value %s, %lf, %d\n", var->name, value, num, bool);
}

void addFilePointer(openFile *pointer) {
    for (int i = 0; i < openFileCount; i++) {
        if (!strcmp(pointer->path, currentOpenFiles[i]->path)) {
            freeFile(*currentOpenFiles[i]); free(currentOpenFiles[i]); currentOpenFiles[i] = pointer; return;
        }
    }
    currentOpenFiles = (openFile **)realloc(currentOpenFiles, sizeof(openFile*) * (openFileCount + 1));
    currentOpenFiles[openFileCount] = pointer; ++openFileCount;
}

instruction *add_instruction(char *inst, char *arguments[], char *prefix, int args) {
    char *ins = stripSemicolon(inst);    
    instruction *instruct = (instruction *)malloc(sizeof(instruction));
    if (args >= 1) { instruct->arguments = (char **)malloc(sizeof(char*) * args); for (int i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }}
    else { instruct->arguments = NULL; }
    instruct->operation = strdup(ins); instruct->argumentCount = args;
    if (prefix != NULL) { instruct->prefix = strdup(prefix); }
    else { instruct->prefix = NULL; }
    DEBUG_PRINTF("added instruction %s of arg count %d\n", instruct->operation, instruct->argumentCount);
    free(ins);
    return instruct;
}

variable create_variable(char *name) {
    variable var = { .name = strdup(name) };
    DEBUG_PRINTF("created variable %s\n", var.name);
    return var;
}

variable create_variable_with_value(char *name, int type, char *value, double num, int bool) {
    variable var = { .name = strdup(name) };
    switch (type) {
        case STR: set_variable_value(&var, type, value, 0.0, 0); break;
        case NUM: set_variable_value(&var, type, NULL, num, 0); break;
        case BOOL: set_variable_value(&var, type, NULL, 0.0, bool); break;
        default: cry("Invalid type!\n");
    }
    DEBUG_PRINTF("created variable %s\n", var.name);
    return var;
}

label create_label(char *name, int location) {
    label label = { .name = stripSemicolon(name), .location = location };
    DEBUG_PRINTF("\ncreated label %s on line %d\n", name, location);
    return label;
}

list *create_list(char *name) {
    list *new = (list *)malloc(sizeof(list));
    new->variables = (variable *)malloc(sizeof(variable));
    memset(new->variables, 0, sizeof(variable));
    new->name = strdup(name);
    new->elements = 0;
    DEBUG_PRINTF("\ncreated list %s\n", name);
    return new;
}

operator create_operator(char *name, char *prefix, void (*functionPointer)(openFile*), int minimumArguments) {
    operator op = { .name = strdup(name), .prefix = (prefix != NULL) ? strdup(prefix) : NULL, .functionPointer = functionPointer, .minArgs = minimumArguments};
    return op;
}

list *addListToLists(list **lists, char *name, int *listCount) {
    *lists = (list *)realloc(*lists, sizeof(list) * (*listCount + 1));
    if (*lists == NULL) cry("heyo, lists failed to allocate here.");
    list *created = create_list(name);
    (*lists)[*listCount] = *created;
    free(created);
    *listCount += 1;
    return (lists)[*listCount];
}

void addOperator(operator *op, InstructionSet *isa) {
    isa->set = (operator *)realloc(isa->set, (isa->count + 1) * sizeof(operator));
    isa->set[isa->count] = *op;
    isa->count += 1;
}

void addPrefix(char *prefix, InstructionSet *isa) {
    isa->prefixes = (char **)realloc(isa->prefixes, (isa->prefixCount + 1) * sizeof(char *));
    isa->prefixes[isa->prefixCount] = strdup(prefix);
    isa->prefixCount += 1;
}

void varcpy(variable *dest, variable *src) { 
    if (src->type == NUM) { set_variable_value(dest, src->type, NULL, src->num, 0); }
    else if (src->type == STR) { set_variable_value(dest, src->type, src->str, 0.0, 0); }
    else if (src->type == BOOL) { set_variable_value(dest, src->type, NULL, 0.0, src->bool); }
}

void appendElement(list *li, variable var) {
    li->variables = (variable *)realloc(li->variables, (sizeof(variable) * (li->elements + 1))); 
    if (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
    memset(&li->variables[li->elements], 0, sizeof(variable));
    varcpy(&li->variables[li->elements], &var);
    li->elements += 1;
}

void removeElement(list *li, int element) {
    DEBUG_PRINTF("%d %d\n", element, li->elements);
    freeVariable(li->variables[element]); // free the specified element in the array
    li->elements -= 1;
    if (element != li->elements) { for (int i = element + 1; i < li->elements + 1; i++) { memcpy(&li->variables[i - 1], &li->variables[i], sizeof(variable)); }} 
    li->variables = (variable *)realloc(li->variables, sizeof(variable) * (li->elements));
    if (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
}

char *grabStringOfNumber(double num) {
    char *buffer = (char *)malloc(50 * sizeof(char) + 1);
    snprintf(buffer, 50, "%lf", num);
    int len = strlen(buffer); int zeroes = 0;
    for (int i = 1; i < 7; i++) { if (buffer[len - i] == '0') { zeroes++; buffer[len - i] = '\0'; } else break; } // yoink trailing zeroes
    if (zeroes == 6) { buffer[len - 7] = '\0'; } // terminate it if it's just trailing zeroes
    return buffer;
}

size_t grabLengthOfNumber(double num) { char *temp = grabStringOfNumber(num); size_t len = strlen(temp); free(temp); return len; }

size_t stringLenFromVar(variable var) {
    if (var.type == STR) { return strlen(var.str); }
    else if (var.type == BOOL) { return var.bool ? 4 : 5; } // optimised by just fucking yeeting the strlen check since it returns a constant string's length
    else if (var.type == NUM) { return grabLengthOfNumber(var.num); }
    else { return 0; }
}

char *stringFromVar(variable var) {
    if (var.type == STR) { return strdup(var.str); }
    else if (var.type == BOOL) { return var.bool ? strdup("true") : strdup("false"); }
    else if (var.type == NUM) { return grabStringOfNumber(var.num); }
    else { return NULL; }
}

variable *findVar(char *name, int createIfNotFound, openFile *caller) {
    int location = 0; int found = 0; 
    variable **variableSet = NULL; int *count = NULL;
    int setCount = functionsDeep + openFileCount; variable ***setsToSearch = (variable ***)malloc(setCount * (sizeof(variable **))); int **counts = (int **)malloc(setCount * sizeof(int *));
    if (functionsDeep > 0) { for (int i = 0; i < functionsDeep; i++) { setsToSearch[i] = &functionStack[functionsDeep - 1 - i]->passedVariables; counts[i] = &functionStack[functionsDeep - 1 - i]->numVars; }}
    for (int i = functionsDeep; i < setCount; i++) { setsToSearch[i] = &currentOpenFiles[i - functionsDeep]->variables; counts[i] = &currentOpenFiles[i - functionsDeep]->variableCount; }
    for (int i = 0; i < setCount; i++) {
        count = counts[i]; variableSet = setsToSearch[i];
        for (int k = 0; k < (*count); k++) {
            if (strcmp((*variableSet)[k].name, name) == 0) { 
                DEBUG_PRINTF("found variable %s at %d\n", (*variableSet)[k].name, k);
                found = 1; break; 
            }
            location++;
        }
        if (found) break;
    }

    if (!found && createIfNotFound) { 
        if (functionsDeep != 0) { variableSet = &functionStack[functionsDeep - 1]->passedVariables; count = &functionStack[functionsDeep - 1]->numVars; }
        else if (caller != NULL) { variableSet = &caller->variables; count = &caller->variableCount; }
        (*variableSet) = realloc(*variableSet, sizeof(variable) * (*count + 1));
        (*variableSet)[*count] = create_variable(name); // placholdr
        location = *count; (*count)++; found = 1;
    }

    if (found) { variable *ret = &(*variableSet)[location]; free(setsToSearch); free(counts); return ret; }
    else { free(setsToSearch); free(counts); return NULL; }
}

label *findLabel(label *labelSet, int count, char name[]) {
    int location = 0; int found = 0;
    for (int k = 0; k < count; k++) { 
        if (strcmp(labelSet[k].name, name) == 0) { 
            DEBUG_PRINTF("found label %s at %d\n", labelSet[location].name, location);
            found = 1; break; 
        } 
        location++; 
    }
    if (found) { return &(labelSet)[location]; }
    else return NULL;
}

list *findList(list *listSet, int count, char name[]) {
    int location = 0; int found = 0;
    for (int k = 0; k < count; k++) { 
        if (strcmp(listSet[k].name, name) == 0) { 
            DEBUG_PRINTF("found list %s at %d\n", listSet[location].name, location);
            found = 1; break; 
        } 
        location++; 
    }
    if (found) { return &(listSet)[location]; } 
    else { return NULL; }
} 

void preprocessLabels(openFile *new) {
    for (int i = 0; i < new->instructionCount; i++) {
        if (strcmp(new->instructions[i]->operation, "label") == 0) { 
            if (findLabel(new->labels, new->labelCount, new->instructions[i]->arguments[0]) != NULL) { printf("That name (%s) has already been taken! Execution will continue; this label will be disregarded.\n", new->instructions[i]->arguments[0]); continue; }
            new->labels = (label *)realloc(new->labels, sizeof(label) * (new->labelCount + 1)); 
            new->labels[new->labelCount] = create_label(new->instructions[i]->arguments[0], i - 1); 
            new->labelCount += 1;
        }
    }
}

char *grabUserInput(const int maxSize) {
    char *val = calloc(maxSize + 1, sizeof(char)); 
    if (!val) return NULL;
    if(!fgets(val, maxSize, stdin)) { free(val); return NULL; }
    val[strcspn(val, "\n")] = '\0'; // compensate for the newline by fucking yeeting it out of existence
    DEBUG_PRINT(val);
    return val;
}

void strip(char *string, char character) {
    int len = strlen(string);
    for (int i = 0; i < len; i++) {
        if (string[i] == character) {
            memmove(string + i, string + 1 + i, len - i);
            len -= 1; i--;
        }
    }
} 

char *stringFromString(char *string, int *offset) { // created to reduce reliance on things like sscanf, because fuck you, i make my own stdlib
    DEBUG_PRINTF("offset before: %d, ", *offset);
    while (string[*offset] == ' ' || string[*offset] == '\n' || string[*offset] == '\r' || string[*offset] == '\0') { if (string[*offset] == '\0') { return NULL; } *offset += 1; } // my cs teacher was right. carriage returns are important. i shall now scream into the abyss
    DEBUG_PRINTF("offset after: %d\n", *offset);
    int len = 0;
    while (string[*offset + len] != ' ' && string[*offset + len] != '\n' && string[*offset + len] != '\r' && string[*offset + len] != '\0') { len += 1; } // now repeat invertedly because yes
    char *final = (char *)calloc(len + 1, sizeof(char)); if (final == NULL) cry("smth died i think, hence i shall now die\n");
    memcpy(final, string + *offset, len);
    final[len] = '\0'; *offset += len;
    DEBUG_PRINT(final);
    return final;
}

instruction *parseInstructions(char *string, InstructionSet isa) {
    int offset = 0; char **args = NULL; int argc = 0;
    char *operation = stringFromString(string, &offset); loweriseInPlace(operation); DEBUG_PRINT(operation); DEBUG_PRINTF("%d", (int)strlen(operation)); char *prefix = NULL;
    while (strcmp(operation, "please") == 0 || operation == NULL) { free(operation); operation = stringFromString(string, &offset); loweriseInPlace(operation); }
    for (int i = 0; i < isa.prefixCount; i++) { if (strcmp(operation, isa.prefixes[i]) == 0) { prefix = strdup(operation); free(operation); operation = stringFromString(string, &offset); loweriseInPlace(operation); }}
    char *temp = stripSemicolon(operation); args = (char **)malloc(sizeof(char *)); DEBUG_PRINT(temp); DEBUG_PRINTF("\n\n%d\n\n", (int)strlen(temp));
    if (strcmp(temp, operation) == 0) {
        for (int i = offset; i < (int)strlen(string) + 1; i++) {
            char *argTemp = stringFromString(string, &i); if (argTemp == NULL) { break; } stripSemicolonInPlace(argTemp);
            if (strcmp(argTemp, "") == 0) { int len = strlen(args[argc - 1]); args[argc - 1] = (char *)realloc(args[argc - 1], len + 2); args[argc - 1][len] = ' '; args[argc - 1][len + 1] = '\0'; free(argTemp); break; }
            args = realloc(args, sizeof(char *) * (argc + 1));
            args[argc] = strdup(argTemp); argc++;
            DEBUG_PRINTF("arg: %s\n", argTemp);
            free(argTemp);
        }
    }
    instruction *new = add_instruction(temp, args, prefix, argc);
    if (argc >= 1) { for (int i = 0; i < argc; i++) { DEBUG_PRINTF("instruction %s has arg \"%s\"\n", temp, args[i]); free(args[i]); }}
    free(operation); free(temp); free(args); if (prefix != NULL) { free(prefix); }
    return new;
}

char *unParseInstructions(instruction *inst) {
    size_t size = 0; size += strlen(inst->operation) + 1; 
    for (int i = 0; i < inst->argumentCount; i++) { size += strlen(inst->arguments[i]) + 1; }
    if (inst->prefix != NULL) { size += strlen(inst->operation) + 1; }
    size += 2; // nullterm and semicolon ofc
    char *final = (char *)calloc(size, sizeof(char)); if (!final) return NULL;
    if (inst->prefix != NULL) { strcat(final, inst->prefix); strcat(final, " "); } strcat(final, inst->operation);
    for (int i = 0; i < inst->argumentCount; i++) {
        strcat(final, " "); // space between items ofc
        strcat(final, inst->arguments[i]);
    }
    strcat(final, ";\0");
    return final;
}

int readFileToAndIncludingChar(FILE* file, char character) { // verbose, much?
    char currentChar = 0; int count = 0;
    while (currentChar != character) { currentChar = (char)fgetc(file); if (!feof(file)) { count += 1; } else {break;} if (currentChar == '\n' || currentChar == '\r') { DEBUG_PRINT("\\n"); } else { DEBUG_PRINTF("%c", currentChar); }} // push the pointer forwards
    return count;
}

openFile *openSimasFile(const char path[]) {
    for (int i = 0; i < openFileCount; i++) {
        if (!strcmp(path, currentOpenFiles[i]->path)) {
            return NULL;
        }
    }
    FILE *file = fopen(path, "rb");
    openFile *new = calloc(1, sizeof(openFile));

    if (file == NULL) { printf("failed to find a simas file!\n"); return new; }

    new->path = strdup(path);

    while (!feof(file)) {
        DEBUG_PRINT("goin back for more\n"); 
        int size = readFileToAndIncludingChar(file, ';'); DEBUG_PRINTF("\n%d\n", size);
        if (feof(file)) break;

        fseek(file, size * -1, SEEK_CUR);
        char *buffer = (char *)calloc(size + 1, sizeof(char)); 
        fread(buffer, sizeof(char), size, file);
        buffer[size] = '\0';

        if (strchr(buffer, '@') == NULL) {
            new->instructions = (instruction **)realloc(new->instructions, sizeof(instruction *) * (new->instructionCount + 1));
            if (new->instructions == NULL) cry("welp, cant add more functions, guess its time to die now");
            new->instructions[new->instructionCount] = parseInstructions(buffer, ValidInstructions);
            new->instructionCount += 1;
        }
        
        free(buffer);
    }
    for (int i = 0; i < new->instructionCount; i++) { DEBUG_PRINTF("%d: %s\n", i, new->instructions[i]->operation); }
    fclose(file);
    return new;
}

/* actual function code / helpers                                                       */
int grabType(char *input) {
    char *type = lowerise(input);
    if (strcmp(type, "str") == 0) { free(type); return STR; }
    else if (strcmp(type, "num") == 0) { free(type); return NUM; }
    else if (strcmp(type, "bool") == 0) { free(type); return BOOL; }
    else if (strcmp(type, "in") == 0) { free(type); return IN; }
    else { free(type); return -1; }
}

char *formatList(list li) {
    char *final;
    size_t bytes = 3;
    for (int i = 0; i < li.elements; i++) {
        bytes += stringLenFromVar(li.variables[i]) + 2; 
        if (li.variables[i].type == STR) { bytes += 2; } // quotes
    }
    final = (char *)malloc(bytes); if (final == NULL) cry("List Formatting failed!");
    final[0] = '\0';

    strcat(final, "[");
    for (int i = 0; i < li.elements; i++) {
        char *temp = stringFromVar(li.variables[i]);
        if (temp) {
            if (li.variables[i].type == STR) strcat(final, "\"");
            strcat(final, temp);
            if (li.variables[i].type == STR) strcat(final, "\"");
            free(temp);
            if (i + 1 != li.elements) strcat(final, ","); // make sure no trailing comma is left
        }
    }
    strcat(final, "]");
    return final;
}

void unFormatList(list *li, char *string) {
    int type; int start = 0;
    while (1) { if (string[start] == '[') { break; } start += 1; }
    for (int i = start; i < (int)strlen(string); i++) {
        char c = string[i]; int length = 0;
        if (c == ']') break;
        if (c == '[' || c == ',') continue;
        if (c == '"') { type = STR; continue; }
        if (type != STR) { if (isdigit(c)) { type = NUM; } else { type = BOOL; }}

        while ((c = string[i + length]) != ',' && (c = string[i + length]) != '"' && (c = string[i + length]) != '[' && (c = string[i + length]) != ']') { length += 1; DEBUG_PRINT(&c); }

        char *value = (char *)calloc(length + 1, sizeof(char));
        for (int j = 0; j < length; j++) { value[j] = string[i + j]; }
        i += length; 
        value[length] = '\0';
        DEBUG_PRINT(value);

        variable var; var.type = type;
        if (type == NUM) { var.num = (double)atof(value); }
        else if (type == STR) { var.str = value; }
        else if (type == BOOL) { var.bool = trueOrFalse(value); }
        appendElement(li, var); type = 0; free(value);
    }
}

void convertLiteralNewLineToActualNewLine(char *string) {
    int sizeOf = strlen(string);
    for (int i = 1; i < sizeOf; i++) { 
        if (string[i] == 'n' && string[i - 1] == '\\') { 
            string[i - 1] = '\n';
            memcpy(string + i, string + 1 + i, sizeOf - i);
            i -= 1; 
        }
    } 
}

char *joinStringsSentence(char **strings, int stringCount, int offset) {
    char *finalString = NULL; int sizeOf = 0;
    if (stringCount == 1) { finalString = strdup(strings[0]); return finalString; }
    for (int i = offset; i < stringCount; i++) { sizeOf += strlen(strings[i]) + 1;}
    finalString = (char *)calloc(sizeOf + 1, sizeof(char)); if (finalString == NULL) cry("unable to string\nplease do not the string\n"); 
    for (int i = offset; i < stringCount; i++) { strcat(finalString, strings[i]); if (i + 1 < stringCount) { strcat(finalString, " "); }} // fuck yo optimisation
    convertLiteralNewLineToActualNewLine(finalString);
    return finalString;
}

int checkVarTruthiness(variable *var) {
    switch (var->type) {
        case NUM: if (var->num != 0.0) { return 1; } else return 0;
        case BOOL: if (var->bool != 0) { return 1; } else return 0;
        case STR: if (var->str != NULL) { return trueOrFalse(var->str); } else return 0;
        default: return 0;
    }
}

void convert(variable *var, int type) {
    DEBUG_PRINTF("\nConverted from type %d to type %d\n", var->type, type);
    if (var->type != type) {
        if (var->type == NUM) {
            if (type == BOOL) {
                if (var->num != 0.0) { var->bool = 1; }
                else { var->bool = 0; }
            } else if (type == STR) {
                var->str = grabStringOfNumber(var->num);
            }
        } else if (var->type == BOOL) {
            int truth = var->bool;
            if (type == NUM) { var->num = (double)truth; }
            else if (type == STR) {
                if (truth) var->str = strdup("true");
                if (!truth) var->str = strdup("false");
            }
        } else if (var->type == STR) {
            char *temp = strdup(var->str); if (var->str != NULL) { free(var->str); }
            if (type == BOOL) { var->bool = trueOrFalse(temp); }
            else if (type == NUM) { var->num = (double)atof(temp); }
            free(temp);
        }
        var->type = type;
    } 
    else if (var->type == type) { /* do nothing, as you cannot convert to the same type, fucknuts */ } 
    else { cry("invalid variable type.\n"); }
}

int areTwoVarsEqual(variable *var1, variable *var2) {
    if (var1->type != var2->type) { return 0; }
    if (var1->type == STR && strcmp(var1->str, var2->str) == 0) { return 1; }
    else if (var1->type == NUM && var1->num == var2->num) { return 1; }
    else if (var1->type == BOOL && var1->bool == var2->bool) { return 1; }
    return 0;
}

char *readFile(char path[]) {
    FILE *file = fopen(path, "r");
    if (file == NULL) cry("cannot open le file!");
    char *contents = NULL; 
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file); 
    contents = (char *)calloc(length + 1, sizeof(char));
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
    if (type == IN) { // let the user type whatever bullshit is on their minds
        type = STR; val = grabUserInput(100);
    } else if (type == STR) { 
        val = strdup(value); 
    }
    set_variable_value(var, type, val, num, bool);
    if (val) free(val);
}

void negateBoolean(variable *var) { if (var->type == BOOL) { var->bool = !var->bool; } else { cry("NOT must be used on a bool!"); }}
void writeFromVar(variable *var, char *path) { char *variable = stringFromVar(*var); writeFile(path, variable); free(variable); } 
void labelJump(label *jump, int *programCounter) { *programCounter = jump->location; }
void equalityCheckVarVsConst(variable **variables, int *variableCount, char **arguments, int flip) {
    variable *var1 = findVar(arguments[1], 0, NULL);
    variable var2; int output = 0; int type = grabType(arguments[0]); var2.type = type; var2.str = NULL;
    if (type == NUM) { var2.num = (double)atof(arguments[2]); }
    else if (type == STR) { var2.str = strdup(arguments[2]); }
    else if (type == BOOL) { var2.bool = trueOrFalse(arguments[2]); } 
    if (var1 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, &var2) : areTwoVarsEqual(var1, &var2);
    if (var1->type == STR && var1->str != NULL) free(var1->str);
    var1->type = BOOL;
    var1->bool = output;
    if (type == STR && var2.str != NULL) free(var2.str);
}

void equalityCheckVarVsVar(variable *var1, variable *var2, int flip) {
    int output = 0;
    if (var1 == NULL || var2 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, var2) : areTwoVarsEqual(var1, var2);
    if (var1->type == STR && var1->str != NULL) free(var1->str);
    var1->type = BOOL;
    var1->bool = output;
}

void jumpConditionally(label *jump, variable *var, int *programCounter, int flip) {
    int allowed = checkVarTruthiness(var);
    if (flip) { allowed = !allowed; }
    if (allowed) labelJump(jump, programCounter);
}

void standardMath(char **arguments, char operation, openFile *caller) {
    double op2 = 0;
    variable *var1 = findVar(arguments[1], 1, caller); 
    variable *var2 = findVar(arguments[2], 0, NULL);
    if (var1->type != NUM) cry("You can only do math on a 'num' type variable!");
    if (var2 == NULL) { op2 = atof(arguments[2]); }
    else if (var2->type != NUM) cry("You can only do math on a 'num' type variable!");
    else { op2 = var2->num; }
    switch (operation) {
        case '+': var1->num += op2; break;
        case '-': var1->num -= op2; break;
        case '*': var1->num *= op2; break;
        case '/': if (op2 == 0.0) {cry("div by zero error\n");} else{ var1->num /= op2;} break;// this is when we tell the user to eat shit and die, nerd
        default: var1->num = 0;
    }
}

void variableSet(variable **variables, int *variableCount, char **arguments, int argumentCount, openFile *caller) { // confusing names, fuck you
    int type = grabType(arguments[0]); char *concatenated = NULL;
    if (type == STR) concatenated = joinStringsSentence(arguments, argumentCount, 2); // stupid switch rules
    switch (type) {
        case IN: setVar(findVar(arguments[1], 1, caller), type, NULL, 0, 0); break;
        case STR: setVar(findVar(arguments[1], 1, caller), type, concatenated, 0.0, 0); break;
        case NUM: setVar(findVar(arguments[1], 1, caller), type, NULL, (double)atof(arguments[2]), 0); break; 
        case BOOL: setVar(findVar(arguments[1], 1, caller), type, NULL, 0.0, trueOrFalse(arguments[2])); break;
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

void compareNums(variable **variables, int *variableCount, char **arguments, char operation) {
    variable *var1 = findVar(arguments[1], 0, NULL);
    variable *var2 = findVar(arguments[2], 0, NULL);
    double operand1 = 0; double operand2 = 0;
    if (var1 == NULL) { cry("No variable!"); }
    else if (var1->type != NUM) { cry("Operand must be of \"num\" type!\n"); }
    else { operand1 = var1->num; }
    if (var2 == NULL) { operand2 = (double)atof(arguments[2]); }
    else if (var2->type != NUM) { cry("Operand must be of \"num\" type!\n"); }
    else { operand2 = var2->num; }

    var1->type = BOOL;

    switch (operation) {
        case '>': var1->bool = (operand1 > operand2); break;
        case ']': var1->bool = (operand1 >= operand2); break;
        case '<': var1->bool = (operand1 < operand2); break;
        case '[': var1->bool = (operand1 <= operand2); break;
        default: var1->bool = 0; break;
    }
}

void compareBools(variable **variables, int *variableCount, char **arguments, char operation, char flip) {
    variable *var1 = findVar(arguments[1], 0, NULL);
    variable *var2 = findVar(arguments[2], 0, NULL);
    int operand1 = 0; int operand2 = 0;
    if (var1 == NULL) { cry("No variable!"); } // can't save SHIT if you dont have a variable
    else { operand1 = checkVarTruthiness(var1); }
    if (var2 == NULL) { if (grabType(arguments[0])) {operand2 = trueOrFalse(arguments[2]);} else { operand2 = atoi(arguments[2]); }}
    else { operand2 = checkVarTruthiness(var2); }
    if (var1->type == STR && var1->str != NULL) free(var1->str);
    var1->type = BOOL;
    switch (operation) {
        case '&': var1->bool = flip ? (operand1 && operand2) : !(operand1 && operand2); break;
        case '|': var1->bool = flip ? (operand1 || operand2) : !(operand1 || operand2); break;
        case '!': var1->bool = (operand1 != operand2); break;
        default: var1->bool = 0; break;
    }
}

void loadList(list **lists, int *listCount, char *name, char *path) {
    list *li = findList(*lists, *listCount, name); char *temp = readFile(path);
    if (li == NULL) { addListToLists(lists, name, listCount); li = findList(*lists, *listCount, name); }
    else { freeList(*li); li->name = strdup(name); li->elements = 0; li->variables = (variable *)malloc(sizeof(variable)); memset(li->variables, 0, sizeof(variable)); }
    unFormatList(li, temp); free(temp);
}

void listAppendConstant(list *li, char **arguments, int *argumentCount) {
    int type = grabType(arguments[1]);
    variable var; var.type = type;
    if (type == NUM) { var.num = (double)atof(arguments[2]); }
    else if (type == BOOL) { var.bool = trueOrFalse(arguments[2]); }
    else if (type == STR) { var.str = joinStringsSentence(arguments, *argumentCount, 2); }
    appendElement(li, var); if (type == STR && var.str) free(var.str);
}

void registerFunction(openFile *current) {
    char *name = strdup(current->instructions[current->programCounter]->arguments[0]);
    int count = atoi(current->instructions[current->programCounter]->arguments[1]);
    int end = 0;

    for (int i = current->programCounter; i < current->instructionCount; i++) {
        if (!strcmp("end", current->instructions[i]->operation)) { end = i; }
    } 

    for (int i = 0; i < current->functionCount; i++) { if (!strcmp(name, ((function **)&current->functions)[i]->location.name)) { free(name); current->programCounter = end; return; }}

    function new = { .location.name = strdup(name), .location.location = current->programCounter, .paramCount = count, .endLocation = end, .passedVariables = (variable *)malloc(sizeof(variable) * count), .paramTypes = (char *)calloc(count + 1, sizeof(char)), .parent = current, .numVars = count }; 

    current->functions = realloc(((function *)current->functions), sizeof(function) * (current->functionCount + 1));
    ((function *)current->functions)[current->functionCount] = new; current->functionCount += 1;
    free(name); current->programCounter = end; // skip to point of execution
}

void callFunction(openFile *current) {
    function *executive = NULL; // got his business suit on n everythin'
    for (int i = 0; i < openFileCount; i++) {
        function **checkArray = (function **)&currentOpenFiles[i]->functions;
        for (int j = 0; j < currentOpenFiles[i]->functionCount; j++) {
            if (!strcmp(current->instructions[current->programCounter]->arguments[0], checkArray[j]->location.name)) {
                executive = checkArray[j]; break; 
            }
        }
    }
    if (executive == NULL) return; // just don't bother
    executive->passedVariables = (variable *)malloc(sizeof(variable) * executive->paramCount); executive->numVars = executive->paramCount;
    char *varName = (char *)calloc(strlen(executive->location.name) + 2, 1);
    varName[0] = '$'; strcat(varName, executive->location.name);
    variable *freed = findVar(varName, 0, NULL); 
    if (freed != NULL) { freeVariable(*freed); }
    free(varName);
    for (int i = 0; i < executive->paramCount; i++) {
        executive->paramTypes[i] = tolower(current->instructions[current->programCounter]->arguments[(i * 2) + 1][0]); // starts at 0 which is the name, every OTHER arg is a TYPE specification, so multiply by 2 and add one to seek to the next one, and since it's only one char, grab just the first one & lowerise it
        executive->caller = current; executive->callLocation = current->programCounter;
        char *number = grabStringOfNumber((double)i);
        char *fullName = (char *)calloc(strlen(number) + 2, sizeof(char));
        fullName[0] = '$'; strcat(fullName, number);
        executive->passedVariables[i].name = strdup(fullName);
        executive->passedVariables[i].type = 0;
        if (executive->paramTypes[i] == 'v') { varcpy(&executive->passedVariables[i], findVar(current->instructions[current->programCounter]->arguments[(i * 2) + 2], 0, NULL)); } // same math here
        else if (executive->paramTypes[i] == 's') { variable tempstr = { .type = STR, .str = strdup(current->instructions[current->programCounter]->arguments[(i * 2) + 2]) }; varcpy(&executive->passedVariables[i], &tempstr); freeVariable(tempstr); }
        else if (executive->paramTypes[i] == 'b') { variable tempbool = { .type = BOOL, .bool = trueOrFalse(current->instructions[current->programCounter]->arguments[(i * 2) + 2])}; varcpy(&executive->passedVariables[i], &tempbool); freeVariable(tempbool); }
        else if (executive->paramTypes[i] == 'n') { variable tempnum = { .type = NUM, .num = (double)atof(current->instructions[current->programCounter]->arguments[(i * 2) + 2])}; varcpy(&executive->passedVariables[i], &tempnum); freeVariable(tempnum); }
        free(fullName); free(number); DEBUG_PRINT(executive->paramTypes);
    }
    if (functionStack != NULL) {functionStack = (function **)realloc(functionStack, sizeof(function *) * (functionsDeep + 1));}
    else { functionStack = (function **)malloc(sizeof(function *)); }
    functionStack[functionsDeep] = executive; 
    ++functionsDeep;
    current->programCounter = executive->location.location;
}

void returnFunction(openFile *current) {
    if (!functionsDeep || functionStack == NULL) return;
    if (current->instructions[current->programCounter]->argumentCount > 1) {
        char *varName = (char *)calloc(strlen(functionStack[functionsDeep - 1]->location.name) + 2, 1);
        varName[0] = '$'; strcat(varName, functionStack[functionsDeep - 1]->location.name);
        variable *returnVar = findVar(varName, 1, functionStack[functionsDeep - 1]->parent);
        char type = current->instructions[current->programCounter]->arguments[0][0]; 
        if (type == 'v') { varcpy(returnVar, findVar(current->instructions[current->programCounter]->arguments[1], 0, NULL)); }
        if (type == 's') { variable tempstr = { .type = STR, .str = strdup(current->instructions[current->programCounter]->arguments[1]) }; varcpy(returnVar, &tempstr); freeVariable(tempstr); }
        if (type == 'b') { variable tempbool = { .type = BOOL, .bool = trueOrFalse(current->instructions[current->programCounter]->arguments[1])}; varcpy(returnVar, &tempbool); freeVariable(tempbool); }
        if (type == 'n') { variable tempnum = { .type = NUM, .num = (double)atof(current->instructions[current->programCounter]->arguments[1])}; varcpy(returnVar, &tempnum); freeVariable(tempnum); }
        free(varName);
        current->variables = realloc(current->variables, sizeof(variable) * (current->variableCount + 1));
        memset(&current->variables[current->variableCount], 0, sizeof(variable));
        current->variables[current->variableCount].name = strdup(returnVar->name);
        varcpy(&current->variables[current->variableCount], returnVar); current->variableCount += 1; 
    }
    for (int i = 0; i < functionStack[functionsDeep - 1]->paramCount; i++) { freeVariable(functionStack[functionsDeep - 1]->passedVariables[i]); functionStack[functionsDeep - 1]->paramTypes[i] = '\0';}
    current->programCounter = functionStack[functionsDeep - 1]->callLocation;
    functionsDeep -= 1; 
    if (functionsDeep) { functionStack = (function **)realloc(functionStack, sizeof(function *) * functionsDeep); }
    else { free(functionStack); functionStack = NULL; }
}

void endOfFunction(void) { if (!functionsDeep || functionStack == NULL) return; functionStack[functionsDeep - 1]->parent->programCounter = functionStack[functionsDeep - 1]->location.location; }
void importFile(openFile *current) { openFile *new = openSimasFile(current->instructions[current->programCounter]->arguments[0]); if (new != NULL) { addFilePointer(new); executeFile(new); }}

/* function wrappers using openFile                                                     */
/* rule of thumb: these should be EXACTLY one function call.                            */
/* if there's more than one function being called that isn't being passed as an arg...  */
/* ...you're doing something wrong                                                      */
/* (some exceptions allowed because c is voodoo sometimes, so like max 3 statements)    */
/* this is the ENTIRE SIMAS stdlib up here, fyi                                         */

/* console i/o      */
void con_prints(openFile *file) { printf(" "); fflush(stdout); }
void con_println(openFile *file) { printf("\n"); fflush(stdout); }
void con_printv(openFile *file) { freeAndPrint(stringFromVar(*findVar(file->instructions[file->programCounter]->arguments[0], 0, NULL))); }
void con_printc(openFile *file) { freeAndPrint(joinStringsSentence(file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount, 0)); }
/*file i/o          */
void fio_read(openFile *file) { char *read = readFile(file->instructions[file->programCounter]->arguments[0]); set_variable_value(findVar(file->instructions[file->programCounter]->arguments[1], 1, file), STR, read, 0.0, 0); free(read); }
void fio_write(openFile *file) { freeAndWrite(file->instructions[file->programCounter]->arguments[0], joinStringsSentence(file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount, 1)); }
void fio_writev(openFile *file) { writeFromVar(findVar(file->instructions[file->programCounter]->arguments[1], 0, NULL), file->instructions[file->programCounter]->arguments[0]); }
/* misc             */
void etc_not(openFile *file) { negateBoolean(findVar(file->instructions[file->programCounter]->arguments[0], 0, NULL));  }
void etc_simas(openFile *file) { printf("%s", poem); }
void etc_import(openFile *file) { importFile(file); }
/* jumps            */
void jmp_jump(openFile *file) { labelJump(findLabel(file->labels, file->labelCount, file->instructions[file->programCounter]->arguments[0]), &file->programCounter); }
void jmp_jumpv(openFile *file) { jumpConditionally(findLabel(file->labels, file->labelCount, file->instructions[file->programCounter]->arguments[0]), findVar(file->instructions[file->programCounter]->arguments[1], 0, NULL), &file->programCounter, 0); }
void jmp_jumpnv(openFile *file) { jumpConditionally(findLabel(file->labels, file->labelCount, file->instructions[file->programCounter]->arguments[0]), findVar(file->instructions[file->programCounter]->arguments[1], 0, NULL), &file->programCounter, 1); }
/* math             */
void mat_add(openFile *file) { standardMath(file->instructions[file->programCounter]->arguments, '+', file); }
void mat_sub(openFile *file) { standardMath(file->instructions[file->programCounter]->arguments, '-', file); }
void mat_mul(openFile *file) { standardMath(file->instructions[file->programCounter]->arguments, '*', file); }
void mat_div(openFile *file) { standardMath(file->instructions[file->programCounter]->arguments, '/', file); }
/* variable ops     */
void var_set(openFile *file) { variableSet(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount, file); }
void var_type(openFile *file) { grabTypeFromVar(*findVar(file->instructions[file->programCounter]->arguments[0], 0, NULL), findVar(file->instructions[file->programCounter]->arguments[1], 1, file)); }
void var_conv(openFile *file) { convert(findVar(file->instructions[file->programCounter]->arguments[0], 0, NULL), grabType(file->instructions[file->programCounter]->arguments[1])); }
void var_copy(openFile *file) { variable *var = findVar(file->instructions[file->programCounter]->arguments[1], 1, file); varcpy(var, findVar(file->instructions[file->programCounter]->arguments[0], 0, NULL)); } 
/* comparison       */
void cmp_gt(openFile *file) { compareNums(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '>'); }
void cmp_gte(openFile *file) { compareNums(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, ']'); }
void cmp_st(openFile *file) { compareNums(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '<'); }
void cmp_ste(openFile *file) { compareNums(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '['); }
void cmp_eqv(openFile *file) { equalityCheckVarVsVar(findVar(file->instructions[file->programCounter]->arguments[1], 0, NULL), findVar(file->instructions[file->programCounter]->arguments[2], 0, NULL), 0); }
void cmp_neqv(openFile *file) { equalityCheckVarVsVar(findVar(file->instructions[file->programCounter]->arguments[1], 0, NULL), findVar(file->instructions[file->programCounter]->arguments[2], 0, NULL), 1); }
void cmp_eqc(openFile *file) { equalityCheckVarVsConst(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, 0); }
void cmp_neqc(openFile *file) { equalityCheckVarVsConst(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, 1); }
void cmp_and(openFile *file) { compareBools(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '&', 0); }
void cmp_nand(openFile *file) { compareBools(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '&', 1); }
void cmp_or(openFile *file) { compareBools(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '|', 0); }
void cmp_nor(openFile *file) { compareBools(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '|', 1); }
void cmp_xor(openFile *file) { compareBools(&file->variables, &file->variableCount, file->instructions[file->programCounter]->arguments, '!', 0); }
/* list ops         */
void lis_del(openFile *file) { removeElement(findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0]), atoi(file->instructions[file->programCounter]->arguments[1]) - 1); }
void lis_appv(openFile *file) { appendElement(findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0]), *findVar(file->instructions[file->programCounter]->arguments[2], 0, NULL)); }
void lis_show(openFile *file) { freeAndPrint(formatList(*findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0]))); }
void lis_new(openFile *file) { addListToLists(&file->lists, file->instructions[file->programCounter]->arguments[0], &file->listCount); }
void lis_upv(openFile *file) { varcpy(&findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0])->variables[atoi(file->instructions[file->programCounter]->arguments[1]) - 1], findVar(file->instructions[file->programCounter]->arguments[3], 0, NULL)); }
void lis_acc(openFile *file) { varcpy(findVar(file->instructions[file->programCounter]->arguments[2], 1, file), &findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0])->variables[atoi(file->instructions[file->programCounter]->arguments[1]) - 1]); }
void lis_load(openFile *file) { loadList(&file->lists, &file->listCount, file->instructions[file->programCounter]->arguments[0], file->instructions[file->programCounter]->arguments[1]); }
void lis_len(openFile *file) { set_variable_value(findVar(file->instructions[file->programCounter]->arguments[1], 1, file), NUM, NULL, findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0])->elements, 0); }
void lis_dump(openFile *file) { freeAndWrite(file->instructions[file->programCounter]->arguments[1], formatList(*findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0]))); }
void lis_upc(openFile *file) { char **arguments = file->instructions[file->programCounter]->arguments; variable var = create_variable_with_value(NULL, grabType(arguments[2]), joinStringsSentence(arguments, file->instructions[file->programCounter]->argumentCount, 4), (double)atof(arguments[4]), trueOrFalse(arguments[3])); varcpy(&findList(file->lists, file->listCount, arguments[0])->variables[atoi(arguments[1]) - 1], &var); }
void lis_appc(openFile *file) { listAppendConstant(findList(file->lists, file->listCount, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments, &file->instructions[file->programCounter]->argumentCount); }
/* function ops     */
void fun_fun(openFile *file) { registerFunction(file); }
void fun_ret(openFile *file) { returnFunction(file); }
void fun_end(openFile *file) { endOfFunction(); }
void fun_call(openFile *file) { callFunction(file); }

void setUpStdlib(void) {
    addPrefix("list", &ValidInstructions);
    operator printv = create_operator("print", NULL, con_printv, 1); addOperator(&printv, &ValidInstructions);
    operator println = create_operator("println", NULL, con_println, 0); addOperator(&println, &ValidInstructions);
    operator prints = create_operator("prints", NULL, con_prints, 0); addOperator(&prints, &ValidInstructions);
    operator printc = create_operator("printc", NULL, con_printc, 1); addOperator(&printc, &ValidInstructions);
    operator read = create_operator("read", NULL, fio_read, 2); addOperator(&read, &ValidInstructions);
    operator write = create_operator("write", NULL, fio_write, 2); addOperator(&write, &ValidInstructions);
    operator writev = create_operator("writev", NULL, fio_writev, 2); addOperator(&writev, &ValidInstructions);
    operator not = create_operator("not", NULL, etc_not, 1); addOperator(&not, &ValidInstructions);
    operator poem = create_operator("poem", NULL, etc_simas, 0); addOperator(&poem, &ValidInstructions);
    operator import = create_operator("import", NULL, etc_import, 1); addOperator(&import, &ValidInstructions);
    operator add = create_operator("add", NULL, mat_add, 3); addOperator(&add, &ValidInstructions);
    operator sub = create_operator("sub", NULL, mat_sub, 3); addOperator(&sub, &ValidInstructions);
    operator mul = create_operator("mul", NULL, mat_mul, 3); addOperator(&mul, &ValidInstructions);
    operator div = create_operator("div", NULL, mat_div, 3); addOperator(&div, &ValidInstructions);
    operator set = create_operator("set", NULL, var_set, 2); addOperator(&set, &ValidInstructions);
    operator type = create_operator("type", NULL, var_type, 2); addOperator(&type, &ValidInstructions);
    operator conv = create_operator("conv", NULL, var_conv, 2); addOperator(&conv, &ValidInstructions);
    operator copy = create_operator("copy", NULL, var_copy, 2); addOperator(&copy, &ValidInstructions);
    operator gt = create_operator("gt", NULL, cmp_gt, 3); addOperator(&gt, &ValidInstructions);
    operator gte = create_operator("gte", NULL, cmp_gte, 3); addOperator(&gte, &ValidInstructions);
    operator st = create_operator("st", NULL, cmp_st, 3); addOperator(&st, &ValidInstructions);
    operator ste = create_operator("ste", NULL, cmp_ste, 3); addOperator(&ste, &ValidInstructions);
    operator eqv = create_operator("eqv", NULL, cmp_eqv, 3); addOperator(&eqv, &ValidInstructions);
    operator neqv = create_operator("neqv", NULL, cmp_neqv, 3); addOperator(&neqv, &ValidInstructions);
    operator eqc = create_operator("eqc", NULL, cmp_eqc, 3); addOperator(&eqc, &ValidInstructions);
    operator neqc = create_operator("neqc", NULL, cmp_neqc, 3); addOperator(&neqc, &ValidInstructions);
    operator and = create_operator("and", NULL, cmp_and, 3); addOperator(&and, &ValidInstructions);
    operator nand = create_operator("nand", NULL, cmp_nand, 3); addOperator(&nand, &ValidInstructions);
    operator or = create_operator("or", NULL, cmp_or, 3); addOperator(&or, &ValidInstructions);
    operator nor = create_operator("nor", NULL, cmp_nor, 3); addOperator(&nor, &ValidInstructions);
    operator xor = create_operator("xor", NULL, cmp_xor, 3); addOperator(&xor, &ValidInstructions);
    operator jump = create_operator("jump", NULL, jmp_jump, 1); addOperator(&jump, &ValidInstructions);
    operator jumpv = create_operator("jumpv", NULL, jmp_jumpv, 2); addOperator(&jumpv, &ValidInstructions);
    operator jumpnv = create_operator("jumpnv", NULL, jmp_jumpnv, 2); addOperator(&jumpnv, &ValidInstructions);
    operator del = create_operator("del", "list", lis_del, 1); addOperator(&del, &ValidInstructions);
    operator appv = create_operator("appv", "list", lis_appv, 2); addOperator(&appv, &ValidInstructions);
    operator show = create_operator("show", "list", lis_show, 0); addOperator(&show, &ValidInstructions);
    operator new = create_operator("new", "list", lis_new, 0); addOperator(&new, &ValidInstructions);
    operator upv = create_operator("upv", "list", lis_upv, 3); addOperator(&upv, &ValidInstructions);
    operator acc = create_operator("acc", "list", lis_acc, 2); addOperator(&acc, &ValidInstructions);
    operator load = create_operator("load", "list", lis_load, 2); addOperator(&load, &ValidInstructions);
    operator len = create_operator("len", "list", lis_len, 1); addOperator(&len, &ValidInstructions);
    operator dump = create_operator("dump", "list", lis_dump, 1); addOperator(&dump, &ValidInstructions);
    operator upc = create_operator("upc", "list", lis_upc, 3); addOperator(&upc, &ValidInstructions);
    operator appc = create_operator("appc", "list", lis_appc, 2); addOperator(&appc, &ValidInstructions);
    operator fun = create_operator("fun", "list", fun_fun, 1); addOperator(&fun, &ValidInstructions);
    operator ret = create_operator("ret", "list", fun_ret, 0); addOperator(&ret, &ValidInstructions);
    operator end = create_operator("end", "list", fun_end, 0); addOperator(&end, &ValidInstructions);
    operator call = create_operator("call", "list", fun_call, 0); addOperator(&call, &ValidInstructions);
}

operator *locateOperator(char *name, char *prefix) {
    for (int i = 0; i < ValidInstructions.count; i++) { 
        if (strcmp(ValidInstructions.set[i].name, name) == 0) {
            if (ValidInstructions.set[i].prefix != NULL && prefix != NULL) {
                if (strcmp(ValidInstructions.set[i].prefix, prefix) != 0) continue;
            }
            return &ValidInstructions.set[i];
        }
    }
    return NULL;
}

int executeInstruction(openFile *cur) { // all of these are defined up here so this function can operate independently of any files
    DEBUG_PRINTF("\nExecuting instruction %s on line %d.\n", cur->instructions[cur->programCounter]->operation, cur->programCounter);
    if (strcmp(cur->instructions[cur->programCounter]->operation, "quit") == 0) { return 0; } 
    if (strlen(cur->instructions[cur->programCounter]->operation) == 0) return 1;
    operator *current = locateOperator(cur->instructions[cur->programCounter]->operation, cur->instructions[cur->programCounter]->prefix);
    if (current != NULL) ((void(*)(openFile*))current->functionPointer)(cur);
    return 1;
}

void executeFile(openFile *current) {
    preprocessLabels(current);
    for (current->programCounter = current->programCounter; current->programCounter < current->instructionCount; current->programCounter++) { if (executeInstruction(current) == 0) break; }
}

int main(int argc, const char * argv[]) {
    currentOpenFiles = (openFile **)calloc(1, sizeof(openFile *));
    setUpStdlib();
    if (argc >= 2) { 
        if (argc >= 3) { if (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "--debug") == 0) { debugMode = 1; printf("debug mode enabled\n"); }}
        openFile *new = openSimasFile(argv[1]); addFilePointer(new); executeFile(new);
    }

    freeInstructionSet(ValidInstructions);

    for (int i = 0; i < openFileCount; i++) { freeFile(*currentOpenFiles[i]); free(currentOpenFiles[i]); }
    free(currentOpenFiles);
    return 0;
} // we are the shinglefuckers of bong juice ltd.