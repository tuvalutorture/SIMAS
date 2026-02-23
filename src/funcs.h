#ifndef FUNCS_H
#define FUNCS_H

#include "runtime.h"

typedef struct function function;

struct function {
    int start, end;
    int parameterCount;
    char *retName;
};

void registerFunction(openFile *caller, char **arguments, int argumentCount);
void executeFunction(openFile *caller, char **arguments, int argumentCount);

#endif