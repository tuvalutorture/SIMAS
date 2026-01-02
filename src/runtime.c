#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "strings.h"
#include "runtime.h"
#include "hashmap.h"
#include "variables.h"
#include "functions.h"
#include "helpers.h"

InstructionSet ValidInstructions;
HashMap ValidCommands;

int debugMode = 0;
int commandPrompt = 0;

void cry(char *msg) { puts(msg); exit(2847172); }

void freeInstructionSet(InstructionSet *isa) { freeHashMap(isa->operations); freeHashMap(isa->prefixes); }
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

void preprocessLabels(openFile *new) {
    int i;
    new->labels = create_hashmap(new->labels.buckets); 
    for (i = 0; i < new->instructionCount; i++) {
        if (strcmp(new->instructions[i]->operation, "label") == 0) { 
            int *location = (int *)malloc(sizeof(int));
            *location = i - 1;
            if (new->instructions[i]->arguments[0][0] == '$') { free(location); new->programCounter = i; handleError("name is reserved", 99, 0, new); }
            addItemToMap(&new->labels, location, new->instructions[i]->arguments[0], free);
        }
    }
}

void snadmwithc(void) { /* sandwich hrhehehheheheheheheheheeeehheheherhehehehhehhehhehhhehhehhehehehhehhehhehhehehehehhehheheheehhehehhehehnehehehehe */
    debugMode = !debugMode;
    if (debugMode) { puts("debug mode enabled"); }
    else { puts("debug mode disabled"); }
}

int findNumberArgs(instruction *inst, InstructionSet isa) { 
    char *temp = buildStringFromInstruction(inst);
    operation *search = ((operation *)searchHashMap(&isa.operations, temp));
    free(temp); 
    if (search != NULL) { return search->minArgs; } return -1; 
}

instruction *add_instruction(char *inst, char *arguments[], char *prefix, int args) {
    int i; char *ins = stripSemicolon(inst);
    instruction *instruct = (instruction *)malloc(sizeof(instruction));
    if (args >= 1) { instruct->arguments = (char **)malloc(sizeof(char*) * args); for (i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }}
    else { instruct->arguments = NULL; }
    instruct->operation = stroustrup(ins); instruct->argumentCount = args;
    if (prefix != NULL) { instruct->prefix = stroustrup(prefix); }
    else { instruct->prefix = NULL; }
    DEBUG_PRINTF("added instruction %s of arg count %d\n", instruct->operation, instruct->argumentCount);
    free(ins);
    return instruct;
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


void addOperation(char *name, char *prefix, void (*functionPointer)(void*), int minimumArguments) { 
    operation *op = (operation *)malloc(sizeof(operation)); char *joined;
    op->functionPointer = functionPointer; 
    op->minArgs = minimumArguments;
    if (prefix != NULL) { char **temp; temp = (char **)malloc(sizeof(char *) * 2); temp[0] = prefix; temp[1] = name; joined = joinStringsSentence(temp, 2, 0); free(temp); }
    else { joined = stroustrup(name); }
    addItemToMap(&ValidInstructions.operations, op, joined, free);
    free(joined);
}

void addCommand(char *name, char *(*commandPointer)(instruction*, openFile*)) { 
    command *cmd = (command *)malloc(sizeof(command));
    cmd->commandPointer = commandPointer;
    addItemToMap(&ValidCommands, cmd, name, free);
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
        buffer = (char *)calloc(size + 1, sizeof(char)); 
        fread(buffer, sizeof(char), size, file);
        buffer[size] = '\0';

        if (strchr(buffer, '@') == NULL) {
            new.instructions = (instruction **)realloc(new.instructions, sizeof(instruction *) * (new.instructionCount + 1));
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
                passed->instructions = (instruction **)realloc(passed->instructions, sizeof(instruction *) * (passed->instructionCount + 1));
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

void labelJump(int *location, int *programCounter) { *programCounter = *location; }
void jumpConditionally(int *location, variable *var, int *programCounter, int flip) {
    int allowed = boolFromVar(var);
    if (flip) { allowed = !allowed; }
    if (allowed) labelJump(location, programCounter);
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
void lis_new(openFile *file) { list *new = (list *)calloc(1, sizeof(list)); if (file->instructions[file->programCounter]->arguments[0][0] == '$') { free(new); handleError("name is reserved", 99, 0, file); } new->elements = (int *)calloc(1, sizeof(int)); addItemToMap(&file->lists, new, file->instructions[file->programCounter]->arguments[0], (void (*)(void *))freeList); }
void lis_upv(openFile *file) { varcpy(indexList(file, (list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments[1]), searchHashMap(&file->variables, file->instructions[file->programCounter]->arguments[3])); }
void lis_acc(openFile *file) { varcpy(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[2]), indexList(file, (list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments[1])); }
void lis_load(openFile *file) { loadList(&file->lists, file->instructions[file->programCounter]->arguments[0], file->instructions[file->programCounter]->arguments[1]); }
void lis_len(openFile *file) { set_variable_value(createVarIfNotFound(&file->variables, file->instructions[file->programCounter]->arguments[1]), NUM, NULL, *((list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))->elements, 0); }
void lis_dump(openFile *file) { freeAndWrite(file->instructions[file->programCounter]->arguments[1], formatList(*(list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]))); }
void lis_upc(openFile *file) { listUpdateConstant(file, ((list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0])), file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); } 
void lis_appc(openFile *file) { listAppendConstant(searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }
void lis_copy(openFile *file) { list *li = searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[1]); if (!li) { li = (list *)calloc(1, sizeof(list)); li->elements = (int *)calloc(1, sizeof(int)); addItemToMap(&file->lists, li, file->instructions[file->programCounter]->arguments[1], (void (*)(void *))freeList); } listcpy(li, searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0])); }
void lis_alias(openFile *file) { if (file->instructions[file->programCounter]->arguments[0][0] == '$') { handleError("cannot create alias to reserved list", 95, 0, file); } setAlias(file, (list *)searchHashMap(&file->lists, file->instructions[file->programCounter]->arguments[0]), file->instructions[file->programCounter]->arguments[1]); }
/* function ops     */
void fun_fun(openFile *file) { registerFunction(file, file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }
void fun_call(openFile *file) { executeFunction(file, file->instructions[file->programCounter]->arguments, file->instructions[file->programCounter]->argumentCount); }

/* command functions for the CLI */
char *cmd_quit(instruction *inst, openFile *file) { freeInstruction(inst); return NULL; } /* this is what we call a pro gamer move */
char *cmd_clear(instruction *inst, openFile *file) { freeFile(*file); memset(file, 0, sizeof(openFile)); return ""; }
char *cmd_debug(instruction *inst, openFile *file) { snadmwithc(); return ""; }
char *cmd_load(instruction *inst, openFile *file) { if (inst->argumentCount) { freeFile(*file); *file = openSimasFile(inst->arguments[0]); if (file->path) { return "loaded successfully\n"; }}  else return "you need to specify a file\n"; return ""; }
char *cmd_dump(instruction *inst, openFile *file) { int i; for (i = 0; i < file->instructionCount; i++) { char *string = unParseInstructions(file->instructions[i]); printf("%d: %s\n", i + 1, string); free(string); } return ""; /* dummy ret */ }
char *cmd_run(instruction *inst, openFile *file) { if (file->instructionCount) { executeFile(file, 0); cleanFile(file); return ""; } else { return "no instructions to execute\n"; }}
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

void setUpStdlib(void) {
    ValidInstructions.operations = create_hashmap(53); ValidInstructions.prefixes = create_hashmap(1);
    addItemToMap(&ValidInstructions.prefixes, "list", "list", NULL);
    addOperation("label", NULL, NULL, 1); /* no-op */
    addOperation("end", NULL, NULL, 1); /* no-op */
    addOperation("ret", NULL, NULL, 0); /* no-op */
    addOperation("print", NULL, (void(*)(void*))con_printv, 1); 
    addOperation("println", NULL, (void(*)(void*))con_println, 0);
    addOperation("prints", NULL, (void(*)(void*))con_prints, 0); 
    addOperation("printc", NULL, (void(*)(void*))con_printc, 1); 
    addOperation("read", NULL, (void(*)(void*))fio_read, 2); 
    addOperation("write", NULL, (void(*)(void*))fio_write, 2);
    addOperation("writev", NULL, (void(*)(void*))fio_writev, 2); 
    addOperation("not", NULL, (void(*)(void*))etc_not, 1);
    addOperation("quit", NULL, (void(*)(void*))etc_quit, 0);
    addOperation("add", NULL, (void(*)(void*))mat_add, 3);
    addOperation("sub", NULL, (void(*)(void*))mat_sub, 3);
    addOperation("mul", NULL, (void(*)(void*))mat_mul, 3);
    addOperation("div", NULL, (void(*)(void*))mat_div, 3);
    addOperation("set", NULL, (void(*)(void*))var_set, 2);
    addOperation("type", NULL, (void(*)(void*))var_type, 2);
    addOperation("conv", NULL, (void(*)(void*))var_conv, 2);
    addOperation("copy", NULL, (void(*)(void*))var_copy, 2);
    addOperation("ptr", NULL, (void(*)(void*))var_ptr, 2);
    addOperation("gt", NULL, (void(*)(void*))cmp_gt, 3); 
    addOperation("gte", NULL, (void(*)(void*))cmp_gte, 3); 
    addOperation("st", NULL, (void(*)(void*))cmp_st, 3); 
    addOperation("ste", NULL, (void(*)(void*))cmp_ste, 3); 
    addOperation("eqv", NULL, (void(*)(void*))cmp_eqv, 3); 
    addOperation("neqv", NULL, (void(*)(void*))cmp_neqv, 3);
    addOperation("eqc", NULL, (void(*)(void*))cmp_eqc, 3); 
    addOperation("neqc", NULL, (void(*)(void*))cmp_neqc, 3);
    addOperation("and", NULL, (void(*)(void*))cmp_and, 3); 
    addOperation("nand", NULL, (void(*)(void*))cmp_nand, 3);
    addOperation("or", NULL, (void(*)(void*))cmp_or, 3); 
    addOperation("nor", NULL, (void(*)(void*))cmp_nor, 3);
    addOperation("xor", NULL, (void(*)(void*))cmp_xor, 3);
    addOperation("jump", NULL, (void(*)(void*))jmp_jump, 1);
    addOperation("jumpv", NULL, (void(*)(void*))jmp_jumpv, 2); 
    addOperation("jumpnv", NULL, (void(*)(void*))jmp_jumpnv, 2);
    addOperation("del", "list", (void(*)(void*))lis_del, 2); 
    addOperation("appv", "list", (void(*)(void*))lis_appv, 3); 
    addOperation("show", "list", (void(*)(void*))lis_show, 1); 
    addOperation("new", "list", (void(*)(void*))lis_new, 1); 
    addOperation("upv", "list", (void(*)(void*))lis_upv, 4); 
    addOperation("acc", "list", (void(*)(void*))lis_acc, 3); 
    addOperation("load", "list", (void(*)(void*))lis_load, 2); 
    addOperation("len", "list", (void(*)(void*))lis_len, 2); 
    addOperation("dump", "list", (void(*)(void*))lis_dump, 2); 
    addOperation("upc", "list", (void(*)(void*))lis_upc, 4); 
    addOperation("appc", "list", (void(*)(void*))lis_appc, 3); 
    addOperation("copy", "list", (void(*)(void*))lis_copy, 2); 
    addOperation("alias", "list", (void(*)(void*))lis_alias, 2);
    addOperation("copyl", NULL, (void(*)(void*))lis_copy, 2); 
    addOperation("fun", NULL, (void(*)(void *))fun_fun, 2);
    addOperation("call", NULL, (void(*)(void *))fun_call, 2);
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

void executeInstruction(openFile *cur) { /* all of these are defined up here so this function can operate independently of any files */
    operation *found; char *string;
    if (strlen(cur->instructions[cur->programCounter]->operation) == 0) return;
    string = buildStringFromInstruction(cur->instructions[cur->programCounter]);
    DEBUG_PRINTF("\nExecuting instruction %s on line %d.\n", string, cur->programCounter);
    found = searchHashMap(&ValidInstructions.operations, string); free(string);
    if (found != NULL && found->functionPointer != NULL) { (found->functionPointer)(cur); }
}

void executeFile(openFile *current, int doFree) {
    preprocessLabels(current); current->lists = create_hashmap(1); current->variables = create_hashmap(10); current->functions = create_hashmap(1);
    for (current->programCounter = 0; current->programCounter < current->instructionCount; current->programCounter++) { executeInstruction(current); if (commandPrompt == 2) { break; }}
    if (doFree) freeFile(*current);
}