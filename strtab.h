#ifndef STRTAB_H
#define STRTAB_H
#define MAXIDS 1000

enum dataType {INT_TYPE, CHAR_TYPE, VOID_TYPE};
enum symbolType {SCALAR, ARRAY, FUNCTION};

typedef struct argInfo{
    int data_type;
    int symbol_type;
};

typedef struct param{
    int data_type;
    int symbol_type;
    struct param* next;
} param;

typedef struct strEntry{
    char* id;
    char* scope;
    int   data_type;
    int   symbol_type;
    int   return_type;
    int   parameters;
    struct argInfo   argTypes[10];
    int   size; //Num elements if array, num params if function
    param*  params;
} symEntry;


struct strEntry strTable[MAXIDS];

char* scope;

int ST_insert(char *id, char* scope, int data_type, int symbol_type);

int ST_insert_func(char *id, char *scope, int return_type, int parameters);


int ST_insert_size(char *id, char *scope, int data_type, int symbol_type, int size);

int ST_lookup(char *id, char *scope);

int ST_lookup_type(char *id);

int ST_lookup_size(char *id);

int ST_lookup_sym_type(char *id);

void print_sym_tab();

void add_param(int data_type, int symbol_type);

void connect_params(int i, int num_params);

char* get_symbol_id(int i);

void new_scope();
void up_scope();

#endif
