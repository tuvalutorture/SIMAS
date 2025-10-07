/* CMAS (C SIMAS "SIMple ASsembly") interpreter             */
/* written by tuvalutorture                                 */
/* a feral child powered purely by rhcp and soda            */
/* pull on my lever it's, my guilty pleasure yes,           */
/* yes this codebase is abhorrent                           */
/* rome wasn't built in a day, but this was built in a week */
/* the gods of olympus have abandonded me                   */

/* "G3" was a term used by apple to refer to the PowerPC    */
/* 750 series CPU back in the late 90's / early 2000's when */
/* they produced computers using the PPC 750 processor,     */
/* as "G3" was a marketing term. PowerPC is a cpu           */
/* architecture based on the IBM "POWER" Architecture,      */
/* and was formed as a result of Apple, IBM, and Motorola   */
/* creating a new architecture, due to the Motorola 68k     */
/* series line slowly becoming a dead-end for Apple         */
/* Computers due to its inabilities to outperform Intel     */
/* and other such x86 machines. "G4" and "G5"               */
/* would later be used for subsequent Apple computers       */
/* featuring later PowerPC processors, with G4 referring to */
/* the 7400/8400 families of CPU (which were G3-based       */
/* 32-bit CPUs featuring AltiVec, or "Velocity Engine" for  */
/* enhanced media capabilities), and G5 referred to the     */
/* 7500/8500 series CPUs, and were a new 64-bit             */
/* architecture, as well as were the first PowerPC          */
/* processors to feature Dual-core capabilities without     */
/* requiring 2 separate CPUs. Additionally, PowerPC would   */
/* see success outside of Apple Computers, such as the G3   */
/* would see use in other platforms, such as the Nintendo   */
/* GameCube, or a G5-based chip being used in the Xbox 360. */
/* However, due to rampant heat issues and unscalability    */
/* of the G5 architecture as a whole, Apple Computers would */
/* choose to ditch the PowerPC architecture in favor of     */
/* Intel's x86 offerings, which would also put it on par    */
/* with other standard home computers at the time, which    */
/* typically ran x86 processors with Windows, often called  */
/* "Wintel" computers. This switch to the x86 platform      */
/* had the unintended consequence of enabling users to run  */
/* Windows natively alongside their Mac OS X installation.  */
/* At first, this was not officially supported by Apple,    */
/* but Apple later chose to add support in Mac OS X 10.5    */
/* and is still present in the operating system, but is     */
/* notably missing from M-series Macintoshes, as the ARM    */
/* architecture is incompatible with Windows, despite an    */
/* official ARM port of Windows existing. However, it       */
/* cannot be natively installed, as it has no drivers nor   */
/* bootloader compatibility for the M-series Macs, as new   */
/* Macintoshes require a more specialized boot process.     */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define STR 1
#define NUM 2
#define BOOL 3
#define IN 4

#define DEBUG_PRINT if (debugMode) printf

int debugMode = 0;

int listCount = 0;
int openFiles = 0;

typedef struct {
    char *name;
    int type;
    char *value; // 10k should be enough for anyone - wait no, its dynamic now, nvm
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
    int instructionCount;
    int variableCount;
    int labelCount;
} openFile;

char *validInstructions[][15] = { // the row indicates how many arguments they should typically have
    {"println", "prints", "printc", "please", "@", "quit", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"jump", "not", "print", "label", "type", "write", "import", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"copy", "conv", "jumpv", "writev", "read", "list", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"eqc", "eqv", "neqc", "neqv", "add", "sub", "mul", "div", "set", "ste", "st", "gte", "gt", "and", "or"}
};

char *listInstructions[][3] = {
    {"new", "show", NULL},
    {"dump", "del", "load"},
    {"appv", "appc", "acc"},
    {"upv", "upc", NULL}
};

openFile *files;

void freeInstruction(instruction *inst) {
    if (inst == NULL) return;
    free(inst->operation);
    for (int i = 0; i < inst->argumentCount; i++) {
        free(inst->arguments[i]);
    }
    free(inst);
}
void cleanUp(instruction **instructionSet, variable *variables, label *labelSet, int numInstructions, int numVariables, int numLabels); // forward decl...

void freeVariable(variable var) { free(var.name); free(var.value); }
void freeLabel(label labia) { free(labia.name); } // haha minge funny ._.
void freeList(list lis) { free(lis.name); for (int i = 0; i < lis.elements; i++) { freeVariable(lis.variables[i]); }}
void freeFile(openFile file) {
    free(file.path);
    cleanUp(file.instructions, file.variables, file.labels, file.instructionCount, file.variableCount, file.labelCount);
}

void cleanUp(instruction **instructionSet, variable *variables, label *labelSet, int numInstructions, int numVariables, int numLabels) { // hey, if it doesnt work, it'll exit out anyways amirite?
    for (int i = 0; i < numInstructions; i++) { freeInstruction(instructionSet[i]); }
    for (int i = 0; i < numVariables; i++) { freeVariable(variables[i]); }
    for (int i = 0; i < numLabels; i++) { freeLabel(labelSet[i]); }

    free(instructionSet); free(variables); free(labelSet);
}

char *stripSemicolon(char input[]) { char *string = strdup(input); int position = strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; return string; }
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
            if (strcmp(stripSemicolon(instruction), listInstructions[k][l]) == 0) { return k + 2; }
        }
    }
    return -1;
}

char *lowerize(const char input[]) { char *string = strdup(input); int len = strlen(string); for (int i = 0; i < len; i++) { string[i] = tolower(string[i]); } return string; }
void toLowerString(char string[]) { int len = strlen(string); for (int i = 0; i < len; i++) { string[i] = tolower(string[i]); }} // mutates the string to save a couple of cycles

void cry(char sob[]) { printf(sob); exit((int)2384708919); }

instruction *add_instruction(char inst[], char *arguments[]) {
    int args = findNumberArgs(inst); // prevents the feeding of garbage arguments by verifying it against the list of instructions and how many args they'll typically have
    if (args == -1) { printf("%s is not an instruction!\n", inst); exit(1); }
    if (strcmp(inst, "printc") == 0) args = 1; // special case 'cause fuck you and fuck my sanity
    if (strcmp(inst, "set") == 0 && strcmp(lowerize(arguments[0]), "in") == 0 || strcmp(inst, "write") == 0) args = 2; // also a special case
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
    new.variables = (variable *)malloc(sizeof(variable)); // create one dummy variable
    new.variables[0] = create_variable("", STR, "");
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
    toLowerString(type);
    if (strcmp(type, "str") == 0) { return STR; }
    else if (strcmp(type, "num") == 0) { return NUM; }
    else if (strcmp(type, "bool") == 0) { return BOOL; }
    else if (strcmp(type, "in") == 0) { return IN; }
    else { return -1; }
}

/* 
void appendElement(list *li, variable var) {
    li->variables = realloc(li->variables, sizeof(variable) * (li->elements + 1));
    if (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
    set_variable_value(&li->variables[li->elements], var.value);
    li->elements++;
}

void removeElement(list *li, int element) {
    freeVariable(li->variables[element]);
    for (int i = element + 1; i < li->elements; i++) { set_variable_value(&li->variables[i - 1], li->variables[i].value); }
    li->variables = realloc(li->variables, sizeof(variable) * (li->elements - 1));
    if (li->variables == NULL) cry("SHIT, A MALLOC FAILED");
    li->elements--;
}

char *formatList(list li) {

}

int findList(char name[]) {
    int location = 0; int found = 0;
    for (int k = 0; k < listCount; k++) { 
        if (strcmp(lists[k].name, name) == 0) { 
            DEBUG_PRINT("found list %s at %d\n", lists[location].name, location);
            found = 1; break; 
        } 
        location++; 
    }
    if (found) { return location; } else { return -1; }
} 
*/

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

void executeFile(openFile current); // forward decl. so it can be used in openFile

openFile openSimasFile(const char path[]) {
    FILE *file = fopen(path, "r");
    if (file == NULL) { cry("failed to find a simas file!\n"); }

    for (int i = 0; i < openFiles; i++) { if (strcmp(path, files[i].path) == 0) cry("Already opened!"); }

    openFile new;
    memset(&new, 0, sizeof(openFile));
    new.path = strdup(path);

    while (1) {
        char buffer[100]; char buffer2[100]; char string[100];
        char *args[10];
        int argc = 0; int expectedArgs = 0;
        fscanf(file, "%s", &buffer); 
        toLowerString(buffer);
        if (feof(file)) break;

        if (strcmp(buffer, "please") == 0) { continue; }

        if (strchr(buffer, '@') != NULL) { 
            if (strchr(buffer, ';') != NULL) { continue; }
            char currentChar = ' '; 
            while (currentChar != ';') { fscanf(file, "%c", &currentChar); } 
            continue; 
        }

        expectedArgs = findNumberArgs(buffer);

        if (expectedArgs >= 1) { fscanf(file, "%s", &buffer2); args[argc] = strdup(buffer2); argc++; DEBUG_PRINT("first arg: %s\n", buffer2); }
        if (strcmp(buffer, "label") == 0) { new.labels = (label *)realloc(new.labels, sizeof(label) * (new.labelCount + 1)); new.labels[new.labelCount] = create_label(buffer2, new.instructionCount - 1); new.labelCount += 1; continue; }

        if ((strcmp(buffer, "printc") == 0 || strcmp(buffer, "write") == 0) || (grabType(buffer2) == STR && (strcmp(buffer, "set") == 0) || (strcmp(buffer, "eqc") == 0) || (strcmp(buffer, "neqc") == 0))) {
            if (strcmp(buffer, "eqc") == 0 || strcmp(buffer, "neqc") == 0 || strcmp(buffer, "set") == 0) { // we're gonna force these ones to add an extra variable 'cause otherwise it breaks
                char variable[100];
                fscanf(file, "%s", &variable);
                args[argc] = strdup(variable); argc++; DEBUG_PRINT("second arg: %s\n", variable); 
                expectedArgs = 3;
            }
            fseek(file, 1, SEEK_CUR); /* skip the space */
            for (int i = 0; i < 99; i++) {
                fscanf(file, "%c", &string[i]);
                if (string[i] == ';') { string[i] = '\0'; break; }
                
                if (string[i] == 'n' && string[i - 1] == '\\') { string[i - 1] = '\n'; i--; continue; }
            }
            DEBUG_PRINT("created string \"%s\"\n", string);
            args[argc] = strdup(string); argc++; 
        }

        if (expectedArgs > argc) {
            for (int i = 0; i < expectedArgs - 1; i++) {
                if (strcmp(buffer2, "in") == 0) i = 1;
                fscanf(file, "%s", &buffer2); // sscanf is not soa:ZKXHdbkALDhbfiolSEDJGKLSDGHKASLFDGHKSLDAFGJHLKS sasketchawan or whatever
                strcpy(buffer2, stripSemicolon(buffer2));
                if (findNumberArgs(buffer2) != -1 || !feof(file) || expectedArgs != -1) {
                    args[argc] = strdup(buffer2); argc++;
                    DEBUG_PRINT("arg: %s\n", buffer2);
                } else {
                    fseek(file, -1 * (strlen(buffer2) + 1), SEEK_CUR); // back the FUCK UP
                    break;
                }
            }
        }

        new.instructions = (instruction **)realloc(new.instructions, sizeof(instruction *) * (new.instructionCount + 1));
        new.instructions[new.instructionCount] = add_instruction(buffer, args);
        for (int i = 0; i < new.instructions[new.instructionCount]->argumentCount; i++) { DEBUG_PRINT("instruction %s has arg %s\n", buffer, args[i]); free(args[i]); }
        new.instructionCount++;
    }

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
    if (type == BOOL) { toLowerString(var->value); }
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
        char current_instruction[100]; strcpy(current_instruction, current.instructions[j]->operation);
        DEBUG_PRINT("\nExecuting instruction %s on line %d.\n", current_instruction, j); 

        if (strcmp(current_instruction, "label") == 0) { continue; }
        else if (strcmp(current_instruction, "import") == 0) { executeFile(openSimasFile(current.instructions[j]->arguments[0])); }

        else if (strcmp(current_instruction, "copy") == 0) {
            variable *var1 = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[0], 0); 
            variable *var2 = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 1); 
            set_variable_value(var2, var1->value);
            var2->type = var1->type;
        } else if (strcmp(current_instruction, "jumpv") == 0) {
            label *jump = findLabel(current.labels, current.labelCount, current.instructions[j]->arguments[0]);
            variable *var = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 0); 
            int allowed = 0;
            if (var->type == NUM) { if (atof(var->value) != 0.0) { allowed = 1; }}
            else if (var->type == BOOL) { if (strcmp(var->value, "true") == 0) { allowed = 1; }}
            else { cry("strings cannot be compared"); }
            if (allowed) { if (debugMode == 2) { printf("**pause on jumpv to label %s, press any key to continue**", jump->name); getc(stdin); } j = jump->location; }
        } else if (strcmp(current_instruction, "jump") == 0) {
            label *jump = findLabel(current.labels, current.labelCount, current.instructions[j]->arguments[0]);
            if (debugMode == 2) { printf("**pause on jump to label %s, press any key to continue**", jump->name); getc(stdin); } 
            j = jump->location; 
        }  
        else if (strcmp(current_instruction, "not") == 0) {
            variable *var = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[0], 0); 
            if (var->type == BOOL) { if (strcmp(var->value, "true") == 0) { strcpy(var->value, "false"); } else { strcpy(var->value, "true"); }} else { cry("NOT must be used on a bool!"); }
        }
        else if (strcmp(current_instruction, "read") == 0) { 
            variable *var = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 1); 
            var->type = STR;
            set_variable_value(var, readFile(current.instructions[j]->arguments[0])); 
        } 
        else if (strcmp(current_instruction, "add") == 0 || strcmp(current_instruction, "sub") == 0 || strcmp(current_instruction, "mul") == 0 || strcmp(current_instruction, "div") == 0) { 
            int operation; float op1, op2;
            if (strcmp(current_instruction, "add") == 0) operation = 1;
            else if (strcmp(current_instruction, "sub") == 0) operation = 2;
            else if (strcmp(current_instruction, "mul") == 0) operation = 3;
            else if (strcmp(current_instruction, "div") == 0) operation = 4;
            variable *var1 = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 1); 
            variable *var2 = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[2], 0);
            if (var1->type != NUM) cry("You can only do math on a 'num' type variable!");
            if (var2 == NULL) { op2 = atof(current.instructions[j]->arguments[2]); }
            else if (var2->type != NUM) cry("You can only do math on a 'num' type variable!");
            else { op2 = atof(var2->value); }
            op1 = atof(var1->value);

            float output = doMath(operation, op1, op2); 
            char tempStr[100];
            if (output - (int)output != 0) { sprintf(tempStr, "%f", output); }
            else { sprintf(tempStr, "%d", (int)output); }
            set_variable_value(var1, tempStr);
        } 

        else if (strcmp(current_instruction, "set") == 0) { 
            int type = grabType(current.instructions[j]->arguments[0]);
            if (type == IN) { setVar(findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 1), type, ""); }
            else { setVar(findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 1), type, current.instructions[j]->arguments[2]); }
        }

        else if (strcmp(current_instruction, "printc") == 0) { printf("%s", current.instructions[j]->arguments[0]); } 
        else if (strcmp(current_instruction, "println") == 0) { printf("\n"); } 
        else if (strcmp(current_instruction, "prints") == 0) { printf(" "); } 
        else if (strcmp(current_instruction, "print") == 0) { printf("%s", findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[0], 0)->value); } 
        else if (strcmp(current_instruction, "quit") == 0) { exit(0); } 
        else if (strcmp(current_instruction, "conv") == 0) { conv(findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[0], 0), grabType(current.instructions[j]->arguments[1])); } 

        else if (strcmp(current_instruction, "st") == 0 || strcmp(current_instruction, "ste") == 0 || strcmp(current_instruction, "gt") == 0 || strcmp(current_instruction, "gte") == 0 ||
                strcmp(current_instruction, "and") == 0 || strcmp(current_instruction, "or") == 0 || strcmp(current_instruction, "eqc") == 0 || strcmp(current_instruction, "eqv") == 0 ||
                strcmp(current_instruction, "neqc") == 0 || strcmp(current_instruction, "neqv") == 0) {

            variable *var1 = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 0);
            variable *var2 = findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[2], 0);
            char *operand1, *operand2; int output;
            if (var1 == NULL) { cry("No variable!"); }
            else { operand1 = strdup(var1->value); }
            if (var2 == NULL) { operand2 = strdup(current.instructions[j]->arguments[2]); }
            else { operand2 = strdup(var2->value); }

            if (strcmp(current_instruction, "st") == 0) { 
                if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
                output = compare('<', operand1, operand2); 
            }
            else if (strcmp(current_instruction, "ste") == 0) {
                if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n"); 
                output = compare('[', operand1, operand2); 
            }
            else if (strcmp(current_instruction, "gt") == 0) { 
                if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
                output = compare('>', operand1, operand2); 
            }
            else if (strcmp(current_instruction, "gte") == 0) { 
                if (var1->type != NUM || (var2 != NULL && var2->type != NUM)) cry("Operand must be of \"num\" type!\n");
                output = compare(']', operand1, operand2); 
            }

            else if (strcmp(current_instruction, "and") == 0) { 
                if (var1->type != BOOL || (var2 != NULL && var2->type != BOOL)) cry("Operand must be of \"bool\" type!\n");
                output = compare('&', operand1, operand2); 
            }
            else if (strcmp(current_instruction, "or") == 0) { 
                if (var1->type != BOOL || (var2 != NULL && var2->type != BOOL)) cry("Operand must be of \"bool\" type!\n");
                output = compare('|', operand1, operand2); 
            }

            else if (strcmp(current_instruction, "eqc") == 0) { 
                free(operand2); operand2 = strdup(current.instructions[j]->arguments[2]);
                output = compare('e', operand1, operand2); 
            }
            else if (strcmp(current_instruction, "eqv") == 0) { output = compare('e', operand1, operand2); }
            else if (strcmp(current_instruction, "neqc") == 0) { 
                free(operand2); operand2 = strdup(current.instructions[j]->arguments[2]);
                output = compare('n', operand1, operand2); 
            }
            else if (strcmp(current_instruction, "neqv") == 0) { output = compare('n', operand1, operand2); }

            var1->type = BOOL;
            if (output) set_variable_value(var1, "true");
            else set_variable_value(var1, "false");

            free(operand1); free(operand2);
        }
        
        else if (strcmp(current_instruction, "write") == 0) { writeFile(current.instructions[j]->arguments[0], current.instructions[j]->arguments[1]); } 
        else if (strcmp(current_instruction, "writev") == 0) { writeFile(current.instructions[j]->arguments[0], findVar(&current.variables, &current.variableCount, current.instructions[j]->arguments[1], 0)->value); } 
        /*         
        else if (strcmp(current_instruction, "list") == 0) {
            char *listInstruction = strdup(instructions[j]->arguments[0]);
            char *listName = strdup(instructions[j]->arguments[1]);
            int location;
            if (listCount) location = findList(listName);
            toLowerString(listInstruction);
            if (strcmp(listInstruction, "new") == 0) {
                lists = (list *)realloc(lists, sizeof(list) * (listCount));
                lists[listCount] = create_list(instructions[j]->arguments[1]);
                listCount++;
            }
            else if (strcmp(listInstruction, "appv") == 0) { appendElement(&lists[location], storedVariables[findVar(instructions[j]->arguments[3])]); }
            else if (strcmp(listInstruction, "appc") == 0) { 
                variable var = { .type = grabType(instructions[j]->arguments[2]), .value = instructions[j]->arguments[3] };
                appendElement(&lists[location], var); 
            }
            free(listInstruction);
            free(listName);
        } 
        */
        else { cry("Invalid instruction!\nUse \"--debug\" to find the issue & report it on the repository here:\nhttps://github.com/tuvalutorture/SIMAS/ \n(i am sorry, but this codebase is held together with duct tape T_T)"); }
    }

    cleanUp(current.instructions, current.variables, current.labels, current.instructionCount, current.variableCount, current.labelCount);
}

int main(int argc, const char * argv[]) {
    if (argc < 2) { printf("usage: simas <file.simas>\n"); return 1; }
    if (argc >= 3) { if (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "--debug") == 0) { debugMode = 1; printf("debug mode enabled\n"); }}
    if (argc == 4) { if (strcmp(argv[3], "-j") == 0 || strcmp(argv[3], "--jmp") == 0) { debugMode = 2; printf("jump debugger enabled\n"); }}
    executeFile(openSimasFile(argv[1]));
    return 0;
} // we are the shinglefuckers of xingbing ltd.