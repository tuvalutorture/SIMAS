#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "strings.h"
#include "variables.h"
#include "hashmap.h"
#include "runtime.h"
#include "helpers.h"

/* var ops  */
void negateBoolean(variable *var) { int result = !boolFromVar(var); set_variable_value(var, BOOL, NULL, 0, result); }
void writeFromVar(variable *var, char *path) { char *variable = stringFromVar(var); writeFile(path, variable); free(variable); } 
void equalityCheckVarVsConst(HashMap *varMap, char **arguments, int flip) {
    variable *var1 = searchHashMap(varMap, arguments[1]), *var2 = create_variable();
    int output = 0, type = grabType(arguments[0]); *var2->type = type; var2->data->str = NULL;
    if (type == NUM) { var2->data->num = atof(arguments[2]); }
    else if (type == STR) { var2->data->str = stroustrup(arguments[2]); }
    else if (type == BOOL) { var2->data->boolean = trueOrFalse(arguments[2]); } 
    if (var1 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, var2) : areTwoVarsEqual(var1, var2);
    if (*var1->type == STR && var1->data->str != NULL) free(var1->data->str);
    *var1->type = BOOL;
    var1->data->boolean = output;
    freeVariable(var2);
}

void equalityCheckVarVsVar(variable *var1, variable *var2, int flip) {
    int output = 0;
    if (var1 == NULL || var2 == NULL) { cry("No variable!"); }
    output = flip ? !areTwoVarsEqual(var1, var2) : areTwoVarsEqual(var1, var2);
    if (*var1->type == STR && var1->data->str != NULL) free(var1->data->str);
    *var1->type = BOOL;
    var1->data->boolean = output;
}

void setPointer(openFile *current, variable *src, char *name) {
    variable *new;
    if (!src) handleError("invalid pointer target", 19, 0, current);
    if (name[0] == '$') handleError("name is reserved", 99, 0, current);
    new = searchHashMap(&current->variables, name);
    if (new != NULL && !new->isPtr) { handleError("cannot convert a variable to pointer", 36, 0, current); }
    else if (new == NULL) new = (variable *)calloc(1, sizeof(variable)); 
    new->data = src->data; new->type = src->type;
    new->isPtr = 1; addItemToMap(&current->variables, new, name, (void(*)(void *))freeVariable);
}

void convert(variable *var, int type) {
    DEBUG_PRINTF("\nConverted from type %d to type %d\n", *var->type, type);
    if (*var->type != type) {
        if (*var->type == NUM) {
            if (type == BOOL) {
                if (var->data->num != 0.0) { var->data->boolean = 1; }
                else { var->data->boolean = 0; }
            } else if (type == STR) {
                var->data->str = grabStringOfNumber(var->data->num);
            }
        } else if (*var->type == BOOL) {
            int truth = var->data->boolean;
            if (type == NUM) { var->data->num = (double)truth; }
            else if (type == STR) {
                if (truth) var->data->str = stroustrup("true");
                if (!truth) var->data->str = stroustrup("false");
            }
        } else if (*var->type == STR) {
            char *temp = stroustrup(var->data->str); if (var->data->str != NULL) { free(var->data->str); }
            if (type == BOOL) { var->data->boolean = trueOrFalse(temp); }
            else if (type == NUM) { var->data->num = atof(temp); }
            free(temp);
        }
        *var->type = type;
    } 
    else if (*var->type == type) { /* do nothing, as you cannot convert to the same type, fucknuts */ } 
    else { cry("invalid variable type.\n"); }
}

void setVar(variable *var, int type, char* value, double num, int boolean) {
    char *val = NULL;
    if (type == IN) { /* let the user type whatever bullshit is on their minds */
        type = STR; val = grabUserInput(100);
    } else if (type == STR) { 
        val = stroustrup(value); 
    }
    set_variable_value(var, type, val, num, boolean);
    if (val) free(val);
}

void standardMath(openFile *current, char **arguments, char operation) {
    double op1 = 0, op2 = 0, result = 0; int returnType = grabType(arguments[0]); char *numStr = NULL;
    variable *var1 = searchHashMap(&current->variables, arguments[1]), *var2 = searchHashMap(&current->variables, arguments[2]); 
    if (var1 == NULL) { var1 = create_variable(); addItemToMap(&current->variables, var1, arguments[1], (void(*)(void *))freeVariable); *var1->type = NUM; }
    op1 = numFromVar(var1);
    if (var2 == NULL) { op2 = coerceStringToNum(arguments[2]); }
    else { op2 = numFromVar(var2); }
    DEBUG_PRINTF("%f %c %f\n", op1, operation, op2);
    switch (operation) {
        case '+': result = op1 + op2; break;
        case '-': result = op1 - op2; break;
        case '*': result = op1 * op2; break;
        case '/': if (op2 == 0.0) { handleError("div by zero", 49, 0, current); } else{ result = op1 / op2; } break;
        default: op1 = 0;
    }
    DEBUG_PRINTF("%f\n", result);
    if (returnType == STR) numStr = grabStringOfNumber(result);
    set_variable_value(var1, returnType, numStr, result, (result ? 1 : 0));
    if (numStr) free(numStr);
}

void variableSet(openFile *current, char **arguments, int argumentCount) {
    int type = grabType(arguments[0]); char *concatenated = NULL;
    if (arguments[1][0] == '$') handleError("name is reserved", 99, 0, current);
    if (type == STR) concatenated = joinStringsSentence(arguments, argumentCount, 2);
    switch (type) {
        case IN: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, NULL, 0, 0); break;
        case STR: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, concatenated, 0.0, 0); break;
        case NUM: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, NULL, atof(arguments[2]), 0); break; 
        case BOOL: setVar(createVarIfNotFound(&current->variables, arguments[1]), type, NULL, 0.0, trueOrFalse(arguments[2])); break;
        default: handleError("invalid type specification", 30, 0, current);
    }
    free(concatenated);
}

void grabTypeFromVar(variable check, variable *var) {
    switch (*check.type) {
        case NUM: set_variable_value(var, STR, "num", 0.0, 0); break;
        case BOOL: set_variable_value(var, STR, "bool", 0.0, 0); break;
        case STR: set_variable_value(var, STR, "str", 0.0, 0); break;
        default: set_variable_value(var, STR, "none", 0.0, 0); break;
    }
}

void compareNums(HashMap *varMap, char **arguments, char operation) {
    variable *var1 = searchHashMap(varMap, arguments[1]), *var2 = searchHashMap(varMap, arguments[2]);
    double operand1 = 0, operand2 = 0; int result = 0, returnType = grabType(arguments[0]); char *boolStr;
    if (var1 == NULL) { var1 = create_variable(); addItemToMap(varMap, var1, arguments[1], (void(*)(void *))freeVariable); *var1->type = NUM; }
    operand1 = numFromVar(var1); 
    if (var2 == NULL) { operand2 = atof(arguments[2]); }
    else { operand2 = numFromVar(var2); }
    switch (operation) {
        case '>': result = (operand1 > operand2); break;
        case ']': result = (operand1 >= operand2); break;
        case '<': result = (operand1 < operand2); break;
        case '[': result = (operand1 <= operand2); break;
        default: result = 0; break;
    }
    if (returnType == STR) boolStr = result ? "true" : "false"; /* it's just gonna get strdup'd anyways */
    set_variable_value(var1, returnType, boolStr, (double)result, result);
}

void compareBools(HashMap *varMap, char **arguments, char operation, char flip) {
    variable *var1 = searchHashMap(varMap, arguments[1]), *var2 = searchHashMap(varMap, arguments[2]);
    int operand1 = 0, operand2 = 0, result = 0, returnType = grabType(arguments[0]); char *boolStr = NULL;
    if (var1 == NULL) { var1 = create_variable(); addItemToMap(varMap, var1, arguments[1], (void(*)(void *))freeVariable); *var1->type = BOOL; } 
    operand1 = boolFromVar(var1); 
    if (var2 == NULL) { operand2 = coerceStringToBool(arguments[2]); }
    else { operand2 = boolFromVar(var2); }
    switch (operation) {
        case '&': result = (operand1 && operand2); break;
        case '|': result = (operand1 || operand2); break;
        case '!': result = (operand1 != operand2); break;
        default: result = 0; break;
    }
    if (flip) result = !result;
    if (returnType == STR) boolStr = result ? "true" : "false";
    set_variable_value(var1, returnType, boolStr, (double)result, result);
}

int areTwoVarsEqual(variable *var1, variable *var2) {
    if (*var1->type == STR) { 
        char *temp = stringFromVar(var2); int ret = 0; 
        if (temp == NULL) { return 0; } if (var1->data->str == NULL) { free(temp); return 0; }
        ret = strcmp(var1->data->str, temp) ? 0 : 1; /* it shall only return true IF the comparison is true, which for strcmp is zero, oddly enoguh */
        free(temp);
        return ret; 
    }
    else if (*var1->type == NUM && var1->data->num == numFromVar(var2)) { return 1; }
    else if (*var1->type == BOOL && var1->data->boolean == boolFromVar(var2)) { return 1; }
    return 0;
}

/* list ops */
void appendElementToList(list *li, variable *var) {
    variable *new = create_variable(), **tmp;
    varcpy((variable *)new, var);
    tmp = (variable **)realloc(li->variables, (*li->elements + 1) * sizeof(variable *)); /* grow that bitch */
    if (tmp) { li->variables = tmp; } else { cry("reallocation failed!\n"); }
    li->variables[*li->elements] = new; *li->elements += 1;
}

void removeElementFromList(list *li, int element) {
    variable **tmp; size_t reallocSize = (*li->elements - 1) * sizeof(variable *);
    freeVariable(li->variables[element]);
    memmove(li->variables + element, li->variables + element + 1, (*li->elements - element - 1) * sizeof(variable *));
    if (reallocSize == 0) reallocSize = sizeof(variable *);
    tmp = (variable **)realloc(li->variables, reallocSize); /* shrink that bitch */
    if (tmp) { li->variables = tmp; } else { cry("reallocation failed!\n"); }
    *li->elements -= 1;
}

char *formatList(list li) {
    char *final; int i; size_t bytes = 3;
    for (i = 0; i < *li.elements; i++) {
        bytes += stringLenFromVar(*li.variables[i]) + 2; 
        if (*li.variables[i]->type == STR) { bytes += 2; } /* quotes */
    }
    final = (char *)malloc(bytes); if (final == NULL) cry("List Formatting failed!");
    final[0] = '\0';

    strcat(final, "[");
    for (i = 0; i < *li.elements; i++) {
        char *temp = stringFromVar(li.variables[i]);
        if (temp) {
            if (*li.variables[i]->type == STR) strcat(final, "\"");
            strcat(final, temp);
            if (*li.variables[i]->type == STR) strcat(final, "\"");
            free(temp);
            if (i + 1 != *li.elements) strcat(final, ","); /* make sure no trailing comma is left */
        }
    }
    strcat(final, "]");
    return final;
}

void unFormatList(list *li, char *string) {
    int type = 0, i, j, start = 0; variable *var = create_variable();
    while (1) { if (string[start] == '[') { break; } start += 1; }
    for (i = start; i < (int)strlen(string); i++) {
        char *value, c = string[i]; int length = 0;
        if (c == ']') break; 
        if (c == '[' || c == ',') continue;
        if (c == '"') { type = STR; continue; }
        if (*var->type == STR && var->data->str != NULL) { free(var->data->str); }
        if (type != STR) { if (isdigit(c)) { type = NUM; } else { type = BOOL; }}

        while ((c = string[i + length]) != ',' && (c = string[i + length]) != '"' && (c = string[i + length]) != '[' && (c = string[i + length]) != ']') { length += 1; DEBUG_PRINT(&c); }

        value = (char *)calloc(length + 1, sizeof(char));
        for (j = 0; j < length; j++) { value[j] = string[i + j]; }
        i += length; 
        value[length] = '\0';
        DEBUG_PRINT(value);

        *var->type = type;
        if (type == NUM) { var->data->num = atof(value); }
        else if (type == STR) { var->data->str = stroustrup(value); }
        else if (type == BOOL) { var->data->boolean = trueOrFalse(value); }
        appendElementToList(li, var); type = 0; free(value);
    }
    freeVariable(var);
}

void loadList(HashMap *listMap, char *name, char *path) {
    list *new = searchHashMap(listMap, name); char *temp = readFile(path);
    if (new != NULL) { int i; for (i = 0; i < *new->elements; i++) { freeVariable(new->variables[i]); } *new->elements = 0; new->variables = (variable **)realloc(new->variables, sizeof(variable *)); } /* to preserve it for aliases */
    else { new = (list *)calloc(1, sizeof(list)); new->elements = (int *)calloc(1, sizeof(int)); addItemToMap(listMap, new, name, (void (*)(void *))freeList); }
    unFormatList(new, temp); free(temp);
}

variable *indexList(openFile *current, list *li, char *indexArg) {
    int index; variable *src = searchHashMap(&current->variables, indexArg);
    if (src != NULL) { index = numFromVar(src); } else { index = atoi(indexArg); }
    if (index > *li->elements || index < 1) handleError("invalid list index", 20, 0, current);
    return (li->variables[index - 1]);
}

void listAppendConstant(list *li, char **arguments, int argumentCount) {
    int type = grabType(arguments[1]);
    variable *var = create_variable(); *var->type = type;
    if (type == NUM) { var->data->num = coerceStringToNum(arguments[2]); }
    else if (type == BOOL) { var->data->boolean = coerceStringToBool(arguments[2]); }
    else if (type == STR) { var->data->str = joinStringsSentence(arguments, argumentCount, 2); }
    appendElementToList(li, var); freeVariable(var);
}

void listUpdateConstant(openFile *current, list *li, char **arguments, int argumentCount) {
    char *sentence = joinStringsSentence(arguments, argumentCount, 3); 
    variable *var = create_variable();
    set_variable_value(var, grabType(arguments[2]), sentence, atof(arguments[3]), trueOrFalse(arguments[3]));  
    free(sentence); varcpy(indexList(current, li, arguments[1]), var); freeVariable(var);
}

void setAlias(openFile *current, list *src, char *name) {
    list *new;
    if (!src) handleError("invalid list target", 27, 0, current);
    if (name[0] == '$') handleError("name is reserved", 99, 0, current);
    new = searchHashMap(&current->lists, name);
    if (new != NULL && !new->isAlias) { handleError("cannot convert a list to alias", 37, 0, current); }
    else if (new == NULL) new = (list *)calloc(1, sizeof(list)); 
    new->variables = src->variables; new->elements = src->elements;
    new->isAlias = 1; addItemToMap(&current->lists, new, name, (void(*)(void *))freeList);
}

/* file i/o */
char *readFile(char path[]) {
    FILE *file = fopen(path, "r"); char *contents = NULL; long length;
    if (file == NULL) cry("cannot open le file!");
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file); 
    contents = (char *)calloc(length + 1, sizeof(char));
    fread(contents, 1, length, file);
    contents[length] = '\0';
    fclose(file);
    DEBUG_PRINT(contents);
    return contents;
}

void writeFile(char *path, char *value) {
    FILE *file = fopen(path, "w");
    if (file == NULL) { cry("failed to write to file!"); }
    fprintf(file, "%s", value);
    fclose(file);
}

void freeAndWrite(char *path, char *value) { 
    writeFile(path, value); free(value); 
}