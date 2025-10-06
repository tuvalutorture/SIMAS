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

int vars = 0; int instructionCount = 0; int labelCount = 0;

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

char *validInstructions[][15] = {
    {"println", "prints", "printc", "please", "@", "quit", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"jump", "not", "print", "label", "type", "write", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"copy", "conv", "jumpv", "writev", "read", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {"eqc", "eqv", "neqc", "neqv", "add", "sub", "mul", "div", "set", "ste", "st", "gte", "gt", "and", "or"}
};

variable *storedVariables; instruction **instructions; label *labels;

void freeInstruction(instruction *inst) {
    if (inst == NULL) return;
    free(inst->operation);
    for (int i = 0; i < inst->argumentCount; i++) {
        free(inst->arguments[i]);
    }
    free(inst);
}

void freeVariable(variable var) { free(var.name); free(var.value); }
void freeLabel(label labia) { free(labia.name); } // haha minge funny ._.

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

char *lowerize(const char input[]) { char *string = strdup(input); int len = strlen(string); for (int i = 0; i < len; i++) { string[i] = tolower(string[i]); } return string; }
void toLowerString(char string[]) { int len = strlen(string); for (int i = 0; i < len; i++) { string[i] = tolower(string[i]); }} // mutates the string to save a couple of cycles

void cleanUp() { // hey, if it doesnt work, it'll exit out anyways amirite?
    for (int i = 0; i < instructionCount; i++) { freeInstruction(instructions[i]); }
    for (int i = 0; i < vars; i++) { freeVariable(storedVariables[i]); }
    for (int i = 0; i < labelCount; i++) { freeLabel(labels[i]); }

    free(instructions); free(storedVariables); free(labels);
}

void cry(char sob[]) { 
    perror(sob); 
    cleanUp();
    exit(2384708919); 
}

instruction *add_instruction(char inst[], char *arguments[]) {
    int args = findNumberArgs(inst);
    if (args == -1) { printf("%s is not an instruction!\n", inst); exit(1); }
    if (strcmp(inst, "printc") == 0) args = 1; // special case 'cause fuck you and fuck my sanity
    if (strcmp(inst, "set") == 0 && strcmp(arguments[0], "in") == 0 || strcmp(inst, "write") == 0) args = 2; // also a special case
    instruction *instruct = (instruction *)malloc(sizeof(instruction) + sizeof(char*) * args);
    instruct->operation = strdup(stripSemicolon(inst)); instruct->argumentCount = args;
    for (int i = 0; i < args; i++) { instruct->arguments[i] = strdup(stripSemicolon(arguments[i])); }
    DEBUG_PRINT("added instruction %s of arg count %d\n", instruct->operation, instruct->argumentCount);
    return instruct;
}

variable create_variable(char name[], int type, char value[]) {
    variable var = { .name = strdup(name), .type = type, .value = strdup(value), .valueLength = strlen(value) + 1 };
    DEBUG_PRINT("created variable %s of type %d with value %s\n", var.name, var.type, var.value);
    return var;
}

label create_label(char name[], int location) {
    label label = { .name = strdup(stripSemicolon(name)), .location = location };
    DEBUG_PRINT("\ncreated label %s on line %d\n", name, location);
    return label;
}

void set_variable_value(variable *var, char value[]) {
    var->value = (char *)realloc(var->value, strlen(value) + 1);
    if (var->value == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    var->valueLength = strlen(value) + 1;
    strcpy(var->value, value);
    DEBUG_PRINT("\nvariable %s now has value %s\n", var->name, var->value);
}

int grabType(char type[]) {
    strcpy(type, lowerize(type));
    if (strcmp(type, "str") == 0) { return STR; }
    else if (strcmp(type, "num") == 0) { return NUM; }
    else if (strcmp(type, "bool") == 0) { return BOOL; }
    else if (strcmp(type, "in") == 0) { return IN; }
    else { return -1; }
}

int findVar(int instruction, int argument) {
    int location = 0; int found = 0;
    for (int k = 0; k < vars; k++) {
        if (strcmp(storedVariables[k].name, instructions[instruction]->arguments[argument]) == 0) { 
            DEBUG_PRINT("found variable %s at %d\n", storedVariables[k].name, k);
            found = 1; break; 
        }
        location++;
    }
    if (found) { return location; } else { return -1; }
}

int findLabel(int instruction, int argument) {
    int location = 0; int found = 0;
    for (int k = 0; k < 50; k++) { 
        if (strcmp(labels[k].name, instructions[instruction]->arguments[argument]) == 0) { 
            DEBUG_PRINT("found label %s at %d\n", labels[location].name, location);
            found = 1; break; 
        } 
        location++; 
    }
    if (found) { return location; } else { return -1; }
}

void openStartingFiles(const char path[]) {
    FILE *file = fopen(path, "r");
    if (file == NULL) { cry("failed to find a simas file!\n"); }
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
        if (strcmp(buffer, "label") == 0) { labels[labelCount] = create_label(buffer2, instructionCount - 1); labelCount += 1; continue; }
        if ((strcmp(buffer, "printc") == 0 || strcmp(buffer, "write") == 0) || (grabType(buffer2) == STR && (strcmp(buffer, "set") == 0) || (strcmp(buffer, "eqc") == 0) || (strcmp(buffer, "neqc") == 0))) {
            if (strcmp(buffer, "eqc") == 0 || strcmp(buffer, "neqc") == 0 || strcmp(buffer, "set") == 0) { // we're gonna force these ones to add an extra variable 'cause otherwise it breaks
                char variable[100];
                fscanf(file, "%s", &variable);
                args[argc] = strdup(variable); argc++; DEBUG_PRINT("second arg: %s\n", variable); 
                expectedArgs = 3;
            }
            fseek(file, 1, SEEK_CUR); /* skip the space */
            for (int i = 0; i < 999; i++) {
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

        instructions[instructionCount] = add_instruction(buffer, args);

        for (int i = 0; i < instructions[instructionCount]->argumentCount; i++) {
            DEBUG_PRINT("instruction %s has arg %s\n", instructions[instructionCount]->operation, instructions[instructionCount]->arguments[i]);
            free(args[i]);
        }

        instructionCount++;
    }
}

void doMath(int operation, int instruction) { // 1 for addition, 2 for subtraction, 3 for mult, 4 for div
    int isFloatMath = 1;
    float operand1 = 0; float operand2 = 0;
    int location1 = findVar(instruction, 1); 
    int location2 = findVar(instruction, 2);
    
    if (location1 == -1) { operand1 = atof(instructions[instruction]->arguments[1]); } 
    else {
        if (storedVariables[location1].type == NUM) { operand1 = atof(storedVariables[location1].value); }
        else { cry("Operand must be of \"num\" type!\n"); }
    }
    if (location2 == -1) { operand2 = atof(instructions[instruction]->arguments[2]); } 
    else {
        if (storedVariables[location2].type == NUM) { operand2 = atof(storedVariables[location2].value); }
        else { cry("Operand must be of \"num\" type!\n"); }
    }
    if (operation == 4 && operand2 == 0) { cry("div by 0 error. eat shit and die, nerd\n"); }

    float output;
    switch (operation) {
        case 1: output = operand1 + operand2; break;
        case 2: output = operand1 - operand2; break;
        case 3: output = operand1 * operand2; break;
        case 4: output = operand1 / operand2; break;
        default: output = -1; break;
    }
    int temp = (int)output;
    if ((float)temp == output) { isFloatMath = 0; } // very cursed float check.
    char tempStr[100];
    
    if (isFloatMath) { sprintf(tempStr, "%f", output); }
    else { sprintf(tempStr, "%d", (int)output); }
    
    set_variable_value(((location1 != -1) ? &storedVariables[location1] : &storedVariables[location2]), tempStr);
}

void conv(int instruction) {
    int location = findVar(instruction, 0);
    int type = grabType(instructions[instruction]->arguments[1]);
    if (storedVariables[location].type != type) {
        if (type == NUM || type == BOOL) {
            if (storedVariables[location + 1].type == NUM) {
                if (atof(storedVariables[location].value) > 0) {
                    strcpy(storedVariables[location].value, "true");
                } else {
                    strcpy(storedVariables[location].value, "false");
                }
            } else if (storedVariables[location + 1].type == BOOL) {
                if (strstr(storedVariables[location].value, "true")) {
                    strcpy(storedVariables[location].value, "1");
                } else {
                    strcpy(storedVariables[location].value, "0");
                }
            }
        }
        storedVariables[location].type = type;
    } 
    else if (storedVariables[location].type == type) { /* do nothing, as you cannot convert to the same type, fucknuts */ } 
    else { cry("invalid variable type.\n"); }
}


void readFile(int instruction) {
    FILE *file = fopen(instructions[instruction]->arguments[0], "rb");
    if (file == NULL) {cry("unable to open file.");}
    int location = findVar(instruction, 1);
    char *temp; long length;
    fseek(file, 0, SEEK_END); length = ftell(file);
    fseek(file, 0, SEEK_SET); temp = malloc(length);
    fread(temp, 1, length, file);
    storedVariables[location].type = STR;
    storedVariables[location].valueLength = strlen(temp) + 1;
    storedVariables[location].value = (char *)malloc(storedVariables[location].valueLength);
    strcpy(storedVariables[location].value, temp);
    fclose(file);
}

void setVar(int instruction) {
    int type = grabType(instructions[instruction]->arguments[0]);
    int location = findVar(instruction, 1);
    if (location == -1) {
        storedVariables[vars] = create_variable(instructions[instruction]->arguments[1], 0, "");
        location = vars;
        vars++;
    }
    if (type == IN) { // let the user type whatever bullshit is on their minds
        storedVariables[location].type = STR;
        char temp[100];
        fgets(temp, 99, stdin);
        temp[strcspn(temp, "\n")] = '\0'; // compensate for the newline by fucking yeeting it out of existence
        set_variable_value(&storedVariables[location], temp);
    } else { storedVariables[location].type = type; set_variable_value(&storedVariables[location], instructions[instruction]->arguments[2]); }
    if (type == BOOL) { toLowerString(storedVariables[location].value); }
}

void compare(int operation, int instruction) { // oh look 10 fucking functions in one. yippers
    float operand1 = 0; float operand2 = 0;
    int location1 = findVar(instruction, 1); int location2 = findVar(instruction, 2);
    
    if (operation == '>' || operation == ']' || operation == '<' || operation == '[') {
        if (location1 == -1) { operand1 = atof(instructions[instruction]->arguments[1]); }
        else {
            if (storedVariables[location1].type == NUM) { operand1 = atof(storedVariables[location1].value); }
            else { cry("Operand must be of \"num\" type!\n"); }
        }
        
        if (location2 == -1) { operand2 = atof(instructions[instruction]->arguments[2]); }
        else {
            if (storedVariables[location2].type == NUM) { operand2 = atof(storedVariables[location2].value); }
            else { cry("Operand must be of \"num\" type!\n"); }
        }
    } else if (operation == '&' || operation == '|') {
        if (location1 == -1) {
            if (strcmp(instructions[instruction]->arguments[1], "true") == 0) { operand1 = 1; }
        } else {
            if (storedVariables[location1].type == BOOL) {
                if (strstr(storedVariables[location1].value, "true")) { operand1 = 1; }
            } else { cry("Operand must be of \"bool\" type!\n"); }
        }
        
        if (location2 == -1) {
            if (strcmp(instructions[instruction]->arguments[2], "true") == 0) { operand2 = 1; }
        } else {
            if (storedVariables[location2].type == BOOL) {
                if (strstr(storedVariables[location2].value, "true")) { operand2 = 1; }
            } else { cry("Operand must be of \"bool\" type!\n"); }
        }
    }

    int output = 0;
    switch (operation) {
        case 'e':
            if (location1 != -1) { if (strcmp(storedVariables[location1].value, instructions[instruction]->arguments[2]) == 0) { output = 1; }}
            else if (location2 != -1) {if (strcmp(storedVariables[location2].value, instructions[instruction]->arguments[1]) == 0) { output = 1; }}
            break;
        case 'E':
            if (strcmp(storedVariables[location1].value, storedVariables[location2].value) == 0) { output = 1; } break;
        case 'n':
            if (location1 != -1) { if (strcmp(storedVariables[location1].value, instructions[instruction]->arguments[2]) != 0) { output = 1; }}
            else if (location2 != -1) { if (strcmp(storedVariables[location2].value, instructions[instruction]->arguments[1]) != 0) { output = 1; }}
            break;
        case 'N': if (strcmp(storedVariables[location1].value, storedVariables[location2].value) != 0) { output = 1; } break;
        case '>': if (operand1 > operand2) { output = 1; } break;
        case ']': if (operand1 >= operand2) { output = 1; } break;
        case '<': if (operand1 < operand2) { output = 1; } break;
        case '[': if (operand1 <= operand2) { output = 1; } break;
        case '|': if (operand1 || operand2) { output = 1; } break;
        case '&': if (operand1 && operand2) { output = 1; } break;
        default: output = -1; break;
    }

    if (location1 != -1) {
        storedVariables[location1].type = BOOL;
        if (output) strcpy(storedVariables[location1].value, "true");
        else strcpy(storedVariables[location1].value, "false");
    } else if (location2 != -1) {
        storedVariables[location2].type = BOOL;
        if (output) strcpy(storedVariables[location2].value, "true");
        else strcpy(storedVariables[location2].value, "false");
    }
}

int main(int argc, const char * argv[]) {
    if (argc < 2) { printf("usage: simas <file.simas>\n"); return 1; }
    if (argc >= 3) { if (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "--debug") == 0) { debugMode = 1; printf("debug mode enabled\n"); }}
    if (argc == 4) { if (strcmp(argv[3], "-j") == 0 || strcmp(argv[3], "--jmp") == 0) { debugMode = 2; printf("jump debugger enabled\n"); }}
    instructions = (instruction **)malloc(sizeof(instruction *) * 300);
    storedVariables = (variable *)malloc(sizeof(variable) * 150);
    labels = (label *)malloc(sizeof(label) * 50);
    if (instructions == NULL || storedVariables == NULL || labels == NULL) cry("hey i think a malloc or 2 failed. js sayin ykyk");
    openStartingFiles(argv[1]);
    for (int j = 0; j < instructionCount; j++) {
        char current_instruction[100]; strcpy(current_instruction, instructions[j]->operation);
        if (debugMode) {
            printf("\nExecuting instruction %s on line %d with args: ", current_instruction, j);
            for (int i = 0; i < instructions[j]->argumentCount; i++) { printf("%s ", instructions[j]->arguments[i]); }
            printf(".");
        }
        if (strcmp(current_instruction, "label") == 0) { continue; }

        else if (strcmp(current_instruction, "write") == 0) {
            FILE *file = fopen(instructions[j]->arguments[0], "w");
            if (file == NULL) { cry("failed to write to file!"); }
            fprintf(file, "%s", instructions[j]->arguments[1]);
            fclose(file);
        } else if (strcmp(current_instruction, "writev") == 0) {
            FILE *file = fopen(instructions[j]->arguments[0], "w");
            if (file == NULL) { cry("failed to write to file!"); }
            int location = findVar(j, 1);
            fprintf(file, "%s", storedVariables[location].value);
            fclose(file);
        } else if (strcmp(current_instruction, "copy") == 0) {
            int location1 = findVar(j, 0); int location2 = findVar(j, 1); 
            if (location2 == -1) { storedVariables[vars] = create_variable(instructions[j]->arguments[1], 0, ""); location2 = vars; vars++; }
            set_variable_value(&storedVariables[location2], storedVariables[location1].value);
            storedVariables[location2].type = storedVariables[location1].type;
        } else if (strcmp(current_instruction, "jumpv") == 0) {
            int jumpLocation = findLabel(j, 0); int location = findVar(j, 1); int allowed = 0;
            if (storedVariables[location].type == NUM) { if (atof(storedVariables[location].value) != 0.0) { allowed = 1; }}
            else if (storedVariables[location].type == BOOL) { if (strcmp(storedVariables[location].value, "true") == 0) { allowed = 1; }}
            else { cry("strings cannot be compared"); }
            if (allowed) { if (debugMode == 2) { printf("**pause on jumpv to label %s, press any key to continue**", labels[jumpLocation].name); getc(stdin); } j = labels[jumpLocation].location; }
        } else if (strcmp(current_instruction, "jump") == 0) {
            int location = findLabel(j, 0); 
            if (debugMode == 2) { printf("**pause on jump to label %s, press any key to continue**", labels[location].name); getc(stdin); } 
            j = labels[location].location; 
        }  
        else if (strcmp(current_instruction, "not") == 0) {
            int location = findVar(j, 0);
            if (storedVariables[location].type == BOOL) {
                if (strcmp(storedVariables[location].value, "true") == 0) { strcpy(storedVariables[location].value, "false"); }
                else { strcpy(storedVariables[location].value, "true"); }
            } else { cry("NOT must be used on a bool!"); }
        }
        else if (strcmp(current_instruction, "add") == 0) { doMath(1, j); } 
        else if (strcmp(current_instruction, "sub") == 0) { doMath(2, j); } 
        else if (strcmp(current_instruction, "mul") == 0) { doMath(3, j); } 
        else if (strcmp(current_instruction, "div") == 0) { doMath(4, j); } 
        else if (strcmp(current_instruction, "printc") == 0) { printf("%s", instructions[j]->arguments[0]); } 
        else if (strcmp(current_instruction, "println") == 0) { printf("\n"); } 
        else if (strcmp(current_instruction, "prints") == 0) { printf(" "); } 
        else if (strcmp(current_instruction, "print") == 0) { printf("%s", storedVariables[findVar(j, 0)].value); } 
        else if (strcmp(current_instruction, "quit") == 0) { exit(0); } 
        else if (strcmp(current_instruction, "set") == 0) { setVar(j); }
        else if (strcmp(current_instruction, "conv") == 0) { conv(j); } 
        else if (strcmp(current_instruction, "st") == 0) { compare('<', j); }
        else if (strcmp(current_instruction, "ste") == 0) { compare('[', j); }
        else if (strcmp(current_instruction, "gt") == 0) { compare('>', j); }
        else if (strcmp(current_instruction, "gte") == 0) { compare(']', j); }
        else if (strcmp(current_instruction, "and") == 0) { compare('&', j); }
        else if (strcmp(current_instruction, "or") == 0) { compare('|', j); }
        else if (strcmp(current_instruction, "eqc") == 0) { compare('e', j); }
        else if (strcmp(current_instruction, "eqv") == 0) { compare('E', j); }
        else if (strcmp(current_instruction, "neqc") == 0) { compare('n', j); }
        else if (strcmp(current_instruction, "neqv") == 0) { compare('N', j); }
        else if (strcmp(current_instruction, "read") == 0) { readFile(j); } 
        else { cry("Invalid instruction!\nUse \"--debug\" to find the issue & report it on the repository here:\nhttps://github.com/tuvalutorture/SIMAS/ \n(i am sorry, but this codebase is held together with duct tape T_T)"); }
    }

    cleanUp();

    return 0;
} // we are the shinglefuckers of xingbing ltd.