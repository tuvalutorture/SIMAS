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

/* ok ok ok you're here for code, so here's code:                   */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define STR 1
#define NUM 2
#define BOOL 3
#define IN 4

#define DEBUG_PRINT if (debugMode) printf // the definition of "holy fucking hell kill it with fire" but it still works because macros are god's cmd+f

int debugMode = 0;

typedef struct {
    char *name;
    int type;
    char *value; // 640k should be enough for anyone -- gill bates
    int valueLength;
} variable;

typedef struct {
    char *operation;
    int argumentCount;
    char *arguments[];
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
    char *path;
    instruction **instructions;
    variable *variables;
    label *labels;
    list *lists; // its a struct inside a struct inside a struct. yum
    int instructionCount;
    int variableCount;
    int labelCount;
    int listCount;
} openFile;

char *validInstructions[][15] = { // the row indicates how many arguments they should typically have
    {"println", "prints", "printc", "please", "@", "quit", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"jump", "not", "print", "label", "import", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"copy", "conv", "jumpv", "writev", "read", "list", "write", "type", NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"eqc", "eqv", "neqc", "neqv", "add", "sub", "mul", "div", "set", "ste", "st", "gte", "gt", "and", "or"}
};

char *listInstructions[][5] = {
    {"new", "show", NULL, NULL, NULL},
    {"dump", "del", "load", "len", "load"},
    {"appv", "appc", "acc", NULL, NULL},
    {"upv", "upc", NULL, NULL, NULL}
};

void freeInstruction(instruction *inst) {
    if (inst == NULL) return;
    free(inst->operation);
    for (int i = 0; i < inst->argumentCount; i++) {
        free(inst->arguments[i]);
    }
    free(inst);
}

void freeVariable(variable var) { free(var.name); free(var.value); }
void freeLabel(label l) { free(l.name); }
void freeList(list lis) { free(lis.name); for (int i = 0; i < lis.elements; i++) { freeVariable(lis.variables[i]); }}
void freeFile(openFile file) {
    for (int i = 0; i < file.instructionCount; i++) { freeInstruction(file.instructions[i]); }
    for (int i = 0; i < file.variableCount; i++) { freeVariable(file.variables[i]); }
    for (int i = 0; i < file.labelCount; i++) { freeLabel(file.labels[i]); }
    for (int i = 0; i < file.listCount; i++) { freeList(file.lists[i]); }

    free(file.instructions); free(file.variables); free(file.labels); free(file.lists); free(file.path);
}

char *stripSemicolon(char input[]) { char *string = strdup(input); int position = strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; return string; }
char *lowerize(const char input[]) { char *string = strdup(input); int len = strlen(string); for (int i = 0; i < len; i++) { string[i] = tolower(string[i]); } return string; }
void lowerizeInPlace(char string[]) { int len = strlen(string); for (int i = 0; i < len; i++) { string[i] = tolower(string[i]); }} // mutates the string to save a couple of cycles
void stripSemicolonInPlace(char string[]) { int position = strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; }

int findNumberArgs(char instruction[]) {
    for (int i = 0; i < sizeof(validInstructions) / sizeof(validInstructions[0]); i++) {
        for (int j = 0; j < sizeof(validInstructions[i]) / sizeof(validInstructions[i][0]); j++) { 
            if (validInstructions[i][j] == NULL) { break; }
            if (strcmp(stripSemicolon(instruction), validInstructions[i][j]) == 0) { return i; }
        }
    }
    return -1;
}

int findListArgs(char instruction[]) {
    for (int k = 0; k < sizeof(listInstructions) / sizeof(listInstructions[0]); k++) {
        for (int l = 0; l < sizeof(listInstructions[k]) / sizeof(listInstructions[k][0]); l++) { 
            if (listInstructions[k][l] == NULL) { break; }
            if (strcmp(stripSemicolon(instruction), listInstructions[k][l]) == 0) { return k; }
        }
    }
    return -1;
}

void cry(char sob[]) { printf("%s", sob); exit((int)2384708919); }

instruction *add_instruction(char inst[], char *arguments[]) {
    int args = 0;
    if (strcmp(inst, "list") == 0) { args = findListArgs(arguments[0]) + 2; }
    else { args = findNumberArgs(inst); } // prevents the feeding of garbage arguments by verifying it against the list of instructions and how many args they'll typically have
    if (args == -1) { printf("%s is not an instruction!\n", inst); exit(1); }
    if (strcmp(inst, "printc") == 0) args = 1;  // special case 'cause fuck you and fuck my sanity
    if (strcmp(inst, "set") == 0 && strcmp(lowerize(arguments[0]), "in") == 0) args = 2;
    
    instruction *instruct = (instruction *)malloc(sizeof(instruction) + sizeof(char*) * args);
    instruct->operation = stripSemicolon(inst); instruct->argumentCount = args;
    for (int i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }
    DEBUG_PRINT("added instruction %s of arg count %d\n", instruct->operation, instruct->argumentCount);
    return instruct;
}

variable create_variable(char name[], int type, char value[]) {
    variable var = { .name = strdup(name), .type = type, .value = strdup(value), .valueLength = strlen(value) + 1 };
    DEBUG_PRINT("created variable %s of type %d with value %s\n", var.name, var.type, var.value);
    return var;
}

label create_label(char name[], int location) {
    label label = { .name = stripSemicolon(name), .location = location };
    DEBUG_PRINT("\ncreated label %s on line %d\n", name, location);
    return label;
}

list create_list(char name[]) {
    list new;
    new.variables = NULL;
    new.name = strdup(name);
    new.elements = 0;
    return new;
}

void set_variable_value(variable *var, char value[]) {
    var->value = (char *)realloc(var->value, strlen(value) + 1);
    if (var->value == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    var->valueLength = strlen(value) + 1;
    strcpy(var->value, value);
    DEBUG_PRINT("\nvariable %s now has value %s\n", var->name, var->value);
}

int grabType(char type[]) {
    lowerizeInPlace(type);
    if (strcmp(type, "str") == 0) { return STR; }
    else if (strcmp(type, "num") == 0) { return NUM; }
    else if (strcmp(type, "bool") == 0) { return BOOL; }
    else if (strcmp(type, "in") == 0) { return IN; }
    else { return -1; }
}

void appendElement(list *li, variable var) {
    li->variables = realloc(li->variables, sizeof(variable) * (li->elements + 1));
    if (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
    li->variables[li->elements] = create_variable(var.name, var.type, var.value);
    li->elements++;
}

void removeElement(list *li, int element) {
    for (int i = element + 1; i < li->elements; i++) { li->variables[i - 1] = create_variable(li->variables[i].name, li->variables[i].type, li->variables[i].value); }
    freeVariable(li->variables[li->elements - 1]); // free the last element in the array now that its been shifted
    li->variables = realloc(li->variables, sizeof(variable) * (li->elements - 1));
    if (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
    li->elements--;
}

char *formatList(list li) {
    char *string;
    int bytes = 3;
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

variable *findVar(variable **variableSet, int *count, char name[], int createIfNotFound) {
    int location = 0; int found = 0;
    for (int k = 0; k < *count; k++) {
        if (strcmp((*variableSet)[k].name, name) == 0) { 
            DEBUG_PRINT("found variable %s at %d\n", (*variableSet)[k].name, k);
            found = 1; break; 
        }
        location++;
    }

    if (!found) { 
        if (createIfNotFound) {
            *variableSet = realloc(*variableSet, sizeof(variable) * (*count + 1));
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
            DEBUG_PRINT("found label %s at %d\n", labelSet[location].name, location);
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
            DEBUG_PRINT("found list %s at %d\n", listSet[location].name, location);
            found = 1; break; 
        } 
        location++; 
    }
    if (found) { return &(listSet)[location]; } 
    else { return NULL; }
} 

openFile openSimasFile(const char path[]) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) { cry("failed to find a simas file!\n"); }

    openFile new;
    memset(&new, 0, sizeof(openFile));
    new.path = strdup(path);

    while (!feof(file)) {
        int standardBufferSize = 100; // so that we can actually free shit! wow!
        DEBUG_PRINT("goin back for more\n");
        char *buffer = (char *)malloc(standardBufferSize); 
        char *buffer2 = (char *)malloc(standardBufferSize);
        char *args[10];
        int argc = 0; int expectedArgs = 0;
        if (buffer == NULL || buffer2 == NULL) cry("welp, no buffers = no parsing = no execution. so guess im dying now");
        fscanf(file, "%99s", buffer); 
        lowerizeInPlace(buffer);
        if (feof(file)) break;

        if (strcmp(buffer, "please") == 0) { continue; }

        if (strchr(buffer, '@') != NULL) { 
            if (strchr(buffer, ';') != NULL) { continue; }
            char currentChar = ' '; 
            while (currentChar != ';') { fscanf(file, "%c", &currentChar); } 
            continue; 
        }

        expectedArgs = findNumberArgs(buffer);

        if (expectedArgs >= 1) { fscanf(file, "%99s", buffer2); args[argc] = stripSemicolon(buffer2); argc++; DEBUG_PRINT("first arg: %s\n", buffer2); }
        if (strcmp(buffer, "label") == 0) { new.labels = (label *)realloc(new.labels, sizeof(label) * (new.labelCount + 1)); new.labels[new.labelCount] = create_label(buffer2, new.instructionCount - 1); new.labelCount += 1; continue; }

        if (strcmp(buffer, "list") == 0) { // find the instruction, as it is guaranteed to be the first argument
            expectedArgs = findListArgs(buffer2) + 2; // there are always at least 2 arguments in a list instruction
            char *buffer3 = (char *)malloc(standardBufferSize); 
            fscanf(file, "%99s", buffer3);  
            DEBUG_PRINT("list arg: %s\n", buffer3);
            args[argc] = stripSemicolon(buffer3); argc++; 
            free(buffer3);
        }

        char *temp = lowerize(buffer2);
        if (strcmp(temp, "in") == 0 && strcmp(buffer, "set")) expectedArgs--;
        free(temp); // no memory leaks here, no siree

        if ((strcmp(buffer, "printc") == 0 || strcmp(buffer, "write") == 0) || (grabType(buffer2) == STR && (strcmp(buffer, "set") == 0) || (strcmp(buffer, "eqc") == 0) || (strcmp(buffer, "neqc") == 0))) {
            if (strcmp(buffer, "eqc") == 0 || strcmp(buffer, "neqc") == 0 || strcmp(buffer, "set") == 0) { // we're gonna force these ones to add an extra variable 'cause otherwise it breaks
                char variable[100];
                fscanf(file, "%99s", &variable);
                args[argc] = strdup(variable); argc++; DEBUG_PRINT("second arg: %s\n", variable); 
                expectedArgs = 3;
            }
            char *string = NULL; int i = 0; char c = fgetc(file); int stringSize = 0; // fgetc here to skip over the extraneous space
            while ((c = fgetc(file)) != ';') {
                if (i + 1 >= stringSize) {
                    stringSize++; // just one. we dont really care for parse speed here.
                    string = realloc(string, stringSize); 
                    if (string == NULL) { cry("oops, done fucked up. dying now"); }
                }
                string[i] = c;
                if (i > 0 && string[i] == 'n' && string[i - 1] == '\\') { string[i - 1] = '\n'; i--; } // replace it with a newline and yeet the pointer backwards 
                i++;
            }
            string[stringSize] = '\0'; // null terminate that bitch
            DEBUG_PRINT("created string \"%s\"\n", string);
            args[argc] = strdup(string); argc++; free(string);
        }

        while (expectedArgs > argc) {
            fscanf(file, "%99s", buffer2); // sscanf is not soa:ZKXHdbkALDhbfiolSEDJGKLSDGHKASLFDGHKSLDAFGJHLKS sasketchawan or whatever
            stripSemicolonInPlace(buffer2);
            if (findNumberArgs(buffer2) != -1 || !feof(file)) {
                args[argc] = strdup(buffer2); argc++;
                DEBUG_PRINT("arg: %s\n", buffer2);
            } else {
                fseek(file, -1 * (strlen(buffer2) + 1), SEEK_CUR); // back the FUCK UP
                break;
            }
        }

        new.instructions = (instruction **)realloc(new.instructions, sizeof(instruction *) * (new.instructionCount + 1));
        if (new.instructions == NULL) cry("welp, cant add more functions, guess its time to die now");
        new.instructions[new.instructionCount] = add_instruction(buffer, args);
        for (int i = 0; i < argc; i++) { DEBUG_PRINT("instruction %s has arg %s\n", buffer, args[i]); free(args[i]); }
        new.instructionCount++;
        free(buffer); free(buffer2);
    }
    fclose(file);
    return new;
}

float doMath(int operation, float operand1, float operand2) { // 1 for addition, 2 for subtraction, 3 for mult, 4 for div
    if (operation == 4 && operand2 == 0) { cry("div by 0 error. eat shit and die, nerd\n"); }
    float output;
    switch (operation) {
        case 1: output = operand1 + operand2; break;
        case 2: output = operand1 - operand2; break;
        case 3: output = operand1 * operand2; break;
        case 4: output = operand1 / operand2; break;
        default: output = -1; break;
    }
    return output; 
}

void conv(variable *var, int type) {
    if (var->type != type) {
        if (type == NUM || type == BOOL) {
            if (var->type == NUM) {
                if (atof(var->value) > 0) { strcpy(var->value, "true"); } 
                else { strcpy(var->value, "false"); }
            } else if (var->type == BOOL) {
                if (strstr(var->value, "true")) { strcpy(var->value, "1"); } 
                else { strcpy(var->value, "0"); }
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
    char *contents; 
    long length;
    fseek(file, 0, SEEK_END); length = ftell(file);
    fseek(file, 0, SEEK_SET); 
    contents = malloc(length + 1);
    fread(contents, 1, length, file);
    contents[length] = '\0';
    fclose(file);
    DEBUG_PRINT("%s\n", contents);
    return contents;
}

void writeFile(char path[], char value[]) {
    FILE *file = fopen(path, "w");
    if (file == NULL) { cry("failed to write to file!"); }
    fprintf(file, "%s", value);
    fclose(file);
}

void setVar(variable *var, int type, char* value) {
    if (type == IN) { // let the user type whatever bullshit is on their minds
        var->type = STR;
        value = malloc(100);
        fgets(value, 99, stdin);
        value[strcspn(value, "\n")] = '\0'; // compensate for the newline by fucking yeeting it out of existence
        set_variable_value(var, value);
    } 
    var->type = type; set_variable_value(var, value);
    if (type == BOOL) { lowerizeInPlace(var->value); }
}

int compare(int operation, char operand1[], char operand2[]) { // oh look 10 fucking functions in one. yippers
    float op1 = 0; float op2 = 0;
    
    if (operation == '>' || operation == ']' || operation == '<' || operation == '[') {
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

void executeFile(openFile current) {
    for (int j = 0; j < current.instructionCount; j++) {
        instruction *current_instruction = current.instructions[j];
        char **arguments = current_instruction->arguments;
        char *operation = current_instruction->operation;
        DEBUG_PRINT("\nExecuting instruction %s on line %d.\n", operation, j); 

        if (strcmp(operation, "label") == 0) { continue; }
        else if (strcmp(operation, "import") == 0) { executeFile(openSimasFile(arguments[0])); }

        else if (strcmp(operation, "copy") == 0) {
            variable *var1 = findVar(&current.variables, &current.variableCount, arguments[0], 0); 
            variable *var2 = findVar(&current.variables, &current.variableCount, arguments[1], 1); 
            set_variable_value(var2, var1->value);
            var2->type = var1->type;
        } else if (strcmp(operation, "jumpv") == 0) {
            label *jump = findLabel(current.labels, current.labelCount, arguments[0]);
            variable *var = findVar(&current.variables, &current.variableCount, arguments[1], 0); 
            int allowed = 0;
            if (var->type == NUM) { if (atof(var->value) != 0.0) { allowed = 1; }}
            else if (var->type == BOOL) { if (strcmp(var->value, "true") == 0) { allowed = 1; }}
            else { cry("strings cannot be compared"); }
            if (allowed) { if (debugMode == 2) { printf("**pause on jumpv to label %s, press any key to continue**", jump->name); getc(stdin); } j = jump->location; }
        } else if (strcmp(operation, "jump") == 0) {
            label *jump = findLabel(current.labels, current.labelCount, arguments[0]);
            if (debugMode == 2) { printf("**pause on jump to label %s, press any key to continue**", jump->name); getc(stdin); } 
            j = jump->location; 
        }  
        else if (strcmp(operation, "not") == 0) {
            variable *var = findVar(&current.variables, &current.variableCount, arguments[0], 0); 
            if (var->type == BOOL) { if (strcmp(var->value, "true") == 0) { strcpy(var->value, "false"); } else { strcpy(var->value, "true"); }} else { cry("NOT must be used on a bool!"); }
        }
        else if (strcmp(operation, "read") == 0) { 
            variable *var = findVar(&current.variables, &current.variableCount, arguments[1], 1); 
            var->type = STR;
            set_variable_value(var, readFile(arguments[0])); 
        } 
        else if (strcmp(operation, "add") == 0 || strcmp(operation, "sub") == 0 || strcmp(operation, "mul") == 0 || strcmp(operation, "div") == 0) { 
            int op; float op1, op2;
            if (strcmp(operation, "add") == 0) op = 1;
            else if (strcmp(operation, "sub") == 0) op = 2;
            else if (strcmp(operation, "mul") == 0) op = 3;
            else if (strcmp(operation, "div") == 0) op = 4;
            variable *var1 = findVar(&current.variables, &current.variableCount, arguments[1], 1); 
            variable *var2 = findVar(&current.variables, &current.variableCount, arguments[2], 0);
            if (var1->type != NUM) cry("You can only do math on a 'num' type variable!");
            if (var2 == NULL) { op2 = atof(arguments[2]); }
            else if (var2->type != NUM) cry("You can only do math on a 'num' type variable!");
            else { op2 = atof(var2->value); }
            op1 = atof(var1->value);

            float output = doMath(op, op1, op2); 
            char tempStr[100];
            if (output - (int)output != 0) { sprintf(tempStr, "%f", output); }
            else { sprintf(tempStr, "%d", (int)output); }
            set_variable_value(var1, tempStr);
        } 
        else if (strcmp(operation, "set") == 0) { 
            int type = grabType(arguments[0]);
            if (type == IN) { setVar(findVar(&current.variables, &current.variableCount, arguments[1], 1), type, ""); }
            else { setVar(findVar(&current.variables, &current.variableCount, arguments[1], 1), type, arguments[2]); }
        }
        else if (strcmp(operation, "type") == 0) {
            variable *check = findVar(&current.variables, &current.variableCount, arguments[0], 0); 
            variable *var = findVar(&current.variables, &current.variableCount, arguments[1], 1); 
            int type = check->type; char *output; var->type = STR;
            switch (type) {
                case NUM: output = strdup("num"); break;
                case BOOL: output = strdup("bool"); break;
                case STR: output = strdup("str"); break;
                default: output = strdup("none"); break;
            }
            set_variable_value(var, output);
        }

        else if (strcmp(operation, "printc") == 0) { printf("%s", arguments[0]); } 
        else if (strcmp(operation, "println") == 0) { printf("\n"); } 
        else if (strcmp(operation, "prints") == 0) { printf(" "); } 
        else if (strcmp(operation, "print") == 0) { printf("%s", findVar(&current.variables, &current.variableCount, arguments[0], 0)->value); } 
        else if (strcmp(operation, "quit") == 0) { exit(0); } 
        else if (strcmp(operation, "conv") == 0) { conv(findVar(&current.variables, &current.variableCount, arguments[0], 0), grabType(arguments[1])); } 
        else if (strcmp(operation, "write") == 0) { writeFile(arguments[0], arguments[1]); } 
        else if (strcmp(operation, "writev") == 0) { writeFile(arguments[0], findVar(&current.variables, &current.variableCount, arguments[1], 0)->value); } 

        else if (strcmp(operation, "st") == 0 || strcmp(operation, "ste") == 0 || strcmp(operation, "gt") == 0 || strcmp(operation, "gte") == 0 ||
                strcmp(operation, "and") == 0 || strcmp(operation, "or") == 0 || strcmp(operation, "eqc") == 0 || strcmp(operation, "eqv") == 0 ||
                strcmp(operation, "neqc") == 0 || strcmp(operation, "neqv") == 0) {

            variable *var1 = findVar(&current.variables, &current.variableCount, arguments[1], 0);
            variable *var2 = findVar(&current.variables, &current.variableCount, arguments[2], 0);
            char *operand1, *operand2; int output;
            if (var1 == NULL) { cry("No variable!"); }
            else { operand1 = strdup(var1->value); }
            if (var2 == NULL) { operand2 = strdup(arguments[2]); }
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
            char *listInstruction = strdup(arguments[0]);
            lowerizeInPlace(listInstruction);

            list *li = findList(current.lists, current.listCount, arguments[1]);

            if (strcmp(listInstruction, "new") == 0) {
                current.lists = (list *)realloc(current.lists, sizeof(list) * (current.listCount));
                current.lists[current.listCount] = create_list(arguments[1]);
                current.listCount++;
            }

            else if (strcmp(listInstruction, "appv") == 0) { appendElement(li, *findVar(&current.variables, &current.variableCount, arguments[3], 0)); }
            else if (strcmp(listInstruction, "appc") == 0) { 
                variable var = { .type = grabType(arguments[2]), .value = arguments[3] };
                appendElement(li, var); 
            }
            else if (strcmp(listInstruction, "show") == 0) { printf("%s", formatList(*li)); }
            else if (strcmp(listInstruction, "dump") == 0) { writeFile(arguments[2], formatList(*li)); }
            
            else if (strcmp(listInstruction, "len") == 0) {
                variable *var = findVar(&current.variables, &current.variableCount, arguments[2], 1); 
                char length[10];
                sprintf(length, "%d", li->elements); DEBUG_PRINT("%s", length);
                *var = create_variable(var->name, NUM, length);
            }

            else if (strcmp(listInstruction, "acc") == 0) {
                variable *var = findVar(&current.variables, &current.variableCount, arguments[3], 1); 
                int element = atoi(arguments[2]) - 1;
                *var = create_variable(var->name, li->variables[element].type, li->variables[element].value);
            }

            else if (strcmp(listInstruction, "del") == 0) { removeElement(li, atoi(arguments[2]) - 1); }

            else if (strcmp(listInstruction, "upv") == 0) { 
                variable *var = findVar(&current.variables, &current.variableCount, arguments[4], 0); 
                li->variables[atoi(arguments[2]) - 1] = *var;
            }
            else if (strcmp(listInstruction, "upc") == 0) { 
                variable var = { .type = grabType(arguments[3]), .value = arguments[4] };
                li->variables[atoi(arguments[2]) - 1] = var; 
            }

            else if (strcmp(listInstruction, "load") == 0) {
                // might add it l8r if im feelin cute
            }

            else { cry("Invalid list instruction!"); }

            free(listInstruction);
        } 
        else { cry("Invalid instruction!\nUse \"--debug\" to find the issue & report it on the repository here:\nhttps://github.com/tuvalutorture/SIMAS/ \n(i am sorry, but this codebase is held together with duct tape T_T)"); }
    }

    freeFile(current);
}

int main(int argc, const char * argv[]) {
    if (argc < 2) { printf("usage: simas <file.simas>\n"); return 1; }
    if (argc >= 3) { if (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "--debug") == 0) { debugMode = 1; printf("debug mode enabled\n"); }}
    if (argc == 4) { if (strcmp(argv[3], "-j") == 0 || strcmp(argv[3], "--jmp") == 0) { debugMode = 2; printf("jump debugger enabled\n"); }}
    executeFile(openSimasFile(argv[1]));
    return 0;
} // we are the shinglefuckers of bong juice ltd.