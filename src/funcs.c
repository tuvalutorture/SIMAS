#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "strings.h"
#include "runtime.h"
#include "funcs.h"
#include "vars.h"
#include "hashmap.h"

extern void fun_fun(openFile *file);
extern void fun_end(openFile *file);

void freeFunc(function *func) {
    free(func->retName);
    free(func);
}

void registerFunction(openFile *caller, char **arguments, int argumentCount) {
    int i; function *new; 
    if (arguments[0][0] == '$') handleError("name is reserved", 99, 0, caller);
    if (argumentCount < 2) handleError("too little arguments", 57, 0, caller);
    if (searchHashMap(&caller->functions, arguments[0]) != NULL) handleError("redefinition of function", 57, 0, caller);
    new = (function *)malloc(sizeof(function));
    new->parameterCount = atoi(arguments[1]);
    new->start = caller->programCounter;
    new->retName = (char *)calloc(strlen(arguments[0]) + 2, sizeof(char));
    new->retName[0] = '$'; strcat(new->retName, arguments[0]);
    for (i = caller->programCounter + 1; i < caller->instructionCount; i++) { 
        if (caller->instructions[i].op->functionPointer == (void(*)(void*))fun_fun) { free(new); handleError("cannot define function within function", 84, 0, caller); return; }
        if (caller->instructions[i].op->functionPointer == (void(*)(void*))fun_end) { new->end = i; break; }
    }
    if (i == caller->instructionCount) { free(new); handleError("no end to function", 84, 0, caller); return; }
    addItemToMap(&caller->functions, new, arguments[0], (void(*)(void*))freeFunc); caller->programCounter = new->end;
}

void callFunction(openFile *caller, char **arguments, int argumentCount) {
    function *func = searchHashMap(&caller->functions, arguments[0]); int i;
    if (argumentCount < (func->parameterCount * 2) + 1) handleError("too little arguments", 57, 0, caller);
    for (i = func->parameterCount; i > 0; i--) {
        list *newList, *sourceList = findList(caller, arguments[i * 2]); variable *newVar, *sourceVar = findVariable(caller, arguments[i * 2]); void *data;
        switch (tolower(*arguments[i * 2 - 1])) {
            case 'v':
                if (!sourceVar) { handleError("var expected", 26, 0, caller); return; }
                newVar = create_variable(); varcpy(newVar, sourceVar); data = newVar; break;
            case 'p':
                if (!sourceVar) { handleError("var expected", 26, 0, caller); return; }
                newVar = createPointer(sourceVar); data = newVar; break;
            case 'l':
                if (!sourceList) { handleError("list expected", 26, 0, caller); return; }
                newList = create_list(); listcpy(newList, sourceList); data = newList; break;
            case 'a':
                if (!sourceList) { handleError("list expected", 26, 0, caller); return; }
                newList = createAlias(sourceList); data = newList; break;
            case 's':
                newVar = create_variable(); *newVar->type = STR; newVar->data->str = stroustrup(arguments[i * 2]);
                data = newVar; break;
            case 'b':
                newVar = create_variable(); *newVar->type = BOOL; newVar->data->boolean = trueOrFalse(arguments[i * 2]);
                data = newVar; break;
            case 'n':
                newVar = create_variable(); *newVar->type = NUM; newVar->data->num = (double)coerceStringToNum(arguments[i * 2]);
                data = newVar; break;
            default: handleError("invalid type specification", 30, 0, caller); return;
        }
        push(&caller->stack, data);
    }
    func->callingArgs = arguments;
    func->returnSpot = caller->programCounter;
    caller->programCounter = func->start;
    push(&caller->stack, func);
}

void returnFunction(openFile *caller, char **arguments, int argumentCount) {
    function *func = pop(&caller->stack); int i;
    if (func == NULL) { handleError("invalid return", 30, 0, caller); return; }
    if (argumentCount % 2 != 0 && argumentCount != 0) { handleError("invalid return arguments", 29, 0, caller); return; }
    if (argumentCount == 2) {
        list *newList, *sourceList = findList(caller, arguments[1]); variable *newVar, *sourceVar = findVariable(caller, arguments[1]);;
        switch (tolower(*arguments[0])) {
            case 'v':
                if (!sourceVar) { handleError("var expected", 26, 0, caller); return; }
                newVar = create_variable(); varcpy(newVar, sourceVar);
                addItemToMap(&caller->variables, newVar, func->retName, (void(*)(void *))freeVariable); break;
            case 'l':
                if (!sourceList) { handleError("list expected", 26, 0, caller); return; }
                newList = create_list(); listcpy(newList, sourceList);
                addItemToMap(&caller->lists, newList, func->retName, (void(*)(void *))freeList); break;
            case 's':
                newVar = create_variable();
                *newVar->type = STR; newVar->data->str = stroustrup(arguments[1]);
                addItemToMap(&caller->variables, newVar, func->retName, (void(*)(void *))freeVariable); break;
            case 'b':
                newVar = create_variable(); *newVar->type = BOOL; newVar->data->boolean = trueOrFalse(arguments[1]);
                addItemToMap(&caller->variables, newVar, func->retName, (void(*)(void *))freeVariable); break;
            case 'n':
                newVar = create_variable(); *newVar->type = NUM; newVar->data->num = (double)coerceStringToNum(arguments[1]);
                addItemToMap(&caller->variables, newVar, func->retName, (void(*)(void *))freeVariable); break;

            default: handleError("invalid type specification", 30, 0, caller); return;
        }
    }

    for (i = 0; i < func->parameterCount; i++) {
        if (func->callingArgs[i * 2 + 1][0] == 'l' || func->callingArgs[i * 2 + 1][0] == 'a') freeList(pop(&caller->stack));
        else freeVariable(pop(&caller->stack));
    }

    caller->programCounter = func->returnSpot;
    func->returnSpot = 0;
}