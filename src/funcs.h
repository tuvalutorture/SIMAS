#ifndef FUNCS_H
#define FUNCS_H

#include "runtime.h"

typedef struct function function;
typedef struct stackFrame stackFrame;

struct function {
    int start, end, returnSpot;
    int parameterCount;
    char *retName;
    char **callingArgs;
};

void registerFunction(openFile *caller, char **arguments, int argumentCount);
void executeFunction(openFile *caller, char **arguments, int argumentCount);

void callFunction(openFile *caller, char **arguments, int argumentCount);
void returnFunction(openFile *caller, char **arguments, int argumentCount);

#endif