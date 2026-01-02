#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "runtime.h"

typedef struct function function;

struct function {
    int start, end;
    int parameterCount;
};

void registerFunction(openFile *caller, char **arguments, int argumentCount);
void executeFunction(openFile *caller, char **arguments, int argumentCount);

#endif