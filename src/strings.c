#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "runtime.h"

char *stroustrup(const char *string) {
    int len = strlen(string) + 1;
    char *final = (char *)malloc(len * sizeof(char));
    if (!final) return NULL;
    memcpy(final, string, len);
    return final;
}

void freeAndPrint(char *allocated) { if (allocated != NULL) { printf("%s", allocated); free(allocated); }}
char *stripSemicolon(char *input) { int position; char *string = stroustrup(input); if (strlen(input) == 0) { return string; } position = (int)strlen(string) - 1; if (string[position] == ';') string[position] = '\0'; return string; }
char *lowerize(char *input) { int i, len; char *string = stroustrup(input); if (strlen(input) == 0) { return string; } len = (int)strlen(string); for (i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); } return string; }
void lowerizeInPlace(char *string) { int i, len = (int)strlen(string); for (i = 0; i < len; i++) { string[i] = (char)tolower(string[i]); }}
void stripSemicolonInPlace(char *string) { int i, len = (int)strlen(string); for (i = 0; i < len; i++) { if (string[i] == ';') { string[i] = '\0'; }}}

char *grabStringOfNumber(double num) {
    int i, len, zeroes = 0; char buffer[331], *final;
    sprintf(buffer, "%f", (float)num); /* gatta love that unsafety hehehe... it'll be fine */ 
    len = strlen(buffer);
    for (i = 1; i < 7; i++) { if (buffer[len - i] == '0') { zeroes++; buffer[len - i] = '\0'; } else break; } /* yoink trailing zeroes */
    if (zeroes == 6) { buffer[len - 7] = '\0'; } /* terminate it if it's just trailing zeroes */
    final = (char *)malloc(strlen(buffer) * sizeof(char) + 1);
    strcpy(final, buffer);
    return final;
}

size_t grabLengthOfNumber(double num) { 
    char *temp = grabStringOfNumber(num); 
    size_t len = strlen(temp); 
    free(temp); return len; 
}

void formatEscapes(char *string) {
    int i, sizeOf = strlen(string);
    for (i = 0; i < sizeOf - 1; i++) { 
        if (string[i] == '\\') {
            char insertion = '\0'; 
            switch(tolower((unsigned char)string[i + 1])) {
                case 'r': insertion = '\r'; break;
                case 'n': insertion = '\n'; break;
                case 't': insertion = '\t'; break;
                case '\\': insertion = '\\'; break;
                default: continue; /* just in case there's no actual escape sequence */
            }
            string[i] = insertion;
            memmove(string + i + 1, string + 2 + i, (sizeOf - i - 1));
            string[sizeOf] = '\0'; 
            sizeOf -= 1;
        }
    } 
}

char *joinStringsSentence(char **strings, int stringCount, int offset) {
    char *finalString = NULL; int i, sizeOf = 0;
    if (stringCount == 1) { finalString = stroustrup(strings[0]); formatEscapes(finalString); return finalString; }
    for (i = offset; i < stringCount; i++) { sizeOf += strlen(strings[i]) + 1;}
    finalString = (char *)calloc(sizeOf + 1, sizeof(char)); if (finalString == NULL) cry("unable to string\nplease do not the string\n"); 
    for (i = offset; i < stringCount; i++) { strcat(finalString, strings[i]); if (i + 1 < stringCount) { strcat(finalString, " "); }} /* fuck yo optimization */
    formatEscapes(finalString);
    return finalString;
}

char *buildStringFromInstruction(instruction *inst) {
    char *joined;
    if (inst->prefix != NULL) { char **temp; temp = (char **)malloc(sizeof(char *) * 2); temp[0] = inst->prefix; temp[1] = inst->operation; joined = joinStringsSentence(temp, 2, 0); free(temp); }
    else { joined = stroustrup(inst->operation); }
    return joined;
}

char *unParseInstructions(instruction *inst) {
    int i; size_t size = strlen(inst->operation) + 1; char *final;
    for (i = 0; i < inst->argumentCount; i++) { size += strlen(inst->arguments[i]) + 1; }
    if (inst->prefix != NULL) { size += strlen(inst->prefix) + 1; }
    size += 2; /* nullterm and semicolon ofc */
    final = (char *)calloc(size, sizeof(char)); if (!final) return NULL;
    if (inst->prefix != NULL) { strcat(final, inst->prefix); strcat(final, " "); } 
    strcat(final, inst->operation); 
    for (i = 0; i < inst->argumentCount; i++) {
        strcat(final, " "); /* space between items ofc */
        strcat(final, inst->arguments[i]);
    }
    strcat(final, ";\0");
    return final;
}

char *grabUserInput(const int maxSize) {
    char *val = calloc(maxSize + 1, sizeof(char)); 
    if (!val) return NULL;
    if(!fgets(val, maxSize, stdin)) { free(val); return NULL; }
    val[strcspn(val, "\n")] = '\0'; /* compensate for the newline by fucking yeeting it out of existence */
    DEBUG_PRINT(val);
    return val;
}

void strip(char *string, char character) {
    int i, len = strlen(string);
    for (i = 0; i < len; i++) {
        if (string[i] == character) {
            memmove(string + i, string + 1 + i, len - i);
            len -= 1; i--;
        }
    }
} 

int isWhitespace(char check) {
    if (isspace(check) || check == '\0') return 1; /* same thang but also checks nullterms */
    return 0;
}

char **stringSlicer(char *string, int *elementCount) { /* strtok? what the fuck is that? sounds dangerous, no thanks */
    int len = strlen(string), offset = 0, i, tokens = 0, currentToken = 0; char **arr = NULL;
    while (isWhitespace(string[offset])) { if (string[offset] == '\0') { return NULL; } offset += 1; } /* skip all beginning whitespace, and bail if it's a blank line */
    for (i = offset; i <= len; i++) {
        if (isWhitespace(string[i])) tokens += 1; /* if there's whitespace, it must be the end of a token */
        if (string[i] != '\0') { int prev = i; while (isWhitespace(string[i]) && i <= len) { i++; } if (prev != i) { i--; }} /* keep goin till we hit another real token, then rewind one back since i will increase agains */
    }
    if (!tokens) { DEBUG_PRINTF("\"%s\"\n", string); return NULL; }
    arr = (char **)calloc(tokens, sizeof(char *));
    DEBUG_PRINTF("token count: %d\n", tokens);
    while (offset != len && currentToken < tokens) {
        int tokenLen = 0;
        for (i = offset; i < len; i++) { if (isWhitespace(string[i])) { offset += 1; } else {break;}}
        for (i = offset; i < len; i++) { if (!isWhitespace(string[i])) { tokenLen += 1; } else {break;}}
        DEBUG_PRINTF("allocating for token %d\n", currentToken);
        arr[currentToken] = (char *)malloc(tokenLen + 1);
        memcpy(arr[currentToken], string + offset, tokenLen);
        arr[currentToken][tokenLen] = '\0';
        offset += tokenLen + 1;
        DEBUG_PRINT(arr[currentToken]);
        currentToken += 1;
    }
    *elementCount = tokens;
    return arr;
}

int readFileToAndIncludingChar(FILE* file, char character) { /* verbose, much? */
    char currentChar = 0; int count = 0;
    while (currentChar != character) { 
        currentChar = (char)fgetc(file); 
        if (!feof(file)) { count += 1; } else {break;} 
        if (currentChar == '\n' || currentChar == '\r') { DEBUG_PRINT("\\n"); } 
        else { DEBUG_PRINTF("%c", currentChar); }} /* push the pointer forwards */
    return count;
}