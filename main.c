/* CMAS (C SIMAS "SIMple ASsembly") interpreter        */
/* written by tuvalutorture                            */
/* a feral child powered purely by rhcp and soda       */
/* please kill me                                      */
/* yes this codebase is abhorrent                      */
/* rome wasn't built in a day, but this was built in 5 */
/* the gods of olympus have abandonded me              */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int j; // the entire program rests on this one variable. it uses it like a tape head. don't do this. please
int isFloatMath = 0;
int vars = 0;
int instructionCount = 0;

char **storedVariables; char **instructions;
char labels[100][21];

void openStartingFiles(const char path[300]) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("failed to find a simas file!\n");
        exit(23470913);
    }
    
    for (int k = 0; k < 1000; k++) {
        instructions[k] = malloc(10000);
        char currentChar;
        fscanf(file, "%s", instructions[k]);
        char buffer[10000];
        strcpy(buffer, instructions[k]);
        for (int l = 0; l < 9999; l++) {
            buffer[l] = tolower(buffer[l]);
            if (buffer[l] == '\0') break;
        }
        
        if (strcmp(buffer, "please") == 0) {
            k--;
            continue;
        }
        
        if (instructions[k][0] == '@') {
            while (1) {
                fscanf(file, "%c", &currentChar);
                if (currentChar == ';') { strcpy(instructions[k], "\0"); break; }
            }
            k--; continue;
        }
        char buffer1[10000]; char buffer2[10000]; char buffer3[10000]; strcpy(buffer1, instructions[k]);
        if (k >= 2) { strcpy(buffer2, instructions[k - 1]); strcpy(buffer3, instructions[k - 2]); }
        for (int l = 0; l < 9999; l++) { buffer1[l] = tolower(buffer1[l]); buffer2[l] = tolower(buffer2[l]); buffer3[l] = tolower(buffer3[l]); }
        strcat(instructions[k], "\0");
        if (strcmp(buffer1, "printc") == 0 || strcmp(buffer1, "write") == 0 ||
            (k >= 2 && strcmp(buffer2, "str") == 0 && strcmp(buffer3, "set") == 0) ||
            (k >= 2 && strcmp(buffer2, "str") == 0 && strcmp(buffer3, "eqc") == 0) ||
            (k >= 2 && strcmp(buffer2, "str") == 0 && strcmp(buffer3, "neqc") == 0)) {
            k += 1;
            instructions[k] = malloc(10000);
            fseek(file, 1, SEEK_CUR);
            for (int l = 0; l < 9999; l++) {
                fscanf(file, "%c", &instructions[k][l]);
                if (instructions[k][l] == ';' && instructions[k][l - 1] != '\\') {
                    instructions[k][l] = '\0'; break;
                } else if (instructions[k][l] == ';' && instructions[k][l - 1] == '\\') {
                    instructions[k][l - 1] = ';'; l--; continue;
                }
                
                if (instructions[k][l] == 'n' && instructions[k][l - 1] == '\\') {
                    instructions[k][l - 1] = '\n'; l--; continue;
                }
            }
        }
        if (instructions[k] == NULL) {
            printf("NULL! smth wrong\n");
            exit(99999);
        }
        if (instructions[k][strlen(instructions[k]) - 1] == ';') { instructions[k][strlen(instructions[k]) - 1] = '\0'; }
        if (instructions[k][0] == '\0') { fclose(file); break; }
    }
    
    for (int k = 0; k < 9999; k++) {
        char buffer[10000];
        instructionCount++;
        strcpy(buffer, instructions[k]);
        
        for (int l = 0; l < 9999; l++) {
            buffer[l] = tolower(buffer[l]);
            if (buffer[l] == '\0') break;
        }
        
        if (strcmp(buffer, "num") == 0 || strcmp(buffer, "bool") == 0 || strcmp(buffer, "str") == 0 ||
            strcmp(buffer, "in") == 0 || strcmp(buffer, "add") == 0 || strcmp(buffer, "sub") == 0 ||
            strcmp(buffer, "mul") == 0 || strcmp(buffer, "div") == 0 || strcmp(buffer, "printc") == 0 ||
            strcmp(buffer, "prints") == 0 || strcmp(buffer, "println") == 0 || strcmp(buffer, "print") == 0 ||
            strcmp(buffer, "quit") == 0 || strcmp(buffer, "writev") == 0 || strcmp(buffer, "write") == 0 ||
            strcmp(buffer, "copy") == 0 || strcmp(buffer, "conv") == 0 || strcmp(buffer, "read") == 0 ||
            strcmp(buffer, "jumpv") == 0 || strcmp(buffer, "jump") == 0 || strcmp(buffer, "not") == 0 ||
            strcmp(buffer, "set") == 0 || strcmp(buffer, "ste") == 0 || strcmp(buffer, "st") == 0 ||
            strcmp(buffer, "gte") == 0 || strcmp(buffer, "gt") == 0 || strcmp(buffer, "and") == 0 ||
            strcmp(buffer, "or") == 0 || strcmp(buffer, "eqc") == 0 || strcmp(buffer, "eqv") == 0 ||
            strcmp(buffer, "neqc") == 0 || strcmp(buffer, "neqv") == 0 || strcmp(buffer, "label") == 0) {
            strcpy(instructions[k], buffer);
        }
        
        if (strcmp(instructions[k], "\0") == 0) {
            free(instructions[k]);
            break;
        }
        // printf("\"%s\" at %d\n", instructions[k], k);
    }
    
    int labelCount = 0;
    
    for (int k = 0; k < instructionCount; k++) {
        char *buffer = instructions[k];
        if (strcmp(buffer, "label") == 0) {
            strcpy(labels[labelCount], instructions[k + 1]);
            sprintf(labels[labelCount + 1], "%d", k + 2);
            labelCount += 2;
        }
    }
}

int findVar(void) {
    int location = 0; int found = 0;
    for (int k = 0; k < vars; k += 3) { // 3 because var struct is "[name] [type] [value]"
        if (strcmp(storedVariables[k], instructions[j]) == 0) { found = 1; break; }
        location += 3;
    }
    j++;
    if (found) { return location; }
    else { return -1; }
}

int findLabel(void) {
    int location = 0; int found = 0;
    for (int k = 0; k < 100; k += 2) { if (strcmp(labels[k], instructions[j]) == 0) { found = 1; break; } location += 2; }
    j++;
    if (found) { return location; }
    else { return -1; }
}

void doMath(int operation) { // 1 for addition, 2 for subtraction, 3 for mult, 4 for div
    isFloatMath = 1;
    float operand1 = 0; float operand2 = 0;
    j++;
    int location1 = findVar(); int location2 = findVar();
    
    if (location1 == -1) {
        operand1 = atof(instructions[j - 2]);
    } else {
        if (strcmp(storedVariables[location1 + 1], "num") == 0) { operand1 = atof(storedVariables[location1 + 2]); }
        else { printf("Operand must be of \"num\" type!\n"); exit(1); }
    }
    if (location2 == -1) {
        operand2 = atof(instructions[j - 1]);
    } else {
        if (strcmp(storedVariables[location2 + 1], "num") == 0) { operand2 = atof(storedVariables[location2 + 2]); }
        else { printf("Operand must be of \"num\" type!\n"); exit(1); }
    }
    if (operation == 4 && operand2 == 0) {
        printf("div by 0 error. eat shit and die, nerd\n");
        exit(425367890);
    }
    float output = 0;
    switch (operation) {
        case 1: output = operand1 + operand2; break;
        case 2: output = operand1 - operand2; break;
        case 3: output = operand1 * operand2; break;
        case 4: output = operand1 / operand2; break;
        default: output = -1; break;
    }
    int temp = (int)output;
    if ((float)temp == output) { isFloatMath = 0; }
    if (location1 != -1) {
        if (isFloatMath) { sprintf(storedVariables[location1 + 2], "%f", output); }
        else { sprintf(storedVariables[location1 + 2], "%d", (int)output); }
    } else if (location2 != -1) {
        if (isFloatMath) { sprintf(storedVariables[location2 + 2], "%f", output); }
        else { sprintf(storedVariables[location2 + 2], "%d", (int)output); }
    }
    j--;
}

void conv(void) {
    int location = findVar();
    char *type = instructions[j];
    if (strcmp(storedVariables[location + 1], type) != 0) {
        if (strcmp(type, "num") == 0 || strcmp(type, "num") == 0) {
            if (strcmp(storedVariables[location + 1], "num") == 0) {
                if (atoi(storedVariables[location + 2]) > 0) {
                    strcpy(storedVariables[location + 2], "true");
                } else {
                    strcpy(storedVariables[location + 2], "false");
                }
            } else if (strcmp(storedVariables[location + 1], "bool") == 0) {
                if (strstr(storedVariables[location + 2], "true")) {
                    strcpy(storedVariables[location + 2], "1");
                } else {
                    strcpy(storedVariables[location + 2], "0");
                }
            }
        }
        strcpy(storedVariables[location + 1], type);
    } else if (strcmp(storedVariables[location + 1], type) == 0){
        // do nothing, as you cannot convert to the same type, fucknuts
    } else {
        printf("invalid variable type.\n");
        exit(1);
    }
}

void readFile(void) {
    FILE *file = fopen(instructions[j], "r");
    if (file == NULL) {
        printf("failed to read file! %s at %d\n", instructions[j], j);
        exit(23470913);
    }
    j++;
    int location = findVar();
    memset(storedVariables[location + 2], '\0', 9999);
    for (int k = 0; k < 10000; k++) {
        fscanf(file, "%c", &storedVariables[location + 2][k]);
        if (storedVariables[location + 2][k] == '\0') { break; }
    }
    fclose(file);
    j--;
}

void setVar(void) {
    char *type = instructions[j];
    j++;
    int location = findVar();
    if (location == -1) {
        storedVariables[vars] = malloc(101); storedVariables[vars + 1] = malloc(3); storedVariables[vars + 2] = malloc(10000); // 100 chars for name, 3 for type (its one char but 3 js in case), and 10k for the value (to hold giant strings)
        strcpy(storedVariables[vars], instructions[j - 1]);
        location = vars;
        vars += 3;
    }
    if (strcmp(type, "in") == 0) { // let the user type whatever bullshit is on their minds
        strcpy(storedVariables[location + 1], "str");
        fgets(storedVariables[location + 2], 9999, stdin);
        int pos = (int)strcspn(storedVariables[location + 2], "\n"); // compensate for the newline by fucking yeeting it out of existence
        storedVariables[location + 2][pos] = '\0';
        j--;
    } else { strcpy(storedVariables[location + 1], type); strcpy(storedVariables[location + 2], instructions[j]); }
}

void compare(int operation) { // oh look 10 fucking functions in one. yippers
    float operand1 = 0; float operand2 = 0;
    j++;
    int location1 = findVar(); int location2 = findVar();
    
    if (operation == '>' || operation == ']' || operation == '<' || operation == '[') {
        if (location1 == -1) { operand1 = atof(instructions[j - 2]); }
        else {
            if (strcmp(storedVariables[location1 + 1], "num") == 0) { operand1 = atof(storedVariables[location1 + 2]); }
            else { printf("Operand must be of \"num\" type!\n"); exit(1); }
        }
        
        if (location2 == -1) { operand2 = atof(instructions[j - 1]); }
        else {
            if (strcmp(storedVariables[location2 + 1], "num") == 0) { operand2 = atof(storedVariables[location2 + 2]); }
            else { printf("Operand must be of \"num\" type!\n"); exit(1); }
        }
    } else if (operation == '&' || operation == '|') {
        if (location1 == -1) {
            if (strcmp(instructions[j - 2], "true") == 0) { operand1 = 1; }
        } else {
            if (strcmp(storedVariables[location1 + 1], "bool") == 0) {
                if (strstr(storedVariables[location1 + 2], "true")) { operand1 = 1; }
            } else { printf("Operand must be of \"bool\" type!\n"); exit(1); }
        }
        
        if (location2 == -1) {
            if (strcmp(instructions[j - 1], "true") == 0) { operand2 = 1; }}
        else {
            if (strcmp(storedVariables[location2 + 1], "bool") == 0) {
                if (strstr(storedVariables[location2 + 2], "true")) { operand2 = 1; }
            } else { printf("Operand must be of \"bool\" type!\n"); exit(1); }
        }
    }

    int output = 0;
    switch (operation) {
        case 'e':
            if (location1 != -1) { if (strcmp(storedVariables[location1 + 2], instructions[j - 1]) == 0) { output = 1; }}
            else if (location2 != -1) {if (strcmp(storedVariables[location2 + 2], instructions[j - 2]) == 0) { output = 1; }}
            break;
        case 'E':
            if (strcmp(storedVariables[location1 + 2], storedVariables[location2 + 2]) == 0) { output = 1; } break;
        case 'n':
            if (location1 != -1) { if (strcmp(storedVariables[location1 + 2], instructions[j - 1]) != 0) { output = 1; }}
            else if (location2 != -1) { if (strcmp(storedVariables[location2 + 2], instructions[j - 2]) != 0) { output = 1; }}
            break;
        case 'N': if (strcmp(storedVariables[location1 + 2], storedVariables[location2 + 2]) != 0) { output = 1; } break;
        case '>': if (operand1 > operand2) { output = 1; } break;
        case ']': if (operand1 >= operand2) { output = 1; } break;
        case '<': if (operand1 < operand2) { output = 1; } break;
        case '[': if (operand1 <= operand2) { output = 1; } break;
        case '|': if (operand1 || operand2) { output = 1; } break;
        case '&': if (operand1 && operand2) { output = 1; } break;
        default: output = -1; break;
    }

    if (location1 != -1) {
        strcpy(storedVariables[location1 + 1], "bool");
        if (output) strcpy(storedVariables[location1 + 2], "true");
        else strcpy(storedVariables[location1 + 2], "false");
    } else if (location2 != -1) {
        strcpy(storedVariables[location2 + 1], "bool");
        if (output) strcpy(storedVariables[location2 + 2], "true");
        else strcpy(storedVariables[location2 + 2], "false");
    }
    j--;
}

int main(int argc, const char * argv[]) {
    if (argc < 2) { printf("usage: simas <file.simas>\n"); return 1; }
    instructions = malloc(sizeof(char *) * 1000);
    storedVariables = malloc(sizeof(char *) * 300);
    openStartingFiles(argv[1]);
    for (j = 0; j < instructionCount; j++) {
        char current_instruction[10000];
        strcpy(current_instruction, instructions[j]);
        // printf("Instruction: %s j: %d\n", current_instruction, j);
        j++;
        if (strcmp(current_instruction, "add") == 0) {
            doMath(1);
        } else if (strcmp(current_instruction, "sub") == 0) {
            doMath(2);
        } else if (strcmp(current_instruction, "mul") == 0) {
            doMath(3);
        } else if (strcmp(current_instruction, "div") == 0) {
            doMath(4);
        } else if (strcmp(current_instruction, "printc") == 0) {
            printf("%s", instructions[j]);
        } else if (strcmp(current_instruction, "println") == 0) {
            printf("\n"); j--;
        } else if (strcmp(current_instruction, "prints") == 0) {
            printf(" "); j--;
        } else if (strcmp(current_instruction, "print") == 0) {
            int location = findVar();
            printf("%s", storedVariables[location + 2]);
            j--;
        } else if (strcmp(current_instruction, "quit") == 0) {
            exit(0);
        } else if (strcmp(current_instruction, "write") == 0) {
            FILE *file = fopen(instructions[j], "w");
            if (file == NULL) {
                printf("failed to write to file!\n");
                exit(23470913);
            }
            fprintf(file, "%s", instructions[j + 1]);
            fclose(file);
        } else if (strcmp(current_instruction, "writev") == 0) {
            FILE *file = fopen(instructions[j], "w");
            if (file == NULL) {
                printf("failed to write to file!\n");
                exit(23470913);
            }
            j++;
            int location = findVar();
            fprintf(file, "%s", storedVariables[location + 2]);
            fclose(file);
            j--;
        } else if (strcmp(current_instruction, "copy") == 0) {
            int location1 = findVar(); int location2 = findVar();
            j--;
            if (location2 == -1) {
                storedVariables[vars] = malloc(101); storedVariables[vars + 1] = malloc(3); storedVariables[vars + 2] = malloc(10001);
                strcpy(storedVariables[vars], instructions[j]);
                location2 = vars; vars += 3;
            }
            strcpy(storedVariables[location2 + 2], storedVariables[location1 + 2]);
            strcpy(storedVariables[location2 + 1], storedVariables[location1 + 1]);
        } else if (strcmp(current_instruction, "conv") == 0) {
            conv();
        } else if (strcmp(current_instruction, "read") == 0) {
            readFile();
        } else if (strcmp(current_instruction, "jump") == 0) {
            int location = findLabel(); j = atoi(labels[location + 1]) - 1;
        } else if (strcmp(current_instruction, "jumpv") == 0) {
            int jumpLocation = findLabel(); int location = findVar(); int allowed = 0;
            if (strcmp(storedVariables[location + 1], "num") == 0) { if (atoi(storedVariables[location + 2]) != 0) { allowed = 1; }}
            else if (strcmp(storedVariables[location + 1], "bool") == 0) { if (strcmp(storedVariables[location + 2], "true") == 0) { allowed = 1; }}
            else { printf("strings cannot be compared\n"); exit(1); }
            if (allowed) {
                j = atoi(labels[jumpLocation + 1]) - 1;
            } else {
                j--;
            }
        } else if (strcmp(current_instruction, "not") == 0) {
            int location = findVar();
            if (strcmp(storedVariables[location + 1], "bool") == 0) {
                if (strcmp(storedVariables[location + 2], "true") == 0) { strcpy(storedVariables[location + 2], "false"); }
                else { strcpy(storedVariables[location + 2], "true"); }
            } else { printf("NOT must be used on a bool!\n"); exit(1); }
            j--;
        }
        else if (strcmp(current_instruction, "set") == 0) { setVar(); }
        else if (strcmp(current_instruction, "st") == 0) { compare('<'); }
        else if (strcmp(current_instruction, "ste") == 0) { compare('['); }
        else if (strcmp(current_instruction, "gt") == 0) { compare('>'); }
        else if (strcmp(current_instruction, "gte") == 0) { compare(']'); }
        else if (strcmp(current_instruction, "and") == 0) { compare('&'); }
        else if (strcmp(current_instruction, "or") == 0) { compare('|'); }
        else if (strcmp(current_instruction, "eqc") == 0) { compare('e'); }
        else if (strcmp(current_instruction, "eqv") == 0) { compare('E'); }
        else if (strcmp(current_instruction, "neqc") == 0) { compare('n'); }
        else if (strcmp(current_instruction, "neqv") == 0) { compare('N'); }
    }
    return 0;
}
