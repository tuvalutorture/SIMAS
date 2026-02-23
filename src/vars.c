#include <stdlib.h>
#include <string.h>
#include "strings.h"
#include "vars.h"
#include "hashmap.h"
#include "runtime.h"

void freeVariable(variable *var) { 
    if (var->isPtr == 0) { 
        if (*var->type == STR && var->data->str != NULL) { free(var->data->str); }
        free(var->data); free(var->type);
    } 
    free(var); 
}

void freeList(list *lis) { 
    if (lis->isAlias == 0) { 
        int i; 
        for (i = 0; i < *lis->elements; i++) { freeVariable(lis->variables[i]); } 
        free(lis->variables); free(lis->elements); 
    } 
    free(lis); 
}

void listcpy(list *dest, list *src) {
    int i;
    if (dest->variables != NULL) { for (i = 0; i < *dest->elements; i++) { freeVariable(dest->variables[i]); } dest->variables = (variable **)realloc(dest->variables, *src->elements * sizeof(variable *)); }
    else dest->variables = (variable **)malloc(*src->elements * sizeof(variable *)); 
    if (dest->elements == NULL) dest->elements = (int *)calloc(1, sizeof(int));
    *dest->elements = *src->elements; 
    for (i = 0; i < *src->elements; i++) {
        dest->variables[i] = create_variable();
        varcpy(dest->variables[i], src->variables[i]);
    }
}

variable *create_variable(void) { /* dumb allocation wrapper */
    variable *new = (variable *)calloc(1, sizeof(variable));
    new->data = (variableData *)calloc(1, sizeof(variableData));
    new->type = (int *)calloc(1, sizeof(int));
    new->isPtr = 0;
    return new;
}

int trueOrFalse(char *string) {
    char check[6]; int value = 0;
    if (!string || strlen(string) > 5) return 0;
    strcpy(check, string);
    lowerizeInPlace(check);
    if (strcmp(check, "true") == 0) { value = 1; }
    else if (strcmp(check, "false") == 0) { value = 0; }
    return value;
}

void set_variable_value(variable *var, int type, char *value, double num, int bool) { /* too fucking lazy to pass in one at a time or wutever, so just pass in all of them manually, even if some are blank :3 */
    if (*var->type == STR && var->data->str) free(var->data->str);
    var->data->str = NULL;
    if (type == NUM) { var->data->num = num; }
    else if (type == BOOL) { var->data->boolean = bool; }
    else if (type == STR) {
        if (!value) cry("No value passed in!\n");
        var->data->str = stroustrup(value);
        if (var->data->str == NULL) { cry("nOnOOOO ze MALLOC faILEEEED"); }
    }
    *var->type = type;
}

/* ze horsemen of type feckery */
double coerceStringToNum(char *string) {
    char converted[255];
    if (strlen(string) >= 255) return 0;
    strcpy(converted, string);
    if (!strlen(converted)) return 0.0f;
    /* coerce from booleans */
    if (strcmp(converted, "true") == 0) { return 1.0f; }
    else if (strcmp(converted, "false") == 0) { return 0.0f; }
    /* and then give up and use atof */
    else { return atof(string); }
}

int coerceStringToBool(char *string) {
    char converted[255];
    if (strlen(string) >= 255) return 0;
    strcpy(converted, string);
    lowerizeInPlace(converted);
    if (!strlen(converted)) return 0;
    if (strcmp(converted, "true") == 0) { return 1; }
    else if (strcmp(converted, "false") == 0) { return 0; }
    else { return atoi(string) ? 1 : 0; } /* ternary fuckery to set to 1 if true */
}

char *stringFromVar(variable *var) { /* this already does the job of coercing strings over */
    if (*var->type == STR) { return var->data->str; }
    else if (*var->type == BOOL) { return var->data->boolean ? "true" : "false"; }
    else if (*var->type == NUM) { return grabStringOfNumber(var->data->num); }
    else { return ""; }
}

double numFromVar(variable *src) {
    if (src == NULL) return 0.0f;
    switch (*src->type) {
        case NUM: return src->data->num;
        case STR: return coerceStringToNum(src->data->str);
        case BOOL: return (double)src->data->boolean;
        default: return 0.0f;
    }
}

int boolFromVar(variable *var) {
    switch (*var->type) {
        case NUM: return var->data->num ? 1 : 0;
        case BOOL: return var->data->boolean; 
        case STR: return coerceStringToBool(var->data->str); 
        default: return 0;
    }
}

void varcpy(variable *dest, variable *src) { 
    if (*src->type == NUM) set_variable_value(dest, *src->type, NULL, src->data->num, 0); 
    if (*src->type == STR) set_variable_value(dest, *src->type, src->data->str, 0.0, 0); 
    if (*src->type == BOOL) set_variable_value(dest, *src->type, NULL, 0.0, src->data->boolean); 
}

int grabType(char *input) {
    char type[6];
    if (!input || strlen(input) > 5) return -1;
    strcpy(type, input);
    lowerizeInPlace(type);
    if (strcmp(type, "str") == 0) { return STR; }
    else if (strcmp(type, "num") == 0) { return NUM; }
    else if (strcmp(type, "bool") == 0) { return BOOL; }
    else if (strcmp(type, "in") == 0) { return IN; }
    else { return -1; }
}

size_t stringLenFromVar(variable var) {
    if (*var.type == STR) { return strlen(var.data->str); }
    else if (*var.type == BOOL) { return var.data->boolean ? strlen("true") : strlen("false"); }
    else if (*var.type == NUM) { return grabLengthOfNumber(var.data->num); }
    else { return 0; }
}

variable *createVarIfNotFound(HashMap *varMap, char *name) {
    variable *var = searchHashMap(varMap, name);
    if (!var) { var = create_variable(); addItemToMap(varMap, var, name, (void (*)(void *))freeVariable); }
    return var; 
}