#ifndef RUNTIME_H
#define RUNTIME_H

#include "hashmap.h"

#define DEBUG_PRINTF if (debugMode) printf /* macro abuse at its finest */
#define DEBUG_PRINT(string) if (debugMode) puts(string)

typedef struct command command;
typedef struct operation operation;
typedef struct InstructionSet InstructionSet;
typedef struct instruction instruction;
typedef struct openFile openFile;

struct instruction {
    operation *op;
    char **arguments;
    int argumentCount, argOffset;
};

struct openFile {
    char *path;
    char *instructionSource;
    instruction *instructions;
    HashMap variables;
    HashMap labels;
    HashMap lists; 
    HashMap functions;
    LinkedList stack;
    int instructionCount;
    int programCounter;
};

struct command { /* spoingus my beloved */
    instruction *inst;
    openFile *file;
};

struct operation {
    void (*functionPointer)(void*); /* guys i think this points or smth idk */ 
    int minArgs;
};

struct InstructionSet {
    HashMap operations;
    HashMap prefixes;
};

extern int debugMode;
extern int commandPrompt;
extern InstructionSet ValidInstructions;

void cry(char *msg);
void freeFile(openFile file);
void freeInstructionSet(InstructionSet *isa);
void handleError(char *errorMsg, int errCode, int fatal, openFile *file);
void snadmwithc(void);
openFile openSimasFile(char *path);
void beginCommandLine(char *entryMsg, openFile *passed);
void setUpCommands(void);
void setUpStdlib(void);

void push(LinkedList *stack, void *data);
void *pop(LinkedList *stack);
void *peek(LinkedList *stack, int offset);

void executeFile(openFile *current, int doFree);
void executeInstruction(openFile *cur);

#endif