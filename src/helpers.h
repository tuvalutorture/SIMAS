#ifndef HELPERS_H
#define HELPERS_H

#include "variables.h"
#include "hashmap.h"
#include "runtime.h"

void negateBoolean(variable *var);
void writeFromVar(variable *var, char *path);
void equalityCheckVarVsConst(HashMap *varMap, char **arguments, int flip);
void equalityCheckVarVsVar(variable *var1, variable *var2, int flip);
void setPointer(openFile *current, variable *src, char *name);
void convert(variable *var, int type);
void setVar(variable *var, int type, char* value, double num, int boolean);
void standardMath(openFile *current, char **arguments, char operation);
void variableSet(openFile *current, char **arguments, int argumentCount);
void grabTypeFromVar(variable check, variable *var);
void compareNums(HashMap *varMap, char **arguments, char operation);
void compareBools(HashMap *varMap, char **arguments, char operation, char flip);
int areTwoVarsEqual(variable *var1, variable *var2);

void appendElementToList(list *li, variable *var);
void removeElementFromList(list *li, int element);
char *formatList(list li);
void unFormatList(list *li, char *string);
void loadList(HashMap *listMap, char *name, char *path);
variable *indexList(openFile *current, list *li, char *indexArg);
void listAppendConstant(list *li, char **arguments, int argumentCount);
void listUpdateConstant(openFile *current, list *li, char **arguments, int argumentCount);
void setAlias(openFile *current, list *src, char *name);

char *readFile(char path[]);
void writeFile(char *path, char *value);
void freeAndWrite(char *path, char *value);


#endif