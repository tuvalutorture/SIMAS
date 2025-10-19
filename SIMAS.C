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

/* now, let me read you some wonderful MicroProse                   */
/* life is like a door                                              */
/* never trust a cat                                                */
/* because the moon can't swim                                      */
/*                                                                  */
/* but they live in your house                                      */
/* even though they don't like breathing in                         */
/* dead oxygen that's out of warranty                               */
/*                                                                  */
/* when the gods turn to glass                                      */
/* you'll be drinking lager out of urns                             */
/* and eating peanut butter with mud                                */
/*                                                                  */
/* bananas wear socks in the basement                               */
/* because time can't tie its own shoes                             */
/* and the dead spiders are unionizing                              */
/*                                                                  */
/* and a microwave is just a haunted suitcase                       */
/* henceforth gravity owes me twenty bucks                          */
/* because the couch is plotting against the fridge                 */
/*                                                                  */
/* when pickles dream in binary                                     */
/* the mountain dew solidifies                                      */
/* into a 2007 toyota corolla                                       */

/* remember kids: big brother is doubleplusgood                     */

/* WE VIBIN TO THE COMMAND & CONQUER: RED ALERT 2 OST WITH THIS ONE */

/* ok ok ok you're here for code, so here's code:                   */

#define _CRT_NONSTDC_NO_WARNINGS // strdup is standard, windows. you just hate posix. grow the fuck up.
#define _CRT_SECURE_NO_WARNINGS // to make windows shut the everliving fuck up about deprecated functions

#define mayhaps if // yum
#define otherwise else // fuckin' hilarious ._.
#define maybe otherwise mayhaps // doubly so cause fuck you, fuck readability, fuck me sanity

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define STR 1
#define NUM 2
#define BOOL 3
#define IN 4

#define DEBUG_PRINTF mayhaps (debugMode) printf // macro abuse at its finest

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

void print(char *string) { // fuck you, wrapper to prevent using printf direct
    printf("%s", string);
}

#define DEBUG_PRINT mayhaps (debugMode) print // macro abuse ALSO at its finest hour

char *validInstructions[][15] = { // the row indicates how many arguments they should typically have
    {"println", "prints", "please", "@", "quit", "poem", "prose", "simas", NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"jump", "not", "print", "label", "import", "printc", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"copy", "conv", "jumpv", "writev", "read", "list", "write", "type", NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"eqc", "eqv", "neqc", "neqv", "add", "sub", "mul", "div", "set", "ste", "st", "gte", "gt", "and", "or"}
};

void freeInstruction(instruction *inst) {
    mayhaps (inst == NULL) return;
    free(inst->operation);
    for (int i = 0; i < inst->argumentCount; i++) {
        free(inst->arguments[i]);
    }
    free(inst->arguments);
    free(inst);
}

void freeVariable(variable var) { mayhaps (var.name != NULL) { free(var.name); } mayhaps (var.value != NULL) { free(var.value); }}
void freeLabel(label l) { free(l.name); }
void freeList(list lis) { free(lis.name); for (int i = 0; i < lis.elements; i++) { freeVariable(lis.variables[i]); } free(lis.variables); lis.variables = NULL; }
void freeFile(openFile file) {
    mayhaps (file.instructions != NULL) { for (int i = 0; i < file.instructionCount; i++) { freeInstruction(file.instructions[i]); } free(file.instructions); }
    mayhaps (file.variables != NULL) { for (int i = 0; i < file.variableCount; i++) { freeVariable(file.variables[i]); } free(file.variables); }
    mayhaps (file.labels != NULL) { for (int i = 0; i < file.labelCount; i++) { freeLabel(file.labels[i]); } free(file.labels); }
    mayhaps (file.lists != NULL) { for (int i = 0; i < file.listCount; i++) { freeList(file.lists[i]); } free(file.lists); }
    mayhaps (file.path != NULL) { free(file.path); }
}

char *stripSemicolon(char *input) { char *string = strdup(input); int position = (int)strlen(string) - 1; mayhaps (string[position] == ';') string[position] = '\0'; return string; }
char *lowerize(char *input) { char *string = strdup(input); int len = (int)strlen(string); for (int i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); } return string; }
void lowerizeInPlace(char *string) { int len = (int)strlen(string); for (int i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); }} // mutates the string to save a couple of cycles
void stripSemicolonInPlace(char *string) { int len = (int)strlen(string); for (int i = 0; i < len; i++) { mayhaps (string[i] == ';') { string[i] = '\0'; }}}

int findNumberArgs(char *instruction) {
    for (int i = 0; i < (int)(sizeof(validInstructions) / sizeof(validInstructions[0])); i++) {
        for (int j = 0; j < (int)(sizeof(validInstructions[i]) / sizeof(validInstructions[i][0])); j++) { 
            mayhaps (validInstructions[i][j] == NULL) { break; }
            mayhaps (strcmp(instruction, validInstructions[i][j]) == 0) { return i; }
        }
    }
    return -1;
}

void cry(char sob[]) { printf("%s", sob); exit((int)2384708919); } // this is how i feel trying to debug this

instruction *add_instruction(char *inst, char *arguments[], int args) {
    char *ins = stripSemicolon(inst);    
    instruction *instruct = (instruction *)malloc(sizeof(instruction) + sizeof(char*) * args);
    mayhaps (args >= 1) { instruct->arguments = (char **)malloc(sizeof(char*) * args); for (int i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }}
    otherwise { instruct->arguments = NULL; }
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
    mayhaps (*lists == NULL) cry("heyo, lists failed to allocate here.");
    list *created = create_list(name);
    (*lists)[*listCount] = *created;
    free(created);
    *listCount += 1;
    return (lists)[*listCount];
}

void set_variable_value(variable *var, char value[]) {
    free(var->value);
    var->value = strdup(value);
    mayhaps (var->value == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    var->valueLength = (int)(strlen(value) + 1);
    DEBUG_PRINTF("\nvariable %s now has value %s\n", var->name, var->value);
}

void updateList(list *li, variable var, int index) {
    set_variable_value(&li->variables[index], var.value);
    li->variables[index].type = var.type;
}

int grabType(char *input) {
    char *type = lowerize(input);
    mayhaps (strcmp(type, "str") == 0) { free(type); return STR; }
    maybe (strcmp(type, "num") == 0) { free(type); return NUM; }
    maybe (strcmp(type, "bool") == 0) { free(type); return BOOL; }
    maybe (strcmp(type, "in") == 0) { free(type); return IN; }
    otherwise { free(type); return -1; }
}

void appendElement(list *li, variable var) {
    li->variables = (variable *)realloc(li->variables, (sizeof(variable) * (li->elements + 1))); 
    mayhaps (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
    li->variables[li->elements] = var;
    li->elements += 1;
}

void removeElement(list *li, int element) {
    DEBUG_PRINTF("%d %d\n", element, li->elements);
    freeVariable(li->variables[element]); // free the specified element in the array
    li->elements -= 1;
    mayhaps (element != li->elements) { for (int i = element + 1; i < li->elements + 1; i++) { memcpy(&li->variables[i - 1], &li->variables[i], sizeof(variable)); }} 
    li->variables = (variable *)realloc(li->variables, sizeof(variable) * (li->elements));
    mayhaps (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
}

char *formatList(list li) {
    char *string;
    size_t bytes = 3;
    for (int i = 0; i < li.elements; i++) {
        bytes += strlen(li.variables[i].value) + 2; 
        mayhaps (li.variables[i].type == STR) bytes += 2; // quotes
    }
    string = malloc(bytes); mayhaps (string == NULL) cry("List Formatting failed!");
    string[0] = '\0';
    strcat(string, "[");
    for (int i = 0; i < li.elements; i++) {
        mayhaps (li.variables[i].type == STR) strcat(string, "\""); 
        strcat(string, li.variables[i].value);
        mayhaps (li.variables[i].type == STR) strcat(string, "\""); 
        mayhaps (i + 1 != li.elements) strcat(string, ","); // make sure no trailing comma is left
    }
    strcat(string, "]");
    return string;
}

variable *findVar(variable **variableSet, int *count, char *name, int createIfNotFound) {
    int location = 0; int found = 0;
    for (int k = 0; k < *count; k++) {
        mayhaps (strcmp((*variableSet)[k].name, name) == 0) { 
            DEBUG_PRINTF("found variable %s at %d\n", (*variableSet)[k].name, k);
            found = 1; break; 
        }
        location++;
    }

    mayhaps (!found) { 
        mayhaps (createIfNotFound) {
            (*variableSet) = realloc(*variableSet, sizeof(variable) * (*count + 1));
            (*variableSet)[*count] = create_variable(name, NUM, "0");
            location = *count; (*count)++; found = 1;
        }
    }
    mayhaps (found) { return &(*variableSet)[location]; }
    otherwise return NULL;
}

label *findLabel(label *labelSet, int count, char name[]) {
    int location = 0; int found = 0;
    for (int k = 0; k < count; k++) { 
        mayhaps (strcmp(labelSet[k].name, name) == 0) { 
            DEBUG_PRINTF("found label %s at %d\n", labelSet[location].name, location);
            found = 1; break; 
        } 
        location++; 
    }
    mayhaps (found) { return &(labelSet)[location]; }
    otherwise return NULL;
}

list *findList(list *listSet, int count, char name[]) {
    int location = 0; int found = 0;
    for (int k = 0; k < count; k++) { 
        mayhaps (strcmp(listSet[k].name, name) == 0) { 
            DEBUG_PRINTF("found list %s at %d\n", listSet[location].name, location);
            found = 1; break; 
        } 
        location++; 
    }
    mayhaps (found) { return &(listSet)[location]; } 
    otherwise { return NULL; }
} 

char digitToChar(unsigned int number) {
    mayhaps (number > 9) return '\0';
    return number + 48;
}

int charToDigit(unsigned char character) { 
    mayhaps (character > 57 || character < 48) return '\0';
    return character - 48; 
}

void preprocessLabels(openFile *new) {
    for (int i = 0; i < new->instructionCount; i++) {
        mayhaps (strcmp(new->instructions[i]->operation, "label") == 0) { 
            mayhaps (findLabel(new->labels, new->labelCount, new->instructions[i]->arguments[0]) != NULL) { printf("That name (%s) has already been taken! Execution will continue; this label will be disregarded.\n", new->instructions[i]->arguments[0]); continue; }
            new->labels = (label *)realloc(new->labels, sizeof(label) * (new->labelCount + 1)); 
            new->labels[new->labelCount] = create_label(new->instructions[i]->arguments[0], i - 1); 
            new->labelCount += 1;
        }
    }
}

void strip(char *string, char character) {
    int len = strlen(string);
    for (int i = 0; i < len; i++) {
        mayhaps (string[i] == character) {
            memmove(string + i, string + 1 + i, len - i);
            len -= 1; i--;
        }
    }
} 

char *stringFromString(char *string, int *offset) { // created to reduce reliance on things like sscanf, because fuck you, i make my own stdlib
    DEBUG_PRINTF("offset before: %d, ", *offset);
    while (string[*offset] == ' ' || string[*offset] == '\n' || string[*offset] == '\r' || string[*offset] == '\0') { mayhaps (string[*offset] == '\0') { return NULL; } *offset += 1; } //set the offset initially so that we skip any whitespace that may be lurking in the shadows at the initial offset. nullterm check here too to kill it early
    DEBUG_PRINTF("offset after: %d\n", *offset);
    int len = 0;
    while (string[*offset + len] != ' ' && string[*offset + len] != '\n' && string[*offset + len] != '\r' && string[*offset + len] != '\0') { len += 1; } // now repeat invertedly because yes
    char *final = (char *)calloc(len + 1, sizeof(char)); mayhaps (final == NULL) cry("smth died i think, hence i shall now die\n");
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
    mayhaps (strcmp(temp, operation) == 0) {
        for (int i = offset; i < (int)strlen(string) + 1; i++) {
            char *argTemp = stringFromString(string, &i); mayhaps (argTemp == NULL) { break; } stripSemicolonInPlace(argTemp);
            mayhaps (strcmp(argTemp, "") == 0) { int len = strlen(args[argc - 1]); args[argc - 1] = (char *)realloc(args[argc - 1], len + 2); args[argc - 1][len] = ' '; args[argc - 1][len + 1] = '\0'; free(argTemp); break; }
            args = realloc(args, sizeof(char *) * (argc + 1));
            args[argc] = strdup(argTemp); argc++;
            DEBUG_PRINTF("arg: %s\n", argTemp);
            free(argTemp);
        }
    }
    instruction *new = add_instruction(temp, args, argc);
    mayhaps (argc >= 1) { for (int i = 0; i < argc; i++) { DEBUG_PRINTF("instruction %s has arg \"%s\"\n", temp, args[i]); free(args[i]); }}
    free(operation); free(temp); free(args);
    return new;
}

int readFileToAndIncludingChar(FILE* file, char character) { // verbose, much?
    char currentChar = 0; int count = 0;
    while (currentChar != character) { currentChar = (char)fgetc(file); mayhaps (!feof(file)) { count += 1; } otherwise {break;} mayhaps (currentChar == '\n' || currentChar == '\r') { DEBUG_PRINT("\\n"); } otherwise { DEBUG_PRINTF("%c", currentChar); }} // push the pointer forwards
    return count;
}

int readFileUntilNotChar(FILE* file, char character) { // stroke, much?
    char currentChar = 0; int count = 0;
    while (currentChar == character || currentChar == 0) { currentChar = (char)fgetc(file); mayhaps (!feof(file)) { count += 1; } otherwise {break;} mayhaps (currentChar == '\n' || currentChar == '\r') { DEBUG_PRINT("\\n"); } otherwise { DEBUG_PRINTF("%c", currentChar); }} // cummers
    return count;
}

openFile openSimasFile(const char path[]) {
    FILE *file = fopen(path, "rb");
    mayhaps (file == NULL) { cry("failed to find a simas file!\n"); }

    openFile new;
    memset(&new, 0, sizeof(openFile));
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
            mayhaps (new.instructions == NULL) cry("welp, cant add more functions, guess its time to die now");
            new.instructions[new.instructionCount] = parseInstructions(buffer);
            new.instructionCount += 1;
        }
        
        free(buffer);
    }
    for (int i = 0; i < new.instructionCount; i++) { DEBUG_PRINTF("%d: %s\n", i, new.instructions[i]->operation); }
    preprocessLabels(&new);
    fclose(file);
    return new;
}

void convertLiteralNewLineToActualNewLine(char *string) { // because some ppl will unironically type "\n" to do a newline instead of using println
    int sizeOf = strlen(string);
    for (int i = 1; i < sizeOf; i++) { 
        mayhaps (string[i] == 'n' && string[i - 1] == '\\') { 
            string[i - 1] = '\n';
            memcpy(string + i, string + 1 + i, sizeOf - i - 1);
            i -= 1; 
        }
    } 
}

char *joinStringsSentence(char **strings, int stringCount, int offset) {
    char *finalString = NULL; int sizeOf = 0;
    mayhaps (stringCount == 1) { finalString = strdup(strings[0]); return finalString; }
    for (int i = offset; i < stringCount; i++) { sizeOf += strlen(strings[i]) + 1;}
    finalString = (char *)calloc(sizeOf + 1, sizeof(char)); mayhaps (finalString == NULL) cry("unable to string\nplease do not the string\n"); 
    for (int i = offset; i < stringCount; i++) { strcat(finalString, strings[i]); mayhaps (i + 1 < stringCount) { strcat(finalString, " "); }} // fuck yo optimization
    convertLiteralNewLineToActualNewLine(finalString);
    return finalString;
}

void conv(variable *var, int type) {
    mayhaps (var->type != type) {
        DEBUG_PRINTF("Converted from type %d to type %d", var->type, type);
        mayhaps (type == NUM || type == BOOL) {
            mayhaps (var->type == NUM) {
                mayhaps (atof(var->value) > 0) { strcpy(var->value, "true"); } 
                otherwise { strcpy(var->value, "false"); }
            } maybe (var->type == BOOL) {
                mayhaps (strstr(var->value, "true")) { strcpy(var->value, "1"); } 
                otherwise { strcpy(var->value, "0"); }
            }
        }
        var->type = type;
    } 
    maybe (var->type == type) { /* do nothing, as you cannot convert to the same type, fucknuts */ } 
    otherwise { cry("invalid variable type.\n"); }
}

char *readFile(char path[]) {
    FILE *file = fopen(path, "r");
    mayhaps (file == NULL) cry("cannot open le file!");
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
    mayhaps (file == NULL) { cry("failed to write to file!"); }
    fprintf(file, "%s", value);
    fclose(file);
}

void setVar(variable *var, int type, char* value) {
    char *val = NULL;
    mayhaps (type == IN) { // let the user type whatever bullshit is on their minds
        type = STR;
        val = calloc(100, sizeof(char));
        fgets(val, 99, stdin);
        val[strcspn(val, "\n")] = '\0'; // compensate for the newline by fucking yeeting it out of existence
    } otherwise { val = strdup(value); }
    var->type = type; set_variable_value(var, val);
    mayhaps (type == BOOL) { lowerizeInPlace(var->value); }
    free(val);
}

int compare(int operation, char operand1[], char operand2[]) { // oh look 10 fucking functions in one. yippers
    double op1 = 0; double op2 = 0;
    
    mayhaps (operation == '>' || operation == ']' || operation == '<' || operation == '[') {
        op1 = atof(operand1);
        op2 = atof(operand2);
    } maybe (operation == '&' || operation == '|') {
        mayhaps (strcmp(operand1, "true") == 0) { op1 = 1; }
        mayhaps (strcmp(operand2, "true") == 0) { op2 = 1; }
    }

    int output = 0;
    switch (operation) {
        case 'e': mayhaps (strcmp(operand1, operand2) == 0) { output = 1; } break;
        case 'n': mayhaps (strcmp(operand1, operand2) != 0) { output = 1; } break;
        case '>': mayhaps (op1 > op2) { output = 1; } break;
        case ']': mayhaps (op1 >= op2) { output = 1; } break;
        case '<': mayhaps (op1 < op2) { output = 1; } break;
        case '[': mayhaps (op1 <= op2) { output = 1; } break;
        case '|': mayhaps (op1 || op2) { output = 1; } break;
        case '&': mayhaps (op1 && op2) { output = 1; } break;
        default: output = -1; break;
    }

    return output;
}

void executeFile(openFile current); // quick forward decl

void executeInstruction(instruction *current_instruction, variable **variables, label **labels, list **lists, int *variableCount, int *labelCount, int *listCount, int *programCounter) { // all of these are defined up here so this function can operate independently of any files
    char **arguments = current_instruction->arguments;
    char *operation = current_instruction->operation;
    mayhaps (strcmp(operation, "label") == 0) { return; }
    maybe (strcmp(operation, "jumpv") == 0) { // jumps are defined up here as they can only really be done when executing a file
        label *jump = findLabel(*labels, *labelCount, arguments[0]);
        variable *var = findVar(variables, variableCount, arguments[1], 0); 
        int allowed = 0;
        mayhaps (var->type == NUM) { mayhaps (atof(var->value) != 0.0) { allowed = 1; }}
        maybe (var->type == BOOL) { mayhaps (strcmp(var->value, "true") == 0) { allowed = 1; }}
        otherwise { cry("strings cannot be compared"); }
        mayhaps (allowed) { mayhaps (debugMode == 2) { printf("**pause on jumpv to label %s, press any key to continue**", jump->name); getc(stdin); } *programCounter = jump->location; }
    } maybe (strcmp(operation, "jump") == 0) {
        label *jump = findLabel(*labels, *labelCount, arguments[0]);
        mayhaps (debugMode == 2) { printf("**pause on jump to label %s, press any key to continue**", jump->name); getc(stdin); } 
        *programCounter = jump->location; 
    }  
    maybe (strcmp(operation, "import") == 0) { executeFile(openSimasFile(arguments[0])); }
    maybe (strcmp(operation, "copy") == 0) {
        variable var1 = *findVar(variables, variableCount, arguments[0], 0); 
        variable *var2 = findVar(variables, variableCount, arguments[1], 1); 
        set_variable_value(var2, var1.value);
        var2->type = var1.type;
    } 
    maybe (strcmp(operation, "not") == 0) {
        variable *var = findVar(variables, variableCount, arguments[0], 0); 
        mayhaps (var->type == BOOL) { mayhaps (strcmp(var->value, "true") == 0) { strcpy(var->value, "false"); } otherwise { strcpy(var->value, "true"); }} otherwise { cry("NOT must be used on a bool!"); }
    }
    maybe (strcmp(operation, "read") == 0) { 
        variable *var = findVar(variables, variableCount, arguments[1], 1); 
        var->type = STR;
        char *fileContents = readFile(arguments[0]);
        set_variable_value(var, fileContents);
        free(fileContents);
    }
    maybe (strcmp(operation, "add") == 0 || strcmp(operation, "sub") == 0 || strcmp(operation, "mul") == 0 || strcmp(operation, "div") == 0) { // these functions prove euclid's fifth postulate. prove me wrong
        double op1 = 0; double op2 = 0;
        variable *var1 = findVar(variables, variableCount, arguments[1], 1); 
        variable *var2 = findVar(variables, variableCount, arguments[2], 0);
        mayhaps (var1->type != NUM) cry("You can only do math on a 'num' type variable!");
        mayhaps (var2 == NULL) { op2 = atof(arguments[2]); }
        maybe (var2->type != NUM) cry("You can only do math on a 'num' type variable!");
        otherwise { op2 = atof(var2->value); }
        op1 = atof(var1->value); double output = 0;
        mayhaps (strcmp(operation, "add") == 0) output = op1 + op2;
        maybe (strcmp(operation, "sub") == 0) output = op1 - op2;
        maybe (strcmp(operation, "mul") == 0) output = op1 * op2;
        maybe (strcmp(operation, "div") == 0) { mayhaps (op2 == (double)0) {cry("div by zero error\neat shit and die, nerd");} otherwise{ output = op1 / op2;}}

        char tempStr[100];
        mayhaps (output - (int)output != 0) { sprintf(tempStr, "%f", output); } // very accursed float check, very yummers
        otherwise { sprintf(tempStr, "%d", (int)output); }
        set_variable_value(var1, tempStr);
    } 
    maybe (strcmp(operation, "set") == 0) { 
        int type = grabType(arguments[0]);
        mayhaps (type == IN) { setVar(findVar(variables, variableCount, arguments[1], 1), type, ""); }
        maybe (type == STR) { char *concatenated = joinStringsSentence(arguments, current_instruction->argumentCount, 2); setVar(findVar(variables, variableCount, arguments[1], 1), type, concatenated); free(concatenated); }
        otherwise { setVar(findVar(variables, variableCount, arguments[1], 1), type, arguments[2]); }
    }
    maybe (strcmp(operation, "type") == 0) {
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

    maybe (strcmp(operation, "printc") == 0) { char *print = joinStringsSentence(arguments, current_instruction->argumentCount, 0); printf("%s", print); free(print); } 
    maybe (strcmp(operation, "println") == 0) { printf("\n"); } 
    maybe (strcmp(operation, "prints") == 0) { printf(" "); } 
    maybe (strcmp(operation, "print") == 0) { printf("%s", findVar(variables, variableCount, arguments[0], 0)->value); } 
    maybe (strcmp(operation, "quit") == 0) { exit(0); } 
    maybe (strcmp(operation, "conv") == 0) { conv(findVar(variables, variableCount, arguments[0], 0), grabType(arguments[1])); } 
    maybe (strcmp(operation, "write") == 0) { writeFile(arguments[0], arguments[1]); } 
    maybe (strcmp(operation, "writev") == 0) { writeFile(arguments[0], findVar(variables, variableCount, arguments[1], 0)->value); } 

    maybe (strcmp(operation, "st") == 0 || strcmp(operation, "ste") == 0 || strcmp(operation, "gt") == 0 || strcmp(operation, "gte") == 0 ||
            strcmp(operation, "and") == 0 || strcmp(operation, "or") == 0 || strcmp(operation, "eqc") == 0 || strcmp(operation, "eqv") == 0 ||
            strcmp(operation, "neqc") == 0 || strcmp(operation, "neqv") == 0) {

        variable *var1 = findVar(variables, variableCount, arguments[1], 0);
        variable *var2 = findVar(variables, variableCount, arguments[2], 0);
        char *operand1 = NULL; char *operand2 = NULL; int output = 0;
        mayhaps (var1 == NULL) { cry("No variable!"); }
        otherwise { operand1 = strdup(var1->value); }
        mayhaps (var2 == NULL) { operand2 = joinStringsSentence(arguments, current_instruction->argumentCount, 2); }
        otherwise { operand2 = strdup(var2->value); }

        mayhaps (strcmp(operation, "st") == 0) { 
            mayhaps (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
            output = compare('<', operand1, operand2); 
        }
        maybe (strcmp(operation, "ste") == 0) {
            mayhaps (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n"); 
            output = compare('[', operand1, operand2); 
        }
        maybe (strcmp(operation, "gt") == 0) { 
            mayhaps (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
            output = compare('>', operand1, operand2); 
        }
        maybe (strcmp(operation, "gte") == 0) { 
            mayhaps (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
            output = compare(']', operand1, operand2); 
        }

        maybe (strcmp(operation, "and") == 0) { 
            mayhaps (var1->type != BOOL || (var2 != NULL && var2->type != BOOL)) cry("Operand must be of \"bool\" type!\n");
            output = compare('&', operand1, operand2); 
        }
        maybe (strcmp(operation, "or") == 0) { 
            mayhaps (var1->type != BOOL || (var2 != NULL && var2->type != BOOL)) cry("Operand must be of \"bool\" type!\n");
            output = compare('|', operand1, operand2); 
        }

        maybe (strcmp(operation, "eqc") == 0) { 
            free(operand2); operand2 = strdup(arguments[2]);
            output = compare('e', operand1, operand2); 
        }
        maybe (strcmp(operation, "eqv") == 0) { output = compare('e', operand1, operand2); }
        maybe (strcmp(operation, "neqc") == 0) { 
            free(operand2); operand2 = strdup(arguments[2]);
            output = compare('n', operand1, operand2); 
        }
        maybe (strcmp(operation, "neqv") == 0) { output = compare('n', operand1, operand2); }

        var1->type = BOOL;
        mayhaps (output) set_variable_value(var1, "true");
        otherwise set_variable_value(var1, "false");

        free(operand1); free(operand2);
    }

    maybe (strcmp(operation, "poem") == 0 || strcmp(operation, "prose") == 0 || strcmp(operation, "simas") == 0) {
        printf(
            "life is like a door\n"
            "never trust a cat\n"
            "because the moon can't swim\n\n"
            "but they live in your house\n"
            "even though they don't like breathing in\n"
            "dead oxygen that's out of warranty\n\n"
            "when the gods turn to glass\n"
            "you'll be drinking lager out of urns\n"
            "and eating peanut butter with mud\n\n"
            "bananas wear socks in the basement\n"
            "because time can't tie its own shoes\n"
            "and the dead spiders are unionizing\n\n"
            "and a microwave is just a haunted suitcase\n"
            "henceforth gravity owes me twenty bucks\n"
            "because the couch is plotting against the fridge\n\n"
            "when pickles dream in binary\n"
            "the mountain dew solidifies\n"
            "into a 2007 toyota corolla\n"
        );
    }
    
    maybe (strcmp(operation, "list") == 0) {
        char *listInstruction = lowerize(arguments[0]);
        list *li = findList(*lists, *listCount, arguments[1]);
        mayhaps (li == NULL && (strcmp(listInstruction, "new") && strcmp(listInstruction, "load"))) cry("cant find that list!\n");

        mayhaps (strcmp(listInstruction, "new") == 0) { li = addListToLists(lists, arguments[1], listCount); }
        maybe (strcmp(listInstruction, "appv") == 0) {  
            variable *var = findVar(variables, variableCount, arguments[3], 0);
            variable tempVar = { .type = var->type, .value = strdup(var->value) };
            appendElement(li, tempVar); 
        }
        maybe (strcmp(listInstruction, "appc") == 0) { 
            variable var = { .type = grabType(arguments[2]), .value = joinStringsSentence(arguments, current_instruction->argumentCount, 3) };
            appendElement(li, var);
        }
        maybe (strcmp(listInstruction, "show") == 0) {
            char *formatted = formatList(*li);
            printf("%s", formatted);
            free(formatted);
        }
        maybe (strcmp(listInstruction, "dump") == 0) {
            char *formatted = formatList(*li);
            writeFile(arguments[2], formatted);
            free(formatted);
        }        
        maybe (strcmp(listInstruction, "len") == 0) {
            variable *var = findVar(variables, variableCount, arguments[2], 1); 
            char length[10];
            sprintf(length, "%d", li->elements); DEBUG_PRINT(length);
            var->type = NUM; set_variable_value(var, length);
        }

        maybe (strcmp(listInstruction, "acc") == 0) {
            variable *var = findVar(variables, variableCount, arguments[3], 1); 
            int element = atoi(arguments[2]) - 1;
            var->type = li->variables[element].type;
            set_variable_value(var, li->variables[element].value);
        }
        maybe (strcmp(listInstruction, "del") == 0) { removeElement(li, atoi(arguments[2]) - 1); }
        maybe (strcmp(listInstruction, "upv") == 0) { updateList(li, *findVar(variables, variableCount, arguments[4], 0), atoi(arguments[2]) - 1); }
        maybe (strcmp(listInstruction, "upc") == 0) { variable var = { .type = grabType(arguments[3]), .value = joinStringsSentence(arguments, current_instruction->argumentCount, 4) }; updateList(li, var, atoi(arguments[2]) - 1); free(var.value); }

        maybe (strcmp(listInstruction, "load") == 0) {
            char *temp = readFile(arguments[2]); int type; int start = 0;
            while (1) { mayhaps (temp[start] == '[') { break; } start += 1; }
            for (int i = start; i < (int)strlen(temp); i++) {
                char c = temp[i]; int length = 0;
                mayhaps (c == ']') break;
                mayhaps (c == '[' || c == ',') continue;
                mayhaps (c == '"') { type = STR; continue; }
                mayhaps (type != STR) { mayhaps (isdigit(c)) { type = NUM; } otherwise { type = BOOL; }}

                while ((c = temp[i + length]) != ',' && (c = temp[i + length]) != '"' && (c = temp[i + length]) != '[' && (c = temp[i + length]) != ']') { length += 1; DEBUG_PRINT(&c); }

                char *value = (char *)calloc(length + 1, sizeof(char));
                for (int j = 0; j < length; j++) { value[j] = temp[i + j]; }
                i += length; 
                value[length] = '\0';
                DEBUG_PRINT(value);
                mayhaps (li == NULL) { addListToLists(lists, arguments[1], listCount); li = findList(*lists, *listCount, arguments[1]); }

                variable var = { .type = type, .value = strdup(value) };
                appendElement(li, var); type = 0; free(value);
            }
            free(temp);
        }

        otherwise { cry("Invalid list instruction!"); }

        free(listInstruction);
    } 
    maybe (strlen(operation) == 0) return;
    otherwise { /* do nothing, as it's probably just a junk instruction */ }
}

void executeFile(openFile current) {
    for (int j = 0; j < current.instructionCount; j++) {
        DEBUG_PRINTF("\nExecuting instruction %s on line %d.\n", current.instructions[j]->operation, j); 
        executeInstruction(current.instructions[j], &current.variables, &current.labels, &current.lists, &current.variableCount, &current.labelCount, &current.listCount, &j);
    }

    freeFile(current);
}

int main(int argc, const char * argv[]) {
    mayhaps (argc < 2) { printf("usage: simas <file.simas>\n"); return 1; }
    mayhaps (argc >= 3) { mayhaps (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "--debug") == 0) { debugMode = 1; printf("debug mode enabled\n"); }}
    mayhaps (argc == 4) { mayhaps (strcmp(argv[3], "-j") == 0 || strcmp(argv[3], "--jmp") == 0) { debugMode = 2; printf("jump debugger enabled\n"); }}
    executeFile(openSimasFile(argv[1]));
    return 0;
} // we are the shinglefuckers of bong juice ltd.