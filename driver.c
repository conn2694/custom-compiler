#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<../src/tree.h>
#include<../src/strtab.h>



int main() {

    /* If parse was a success */
    if (!yyparse()){ 
        

        printAst(ast, 1);


        exportASM(ast, 1);
        /*
        printf("\n\nSYMBOL TABLE:\n");
        for(int i = 0; i < MAXIDS; i++){
            if(strTable[i].id != NULL){
                output_entry(i);
                printf("\n");
            }
        }
        */

    }
    return 0;
}
