#ifndef STRINGS_H
#define STRINGS_H

#include <stdio.h>
#include "runtime.h"

char *stroustrup(const char *string);
void freeAndPrint(char *allocated);
char *stripSemicolon(char *input);
char *lowerize(char *input);
void lowerizeInPlace(char *string);
void stripSemicolonInPlace(char *string);
char *grabStringOfNumber(double num);
size_t grabLengthOfNumber(double num);
void formatEscapes(char *string);
char *joinStringsSentence(char **strings, int stringCount, int offset);
char *buildStringFromInstruction(instruction *inst);
char *unParseInstructions(instruction *inst);
char *grabUserInput(const int maxSize);
void strip(char *string, char character);
int isWhitespace(char check);
char **stringSlicer(char *string, int *elementCount);
int readFileToAndIncludingChar(FILE* file, char character);

#endif