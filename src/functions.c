#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "strings.h"
#include "runtime.h"
#include "functions.h"
#include "variables.h"

void registerFunction(openFile *caller, char **arguments, int argumentCount) {
    int i; function *new; 
    if (arguments[0][0] == '$') handleError("name is reserved", 99, 0, caller);
    if (argumentCount < 2) handleError("too little arguments", 57, 0, caller);
    if (searchHashMap(&caller->functions, arguments[0]) != NULL) handleError("redefinition of function", 57, 0, caller);
    new = (function *)malloc(sizeof(function));
    new->parameterCount = atoi(arguments[1]);
    new->start = caller->programCounter; 
    for (i = caller->programCounter + 1; i < caller->instructionCount; i++) { 
        if (strcmp(caller->instructions[i]->operation, "fun") == 0) { free(new); handleError("cannot define function within function", 84, 0, caller); }
        if (strcmp(caller->instructions[i]->operation, "end") == 0) { new->end = i; break; }
    }
    if (i == caller->instructionCount) { free(new); handleError("no end to function", 84, 0, caller); }
    addItemToMap(&caller->functions, new, arguments[0], free); caller->programCounter = new->end;
}

void executeFunction(openFile *caller, char **arguments, int argumentCount) { /* monolith */
    int returnSpot = caller->programCounter, i, varCount = 0, listCount = 0, varIndex = 0, listIndex = 0;  /* ahhhh yes, we love ANSI "no mixed code with declarations!" C. */
    function *func = searchHashMap(&caller->functions, arguments[0]);
    /* we need to ensure that even in functions in function, it will retain the data till we ACTUALLY need to be rid of it                                          */
    /* this, however, comes at the cost of performance. however, this performance penalty is made up for in the lack of need to modify other aspects of the code.   */
    char **varNames, **listNames, *types, *funcName = arguments[0]; variable **varPtrs; list **listPtrs;
    if (func == NULL) handleError("invalid function", 38, 0, caller);
    DEBUG_PRINT(funcName);
    if (argumentCount < (func->parameterCount * 2) + 1) handleError("too little arguments", 57, 0, caller);
    if (func->parameterCount > 0) {
        char **tempNames = (char **)malloc(sizeof(char *) * func->parameterCount);
        types = (char *)malloc(sizeof(char) * func->parameterCount);
        for (i = 0; i < func->parameterCount; i++) {
            char *varName, *temp, varType = tolower(arguments[i * 2 + 1][0]);
            temp = grabStringOfNumber((double)i + 1.0f);
            varName = (char *)calloc(strlen(temp) + 2, sizeof(char));
            varName[0] = '$'; strcat(varName, temp);
            free(temp); tempNames[i] = varName; types[i] = varType;
            if (varType == 'l' || varType == 'a') { listCount += 1;}
            else if (varType == 'v' || varType == 's' || varType == 'b' || varType == 'n' || varType == 'p') { varCount += 1; }
            else { free(tempNames); handleError("invalid type specification", 30, 0, caller); }
        }    
        if (listCount > 0) { listNames = (char **)malloc(sizeof(char *) * listCount); listPtrs = (list **)calloc(listCount, sizeof(list *)); } 
        if (varCount > 0) { varNames = (char **)malloc(sizeof(char *) * varCount); varPtrs = (variable **)calloc(varCount, sizeof(variable *)); }
        for (i = 0; i < func->parameterCount; i++) { 
            if (types[i] == 'l' || types[i] == 'a') { 
                list *src = searchHashMap(&caller->lists, arguments[i * 2 + 2]), *test, *li = (list *)calloc(1, sizeof(list));
                listNames[listIndex] = tempNames[i]; 
                if (src == NULL) { free(tempNames); free(listNames); if (varNames) { free(varNames); } handleError("list expected", 26, 0, caller); }
                if (types[i] == 'l') { li->elements = (int *)calloc(1, sizeof(int)); listcpy(li, src); }
                else { li->variables = src->variables; li->isAlias = 1; li->elements = src->elements; }
                listPtrs[listIndex] = li;
                test = searchHashMap(&caller->lists, listNames[listIndex]);
                if (test != NULL) { deleteItemFromMap(&caller->lists, listNames[listIndex]); } 
                addItemToMap(&caller->lists, li, listNames[listIndex], NULL); 
                listIndex += 1;
            } else { /* since we check types earlier it's safe to assume any non-'l' type will be a var */
                variable *newVar = create_variable(), *old, *test;
                varNames[varIndex] = tempNames[i]; varPtrs[varIndex] = newVar;
                switch (types[i]) {
                    case 'v': old = searchHashMap(&caller->variables, arguments[i * 2 + 2]); if (old == NULL) { free(tempNames); freeVariable(newVar); if (listNames) { free(listNames); } free(varNames); handleError("var expected", 26, 0, caller); } varcpy(newVar, old); break;
                    case 'n': *newVar->type = NUM; newVar->data->num = atof(arguments[i * 2 + 2]); break;
                    case 's': *newVar->type = STR; newVar->data->str = stroustrup(arguments[i * 2 + 2]); break;
                    case 'b': *newVar->type = BOOL; newVar->data->boolean = trueOrFalse(arguments[i * 2 + 2]); break;
                    case 'p': old = searchHashMap(&caller->variables, arguments[i * 2 + 2]); if (old == NULL) { free(tempNames); freeVariable(newVar); if (listNames) { free(listNames); } free(varNames); handleError("var expected", 26, 0, caller); } free(newVar->data); free(newVar->type); newVar->data = old->data; newVar->type = old->type; newVar->isPtr = 1; break;
                }
                test = searchHashMap(&caller->variables, varNames[varIndex]);
                if (test != NULL) { /* there shouldn't be any other $i vars */
                    if (test->data->str != NULL && *test->type == STR) free(test->data->str);
                    deleteItemFromMap(&caller->variables, varNames[varIndex]); 
                }
                addItemToMap(&caller->variables, newVar, varNames[varIndex], NULL);
                varIndex += 1;
            }
        }
        free(tempNames); free(types);
    }
    caller->programCounter = func->start + 1;
    while (strcmp(caller->instructions[caller->programCounter]->operation, "ret")) {
        for (i = 0; i < varCount; i++) { if (searchHashMap(&caller->variables, varNames[i]) == NULL) { addItemToMap(&caller->variables, varPtrs[i], varNames[i], NULL); }} /* re-adds any missing variables, should another function have prematurely deleted/overwritten it */
        for (i = 0; i < listCount; i++) { if (searchHashMap(&caller->lists, listNames[i]) == NULL) { addItemToMap(&caller->lists, listPtrs[i], listNames[i], NULL); }}
        executeInstruction(caller); caller->programCounter += 1;
        if (caller->programCounter == func->end) caller->programCounter = func->start + 2;
    }
    arguments = caller->instructions[caller->programCounter]->arguments; argumentCount = caller->instructions[caller->programCounter]->argumentCount; /* grab the current arguments */
    if (argumentCount >= 2) {
        char *retName = (char *)calloc(strlen(funcName) + 2, sizeof(char)), returnType = tolower(arguments[0][0]); 
        retName[0] = '$'; strcat(retName, funcName);
        if (returnType != 'l') {
            variable *returnedVar = create_variable(), *old;
            switch (returnType) {
                case 'v': old = searchHashMap(&caller->variables, arguments[1]); if (old == NULL) { freeVariable(returnedVar); handleError("var expected", 229, 0, caller); } varcpy(returnedVar, old); break;
                case 'n': *returnedVar->type = NUM; returnedVar->data->num = (double)atof(arguments[1]); break;
                case 's': *returnedVar->type = STR; returnedVar->data->str = stroustrup(arguments[1]); break;
                case 'b': *returnedVar->type = BOOL; returnedVar->data->boolean = trueOrFalse(arguments[1]); break;
                default: freeVariable(returnedVar); handleError("invalid type specification", 30, 0, caller); break;
            }
            old = searchHashMap(&caller->variables, retName); 
            if (old) { freeVariable(old); old = returnedVar; }
            else { addItemToMap(&caller->variables, returnedVar, retName, (void(*)(void *))freeVariable); };
        } else {
            list *src = searchHashMap(&caller->lists, arguments[1]), *returned = (list *)calloc(1, sizeof(list)), *old;
            if (src == NULL) { free(returned); handleError("list expected", 26, 0, caller); }
            listcpy(returned, src);
            old = searchHashMap(&caller->lists, retName);
            if (old) { freeList(old); old = returned; }
            else { addItemToMap(&caller->lists, returned, retName, (void(*)(void *))freeList); }
        }
        free(retName);
    }
    for (i = 0; i < varCount; i++) { deleteItemFromMap(&caller->variables, varNames[i]); free(varNames[i]); freeVariable(varPtrs[i]); }
    for (i = 0; i < listCount; i++) { deleteItemFromMap(&caller->lists, listNames[i]); free(listNames[i]); freeList(listPtrs[i]); }
    if (varCount > 0) { free(varNames); free(varPtrs); } if (listCount > 0) { free(listNames); free(listPtrs); }
    caller->programCounter = returnSpot; /* resume execution */
}