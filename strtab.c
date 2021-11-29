#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "strtab.h"

int hash(char *str)
   {
       unsigned long hash = 5381;
       int c;

       while (c == *str++)
           hash = ((hash << 5) + hash) + c;

       return ((int)((hash)%MAXIDS));
   }
/*
int ST_insert(char *id, char *scope, int data_type, int symbol_type) {

    char *combineStr = malloc(strlen(id) + strlen(scope) + 1);
    strcpy(combineStr, id);
    strcat(combineStr, scope);

    int hashCode = hash(combineStr);


    strTable[hashCode].id = id;
    strTable[hashCode].scope = scope;
    strTable[hashCode].data_type = data_type;
    strTable[hashCode].symbol_type = symbol_type;

    return hashCode;

}
*/
int ST_insert(char *id, char *scope, int data_type, int symbol_type){
    if(id == NULL) return 0;
    // Concatenate the scope and id and use that to create the hash key
    char* keyStr = (char*) malloc(strlen(scope) + strlen(id) + 1);
    strcpy(keyStr, scope);
    strcat(keyStr, id);
    int index = hash(keyStr) % MAXIDS;
    int start = index;
    free(keyStr);

    // Start at the returned hash and probe until we find an empty slot or the scope id combo.
    while (strTable[index].id != NULL && (strcmp(strTable[index].id, id) != 0 || strcmp(strTable[index].scope, scope) != 0)){
        // Just do basic linear probing for now. Not ideal, but it'll work.
        index = (index + 1)%MAXIDS;
        if(index == start)
        {
            yyerror("Symbol table is full.");
            return -1;
        }
    }
    // If index isn't already used, store string there.
    if (strTable[index].id == NULL){
        strTable[index].id = id;
        strTable[index].scope = scope;
        strTable[index].data_type = data_type;
        strTable[index].symbol_type = symbol_type;
    }
    return index;
}

int ST_insert_func(char *id, char *scope, int return_type, int parameters) {
    if(id == NULL) return 0;
    // Concatenate the scope and id and use that to create the hash key
    char* keyStr = (char*) malloc(strlen(scope) + strlen(id) + 1);
    strcpy(keyStr, scope);
    strcat(keyStr, id);
    int index = hash(keyStr) % MAXIDS;
    int start = index;
    free(keyStr);

    // Start at the returned hash and probe until we find an empty slot or the scope id combo.
    while (strTable[index].id != NULL && (strcmp(strTable[index].id, id) != 0 || strcmp(strTable[index].scope, scope) != 0)){
        // Just do basic linear probing for now. Not ideal, but it'll work.
        index = (index + 1)%MAXIDS;
        if(index == start)
        {
            yyerror("Symbol table is full.");
            return -1;
        }
    }
    // If index isn't already used, store string there.
    if (strTable[index].id == NULL){
        strTable[index].id = id;
        strTable[index].scope = scope;
        strTable[index].return_type = return_type;
        strTable[index].parameters = parameters;
    }
    return index;
}

int ST_insert_size(char *id, char* scope, int data_type, int symbol_type, int size) {
    if(id == NULL) return 0;
    // Concatenate the scope and id and use that to create the hash key
    char* keyStr = (char*) malloc(strlen(scope) + strlen(id) + 1);
    strcpy(keyStr, scope);
    strcat(keyStr, id);
    int index = hash(keyStr) % MAXIDS;
    int start = index;
    free(keyStr);

    // Start at the returned hash and probe until we find an empty slot or the scope id combo.
    while (strTable[index].id != NULL && (strcmp(strTable[index].id, id) != 0 || strcmp(strTable[index].scope, scope) != 0)){
        // Just do basic linear probing for now. Not ideal, but it'll work.
        index = (index + 1)%MAXIDS;
        if(index == start)
        {
            yyerror("Symbol table is full.");
            return -1;
        }
    }
    // If index isn't already used, store string there.
    if (strTable[index].id == NULL){
        strTable[index].id = id;
        strTable[index].scope = scope;
        strTable[index].data_type = data_type;
        strTable[index].symbol_type = symbol_type;
        strTable[index].size = size;

    }
    return index;
}



int ST_lookup(char *id, char *scope) {
    if(id == NULL) return 0;
    // Concatenate the scope and id and use that to create the hash key
    char* keyStr = (char*) malloc(strlen(scope) + strlen(id) + 1);
    strcpy(keyStr, scope);
    strcat(keyStr, id);
    int index = hash(keyStr) % MAXIDS;
    int start = index;
    free(keyStr);

    // Start at index and probe until we either find the scope id or an empty slot.
    while (strTable[index].id != NULL && (strcmp(strTable[index].id, id) != 0 || strcmp(strTable[index].scope, scope) != 0)){
        index = (index + 1)%MAXIDS;
        if(index == start)
        {
            yyerror("Symbol table is full.");
            return -1;
        }
    }

    // If we stopped probing because of a empty slot and were searching locally, search globally.
    if (strTable[index].id == NULL && strcmp(scope,"") != 0){
        index = hash(id) % MAXIDS;
        while (strTable[index].id != NULL && (strcmp(strTable[index].id, id) != 0 || strcmp(strTable[index].scope, "") != 0)){
            index = (index + 1)%MAXIDS;
            // Don't need to worry about a full table here.
            // This search only happens if there's an empty spot.
        }
    }

    if (strTable[index].id == NULL){
        yywarning("undeclared variable");
        index = -1;
    }

    return index;
}

void output_entry(int i) {
    printf("%s", strTable[i].id);    
}

char* get_symbol_id(int i) {
    return strTable[i].id;

}
