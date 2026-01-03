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
    char **arguments;
    char *operation;
    char *prefix;
    int argumentCount;
};

struct openFile {
    char *path;
    instruction **instructions;
    HashMap variables;
    HashMap labels;
    HashMap lists; 
    HashMap functions;
    int instructionCount;
    int programCounter;
};

struct command { /* spoingus my beloved */
    char *(*commandPointer)(instruction*, openFile*); 
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
void freeInstructionSet(InstructionSet *isa);
void handleError(char *errorMsg, int errCode, int fatal, openFile *file);
void snadmwithc(void);
openFile openSimasFile(const char path[]);
void beginCommandLine(char *entryMsg, openFile *passed);
void setUpCommands(void);
void setUpStdlib(void);

void executeFile(openFile *current, int doFree);
void executeInstruction(openFile *cur);

#endif