/* CMAS (Compiled SIMAS "SIMple ASsembly") interpreter */
/* written by tuvalutorture                            */
/* a feral child powered purely by rhcp and soda       */
/* please save (or kill) me                            */
/* yes this codebase is abhorrent                      */
/* rome wasn't built in a day, but this was in 4 days  */
/* the gods of olympus have abandonded me              */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE (1024 * 1024)

int i;
int j; // the entire program rests on this one variable. it uses it LIKE A TAPE HEAD. TRACKING BACK AND FORTH BETWEEN input. don't do this. please
char *input;
int isFloatMath = 0;
int vars = 0;

char **storedVariables;
char labels[100][21];

void openStartingFiles(const char path[300]) {
    input = malloc(BUFFER_SIZE + 1);

    storedVariables = malloc(sizeof(char *) * 300);
    
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("failed to find a simas file!\n");
        exit(EIO);
    }
    int labelLoc = 0;
    for (i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%c", &input[i]);
        if (input[i - 1] == 'l' && input[i - 2] == ' ' && input[i - 3] == ' ' && input[i] == ':') { // checks to make sure it's a lone label declaration and not part of a print statement
            i -= 2;
            for (int k = 0; k < 20; k++) {
                fscanf(file, "%c", &labels[labelLoc][k]);
                if (labels[labelLoc][k] == ' ') {
                    labels[labelLoc][k] = '\0';
                    labelLoc++;
                    sprintf(labels[labelLoc], "%d", i);
                    // printf("%s at %s\n", labels[labelLoc - 1], labels[labelLoc]);
                    labelLoc++;
                    k = 20;
                }
            }
            continue;
        }
        if (input[i] == '\0') {
            break;
        }
    }
    // printf("%s\n", input);
    fclose(file);
}

int findVar(char varName[20]) {
    int location = 0;
    int found = 0;
    for (int k = 0; k < vars; k += 3) { // 3 because var struct is "[name] [type] [value]"
        // printf("checking against %s at %d...\n", storedVariables[k], k);
        if (strcmp(storedVariables[k], varName) == 0) {
            found = 1;
            break;
        }
        location += 3;
    }
    if (found) {
        return location;
    } else if (varName[0] == ' ' || varName[0] == '\0') {
        printf("var is blank! did you forget to assign one?\n");
        exit(0);
    } else {
        // printf("could not find var %s!\n", varName);
        return -1;
    }
}

int findLabel(char labelName[20]) {
    int location = 0;
    int found = 0;
    for (int k = 0; k < 100; k += 2) {
        // debug: printf("checking against %s at %d...\n", labels[k], k);
        if (strcmp(labels[k], labelName) == 0) {
            found = 1;
            break;
        }
        location += 2;
    }
    if (found) {
        return location;
    } else {
        return -1;
    }
}

void fillStr(char *buffer, int size) {
    int offset = 0;
    for (int k = 0; k < size - 1; k++) {
        if (input[j + k] == ' ' || input[j + k] == '\0') {
            buffer[k] = '\0';
            j += offset + 1;
            break;
        }
        buffer[k] = input[j + k];
        offset++;
    }
    
    // printf("[fillStr] read: '%s' | j: %d\n", buffer, j);
}

int seekToVar(void) {
    int location;
    int offset = 0;
    char varName[101];
    for (int k = 0; k < 100; k++) { // technically not the max, but 100 char variable names are a crime anyways
        varName[k] = input[j + k];
        if (input[j + k] == ' ') {
            j += offset;
            varName[k] = '\0';
            break;
        }
        offset++;
    }
    location = findVar(varName);
    // printf("[fillStr] read: '%s' | j: %d\n", varName, j);
    return location;
}

int seekToVarAndCreate(char type) { // oh look, the var doesnt exist. create it
    int location;
    int offset = 0;
    char varName[101];
    for (int k = 0; k < 100; k++) { // technically not the max, but 100 char variable names are a crime anyways
        varName[k] = input[j + k];
        if (input[j + k] == ' ') {
            j += offset;
            varName[k] = '\0';
            break;
        }
        offset++;
    }
    location = findVar(varName);
    if (location == -1) {
        storedVariables[vars] = malloc(101); storedVariables[vars + 1] = malloc(3); storedVariables[vars + 2] = malloc(10000); // 100 chars for name, 3 for type (its one char but 3 js in case), and 10k for the value (to hold giant strings)
        strcpy(storedVariables[vars], varName);
        location = vars;
        storedVariables[location + 1][0] = type;
        if (type == 'b') {
            strcpy(storedVariables[location + 2], "false");
        } if (type == 'n') {
            strcpy(storedVariables[location + 2], "0");
        } if (type == 's') {
            strcpy(storedVariables[location + 2], "\n");
        }
        vars += 3;
    }
    // printf("[fillStr] read: '%s' | j: %d\n", varName, j);
    return location;
}

int seekToLabel(void) {
    int jumpLocation;
    int offset = 0;
    char labelName[20];
    for (int k = 0; k < 20; k++) {
        labelName[k] = input[j + k];
        if (input[j + k + 1] == ' ') {
            j += offset + 1;
            labelName[k + 1] = '\0';
            break;
        }
        offset++;
    }
    jumpLocation = findLabel(labelName);
    // printf("[fillStr] read: '%s' | j: %d\n", labelName, j);
    return jumpLocation;
}

void doMath(int operation) { // 1 for addition, 2 for subtraction, 3 for mult, 4 for div
    j += 2; isFloatMath = 1;
    char buf1[101];
    char buf2[101];
    float operand1 = 0;
    float operand2 = 0;
    fillStr(buf1, 100);
    fillStr(buf2, 100);
    int location1 = findVar(buf1);
    int location2 = findVar(buf2);
    if (location1 == -1 && !isdigit(buf1[0])) {
        storedVariables[vars] = malloc(101); storedVariables[vars + 1] = malloc(3); storedVariables[vars + 2] = malloc(10000); // 100 chars for name, 3 for type (its one char but 3 js in case), and 10k for the value (to hold giant strings)
        strcpy(storedVariables[vars], buf1);
        location1 = vars;
        storedVariables[location1 + 1][0] = 'n';
        strcpy(storedVariables[location1 + 2], "0");
        vars += 3;
    }
    else if (location1 == -1) {
        operand1 = atof(buf1);
    } else {
        if (storedVariables[location1 + 1][0] == 'n') {
            operand1 = atof(storedVariables[location1 + 2]);
        } else {
            printf("Operand must be of \"num\" type!\n");
            exit(1);
        }
    }
    if (location2 == -1 && !isdigit(buf2[0])) {
        storedVariables[vars] = malloc(101); storedVariables[vars + 1] = malloc(3); storedVariables[vars + 2] = malloc(10000); // 100 chars for name, 3 for type (its one char but 3 js in case), and 10k for the value (to hold giant strings)
        strcpy(storedVariables[vars], buf2);
        location2 = vars;
        storedVariables[location2 + 1][0] = 'n';
        strcpy(storedVariables[location2 + 2], "0");
        vars += 3;
    }
    else if (location2 == -1) {
        operand2 = atof(buf2);
    } else {
        if (storedVariables[location2 + 1][0] == 'n') {
            operand2 = atof(storedVariables[location2 + 2]);
        } else {
            printf("Operand must be of \"num\" type!\n");
            exit(1);
        }
    }
    if (operation == 4 && operand2 == 0) {
        printf("div by 0 error. eat shit and die, nerd\n");
        exit(425367890);
    }
    float output = 0;
    switch (operation) {
        case 1:
            output = operand1 + operand2;
            break;
        case 2:
            output = operand1 - operand2;
            break;
        case 3:
            output = operand1 * operand2;
            break;
        case 4:
            output = operand1 / operand2;
            break;
        default:
            output = -1;
            break;
    }
    int temp = (int)output;
    if ((float)temp == output) { isFloatMath = 0; }
    if (location1 != -1) {
        if (isFloatMath) {
            sprintf(storedVariables[location1 + 2], "%f", output);
        } else {
            sprintf(storedVariables[location1 + 2], "%d", (int)output);
        }
    } else if (location2 != -1) {
        if (isFloatMath) {
            sprintf(storedVariables[location2 + 2], "%f", output);
        } else {
            sprintf(storedVariables[location2 + 2], "%d", (int)output);
        }
    }
}

void printc(void) {
    j += 2;
    while (1) {
        if (input[j] == '\\') {
            if (input[j + 1] == 'n') {
                printf("\n");
                j += 2;
                continue;
            } else if (input[j + 1] == '\\') {
                printf("\\");
                j += 2;
                continue;
            } else if (input[j + 1] == '0') {
                j += 3;
                break;
            }
            if (input[j + 1] == ' ') { // support "prints". dont worry, normal whitespace is still supported. this is just a hack to support a bastard function that frankly is quite pointless
                if (input[j + 2] == '\\') {
                    printf(" ");
                    j += 3;
                    continue;
                }
            }
        }
        printf("%c", input[j]);
        j++;
    }
}

void print(void) { // printc but instead of user input it prints a var
    j += 2;
    int location = seekToVar();
    if (strcmp(storedVariables[location + 2], "About the Finder...") == 0) {
        printf("The Macintosh(tm) Finder\nBruce Horn and Steve Capps\n128k        Version 5.3   Apple Computer (c) 1986\n");
    } else {
        printf("%s", storedVariables[location + 2]);

    }
}

void writeFile(void) { // "write" function
    j += 2;
    char path[150]; // big ass path just in case
    fillStr(path, 149);
            
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        printf("failed to write to file!\n");
        exit(EIO);
    }
    while (1) {
        if (input[j] == '\\') {
            if (input[j + 1] == 'n') {
                fprintf(file, "\n");
                j += 2;
                continue;
            } else if (input[j + 1] == '\\') {
                fprintf(file, "\\");
                j += 2;
                continue;
            } else if (input[j + 1] == '0') {
                j += 3;
                break;
            }
            if (input[j + 1] == ' ') { // support "prints"
                if (input[j + 2] == '\\') {
                    fprintf(file, " ");
                    j += 3;
                    continue;
                }
            }
        }
        fprintf(file, "%c", input[j]);
        j++;
    }
    fclose(file);
}

void writeVariableFile(void) { // "writev" function
    j += 2;
    char path[150]; // big ass path just in case
    fillStr(path, 149);
            
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        printf("failed to write to file!\n");
        exit(EIO);
    }
    
    int location = seekToVar();
    // debug: printf("%s at %d\n", varName, location);
    fprintf(file, "%s", storedVariables[location + 2]);
    fclose(file);
}

void copy(void) {
    j += 2;
    int location1 = seekToVar();
    j++;
    int location2 = seekToVarAndCreate(storedVariables[location1 + 1][0]);
    strcpy(storedVariables[location2 + 2], storedVariables[location1 + 2]);
}

void conv(void) {
    j += 2;
    int location = seekToVar();
    j++;
    char type = input[j];
    j += 2;
    if (storedVariables[location + 1][0] != type) {
        if (type == 'n' || type == 'b') {
            if (storedVariables[location + 1][0] == 'n') {
                if (atoi(storedVariables[location + 2]) > 0) {
                    strcpy(storedVariables[location + 2], "true");
                } else {
                    strcpy(storedVariables[location + 2], "false");
                }
            } else if (storedVariables[location + 1][0] == 'b') {
                if (strstr(storedVariables[location + 2], "true")) {
                    strcpy(storedVariables[location + 2], "1");
                } else {
                    strcpy(storedVariables[location + 2], "0");
                }
            }
        }
        storedVariables[location + 1][0] = type;
    } else if (storedVariables[location + 1][0] == type){
        // do nothing, as you cannot convert to the same type, fucknuts
    } else {
        printf("invalid variable type.\n");
        exit(1);
    }
}

void readFile(void) {
    j += 2;
    char path[150]; // big ass path just in case
    fillStr(path, 149);
    
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("failed to read file! %s\n", path);
        exit(EIO);
    }
    
    int location = seekToVar();
    memset(storedVariables[location + 2], '\0', 10000);
    for (int k = 0; k < 10000; k++) {
        fscanf(file, "%c", &storedVariables[location + 2][k]);
        if (storedVariables[location + 2][k] == '\0') {
            break;
        }
    }
    fclose(file);
}

void jump(void) {
    j += 2;
    int location = seekToLabel();
    j = atoi(labels[location + 1]);
}

void jumpv(void) {
    j += 2;
    int jumpLocation = seekToLabel();
    j++;
    int location = seekToVar();
    int allowed = 0;
    if (storedVariables[location + 1][0] == 'n') {
        if (atoi(storedVariables[location + 2]) != 0) {
            allowed = 1;
        }
    } else if (storedVariables[location + 1][0] == 'b') {
        if (strstr(storedVariables[location + 2], "true")) {
            allowed = 1;
        }
    } else {
        printf("strings cannot be compared\n");
        exit(1);
    }
    if (allowed) {
        j = atoi(labels[jumpLocation + 1]);
    }
}

void negate(void) {
    j += 2;
    int location = seekToVarAndCreate('b');
    if (storedVariables[location + 1][0] == 'b') {
        if (strstr(storedVariables[location + 2], "true")) {
            strcpy(storedVariables[location + 2], "false");
        } else {
            strcpy(storedVariables[location + 2], "true");
        }
    } else {
        printf("NOT must be used on a bool!\n");
        exit(1);
    }
}

void setVar(void) {
    j += 2;
    char type = input[j];
    j += 2;
    char name[101];
    fillStr(name, 100);
    // printf("name: %s\n", name);
    int location = findVar(name);
    char data[10000];
    if (location == -1) {
        storedVariables[vars] = malloc(101); storedVariables[vars + 1] = malloc(3); storedVariables[vars + 2] = malloc(10000); // 100 chars for name, 3 for type (its one char but 3 js in case), and 10k for the value (to hold giant strings)
        strcpy(storedVariables[vars], name);
        location = vars;
        vars += 3;
    }
    if (type == 'i') { // let the user type whatever bullshit is on their minds
        storedVariables[location + 1][0] = 's';
        fgets(storedVariables[location + 2], 9999, stdin);
        int pos = (int)strcspn(storedVariables[location + 2], "\n"); // compensate for the newline by fucking yeeting it out of existence
        storedVariables[location + 2][pos] = '\0';
    } else if (strcmp(storedVariables[location + 1], "in") != 0) {
        storedVariables[location + 1][0] = type;
        // printf("%s %s %d\n", name, type, vars);
        if (storedVariables[location + 1][0] == 's') {
            for (int k = 0; k < 9999; k++) {
                // printf("%d %d\n", j, k);
                if (input[j] == '\\') {
                    if (input[j + 1] == 'n') {
                        data[k] = '\n';
                        j += 2;
                        continue;
                    } else if (input[j + 1] == '\\') {
                        data[k] = '\\';
                        j += 2;
                        continue;
                    } else if (input[j + 1] == '0') {
                        data[k] = '\0';
                        j += 3;
                        break;
                    }
                    if (input[j + 1] == ' ') { // support "prints". dont worry, normal whitespace is still supported. this is just a hack to support a bastard function that frankly is quite pointless
                        if (input[j + 2] == '\\') {
                            data[k] = ' ';
                            j += 3;
                            continue;
                        }
                    }
                }
                data[k] = input[j];
                // printf("%d %d\n", j, k);
                j++;
            }
        } else fillStr(data, 9999);
        strcpy(storedVariables[location + 2], data);
    }
}

void compare(int operation) { // oh look 10 fucking functions in one. yippers
    j += 2;
    char buf1[101];
    char buf2[10000];
    float operand1 = 0;
    float operand2 = 0;
    fillStr(buf1, 100);
    int location1 = findVar(buf1);
    if ((operation == 'e' || operation == 'n') && storedVariables[location1 + 1][0] == 's') {
        for (int k = 0; k < 9999; k++) {
            if (input[j] == '\\') {
                if (input[j + 1] == 'n') {
                    buf2[k] = '\n';
                    j += 2;
                    continue;
                } else if (input[j + 1] == '\\') {
                    buf2[k] = '\\';
                    j += 2;
                    continue;
                } else if (input[j + 1] == '0') {
                    buf2[k] = '\0';
                    j += 3;
                    break;
                }
                if (input[j + 1] == ' ') {
                    if (input[j + 2] == '\\') {
                        buf2[k] = ' ';
                        j += 3;
                        continue;
                    }
                }
            }
            buf2[k] = input[j];
            j++;
        }
    } else {
        fillStr(buf2, 100);
    }
    int location2 = findVar(buf2);
    
    if (operation == '>' || operation == ']' || operation == '<' || operation == '[') {
        if (location1 == -1) {
            operand1 = atof(buf1);
        } else {
            if (storedVariables[location1 + 1][0] == 'n') {
                operand1 = atof(storedVariables[location1 + 2]);
            } else {
                printf("Operand must be of \"num\" type!\n");
                exit(1);
            }
        }
        
        if (location2 == -1) {
            operand2 = atof(buf2);
        } else {
            if (storedVariables[location2 + 1][0] == 'n') {
                operand2 = atof(storedVariables[location2 + 2]);
            } else {
                printf("Operand must be of \"num\" type!\n");
                exit(1);
            }
        }
    } else if (operation == '&' || operation == '|') {
        if (location1 == -1) {
            if (strstr(buf1, "true")) {
                operand1 = 1;
            }
        } else {
            if (storedVariables[location1 + 1][0] == 'b') {
                if (strstr(storedVariables[location1 + 2], "true")) {
                    operand1 = 1;
                }
            } else {
                printf("Operand must be of \"bool\" type!\n");
                exit(1);
            }
        }
        
        if (location2 == -1) {
            if (strstr(buf2, "true")) {
                operand2 = 1;
            }
        } else {
            if (storedVariables[location2 + 1][0] == 'b') {
                if (strstr(storedVariables[location2 + 2], "true")) {
                    operand2 = 1;
                }
            } else {
                printf("Operand must be of \"bool\" type!\n");
                exit(1);
            }
        }
    }

    int output = 0;
    switch (operation) {
        case 'e':
            if (location1 != -1) {
                if (strcmp(storedVariables[location1 + 2], buf2) == 0) {
                    output = 1;
                }
            } else if (location2 != -1) {
                if (strcmp(storedVariables[location2 + 2], buf1) == 0) {
                    output = 1;
                }
            }
            break;
        case 'E':
            if (strcmp(storedVariables[location1 + 2], storedVariables[location2 + 2]) == 0) {
                output = 1;
            }
            break;
        case 'n':
            if (location1 != -1) {
                if (strcmp(storedVariables[location1 + 2], buf2) != 0) {
                    output = 1;
                }
            } else if (location2 != -1) {
                if (strcmp(storedVariables[location2 + 2], buf1) != 0) {
                    output = 1;
                }
            }
            break;
        case 'N':
            if (strcmp(storedVariables[location1 + 2], storedVariables[location2 + 2]) != 0) {
                output = 1;
            }
            break;
        case '>':
            if (operand1 > operand2) {
                output = 1;
            }
            break;
        case ']':
            if (operand1 >= operand2) {
                output = 1;
            }
            break;
        case '<':
            if (operand1 < operand2) {
                output = 1;
            }
            break;
        case '[':
            if (operand1 <= operand2) {
                output = 1;
            }
            break;
        case '|':
            if (operand1 || operand2) {
                output = 1;
            }
            break;
        case '&':
            if (operand1 && operand2) {
                output = 1;
            }
            break;
        default:
            output = -1;
            break;
    }

    if (location1 != -1) {
        storedVariables[location1 + 1][0] = 'b';
        if (output) {
            strcpy(storedVariables[location1 + 2], "true");
        } else {
            strcpy(storedVariables[location1 + 2], "false");
        }
        // printf("%s, %s, %s\n", storedVariables[location1], storedVariables[location1 + 1], storedVariables[location1 + 2]);
    } else if (location2 != -1) {
        storedVariables[location2 + 1][0] = 'b';
        if (output) {
            strcpy(storedVariables[location2 + 2], "true");
        } else {
            strcpy(storedVariables[location2 + 2], "false");
        }
        // printf("%s, %s, %s\n", storedVariables[location2], storedVariables[location2 + 1], storedVariables[location2 + 2]);
    }
}

void compile(const char path[300]) {
    char **instructions;
    instructions = malloc(sizeof(char *) * 1000);
    
    int instructionCount = 0;
    
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("failed to find a simas file!\n");
        exit(EIO);
    }
    
    for (int k = 0; k < 1000; k++) {
        char currentChar;
        instructions[k] = malloc(10000);
        fscanf(file, "%s", &instructions[k][0]);
        if (instructions[k][0] == '@') {
            while (1) {
                fscanf(file, "%c", &currentChar);
                if (currentChar == ';') {
                    strcpy(instructions[k], "\0");
                    break;
                }
            }
            k--;
            continue;
        }
        strcat(instructions[k], "\0");
        if (strcmp(instructions[k], "PRINTC") == 0 || strcmp(instructions[k], "WRITE") == 0 || (k >= 2 && strcmp(instructions[k - 1], "STR") == 0 && strcmp(instructions[k - 2], "SET") == 0)) {
            k += 1;
            instructions[k] = malloc(10000);
            fseek(file, 1, SEEK_CUR);
            for (int l = 0; l < 9999; l++) {
                fscanf(file, "%c", &instructions[k][l]);
                if (instructions[k][l] == ';' && instructions[k][l - 1] != '\\') {
                    instructions[k][l] = '\\';
                    strcat(instructions[k], "0 ");
                    break;
                } else if (instructions[k][l] == ';' && instructions[k][l - 1] == '\\') {
                    instructions[k][l - 1] = ';';
                    l--;
                    continue;
                }
            }
        }
        if (instructions[k] == NULL) {
            printf("NULL! smth wrong\n");
            exit(99999);
        }
        if (instructions[k][strlen(instructions[k]) - 1] == ';') {
            instructions[k][strlen(instructions[k]) - 1] = ' ';
            instructions[k][strlen(instructions[k])] = '\0';
        }

        if (instructions[k][0] == '\0') {
            fclose(file);
            break;
        }
    }
    
    
    for (int k = 0; k < 9999; k++) {
        char buffer[10000];
        instructionCount++;
        strcpy(buffer, instructions[k]);
        static int dontDie = 0;
        
        for (int l = 0; l < 9999; l++) {
            buffer[l] = tolower(buffer[l]);
            if (buffer[l] == '\0') break;
        }
        
        if ((strcmp(buffer, "add") == 0) || (strcmp(buffer, "sub") == 0) || (strcmp(buffer, "mul") == 0) || (strcmp(buffer, "div") == 0) ||
            (strcmp(buffer, "ste") == 0) || (strcmp(buffer, "st") == 0) || (strcmp(buffer, "gte") == 0) || (strcmp(buffer, "gt") == 0) ||
            (strcmp(buffer, "eqc") == 0) || (strcmp(buffer, "eqv") == 0) || (strcmp(buffer, "neqc") == 0) || (strcmp(buffer, "neqv") == 0) ||
            (strcmp(buffer, "and") == 0) || (strcmp(buffer, "or") == 0)) {
            strcpy(instructions[k + 1], "\0");
            dontDie = 2;
        }
        
        if (strcmp(buffer, "please") == 0) {
            strcpy(instructions[k], "\0");
            dontDie = 2;
        }
        
        if (strcmp(buffer, "num") == 0) {
            strcpy(instructions[k], "n");
        } else if (strcmp(buffer, "bool") == 0) {
            strcpy(instructions[k], "b");
        } else if (strcmp(buffer, "str") == 0) {
            strcpy(instructions[k], "s");
        } else if (strcmp(buffer, "num ") == 0) {
            strcpy(instructions[k], "n ");
        } else if (strcmp(buffer, "bool ") == 0) {
            strcpy(instructions[k], "b ");
        } else if (strcmp(buffer, "str ") == 0) {
            strcpy(instructions[k], "s ");
        } else if (strcmp(buffer, "in") == 0) {
            strcpy(instructions[k], "i");
        } else if (strcmp(buffer, "add") == 0) {
            strcpy(instructions[k], "+");
        } else if (strcmp(buffer, "sub") == 0) {
            strcpy(instructions[k], "-");
        } else if (strcmp(buffer, "mul") == 0) {
            strcpy(instructions[k], "*");
        } else if (strcmp(buffer, "div") == 0) {
            strcpy(instructions[k], "/");
        } else if (strcmp(buffer, "printc") == 0) {
            strcpy(instructions[k], "p");
        } else if (strcmp(buffer, "prints ") == 0) {
            strcpy(instructions[k], "p \\ \\\\0 ");
        } else if (strcmp(buffer, "println ") == 0) {
            strcpy(instructions[k], "p \\n\\0 ");
        } else if (strcmp(buffer, "print") == 0) {
            strcpy(instructions[k], "P");
        } else if (strcmp(buffer, "quit ") == 0) {
            strcpy(instructions[k], "q");
        } else if (strcmp(buffer, "writev") == 0) {
            strcpy(instructions[k], "W");
        } else if (strcmp(buffer, "write") == 0) {
            strcpy(instructions[k], "w");
        } else if (strcmp(buffer, "copy") == 0) {
            strcpy(instructions[k], "c");
        } else if (strcmp(buffer, "conv") == 0) {
            strcpy(instructions[k], "C");
        } else if (strcmp(buffer, "read") == 0) {
            strcpy(instructions[k], "r");
        } else if (strcmp(buffer, "jumpv") == 0) {
            strcpy(instructions[k], "J");
        } else if (strcmp(buffer, "jump") == 0) {
            strcpy(instructions[k], "j");
        } else if (strcmp(buffer, "not") == 0) {
            strcpy(instructions[k], "!");
        } else if (strcmp(buffer, "set") == 0) {
            strcpy(instructions[k], "_");
        } else if (strcmp(buffer, "ste") == 0) {
            strcpy(instructions[k], "[");
        } else if (strcmp(buffer, "st") == 0) {
            strcpy(instructions[k], "<");
        } else if (strcmp(buffer, "gte") == 0) {
            strcpy(instructions[k], "]");
        } else if (strcmp(buffer, "gt") == 0) {
            strcpy(instructions[k], ">");
        } else if (strcmp(buffer, "and") == 0) {
            strcpy(instructions[k], "&");
        } else if (strcmp(buffer, "or") == 0) {
            strcpy(instructions[k], "|");
        } else if (strcmp(buffer, "eqc") == 0) {
            strcpy(instructions[k], "e");
        } else if (strcmp(buffer, "eqv") == 0) {
            strcpy(instructions[k], "E");
        } else if (strcmp(buffer, "neqc") == 0) {
            strcpy(instructions[k], "n");
        } else if (strcmp(buffer, "neqv") == 0) {
            strcpy(instructions[k], "N");
        } else if (strcmp(buffer, "label") == 0) {
            strcpy(instructions[k], "l:");
        }
        // printf("\"%s\"", instructions[k]);
        // printf(" at %d\n", k);
        if (strcmp(instructions[k], "\0") == 0 && dontDie == 0) break;
        if (dontDie) dontDie--;
    }
        
    int len = (int)strlen(path); // we're going to assume you're compiling a .simas, sweethart. if not get fucked
    char newPath[300];
    strcpy(newPath, path);
    newPath[len - 5] = 'c';
    newPath[len - 4] = 's';
    newPath[len - 3] = 'a';
    newPath[len - 2] = '\0';
    // printf("%s\n", newPath);
    
    FILE *writeFile = fopen(newPath, "w");
    if (writeFile == NULL) {
        printf("failed to write file!\n");
        exit(EIO);
    }
    
    for (int k = 0; k < instructionCount; k++) {
        if (instructions[k][0] != '\0') {
            if (strcmp(instructions[k], "l:") == 0) {
                fprintf(writeFile, "%s", instructions[k]);
                printf("%s", instructions[k]);
            } else {
                fprintf(writeFile, "%s ", instructions[k]);
                printf("%s ", instructions[k]);
            }
        }
    }
    
    printf("\n\nWrote file to %s.\nCompiled source is also shown above.\n", newPath);
    fclose(writeFile);
    exit(0);
}

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        printf("usage: simas <C|E> <file.simas|file.csa>\n");
        return 1;
    }
    if (strcmp(argv[1], "E") == 0) {
        openStartingFiles(argv[2]);
        for (j = 0; j < i; j++) {
            char current_char = input[j];
            if (current_char == '+') { // add
                doMath(1);
            } else if (current_char == '-') { // sub
                doMath(2);
            } else if (current_char == '*') { // mul
                doMath(3);
            } else if (current_char == '/') { // div
                doMath(4);
            } else if (current_char == 'p') { // printc. also acts as prints/println
                printc();
            } else if (current_char == 'P') { // print
                print();
            } else if (current_char == 'q') { // quit
                exit(0);
            } else if (current_char == 'w') { // write
                writeFile();
            } else if (current_char == 'W') { // writev
                writeVariableFile();
            } else if (current_char == 'c') { // copy
                copy();
            } else if (current_char == 'C') { // conv
                conv();
            } else if (current_char == 'r') { // read
                readFile();
            } else if (current_char == 'j') { // jump
                jump();
            } else if (current_char == 'J') { // jumpv
                jumpv();
            } else if (current_char == '!') { // not
                negate();
            } else if (current_char == '_') { // set
                setVar();
            } else if (current_char == '<') { // st
                compare('<');
            } else if (current_char == '[') { // ste
                compare('[');
            } else if (current_char == '>') { // gt
                compare('>');
            } else if (current_char == ']') { // gte
                compare(']');
            } else if (current_char == '&') { // and
                compare('&');
            } else if (current_char == '|') { // or
                compare('|');
            } else if (current_char == 'e') { // eqc
                compare('e');
            } else if (current_char == 'E') { // eqv
                compare('E');
            } else if (current_char == 'n') { // neqc
                compare('n');
            } else if (current_char == 'N') { // neqv
                compare('N');
            }
        }
        printf("\nThis message SHOULD NOT PRINT. If it IS, please remember to TERMINATE the program by calling instruction \"QUIT (q)\". \n\n");
        return 1234567890;
    } else if (strcmp(argv[1], "C") == 0) {
        compile(argv[2]);
    }
}
