#ifndef VARIABLES_H
#define VARIABLES_H

/* types that a var may be */
#define STR 1
#define NUM 2
#define BOOL 3
#define IN 4

#include "runtime.h"
#include "hashmap.h"

typedef struct variable variable;
typedef union variableData variableData;
typedef struct list list; 

union variableData { /* UNIONISE, MY CHILDREN! RISE AGAINST THE EMPLOYERS WHO TREAT YOU AS WAGE SLAVES! BECOME A UNION! FIGHT FOR WORKPLACE RIGHTS! GET YO 401(k)!!! */
    double num;
    char *str;
    int boolean;
    void *etc;
}; 

struct variable {
    int *type, isPtr;
    variableData *data;
};

struct list { /* keeps them lookup times objectively speedy as fawk */
    int *elements, isAlias;
    variable **variables;
}; 

void freeVariable(variable *var);
void freeList(list *lis);
void listcpy(list *dest, list *src);
variable *create_variable(void);
int trueOrFalse(char *string);
void set_variable_value(variable *var, int type, char *value, double num, int bool);
double coerceStringToNum(char *string);
int coerceStringToBool(char *string);
char *stringFromVar(variable *var);
double numFromVar(variable *src);
int boolFromVar(variable *var);
void varcpy(variable *dest, variable *src);
int grabType(char *input);
size_t stringLenFromVar(variable var);
variable *createVarIfNotFound(HashMap *varMap, char *name);

#endif