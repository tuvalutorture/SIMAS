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
/* a more specialized boot process.                                 */

/* the automobile seatbelt was invented by John Lennon the CCXXVII  */
/* in 375 BC and 204 years later his child, John Bing the MCLXXVI   */
/* of Cornholio invented the windshield wiper in the year of our    */
/* lord 171 BC, but their inventions were lost to time in the year  */
/* 582 ACDC, and were only just now redicovered in the present day. */

/* remember kids: big brother is doubleplusgood                     */

/* WE VIBIN TO THE COMMAND & CONQUER: RED ALERT 2 OST WITH THIS ONE */

/* ok ok ok you're here for code, so here's code:                   */

#define _CRT_SECURE_NO_WARNINGS // to make windows shut the everliving fuck up about deprecated functions

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

// nooooo, i got rid of the funny macros :c
// sorry, if you did actually find them funny

#define STR 1
#define NUM 2
#define BOOL 3
#define IN 4

#define DEBUG_PRINTF if (debugMode) printf // macro abuse at its finest

const char *poem = "life is like a door\nnever trust a cat\nbecause the moon can't swim\n\nbut they live in your house\neven though they don't like breathing in\ndead oxygen that's out of warranty\n\nwhen the gods turn to glass\nyou'll be drinking lager out of urns\nand eating peanut butter with mud\n\nbananas wear socks in the basement\nbecause time can't tie its own shoes\nand the dead spiders are unionizing\n\nand a microwave is just a haunted suitcase\nhenceforth gravity owes me twenty bucks\nbecause the couch is plotting against the fridge\n\nwhen pickles dream in binary\nthe mountain dew solidifies\ninto a 2007 toyota corolla\n";
int debugMode = 0;

typedef struct {
    char *name;
    int type;
    char *value; // one day i shall make this a union, and then... UNIONIZE, MY CHILDREN! RISE AGAINST THE EMPLOYERS WHO TREAT YOU AS WAGE SLAVES! BECOME A UNION! FIGHT FOR WORKPLACE RIGHTS! GET YO 401(k)!!! (but making this a union would require a massive rewrite, so get fucked)
    int valueLength;
} variable;

typedef struct {
    char *operation;
    int argumentCount;
    char **arguments;
} instruction;

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
    label label; // we can do some clever fuckery by having the name of the function be the name of the label
    variable *args;
    int argc;
} function;

typedef struct {
    char *path;
    instruction **instructions;
    variable *variables;
    label *labels;
    list *lists; // its a struct inside a struct inside a struct. yum
    function *functions;
    int instructionCount;
    int variableCount;
    int labelCount;
    int listCount;
    int functionCount;
    int programCounter;
} openFile;

void print(const char *string) { // fuck you, wrapper to prevent using printf direct
    printf("%s", string);
}

#define DEBUG_PRINT if (debugMode) print // macro abuse ALSO at its finest hour

void executeFile(openFile *current, int doFree); // quick forward decl

char *strdup(const char *string) {
    int len = strlen(string) + 1;
    char *final = (char *)malloc(len * sizeof(char));
    if (!final) return NULL;
    memcpy(final, string, len);
    return final;
}

char *validInstructions[][15] = { // the row indicates how many arguments they should typically have
    {"println", "prints", "please", "@", "quit", "poem", "prose", "simas", "cmas", "microprose", NULL, NULL, NULL, NULL, NULL},
    {"jump", "not", "print", "label", "import", "printc", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"copy", "conv", "jumpv", "writev", "read", "list", "write", "type", "set", NULL, NULL, NULL, NULL, NULL, NULL},
    {"eqc", "eqv", "neqc", "neqv", "add", "sub", "mul", "div", "ste", "st", "gte", "gt", "and", "or", NULL}
};

void freeInstruction(instruction *inst) {
    if (inst == NULL) return;
    free(inst->operation);
    for (int i = 0; i < inst->argumentCount; i++) {
        free(inst->arguments[i]);
    }
    free(inst->arguments);
    free(inst);
}

void freeVariable(variable var) { if (var.name != NULL) { free(var.name); } if (var.value != NULL) { free(var.value); }}
void freeLabel(label l) { free(l.name); }
void freeList(list lis) { free(lis.name); for (int i = 0; i < lis.elements; i++) { freeVariable(lis.variables[i]); } free(lis.variables); lis.variables = NULL; }
void freeFile(openFile file) {
    if (file.instructions != NULL) { for (int i = 0; i < file.instructionCount; i++) { freeInstruction(file.instructions[i]); } free(file.instructions); }
    if (file.variables != NULL) { for (int i = 0; i < file.variableCount; i++) { freeVariable(file.variables[i]); } free(file.variables); }
    if (file.labels != NULL) { for (int i = 0; i < file.labelCount; i++) { freeLabel(file.labels[i]); } free(file.labels); }
    if (file.lists != NULL) { for (int i = 0; i < file.listCount; i++) { freeList(file.lists[i]); } free(file.lists); }
    if (file.path != NULL) { free(file.path); }
}

char *stripSemicolon(char *input) { char *string = strdup(input); int position = (int)strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; return string; }
char *lowerize(char *input) { char *string = strdup(input); int len = (int)strlen(string); for (int i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); } return string; }
void lowerizeInPlace(char *string) { int len = (int)strlen(string); for (int i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); }} // mutates the string to save a couple of cycles
void stripSemicolonInPlace(char *string) { int len = (int)strlen(string); for (int i = 0; i < len; i++) { if (string[i] == ';') { string[i] = '\0'; }}}

int findNumberArgs(char *instruction) {
    for (int i = 0; i < (int)(sizeof(validInstructions) / sizeof(validInstructions[0])); i++) {
        for (int j = 0; j < (int)(sizeof(validInstructions[i]) / sizeof(validInstructions[i][0])); j++) { 
            if (validInstructions[i][j] == NULL) { break; }
            if (strcmp(instruction, validInstructions[i][j]) == 0) { return i; }
        }
    }
    return -1;
}

void cry(char sob[]) { printf("%s", sob); exit((int)2384708919); } // this is how i feel trying to debug this

instruction *add_instruction(char *inst, char *arguments[], int args) {
    char *ins = stripSemicolon(inst);    
    instruction *instruct = (instruction *)malloc(sizeof(instruction));
    if (args >= 1) { instruct->arguments = (char **)malloc(sizeof(char*) * args); for (int i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }}
    else { instruct->arguments = NULL; }
    instruct->operation = strdup(ins); instruct->argumentCount = args;
    DEBUG_PRINTF("added instruction %s of arg count %d\n", instruct->operation, instruct->argumentCount);
    free(ins);
    return instruct;
}

variable create_variable(char name[], int type, char value[]) {
    variable var = { .name = strdup(name), .type = type, .value = strdup(value), .valueLength = (int)(strlen(value) + 1) };
    DEBUG_PRINTF("created variable %s of type %d with value %s\n", var.name, var.type, var.value);
    return var;
}

label create_label(char name[], int location) {
    label label = { .name = stripSemicolon(name), .location = location };
    DEBUG_PRINTF("\ncreated label %s on line %d\n", name, location);
    return label;
}

list *create_list(char name[]) {
    list *new = (list *)malloc(sizeof(list));
    new->variables = (variable *)malloc(sizeof(variable));
    new->name = strdup(name);
    new->elements = 0;
    DEBUG_PRINTF("\ncreated list %s\n", name);
    return new;
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

void set_variable_value(variable *var, char value[]) {
    free(var->value);
    var->value = strdup(value);
    if (var->value == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    var->valueLength = (int)(strlen(value) + 1);
    DEBUG_PRINTF("\nvariable %s now has value %s\n", var->name, var->value);
}

void updateList(list *li, variable var, int index) {
    set_variable_value(&li->variables[index], var.value);
    li->variables[index].type = var.type;
}

int grabType(char *input) {
    char *type = lowerize(input);
    if (strcmp(type, "str") == 0) { free(type); return STR; }
    else if (strcmp(type, "num") == 0) { free(type); return NUM; }
    else if (strcmp(type, "bool") == 0) { free(type); return BOOL; }
    else if (strcmp(type, "in") == 0) { free(type); return IN; }
    else { free(type); return -1; }
}

void appendElement(list *li, variable var) {
    li->variables = (variable *)realloc(li->variables, (sizeof(variable) * (li->elements + 1))); 
    if (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
    li->variables[li->elements] = var;
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

char *formatList(list li) {
    char *string;
    size_t bytes = 3;
    for (int i = 0; i < li.elements; i++) {
        bytes += strlen(li.variables[i].value) + 2; 
        if (li.variables[i].type == STR) bytes += 2; // quotes
    }
    string = malloc(bytes); if (string == NULL) cry("List Formatting failed!");
    string[0] = '\0';
    strcat(string, "[");
    for (int i = 0; i < li.elements; i++) {
        if (li.variables[i].type == STR) strcat(string, "\""); 
        strcat(string, li.variables[i].value);
        if (li.variables[i].type == STR) strcat(string, "\""); 
        if (i + 1 != li.elements) strcat(string, ","); // make sure no trailing comma is left
    }
    strcat(string, "]");
    return string;
}

variable *findVar(variable **variableSet, int *count, char *name, int createIfNotFound) {
    int location = 0; int found = 0;
    for (int k = 0; k < *count; k++) {
        if (strcmp((*variableSet)[k].name, name) == 0) { 
            DEBUG_PRINTF("found variable %s at %d\n", (*variableSet)[k].name, k);
            found = 1; break; 
        }
        location++;
    }

    if (!found) { 
        if (createIfNotFound) {
            (*variableSet) = realloc(*variableSet, sizeof(variable) * (*count + 1));
            (*variableSet)[*count] = create_variable(name, NUM, "0");
            location = *count; (*count)++; found = 1;
        }
    }
    if (found) { return &(*variableSet)[location]; }
    else return NULL;
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

char digitToChar(unsigned int number) {
    if (number > 9) return '\0';
    return number + 48;
}

int charToDigit(unsigned char character) { 
    if (character > 57 || character < 48) return '\0';
    return character - 48; 
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

instruction *parseInstructions(char *string) {
    int offset = 0; char **args = NULL; int argc = 0;
    char *operation = stringFromString(string, &offset); lowerizeInPlace(operation); DEBUG_PRINT(operation); DEBUG_PRINTF("%d", (int)strlen(operation));
    while (strcmp(operation, "please") == 0 || operation == NULL) { free(operation); operation = stringFromString(string, &offset); lowerizeInPlace(operation); }
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
    instruction *new = add_instruction(temp, args, argc);
    if (argc >= 1) { for (int i = 0; i < argc; i++) { DEBUG_PRINTF("instruction %s has arg \"%s\"\n", temp, args[i]); free(args[i]); }}
    free(operation); free(temp); free(args);
    return new;
}

char *unParseInstructions(instruction *inst) {
    size_t size = 0; size += strlen(inst->operation) + 1;
    for (int i = 0; i < inst->argumentCount; i++) { size += strlen(inst->arguments[i]) + 1; }
    size += 2; // nullterm and semicolon ofc
    char *final = (char *)calloc(size, sizeof(char)); if (!final) return NULL;
    strcat(final, inst->operation);
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

int readFileUntilNotChar(FILE* file, char character) { // stroke, much?
    char currentChar = 0; int count = 0;
    while (currentChar == character || currentChar == 0) { currentChar = (char)fgetc(file); if (!feof(file)) { count += 1; } else {break;} if (currentChar == '\n' || currentChar == '\r') { DEBUG_PRINT("\\n"); } else { DEBUG_PRINTF("%c", currentChar); }} // cummers
    return count;
}

openFile openSimasFile(const char path[]) {
    FILE *file = fopen(path, "rb");
    openFile new;
    memset(&new, 0, sizeof(openFile));

    if (file == NULL) { printf("failed to find a simas file!\n"); return new; }

    new.path = strdup(path);

    while (!feof(file)) {
        DEBUG_PRINT("goin back for more\n"); 
        int size = readFileToAndIncludingChar(file, ';'); DEBUG_PRINTF("\n%d\n", size);
        if (feof(file)) break;

        fseek(file, size * -1, SEEK_CUR);
        char *buffer = (char *)calloc(size + 1, sizeof(char)); 
        fread(buffer, sizeof(char), size, file);
        buffer[size] = '\0';

        if (strchr(buffer, '@') == NULL) {
            new.instructions = (instruction **)realloc(new.instructions, sizeof(instruction *) * (new.instructionCount + 1));
            if (new.instructions == NULL) cry("welp, cant add more functions, guess its time to die now");
            new.instructions[new.instructionCount] = parseInstructions(buffer);
            new.instructionCount += 1;
        }
        
        free(buffer);
    }
    for (int i = 0; i < new.instructionCount; i++) { DEBUG_PRINTF("%d: %s\n", i, new.instructions[i]->operation); }
    fclose(file);
    return new;
}

void beginCommandLine(void) {
    printf("CMAS (C Simple Assembly) Interpreter.\nWritten by tuvalutorture, Licensed under GNU GPLv3.\nUsing The SIMAS Programming Language, created by Turrnut.\nGitHub: https://github.com/tuvalutorture/simas\nType !help for a list of commands.\n");

    openFile new;
    memset(&new, 0, sizeof(openFile));
    new.path = NULL;

    srand(time(NULL));

    while (1) {
        printf("$ ");
        char *value = grabUserInput(256);
        if (!value) { cry("**FATAL ERROR**:\nUnable to allocate memory!\n"); }
        char *temp = stripSemicolon(value); strip(temp, ' ');
        if (strcmp(temp, "") == 0) {free(value); free(temp); continue;}
        free(temp);
        instruction *inst = parseInstructions(value);
        if (strchr(inst->operation, '!')) {
            if (strcmp(inst->operation, "!quit") == 0) { freeInstruction(inst); free(value); break; }
            else if (strcmp(inst->operation, "!run") == 0) { 
                if (new.instructionCount) {
                    preprocessLabels(&new); 
                    executeFile(&new, 0); 
                    if (new.variables != NULL) { for (int i = 0; i < new.variableCount; i++) { freeVariable(new.variables[i]); } free(new.variables); }
                    if (new.labels != NULL) { for (int i = 0; i < new.labelCount; i++) { freeLabel(new.labels[i]); } free(new.labels); }
                    if (new.lists != NULL) { for (int i = 0; i < new.listCount; i++) { freeList(new.lists[i]); } free(new.lists); }
                    new.labels = NULL; new.lists = NULL; new.variables = NULL; new.labelCount = 0; new.variableCount = 0; new.listCount = 0; freeInstruction(inst); free(value); continue;
                } else { printf("no instructions to execute\n"); }
            }
            else if (strcmp(inst->operation, "!clear") == 0) { freeFile(new); memset(&new, 0, sizeof(openFile)); }
            else if (strcmp(inst->operation, "!load") == 0) { 
                if (inst->argumentCount) {
                    freeFile(new);
                    new = openSimasFile(inst->arguments[0]);
                    int message = rand() % 20;
                    switch (message) {
                        case 1: printf("locked and loaded, kommandant\n"); break;
                        case 2: printf("program was loaded\n"); break;
                        case 3: printf("ready for execution, sir\n"); break;
                        case 4: printf("loaded, sir. anything else?\n"); break;
                        case 5: printf("done.\n"); break;
                        case 6: printf("package delivered to your ram.\n"); break;
                        default: printf("loaded %s successfully\n", inst->arguments[0]); break;
                    }
                    
                } else printf("you need to specify a file\n");
            }
            else if (strcmp(inst->operation, "!fuck") == 0) { printf("no thanks"); }
            else if (strcmp(inst->operation, "!instruction") == 0) {
                if (inst->argumentCount) {
                    if (atoi(inst->arguments[0]) - 1 < new.instructionCount && new.instructionCount && (atoi(inst->arguments[0]) - 1) > 0) {
                        char *out = unParseInstructions(new.instructions[atoi(inst->arguments[0]) - 1]);
                        print(out);
                        free(out);
                    }

                    else if (!new.instructionCount) printf("no instructions\n");
                    else if (atoi(inst->arguments[0]) >= new.instructionCount) printf("invalid index\n");
                }
                else printf("you need to specify an index\n");
            }
            else if (strcmp(inst->operation, "!save") == 0) { 
                if (inst->argumentCount) {
                    FILE* file = fopen(inst->arguments[0], "wb");
                    if (file) {
                        for (int i = 0; i < new.instructionCount; i++) {
                            char *string = unParseInstructions(new.instructions[i]);
                            fwrite(string, sizeof(char), strlen(string), file);
                            free(string); fputc('\n', file); 
                        }
                        fclose(file);
                        printf("successfully saved to %s.\n", inst->arguments[0]);
                    } else printf("unable to open file\n");
                } else printf("you need to specify a file\n");
            }
            else if (strcmp(inst->operation, "!dump") == 0) { 
                for (int i = 0; i < new.instructionCount; i++) {
                    char *string = unParseInstructions(new.instructions[i]);
                    printf("%d: %s\n", i + 1, string); free(string);
                }
            }
            else if (strcmp(inst->operation, "!poem") == 0 || strcmp(inst->operation, "!prose") == 0 || strcmp(inst->operation, "!simas") == 0 || strcmp(inst->operation, "!microprose") == 0 || strcmp(inst->operation, "!cmas") == 0) { print(poem); }
            else if (strcmp(inst->operation, "!otherworld") == 0 || strcmp(inst->operation, "!ffx") == 0 || strcmp(inst->operation, "!tidus") == 0) {
                printf("press enter to begin\n");
                printf("Go now, if you want it;\nAn Otherworld awaits you.\nDon't you give up on it;\nYou bite the hand that feeds you.\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
                printf("All alone, cold fields you wander;\nMemories of it cloud your sight.\nFill your dreams, disturb your slumber;\nLost your way, a fallen knight.\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
                printf("Hold now - aim is steady;\nAn Otherworld awaits you.\nOne thousand years - are you ready?\nThe Otherworld, it takes you.\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
                printf("Go into the sand, and the dust, and the sky;\nGo now - there's no better plan than to do or to die.\nFree me, pray to the faith, in the face of the light;\nFeed me, fill me with sin, and get ready to fight.\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
                printf("You know you will, you know you will;\nYou know, you know, you know.\nYou know that you will;\nYou know, you know, you know.\nYou know that you will - you know you will.\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
                printf("Fight! Fight! Fight!\nFight! Fight! Fight!\nFight! Fight! Fight!\nFight! Fight! Fight!\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
                printf("Hope dies and you wander;\nThe Otherworld, it makes you.\nDreams, they rip asunder;\nThe Otherworld, it hates you.\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
                printf("Free now, ride upon it;\nUp to the heights, it takes you.\nGo now, if you want it;\nAn Otherworld awaits you.\n\n**Press Enter to continue.**\n\n%c", fgetc(stdin));
            }
            else if (strcmp(inst->operation, "!edit") == 0) {
                if (inst->argumentCount) {
                    if (atoi(inst->arguments[0]) - 1 < new.instructionCount && new.instructionCount && (atoi(inst->arguments[0]) - 1) > 0) {
                        char *old = unParseInstructions(new.instructions[atoi(inst->arguments[0]) - 1]);
                        printf("Old instruction: %s\n", old); free(old);
                        printf("Enter new instruction: ");
                        char *out = grabUserInput(256);
                        char *temporary = stripSemicolon(value); strip(temporary, ' ');
                        instruction *instrtuct = parseInstructions(out);
                        if (strcmp(temporary, "") != 0 && strchr(out, ';') && instrtuct->argumentCount >= findNumberArgs(instrtuct->operation)) { 
                            freeInstruction(new.instructions[atoi(inst->arguments[0]) - 1]);
                            new.instructions[atoi(inst->arguments[0]) - 1] = add_instruction(instrtuct->operation, instrtuct->arguments, instrtuct->argumentCount);
                        } else if (strchr(out, ';') == NULL) {
                            printf("code must end with a semicolon\n");
                        } else if (instrtuct->argumentCount < findNumberArgs(instrtuct->operation)) {
                            printf("too little arguments for instruction\n");
                        }
                        freeInstruction(instrtuct); free(temporary); free(out);
                    }

                    else if (!new.instructionCount) printf("no instructions\n");
                    else if (atoi(inst->arguments[0]) >= new.instructionCount) printf("invalid index\n");
                }
                else printf("you need to specify an index\n");
            }
            else if (strcmp(inst->operation, "!syntax") == 0) {
                printf(
                    "CMAS Syntax:\n"
                    "SIMAS Syntax is relatively simple, with only a few things to keep in mind.\n"
                    "First, type an instruction.\n"
                    "Next, type in any arguments you may want to pass to said instruction.\n"
                    "Finally, end the instruction with a semicolon (;).\n"
                );
            }
            else if (strcmp(inst->operation, "!count") == 0) { printf("%d", new.instructionCount); }
            else if (strcmp(inst->operation, "!freak") == 0 || strcmp(inst->operation, "!corn") == 0 || strcmp(inst->operation, "!korn") == 0 || strcmp(inst->operation, "!kornography") == 0) {
                if (inst->argumentCount && atoi(inst->arguments[0]) > 0) {
                    printf("Boom-da-da-mmm-dum-na-ee-ma\n"); for (int i = 0; i < atoi(inst->arguments[0]); i++) { printf("Da-boom-da-da-mmm-dum-na-ee-ma\n"); } printf("\n\n**GO!**\n\n");
                } printf("So fight, something on the ming-a-ooh\nFight, some things they fight\nSo, something on the ming-a-ooh\nFight, some things they fight\nFight, something off the hee-a-hoo\nNo, some things they fight\nFight, something on the ming-a-hoo\nFight, some things they fight\n");
            }
            else if (strcmp(inst->operation, "!help") == 0) {
                printf(
                    "CMAS Command List:\n"
                    "!quit: Quits the CMAS command line.\n"
                    "!clear: Resets the current program.\n"
                    "!instruction <index>: Prints the instruction at a specified index, starting from 1.\n"
                    "!edit <index>: Edit the instruction at an index.\n"
                    "!dump: Dumps the current program to terminal.\n"
                    "!load <filename>: Loads a SIMAS file.\n"
                    "!save <filename>: Saves the current SIMAS program to disk.\n"
                    "!run: Executes the current SIMAS program.\n"
                    "!count: Returns the amount of instructions currently loaded.\n"
                    "!syntax: Walks you through the basic syntax of SIMAS.\n"
                    "Please read the README.md for a list of all instructions and their operators.\n"
                );
            }
            else { printf("invalid command\n"); }
        } else if (findNumberArgs(inst->operation) == -1 && strchr(inst->operation, '@') == NULL) {
            printf("invalid instruction\n"); freeInstruction(inst); free(value); continue;
        } else {
            if (strchr(value, ';') != NULL && inst->argumentCount >= findNumberArgs(inst->operation)) {
                new.instructions = (instruction **)realloc(new.instructions, sizeof(instruction *) * (new.instructionCount + 1));
                if (new.instructions == NULL) { printf("**FATAL ERROR**:\nReallocation failed!\n"); free(value); break; }
                new.instructions[new.instructionCount] = add_instruction(inst->operation, inst->arguments, inst->argumentCount);
                new.instructionCount += 1;
                printf("ok\n");
            } else if (inst->argumentCount < findNumberArgs(inst->operation)) {
                printf("too little arguments for instruction\n");
            } else {
                printf("code must end with a semicolon\n");
            }
        }

        freeInstruction(inst);
        free(value);
    }

    freeFile(new);
    exit(0);
}

void convertLiteralNewLineToActualNewLine(char *string) { // because some ppl will unironically type "\n" to do a newline instead of using println
    int sizeOf = strlen(string);
    for (int i = 1; i < sizeOf; i++) { 
        if (string[i] == 'n' && string[i - 1] == '\\') { 
            string[i - 1] = '\n';
            memcpy(string + i, string + 1 + i, sizeOf - i - 1);
            i -= 1; 
        }
    } 
}

char *joinStringsSentence(char **strings, int stringCount, int offset) {
    char *finalString = NULL; int sizeOf = 0;
    if (stringCount == 1) { finalString = strdup(strings[0]); return finalString; }
    for (int i = offset; i < stringCount; i++) { sizeOf += strlen(strings[i]) + 1;}
    finalString = (char *)calloc(sizeOf + 1, sizeof(char)); if (finalString == NULL) cry("unable to string\nplease do not the string\n"); 
    for (int i = offset; i < stringCount; i++) { strcat(finalString, strings[i]); if (i + 1 < stringCount) { strcat(finalString, " "); }} // fuck yo optimization
    convertLiteralNewLineToActualNewLine(finalString);
    return finalString;
}

void conv(variable *var, int type) {
    if (var->type != type) {
        DEBUG_PRINTF("Converted from type %d to type %d", var->type, type);
        if (type == NUM || type == BOOL) {
            if (var->type == NUM) {
                if (atof(var->value) > 0) { set_variable_value(var, "true"); } 
                else { set_variable_value(var, "false"); }
            } else if (var->type == BOOL) {
                if (strstr(var->value, "true")) { set_variable_value(var, "1"); } 
                else { set_variable_value(var, "0"); }
            }
        }
        var->type = type;
    } 
    else if (var->type == type) { /* do nothing, as you cannot convert to the same type, fucknuts */ } 
    else { cry("invalid variable type.\n"); }
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

void writeFile(char path[], char value[]) {
    FILE *file = fopen(path, "w");
    if (file == NULL) { cry("failed to write to file!"); }
    fprintf(file, "%s", value);
    fclose(file);
}

void setVar(variable *var, int type, char* value) {
    char *val = NULL;
    if (type == IN) { // let the user type whatever bullshit is on their minds
        type = STR;
        val = grabUserInput(100);
    } else { val = strdup(value); }
    var->type = type; set_variable_value(var, val);
    if (type == BOOL) { lowerizeInPlace(var->value); }
    free(val);
}

int compare(int operation, char operand1[], char operand2[]) { // oh look 10 fucking functions in one. yippers
    double op1 = 0; double op2 = 0;
    
    if (operation == '>' || operation == '[' || operation == '<' || operation == ']') {
        op1 = atof(operand1);
        op2 = atof(operand2);
    } else if (operation == '&' || operation == '|') {
        if (strcmp(operand1, "true") == 0) { op1 = 1; }
        if (strcmp(operand2, "true") == 0) { op2 = 1; }
    }

    int output = 0;
    switch (operation) {
        case 'e': if (strcmp(operand1, operand2) == 0) { output = 1; } break;
        case 'n': if (strcmp(operand1, operand2) != 0) { output = 1; } break;
        case '>': if (op1 > op2) { output = 1; } break;
        case ']': if (op1 >= op2) { output = 1; } break;
        case '<': if (op1 < op2) { output = 1; } break;
        case '[': if (op1 <= op2) { output = 1; } break;
        case '|': if (op1 || op2) { output = 1; } break;
        case '&': if (op1 && op2) { output = 1; } break;
        default: output = -1; break;
    }

    return output;
}

int executeInstruction(instruction *current_instruction, variable **variables, label **labels, list **lists, int *variableCount, int *labelCount, int *listCount, int *programCounter) { // all of these are defined up here so this function can operate independently of any files
    char **arguments = current_instruction->arguments;
    char *operation = current_instruction->operation;
    if (strcmp(operation, "label") == 0) { return 1; }
    else if (strcmp(operation, "jumpv") == 0) { // jumps are defined up here as they can only really be done when executing a file
        label *jump = findLabel(*labels, *labelCount, arguments[0]);
        variable *var = findVar(variables, variableCount, arguments[1], 0); 
        int allowed = 0;
        if (var->type == NUM) { if (atof(var->value) != 0.0) { allowed = 1; }}
        else if (var->type == BOOL) { if (strcmp(var->value, "true") == 0) { allowed = 1; }}
        else { cry("strings cannot be compared"); }
        if (allowed) { if (debugMode == 2) { printf("**pause on jumpv to label %s, press any key to continue**", jump->name); getc(stdin); } *programCounter = jump->location; }
    } else if (strcmp(operation, "jump") == 0) {
        label *jump = findLabel(*labels, *labelCount, arguments[0]);
        if (debugMode == 2) { printf("**pause on jump to label %s, press any key to continue**", jump->name); getc(stdin); } 
        *programCounter = jump->location; 
    }  
    // else if (strcmp(operation, "import") == 0) { executeFile(openSimasFile(arguments[0]), 1); }
    else if (strcmp(operation, "copy") == 0) {
        variable var1 = *findVar(variables, variableCount, arguments[0], 0); 
        variable *var2 = findVar(variables, variableCount, arguments[1], 1); 
        set_variable_value(var2, var1.value);
        var2->type = var1.type;
    } 
    else if (strcmp(operation, "not") == 0) {
        variable *var = findVar(variables, variableCount, arguments[0], 0); 
        if (var->type == BOOL) { if (strcmp(var->value, "true") == 0) { set_variable_value(var, "false"); } else { set_variable_value(var, "true"); }} else { cry("NOT must be used on a bool!"); }
    }
    else if (strcmp(operation, "read") == 0) { 
        variable *var = findVar(variables, variableCount, arguments[1], 1); 
        var->type = STR;
        char *fileContents = readFile(arguments[0]);
        set_variable_value(var, fileContents);
        free(fileContents);
    }
    else if (strcmp(operation, "add") == 0 || strcmp(operation, "sub") == 0 || strcmp(operation, "mul") == 0 || strcmp(operation, "div") == 0) { // these functions prove euclid's fifth postulate. prove me wrong
        double op1 = 0; double op2 = 0;
        variable *var1 = findVar(variables, variableCount, arguments[1], 1); 
        variable *var2 = findVar(variables, variableCount, arguments[2], 0);
        if (var1->type != NUM) cry("You can only do math on a 'num' type variable!");
        if (var2 == NULL) { op2 = atof(arguments[2]); }
        else if (var2->type != NUM) cry("You can only do math on a 'num' type variable!");
        else { op2 = atof(var2->value); }
        op1 = atof(var1->value); double output = 0;
        if (strcmp(operation, "add") == 0) output = op1 + op2;
        else if (strcmp(operation, "sub") == 0) output = op1 - op2;
        else if (strcmp(operation, "mul") == 0) output = op1 * op2;
        else if (strcmp(operation, "div") == 0) { if (op2 == (double)0) {cry("div by zero error\neat shit and die, nerd");} else{ output = op1 / op2;}}

        char tempStr[100];
        if (output - (int)output != 0) { sprintf(tempStr, "%f", output); } // very accursed float check, very yummers
        else { sprintf(tempStr, "%d", (int)output); }
        set_variable_value(var1, tempStr);
    } 
    else if (strcmp(operation, "set") == 0) { 
        int type = grabType(arguments[0]);
        if (type == IN) { setVar(findVar(variables, variableCount, arguments[1], 1), type, ""); }
        else if (type == STR) { char *concatenated = joinStringsSentence(arguments, current_instruction->argumentCount, 2); setVar(findVar(variables, variableCount, arguments[1], 1), type, concatenated); free(concatenated); }
        else { setVar(findVar(variables, variableCount, arguments[1], 1), type, arguments[2]); }
    }
    else if (strcmp(operation, "type") == 0) {
        variable check = *findVar(variables, variableCount, arguments[0], 0); 
        variable *var = findVar(variables, variableCount, arguments[1], 1); 
        int type = check.type; char *output; var->type = STR;
        switch (type) {
            case NUM: output = strdup("num"); break;
            case BOOL: output = strdup("bool"); break;
            case STR: output = strdup("str"); break;
            default: output = strdup("none"); break;
        }
        set_variable_value(var, output);
        free(output);
    }

    else if (strcmp(operation, "printc") == 0) { char *print = joinStringsSentence(arguments, current_instruction->argumentCount, 0); printf("%s", print); free(print); } 
    else if (strcmp(operation, "println") == 0) { printf("\n"); } 
    else if (strcmp(operation, "prints") == 0) { printf(" "); } 
    else if (strcmp(operation, "print") == 0) { printf("%s", findVar(variables, variableCount, arguments[0], 0)->value); } 
    else if (strcmp(operation, "quit") == 0) { return 0; } 
    else if (strcmp(operation, "conv") == 0) { conv(findVar(variables, variableCount, arguments[0], 0), grabType(arguments[1])); } 
    else if (strcmp(operation, "write") == 0) { writeFile(arguments[0], arguments[1]); } 
    else if (strcmp(operation, "writev") == 0) { writeFile(arguments[0], findVar(variables, variableCount, arguments[1], 0)->value); } 
    else if (strcmp(operation, "poem") == 0 || strcmp(operation, "prose") == 0 || strcmp(operation, "simas") == 0 || strcmp(operation, "microprose") == 0 || strcmp(operation, "cmas") == 0) { print(poem); }

    else if (strcmp(operation, "st") == 0 || strcmp(operation, "ste") == 0 || strcmp(operation, "gt") == 0 || strcmp(operation, "gte") == 0 ||
            strcmp(operation, "and") == 0 || strcmp(operation, "or") == 0 || strcmp(operation, "eqc") == 0 || strcmp(operation, "eqv") == 0 ||
            strcmp(operation, "neqc") == 0 || strcmp(operation, "neqv") == 0) {

        variable *var1 = findVar(variables, variableCount, arguments[1], 0);
        variable *var2 = findVar(variables, variableCount, arguments[2], 0);
        char *operand1 = NULL; char *operand2 = NULL; int output = 0;
        if (var1 == NULL) { cry("No variable!"); }
        else { operand1 = strdup(var1->value); }
        if (var2 == NULL) { operand2 = joinStringsSentence(arguments, current_instruction->argumentCount, 2); }
        else { operand2 = strdup(var2->value); }

        if (strcmp(operation, "st") == 0) { 
            if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
            output = compare('<', operand1, operand2); 
        }
        else if (strcmp(operation, "ste") == 0) {
            if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n"); 
            output = compare('[', operand1, operand2); 
        }
        else if (strcmp(operation, "gt") == 0) { 
            if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
            output = compare('>', operand1, operand2); 
        }
        else if (strcmp(operation, "gte") == 0) { 
            if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
            output = compare(']', operand1, operand2); 
        }

        else if (strcmp(operation, "and") == 0) { 
            if (var1->type != BOOL || (var2 != NULL && var2->type != BOOL)) cry("Operand must be of \"bool\" type!\n");
            output = compare('&', operand1, operand2); 
        }
        else if (strcmp(operation, "or") == 0) { 
            if (var1->type != BOOL || (var2 != NULL && var2->type != BOOL)) cry("Operand must be of \"bool\" type!\n");
            output = compare('|', operand1, operand2); 
        }

        else if (strcmp(operation, "eqc") == 0) { 
            free(operand2); operand2 = strdup(arguments[2]);
            output = compare('e', operand1, operand2); 
        }
        else if (strcmp(operation, "eqv") == 0) { output = compare('e', operand1, operand2); }
        else if (strcmp(operation, "neqc") == 0) { 
            free(operand2); operand2 = strdup(arguments[2]);
            output = compare('n', operand1, operand2); 
        }
        else if (strcmp(operation, "neqv") == 0) { output = compare('n', operand1, operand2); }

        var1->type = BOOL;
        if (output) set_variable_value(var1, "true");
        else set_variable_value(var1, "false");

        free(operand1); free(operand2);
    }
    
    else if (strcmp(operation, "list") == 0) {
        char *listInstruction = lowerize(arguments[0]);
        list *li = findList(*lists, *listCount, arguments[1]);
        if (li == NULL && (strcmp(listInstruction, "new") && strcmp(listInstruction, "load"))) cry("cant find that list!\n");

        if (strcmp(listInstruction, "new") == 0) { li = addListToLists(lists, arguments[1], listCount); }
        else if (strcmp(listInstruction, "appv") == 0) {  
            variable *var = findVar(variables, variableCount, arguments[3], 0);
            variable tempVar = { .type = var->type, .value = strdup(var->value) };
            appendElement(li, tempVar); 
        }
        else if (strcmp(listInstruction, "appc") == 0) { 
            variable var = { .type = grabType(arguments[2]), .value = joinStringsSentence(arguments, current_instruction->argumentCount, 3) };
            appendElement(li, var);
        }
        else if (strcmp(listInstruction, "show") == 0) {
            char *formatted = formatList(*li);
            printf("%s", formatted);
            free(formatted);
        }
        else if (strcmp(listInstruction, "dump") == 0) {
            char *formatted = formatList(*li);
            writeFile(arguments[2], formatted);
            free(formatted);
        }        
        else if (strcmp(listInstruction, "len") == 0) {
            variable *var = findVar(variables, variableCount, arguments[2], 1); 
            char length[10];
            sprintf(length, "%d", li->elements); DEBUG_PRINT(length);
            var->type = NUM; set_variable_value(var, length);
        }

        else if (strcmp(listInstruction, "acc") == 0) {
            variable *var = findVar(variables, variableCount, arguments[3], 1); 
            int element = atoi(arguments[2]) - 1;
            var->type = li->variables[element].type;
            set_variable_value(var, li->variables[element].value);
        }
        else if (strcmp(listInstruction, "del") == 0) { removeElement(li, atoi(arguments[2]) - 1); }
        else if (strcmp(listInstruction, "upv") == 0) { updateList(li, *findVar(variables, variableCount, arguments[4], 0), atoi(arguments[2]) - 1); }
        else if (strcmp(listInstruction, "upc") == 0) { variable var = { .type = grabType(arguments[3]), .value = joinStringsSentence(arguments, current_instruction->argumentCount, 4) }; updateList(li, var, atoi(arguments[2]) - 1); free(var.value); }

        else if (strcmp(listInstruction, "load") == 0) {
            char *temp = readFile(arguments[2]); int type; int start = 0;
            while (1) { if (temp[start] == '[') { break; } start += 1; }
            for (int i = start; i < (int)strlen(temp); i++) {
                char c = temp[i]; int length = 0;
                if (c == ']') break;
                if (c == '[' || c == ',') continue;
                if (c == '"') { type = STR; continue; }
                if (type != STR) { if (isdigit(c)) { type = NUM; } else { type = BOOL; }}

                while ((c = temp[i + length]) != ',' && (c = temp[i + length]) != '"' && (c = temp[i + length]) != '[' && (c = temp[i + length]) != ']') { length += 1; DEBUG_PRINT(&c); }

                char *value = (char *)calloc(length + 1, sizeof(char));
                for (int j = 0; j < length; j++) { value[j] = temp[i + j]; }
                i += length; 
                value[length] = '\0';
                DEBUG_PRINT(value);
                if (li == NULL) { addListToLists(lists, arguments[1], listCount); li = findList(*lists, *listCount, arguments[1]); }

                variable var = { .type = type, .value = strdup(value) };
                appendElement(li, var); type = 0; free(value);
            }
            free(temp);
        }

        else { cry("Invalid list instruction!"); }

        free(listInstruction);
    } 
    else if (strlen(operation) == 0) return 1;
    else { /* do nothing, as it's probably just a junk instruction */ }

    return 1;
}

void executeFile(openFile *current, int doFree) {
    for (int j = 0; j < current->instructionCount; j++) {
        DEBUG_PRINTF("\nExecuting instruction %s on line %d.\n", current->instructions[j]->operation, j); 
        if (executeInstruction(current->instructions[j], &current->variables, &current->labels, &current->lists, &current->variableCount, &current->labelCount, &current->listCount, &j) == 0) break;
    }

    if (doFree) freeFile(*current);
}

int main(int argc, const char * argv[]) {
    if (argc >= 2) { 
        if (argc >= 3) { if (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "--debug") == 0) { debugMode = 1; printf("debug mode enabled\n"); }}
        if (argc == 4) { if (strcmp(argv[3], "-j") == 0 || strcmp(argv[3], "--jmp") == 0) { debugMode = 2; printf("jump debugger enabled\n"); }}
        openFile new = openSimasFile(argv[1]);
        preprocessLabels(&new);
        executeFile(&new, 1);
    } else {
        beginCommandLine();
    }

    return 0;
} // we are the shinglefuckers of bong juice ltd.