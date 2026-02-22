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
InstructionSet ValidCommands;

int debugMode = 0;
int commandPrompt = 0;
/* nops for control */
void nop_lab(openFile *file) { }
void nop_end(openFile *file) { }
void nop_ret(openFile *file) { }
void nop_imp(openFile *file) { }

void cry(char *msg) { puts(msg); exit(2847172); }

void freeInstructionSet(InstructionSet *isa) { freeHashMap(isa->operations); freeHashMap(isa->prefixes); }
void freeInstruction(instruction *inst) {
    int i;
    if (inst == NULL) return;
    for (i = 0; i < inst->argumentCount; i++) { DEBUG_PRINTF("freeing %s arg\n", inst->arguments[i]); free(inst->arguments[i]); }
    free(inst->arguments);
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

void preprocessImports(openFile *new) {
    int i; HashMap imports = create_hashmap(10); 
    addItemToMap(&imports, NULL, new->path, NULL); 
    for (i = 0; i < new->instructionCount; i++) {
        DEBUG_PRINTF("%d\n", i);
        if (new->instructions[i]->op->functionPointer == (void(*)(void*))nop_imp) {
            openFile temp;
            memset(&temp, 0, sizeof(openFile));
            if (searchHashMap(&imports, new->instructions[i]->arguments[0]) != NULL) { new->programCounter = i; handleError("cannot import a file more than once", 438, 0, new); }
            temp = openSimasFile(new->instructions[i]->arguments[0]);
            new->instructions = (instruction **)realloc(new->instructions, (new->instructionCount + temp.instructionCount - 1) * sizeof(instruction *));
            memmove(new->instructions + i + temp.instructionCount, new->instructions + i + 1, (new->instructionCount - i - 1) * sizeof(instruction *));
            freeInstruction(new->instructions[i]); /* remove the current import instruction */
            memmove(new->instructions + i, temp.instructions, temp.instructionCount * sizeof(instruction *));
            addItemToMap(&imports, temp.path, temp.path, free); new->instructionCount += temp.instructionCount - 1;
            free(temp.instructions);
        }
    }
    freeHashMap(imports);
}

void preprocessLabels(openFile *new) {
    int i;
    new->labels = create_hashmap(new->labels.buckets); 
    for (i = 0; i < new->instructionCount; i++) {
        if (new->instructions[i]->op->functionPointer == (void(*)(void*))nop_lab) {
            int *location;
            if (searchHashMap(&new->labels, new->instructions[i]->arguments[0]) != NULL) { new->programCounter = i; handleError("redefinition of label", 85, 0, new); }
            location = (int *)malloc(sizeof(int)); *location = i - 1;
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

instruction *add_instruction(InstructionSet isa, char *inst, char **arguments, char *prefix, int args) {
    int i, freeable = 0; char *op;
    instruction *instruct = (instruction *)malloc(sizeof(instruction));
    if (args >= 1) { instruct->arguments = (char **)malloc(sizeof(char*) * args); for (i = 0; i < args; i++) { instruct->arguments[i] = stripSemicolon(arguments[i]); }}
    else { instruct->arguments = NULL; }
    if (prefix != NULL) { op = (char *)calloc(strlen(inst) + strlen(prefix) + 2, sizeof(char)); strcpy(op, prefix); strcat(op, " "); strcat(op, inst); freeable = 1; }
    else { op = inst; }
    instruct->op = searchHashMap(&isa.operations, op); instruct->argumentCount = args;
    DEBUG_PRINTF("added instruction %s of arg count %d\n", op, instruct->argumentCount);
    if (freeable) free(op);
    return instruct;
}

instruction *parseInstructions(char *string, InstructionSet isa) {
    int i, argc = 0, index = 0, arrCount;
    char *operation, *prefix = NULL, **tokenized = stringSlicer(string, &arrCount);
    instruction *new;
    stripSemicolonInPlace(tokenized[arrCount - 1]);
    lowerizeInPlace(tokenized[index]);
    while (strcmp(tokenized[index], "please") == 0) { index += 1; lowerizeInPlace(tokenized[index]);  }
    if (isa.prefixes.items != NULL && searchHashMap(&isa.prefixes, tokenized[index]) != NULL) { prefix = tokenized[index]; index += 1; lowerizeInPlace(tokenized[index]); }
    lowerizeInPlace(tokenized[index]); operation = tokenized[index]; index += 1;
    argc = arrCount - index;
    new = add_instruction(isa, operation, tokenized + index, prefix, argc);
    if (argc >= 1) { for (i = 0; i < argc; i++) { DEBUG_PRINTF("instruction %s has arg \"%s\"\n", operation, tokenized[index + i]); }}
    free(tokenized);
    return new;
}

void addOperation(InstructionSet *set, char *name, char *prefix, void (*functionPointer)(void*), int minimumArguments) {
    operation *op = (operation *)malloc(sizeof(operation)); char *joined;
    op->functionPointer = functionPointer;
    op->minArgs = minimumArguments;
    if (prefix != NULL) { char **temp; temp = (char **)malloc(sizeof(char *) * 2); temp[0] = prefix; temp[1] = name; joined = joinStringsSentence(temp, 2, 0); free(temp); }
    else { joined = stroustrup(name); }
    addItemToMap(&set->operations, op, joined, free);
    free(joined);
}

openFile openSimasFile(char *path) {
    unsigned long i, fileIndex = 0, instructionCount = 0, fileSize; char *fileContents;
    FILE *file = fopen(path, "rb");
    openFile new;
    memset(&new, 0, sizeof(openFile));

    if (file == NULL) { printf("failed to find a simas file!\n"); return new; }

    new.path = stroustrup(path);

    fileContents = readFile(path);
    if (!fileContents) cry("shit died ig");
    fileSize = strlen(fileContents);

    fclose(file);

    for (i = 0; i < fileSize; i++) {
        if (fileContents[i] == '@') instructionCount -= 1;
        if (fileContents[i] == ';') instructionCount += 1;
    }

    new.instructions = (instruction **)malloc(sizeof(instruction *) * instructionCount);
    if (new.instructions == NULL) cry("welp, cant add more functions, guess its time to die now");

    while (fileIndex < fileSize) {
        int size = 0; char *buffer;
        while (fileContents[fileIndex++] != ';' && fileContents[fileIndex - 1] != '\0') { size += 1; }
        DEBUG_PRINTF("\n%d\n", size);
        DEBUG_PRINT("goin back for more\n");
        if (fileIndex > fileSize) { break; }

        buffer = fileContents + fileIndex - size - 1;
        buffer[size] = '\0';
        if (strchr(buffer, '@')) continue;

        new.instructions[new.instructionCount] = parseInstructions(buffer, ValidInstructions);
        if (new.instructions[new.instructionCount]->op->functionPointer == (void(*)(void*))nop_lab) new.labels.buckets += 1;
        new.instructionCount += 1;
    }

    free(fileContents);
    return new;
}

/* command functions for the CLI */
void cmd_quit(command *cmd) { } /* this is what we call a pro gamer move */
void cmd_clear(command *cmd) { freeFile(*cmd->file); memset(cmd->file, 0, sizeof(openFile)); }
void cmd_debug(command *cmd) { snadmwithc(); }
void cmd_load(command *cmd) { if (cmd->inst->argumentCount) { freeFile(*cmd->file); *cmd->file = openSimasFile(cmd->inst->arguments[0]); if (cmd->file->path) { puts("loaded successfully"); }}  else { puts("you need to specify a file"); }}
void cmd_dump(command *cmd) { int i; for (i = 0; i < cmd->file->instructionCount; i++) { char *string = unParseInstructions(cmd->file->instructions[i]); printf("%d: %s\n", i + 1, string); free(string); }}
void cmd_run(command *cmd) { if (cmd->file->instructionCount) { executeFile(cmd->file, 0); cleanFile(cmd->file); } else { puts("no instructions to execute"); }}
void cmd_save(command *cmd) {
    if (cmd->inst->argumentCount) {
        FILE* dest = fopen(cmd->inst->arguments[0], "wb");
        if (dest) {
            int i;
            for (i = 0; i < cmd->file->instructionCount; i++) {
                char *string = unParseInstructions(cmd->file->instructions[i]);
                fwrite(string, sizeof(char), strlen(string), dest);
                free(string); fputc('\n', dest);
            }
            fclose(dest); puts("successfully saved!");
        } else puts("unable to open file");
    } else puts("you need to specify a file to save to");
}
void cmd_help(command *cmd) {
    puts(
        "CMAS Command List:\n"
        "!quit: Quits the CMAS command line.\n"
        "!clear: Resets the current program.\n"
        "!edit <index>: Edit the instruction at an index, starting from 1.\n"
        "!dump: Dumps the current program to terminal.\n"
        "!load <filename>: Loads a SIMAS file.\n"
        "!save <filename>: Saves the current SIMAS program to disk.\n"
        "!run: Executes the current SIMAS program.\n"
        "Please read the README.md for a list of all instructions and their operators."
    );
}

void beginCommandLine(char *entryMsg, openFile *passed) {
    puts(entryMsg);

    if (!ValidCommands.operations.items) setUpCommands();

    while (1) {
        char *value, *temp; instruction *inst;
        commandPrompt = 1; printf("$ ");
        value = grabUserInput(256);
        if (!value) { handleError("Unable to allocate memory\n", 10, 1, passed); }
        temp = stripSemicolon(value); strip(temp, ' ');
        if (strcmp(temp, "") == 0) {free(value); free(temp); continue;} /* blank check */
        free(temp);

        if (value[0] == '!') {
            command cmd;
            inst = parseInstructions(value, ValidCommands);
            if (inst->op == NULL) printf("invalid command\n");
            cmd.file = passed; cmd.inst = inst;
            inst->op->functionPointer(&cmd);
            if (inst->op->functionPointer == (void(*)(void*))cmd_quit) { free(value); freeInstruction(inst); break;}
        } else {
            inst = parseInstructions(value, ValidInstructions);
            if (!inst->op && strchr(value, '@') == NULL) {
                printf("invalid instruction\n");
            } else {
                if (strchr(value, ';') != NULL && inst->argumentCount >= inst->op->minArgs) {
                    int i; instruction *newInstruction = malloc(sizeof(instruction));
                    if (newInstruction == NULL) cry("waaaah");
                    newInstruction->op = inst->op; newInstruction->argumentCount = inst->argumentCount;
                    newInstruction->arguments = (char **)malloc(sizeof(char *) * newInstruction->argumentCount);
                    for (i = 0; i < newInstruction->argumentCount; i++) { newInstruction->arguments[i] = stroustrup(inst->arguments[i]); }
                    passed->instructions = (instruction **)realloc(passed->instructions, sizeof(instruction *) * (passed->instructionCount + 1));
                    if (passed->instructions == NULL) { free(value); handleError("Reallocation of memory failed\n", 11, 1, passed); break; }
                    passed->instructions[passed->instructionCount] = inst;
                    passed->instructionCount += 1;
                    if (inst->op->functionPointer == (void(*)(void*))nop_lab) passed->labels.buckets += 1;
                    printf("ok\n");
                } else if (inst->argumentCount < inst->op->minArgs) {
                    printf("too little arguments for instruction\n");
                } else {
                    printf("code must end with a semicolon\n");
                }
            }
        }
        freeInstruction(inst); free(value);
    }

    freeFile(*passed);
    freeInstructionSet(&ValidCommands);
    freeInstructionSet(&ValidInstructions);

    exit(0);
}

void executeInstruction(openFile *cur) { /* all of these are defined up here so this function can operate independently of any files */
    if (cur->instructions[cur->programCounter]->op == NULL) return;
    cur->instructions[cur->programCounter]->op->functionPointer(cur);
}

void executeFile(openFile *current, int doFree) {
    preprocessImports(current);
    if (current->labels.buckets > 0) preprocessLabels(current);
    current->lists = create_hashmap(10); current->variables = create_hashmap(10); current->functions = create_hashmap(10); /* 10 to provide breathing room before rehashing */
    for (current->programCounter = 0; current->programCounter < current->instructionCount; current->programCounter++) { executeInstruction(current); if (commandPrompt == 2) { break; }}
    if (doFree) freeFile(*current);
}

/* here for ctrl flow */

void labelJump(int *location, int *programCounter) { *programCounter = *location; }
void jumpConditionally(int *location, variable *var, int *programCounter, int flip) {
    int allowed = boolFromVar(var);
    if (flip) { allowed = !allowed; }
    if (allowed) labelJump(location, programCounter);
}

/* funcs...         */
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

void setUpStdlib(void) {
    ValidInstructions.operations = create_hashmap(54); ValidInstructions.prefixes = create_hashmap(1);
    addItemToMap(&ValidInstructions.prefixes, "list", "list", NULL);
    addOperation(&ValidInstructions, "label", NULL, (void(*)(void*))nop_lab, 1); /* no-op */
    addOperation(&ValidInstructions, "end", NULL, (void(*)(void*))nop_end, 1); /* no-op */
    addOperation(&ValidInstructions, "ret", NULL, (void(*)(void*))nop_ret, 0); /* no-op */
    addOperation(&ValidInstructions, "import", NULL, (void(*)(void*))nop_imp, 1); /* no-op */
    addOperation(&ValidInstructions, "print", NULL, (void(*)(void*))con_printv, 1);
    addOperation(&ValidInstructions, "println", NULL, (void(*)(void*))con_println, 0);
    addOperation(&ValidInstructions, "prints", NULL, (void(*)(void*))con_prints, 0);
    addOperation(&ValidInstructions, "printc", NULL, (void(*)(void*))con_printc, 1);
    addOperation(&ValidInstructions, "read", NULL, (void(*)(void*))fio_read, 2);
    addOperation(&ValidInstructions, "write", NULL, (void(*)(void*))fio_write, 2);
    addOperation(&ValidInstructions, "writev", NULL, (void(*)(void*))fio_writev, 2);
    addOperation(&ValidInstructions, "not", NULL, (void(*)(void*))etc_not, 1);
    addOperation(&ValidInstructions, "quit", NULL, (void(*)(void*))etc_quit, 0);
    addOperation(&ValidInstructions, "add", NULL, (void(*)(void*))mat_add, 3);
    addOperation(&ValidInstructions, "sub", NULL, (void(*)(void*))mat_sub, 3);
    addOperation(&ValidInstructions, "mul", NULL, (void(*)(void*))mat_mul, 3);
    addOperation(&ValidInstructions, "div", NULL, (void(*)(void*))mat_div, 3);
    addOperation(&ValidInstructions, "set", NULL, (void(*)(void*))var_set, 2);
    addOperation(&ValidInstructions, "type", NULL, (void(*)(void*))var_type, 2);
    addOperation(&ValidInstructions, "conv", NULL, (void(*)(void*))var_conv, 2);
    addOperation(&ValidInstructions, "copy", NULL, (void(*)(void*))var_copy, 2);
    addOperation(&ValidInstructions, "ptr", NULL, (void(*)(void*))var_ptr, 2);
    addOperation(&ValidInstructions, "gt", NULL, (void(*)(void*))cmp_gt, 3);
    addOperation(&ValidInstructions, "gte", NULL, (void(*)(void*))cmp_gte, 3);
    addOperation(&ValidInstructions, "st", NULL, (void(*)(void*))cmp_st, 3);
    addOperation(&ValidInstructions, "ste", NULL, (void(*)(void*))cmp_ste, 3);
    addOperation(&ValidInstructions, "eqv", NULL, (void(*)(void*))cmp_eqv, 3);
    addOperation(&ValidInstructions, "neqv", NULL, (void(*)(void*))cmp_neqv, 3);
    addOperation(&ValidInstructions, "eqc", NULL, (void(*)(void*))cmp_eqc, 3);
    addOperation(&ValidInstructions, "neqc", NULL, (void(*)(void*))cmp_neqc, 3);
    addOperation(&ValidInstructions, "and", NULL, (void(*)(void*))cmp_and, 3);
    addOperation(&ValidInstructions, "nand", NULL, (void(*)(void*))cmp_nand, 3);
    addOperation(&ValidInstructions, "or", NULL, (void(*)(void*))cmp_or, 3);
    addOperation(&ValidInstructions, "nor", NULL, (void(*)(void*))cmp_nor, 3);
    addOperation(&ValidInstructions, "xor", NULL, (void(*)(void*))cmp_xor, 3);
    addOperation(&ValidInstructions, "jump", NULL, (void(*)(void*))jmp_jump, 1);
    addOperation(&ValidInstructions, "jumpv", NULL, (void(*)(void*))jmp_jumpv, 2);
    addOperation(&ValidInstructions, "jumpnv", NULL, (void(*)(void*))jmp_jumpnv, 2);
    addOperation(&ValidInstructions, "del", "list", (void(*)(void*))lis_del, 2);
    addOperation(&ValidInstructions, "appv", "list", (void(*)(void*))lis_appv, 3);
    addOperation(&ValidInstructions, "show", "list", (void(*)(void*))lis_show, 1);
    addOperation(&ValidInstructions, "new", "list", (void(*)(void*))lis_new, 1);
    addOperation(&ValidInstructions, "upv", "list", (void(*)(void*))lis_upv, 4);
    addOperation(&ValidInstructions, "acc", "list", (void(*)(void*))lis_acc, 3);
    addOperation(&ValidInstructions, "load", "list", (void(*)(void*))lis_load, 2);
    addOperation(&ValidInstructions, "len", "list", (void(*)(void*))lis_len, 2);
    addOperation(&ValidInstructions, "dump", "list", (void(*)(void*))lis_dump, 2);
    addOperation(&ValidInstructions, "upc", "list", (void(*)(void*))lis_upc, 4);
    addOperation(&ValidInstructions, "appc", "list", (void(*)(void*))lis_appc, 3);
    addOperation(&ValidInstructions, "copy", "list", (void(*)(void*))lis_copy, 2);
    addOperation(&ValidInstructions, "alias", "list", (void(*)(void*))lis_alias, 2);
    addOperation(&ValidInstructions, "copyl", NULL, (void(*)(void*))lis_copy, 2);
    addOperation(&ValidInstructions, "fun", NULL, (void(*)(void *))fun_fun, 2);
    addOperation(&ValidInstructions, "call", NULL, (void(*)(void *))fun_call, 2);
}

void setUpCommands() {
    ValidCommands.operations = create_hashmap(8);
    addOperation(&ValidCommands, "!quit", NULL, (void(*)(void *))cmd_quit, 0);
    addOperation(&ValidCommands, "!run", NULL, (void(*)(void *))cmd_run, 0);
    addOperation(&ValidCommands, "!load", NULL, (void(*)(void *))cmd_load, 1);
    addOperation(&ValidCommands, "!save", NULL, (void(*)(void *))cmd_save, 1);
    addOperation(&ValidCommands, "!dump", NULL, (void(*)(void *))cmd_dump, 0);
    addOperation(&ValidCommands, "!clear", NULL, (void(*)(void *))cmd_clear, 0);
    addOperation(&ValidCommands, "!help", NULL, (void(*)(void *))cmd_help, 0);
    addOperation(&ValidCommands, "!debug", NULL, (void(*)(void *))cmd_debug, 0);
}