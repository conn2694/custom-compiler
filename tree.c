#include<tree.h>
#include<strtab.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct regInfo { 
    char* var; 
    char* regType; 
    int num;
    int free; /* 0 for no, 1 for yes */
};
const NUM_REG = 15;
struct regInfo reg[NUM_REG] = {
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 },
    { "                    ", " ", 0, 1 } /* this last register is for a0, the int argument */
};

/* string values for ast node types, makes tree output more readable */
char *nodeNames[33] = {"program", "declList", "decl", "varDecl", "typeSpecifier",
                       "funDecl", "formalDeclList", "formalDecl", "funBody",
                       "localDeclList", "statementList", "statement", "compoundStmt",
                       "assignStmt", "condStmt", "loopStmt", "returnStmt","expression",
                       "relop", "addExpr", "addop", "term", "mulop", "factor",
                       "funcCallExpr", "argList", "integer", "identifier", "var",
                       "arrayDecl", "char", "funcTypeName"};

int ifCount = 1;
int whileCount = 1;

char *typeNames[3] = {"int", "char", "void"};
char *ops[10] = {"+", "-", "*", "/", "<", "<=", "==", ">=", ">", "!="};

tree *maketree(int kind) {
      tree *this = (tree *) malloc(sizeof(struct treenode));
      this->nodeKind = kind;
      this->numChildren = 0;
      return this;
}

tree *maketreeWithVal(int kind, int val) {
      tree *this = (tree *) malloc(sizeof(struct treenode));
      this->nodeKind = kind;
      this->numChildren = 0;
      this->val = val;
      return this;
}

void addChild(tree *parent, tree *child) {
      if (parent->numChildren == MAXCHILDREN) {
          printf("Cannot add child to parent node\n");
          exit(1);
      }
      nextAvailChild(parent) = child;
      child->parent = parent;
      parent->numChildren++;
}



void printAst(tree *node, int nestLevel) {
      char* nodeName = nodeNames[node->nodeKind];
      
      if(strcmp(nodeName,"identifier") == 0){
          if(node->val == -1)
              printf("%s,%s\n", nodeName,"undeclared variable");
          else
              printf("%s,%s\n", nodeName,get_symbol_id(node->val));
      }
      else if(strcmp(nodeName,"integer") == 0){
          printf("%s,%d\n", nodeName,node->val);
      }
      else if(strcmp(nodeName,"char") == 0){
          printf("%s,%c\n", nodeName,node->val);
      }
      else if(strcmp(nodeName,"typeSpecifier") == 0){
          printf("%s,%s\n", nodeName,typeNames[node->val]);
      }
      else if(strcmp(nodeName,"relop") == 0 || strcmp(nodeName,"mulop") == 0 || strcmp(nodeName,"addop") == 0){
          printf("%s,%s\n", nodeName,ops[node->val]);
      }
      else{
          printf("%s\n", nodeName);
      }

      int i, j;

      for (i = 0; i < node->numChildren; i++)  {
          for (j = 0; j < nestLevel; j++)
              printf("    ");
          printAst(getChild(node, i), nestLevel + 1);
      }

}

void flattenList(tree *list, tree *subList){
    for(int i=0; i < subList->numChildren; i++){
        addChild(list,getChild(subList,i));
    }
}


/* Will convert to ASM by going through the tree again */
void exportASM(tree *node, int nestLevel) {
    char* nodeName = nodeNames[node->nodeKind];

    /* TODO: Multiplication/Division and register allocation

    /* We can do register allocation by seeing when an identifier is called, look to see if it's used anytime either before it
        is assigned another value, or it's the end of the program, we can then declare the register as dead and use it again */

    /* Will need to be added to for function declarations that take a variable so we can do callie/caller save */
    if (strcmp(nodeName, "funDecl") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");
        fprintf(filePointer, "%s:\n", get_symbol_id(node->children[0]->children[1]->val));

        /* puts information for current argument in a0 register */
        if (strcmp(nodeNames[node->children[1]->nodeKind], "formalDeclList") == 0) {
            reg[NUM_REG-1].regType = "a";
            reg[NUM_REG-1].num = 0;
            reg[NUM_REG-1].var = get_symbol_id(node->children[1]->children[0]->children[1]->val);

        


        }


    }
    /*
    if (strcmp(nodeName, "declList") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");
        fprintf(filePointer, "\t\t\t.text\n\t\t\t.globl main\n");
       
    }
    */
    /* Resets file since it's being run for the first time*/
    if (strcmp(nodeName, "program") == 0) {
        FILE *filePointer = fopen("mips.asm", "wb");
        fprintf(filePointer, "\t\t\t.text\n\t\t\t.globl main\n");
    }

    if (strcmp(nodeName, "varDecl") == 0) {
        for (int i = 0; i < NUM_REG; i++) {
            if (reg[i].free) {
                /* t0 - t9 */
                if (i < 10) {
                    reg[i].regType = "t";
                    reg[i].num = i;
                }
                /* v1 */
                else if (i == 10) {
                    reg[i].regType = "v";
                    reg[i].num = 1;                    
                }
                /* a1 - a3 */
                else if (i > 10) {
                    reg[i].regType = "a";
                    reg[i].num = i-10;                    
                }
                reg[i].var = get_symbol_id(node->children[1]->val);


                reg[i].free = 0;
                break;
            }
        }

    }
    /* so we can do arithmetic with the return statement */
    if (strcmp(nodeName, "returnStmt") == 0 ) {
        FILE *filePointer = fopen("mips.asm", "ab");
        int value = 0;
        int regHold = 0;
        int count = 0;




        tree* iterate = node;
        /* go through until we reach the first one */
        while (strcmp(nodeNames[iterate->nodeKind], "integer") != 0 && strcmp(nodeNames[iterate->nodeKind], "identifier") != 0) {
            iterate = iterate->children[0];
        } 

        /* when we get to expression we're done reading values */
        while(strcmp(nodeNames[iterate->nodeKind], "expression") != 0) {

            /* immediate */
            if (strcmp(nodeNames[iterate->nodeKind], "integer") == 0) {
                fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, iterate->val);
                count += 1;


            }
            /* variable */
            else if (strcmp(nodeNames[iterate->nodeKind], "identifier") == 0) {

                if (strcmp(nodeNames[iterate->parent->nodeKind], "funcCallExpr") == 0) {
                    printf("test");
                }

                for (int i = 0; i < NUM_REG; i++) {
                    if (strcmp(reg[i].var, get_symbol_id(iterate->val)) == 0) {
                        fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                        break;

                    }
                }
                count += 1;
            }

            /* we can get multiplactions before entering the addExpr portion, this takes care of that */
            if (strcmp(nodeNames[iterate->nodeKind], "term") == 0 && iterate->numChildren > 1) {

                /* variable */
                if (strcmp(nodeNames[iterate->children[2]->children[0]->nodeKind], "var") == 0) {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[0]->val)) == 0) {
                            fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                            break;

                        }
                    }
                    count += 1;

                }
                /* immediate */
                else {
                    fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, iterate->children[2]->children[0]->val);
                    count += 1;


                }

                if (strcmp(ops[iterate->children[1]->val], "*") == 0) {
                    fprintf(filePointer, "\t\t\tmul $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                } 

                else if (strcmp(ops[iterate->children[1]->val], "/") == 0) {
                    fprintf(filePointer, "\t\t\tdiv $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                } 
            }
            /* when we get to the addExpr stages, there is an add (or subtraction) on every stage */
            if (strcmp(nodeNames[iterate->nodeKind], "addExpr") == 0 && iterate->numChildren > 1) {
                /* multiplication/division is involved */
                if (iterate->children[2]->numChildren > 1) {
                    int saveCount = count-1; /* so we can do addOp at the end */
                    tree* mulIterate = iterate;
                    mulIterate = iterate->children[2];

                    while (strcmp(nodeNames[mulIterate->nodeKind], "integer") != 0 && strcmp(nodeNames[mulIterate->nodeKind], "identifier") != 0) {
                        mulIterate = mulIterate->children[0];
                    } 

                    while(strcmp(nodeNames[mulIterate->nodeKind], "addExpr") != 0) {

                        /* initial */
                        if (strcmp(nodeNames[mulIterate->nodeKind], "integer") == 0) {
                            fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, mulIterate->val);
                            count += 1;

                        }
                        /* initial is variable */
                        else if (strcmp(nodeNames[mulIterate->nodeKind], "identifier") == 0) {
                            for (int i = 0; i < NUM_REG; i++) {
                                if (strcmp(reg[i].var, get_symbol_id(mulIterate->val)) == 0) {
                                    fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                    break;
                                }
                            }
                            count += 1;
                        }

                        

                        /* following */
                        if (strcmp(nodeNames[mulIterate->nodeKind], "term") == 0 && mulIterate->numChildren > 1) {
                            /* variable */
                            if (strcmp(nodeNames[mulIterate->children[2]->children[0]->nodeKind], "var") == 0) {
                                for (int i = 0; i < NUM_REG; i++) {
                                    if (strcmp(reg[i].var, get_symbol_id(mulIterate->children[2]->children[0]->children[0]->val)) == 0) {
                                        fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                        break;

                                    }
                                }
                                count += 1;
                            }
                            /* immediate */
                            else {
                                fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, mulIterate->children[2]->children[0]->val);
                                count += 1;
                            }


                            if (strcmp(ops[mulIterate->children[1]->val], "*") == 0) {
                                fprintf(filePointer, "\t\t\tmul $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                            } 

                            else if (strcmp(ops[mulIterate->children[1]->val], "/") == 0) {
                                fprintf(filePointer, "\t\t\tdiv $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                            } 
                        }

                        mulIterate = mulIterate->parent;
                    }
                    /* adding the total multiply to the old add */
                    if (strcmp(ops[mulIterate->children[1]->val], "+") == 0) {
                        fprintf(filePointer, "\t\t\tadd $s%d, $s%d, $s%d\n", saveCount+1, count-1, saveCount);
                    } 

                    else if (strcmp(ops[mulIterate->children[1]->val], "-") == 0) {
                        fprintf(filePointer, "\t\t\tsub $s%d, $s%d, $s%d\n", saveCount+1, count-1, saveCount);
                    } 
                    /* we don't need the temp registers we used for the multiplication so we can just reuse where we were in saveCount + 2
                    (since we need 2 for the instruction done just above, as well as another for the next instruction we do) */
                    count = saveCount + 2;

                }
                /* normal addition/subtraction */
                else {
                    /* variable */
                    if (strcmp(nodeNames[iterate->children[2]->children[0]->children[0]->nodeKind], "var") == 0) {
                        for (int i = 0; i < NUM_REG; i++) {
                            if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[0]->children[0]->val)) == 0) {
                                fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                break;

                            }
                        }
                        count += 1;

                    }
                    /* immediate */
                    else {
                        fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, iterate->children[2]->children[0]->children[0]->val);
                        count += 1;
                    }


                    if (strcmp(ops[iterate->children[1]->val], "+") == 0) {
                        fprintf(filePointer, "\t\t\tadd $s%d, $s%d, $s%d\n", count-1, count-1, count-2);
                    } 

                    else if (strcmp(ops[iterate->children[1]->val], "-") == 0) {
                        fprintf(filePointer, "\t\t\tsub $s%d, $s%d, $s%d\n", count-1, count-1, count-2);
                    } 
                }

            }



            iterate = iterate->parent;
        }
        /* final assignment */


        fprintf(filePointer, "\t\t\tmove $v0, $s%d\n", count-1);
        count = 0;



            
        
    }

    if (strcmp(nodeName, "assignStmt") == 0 ) {
        FILE *filePointer = fopen("mips.asm", "ab");
        int value = 0;
        int regHold = 0;
        int count = 0;

        /* calling a function without assigning it */
        if (node->numChildren == 1) {
            if (strcmp(get_symbol_id(node->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val), "output") == 0) {
                printf("near the end");
                if (strcmp(nodeNames[node->children[0]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(node->children[0]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                            fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n \t\t\tli $v0, 1 \n\t\t\tsyscall\n", reg[i].regType, reg[i].num);
                            break;
                        }
                    }
                }
                else {
                fprintf(filePointer, "\t\t\tli $a0, %d\n \t\t\tli $v0, 1 \n\t\t\tsyscall\n", node->children[0]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->val);
                }
                     
            }
            else {


            /* argument */
            if (node->children[0]->children[0]->children[0]->children[0]->children[0]->numChildren > 1) {
                /* variable */
                if (strcmp(nodeNames[node->children[0]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(node->children[1]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n", reg[i].regType, reg[i].num);
                            break;

                        }
                    }
                }
                else {
                    value = node->children[0]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val;
                    fprintf(filePointer, "\t\t\tli $a0, %d\n", value);

                }



            }


            fprintf(filePointer, "\t\t\tjal %s\n", get_symbol_id(node->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val));
            
            }

        }

        else {

            /* simple assign if there's no children of much of anything  */
            if (node->children[1]->children[0]->numChildren == 1 && node->children[1]->children[0]->children[0]->numChildren == 1)  {
                /* assigning from another register */
                if (strcmp(nodeNames[node->children[1]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(node->children[0]->children[0]->val)) == 0) {
                            for (int j = 0; j < NUM_REG; j++) {
                                if (strcmp(reg[j].var, get_symbol_id(node->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                    fprintf(filePointer, "\t\t\tmove $%s%d, $%s%d\n", reg[i].regType, reg[i].num, reg[j].regType, reg[j].num);
                                    break;
                                }
                            }
                            break;
                        }
                    }

                }
                /* assigning from function return value */
                if (strcmp(nodeNames[node->children[1]->children[0]->children[0]->children[0]->children[0]->nodeKind], "funcCallExpr") == 0) {
                    /* argument */
                    if (node->children[1]->children[0]->children[0]->children[0]->children[0]->numChildren > 1) {
                        /* variable */
                        if (strcmp(nodeNames[node->children[1]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                            for (int i = 0; i < NUM_REG; i++) {
                                if (strcmp(reg[i].var, get_symbol_id(node->children[1]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                        fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n", reg[i].regType, reg[i].num);
                                    break;

                                }
                            }
                        }
                        else {
                            value = node->children[1]->children[0]->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val;
                            fprintf(filePointer, "\t\t\tli $a0, %d\n", value);

                        }



                    }
                    fprintf(filePointer, "\t\t\tjal %s\n", get_symbol_id(node->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val));
                    /* final assignment */
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(node->children[0]->children[0]->val)) == 0) {

                            fprintf(filePointer, "\t\t\tmove $%s%d, $v0\n", reg[i].regType, reg[i].num);
                            break;

                        }
                    }
                }


                /* assigning from immediate */
                else {
                    value = node->children[1]->children[0]->children[0]->children[0]->children[0]->val;

                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(node->children[0]->children[0]->val)) == 0) {

                            fprintf(filePointer, "\t\t\tli $%s%d, %d\n", reg[i].regType, reg[i].num, value);
                            break;

                        }

                    }
                }

            }
            /* multiply, add, sub, div */
            else {


                tree* iterate = node;
                /* statement */
                iterate = iterate->children[1];

                /* go through until we reach the first one */
                while (strcmp(nodeNames[iterate->nodeKind], "integer") != 0 && strcmp(nodeNames[iterate->nodeKind], "identifier") != 0) {


                    iterate = iterate->children[0];

                } 


                /* when we get to expression we're done reading values */
                while(strcmp(nodeNames[iterate->nodeKind], "expression") != 0) {

                    /* immediate */
                    if (strcmp(nodeNames[iterate->nodeKind], "integer") == 0) {
                        fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, iterate->val);
                        count += 1;


                    }
                    /* variable */
                    else if (strcmp(nodeNames[iterate->nodeKind], "identifier") == 0) {
                        /* function call */
                        if (strcmp(nodeNames[iterate->parent->nodeKind], "funcCallExpr") == 0) {
                            /* argument */
                            if (iterate->parent->numChildren > 1) {
                                /* variable */
                                if (strcmp(nodeNames[iterate->parent->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                                    for (int i = 0; i < NUM_REG; i++) {
                                        if (strcmp(reg[i].var, get_symbol_id(iterate->parent->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                            fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n", reg[i].regType, reg[i].num);
                                            break;

                                        }
                                    }
                                }
                                /* immediate */
                                else {
                                    value = iterate->parent->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val;
                                    fprintf(filePointer, "\t\t\tli $a0, %d\n", value);

                                }
                            }
                            fprintf(filePointer, "\t\t\tjal %s\n", get_symbol_id(iterate->parent->children[0]->val));
                            fprintf(filePointer, "\t\t\tmove $s%d, $v0\n", count);
                            count += 1;
                            
                            }
                        /* just a normal variable, no function call */
                        else {
                            for (int i = 0; i < NUM_REG; i++) {
                                if (strcmp(reg[i].var, get_symbol_id(iterate->val)) == 0) {
                                    fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                    break;

                                }
                            }
                            count += 1;
                        }


                    }

                    /* we can get multiplactions before entering the addExpr portion, this takes care of that */
                    if (strcmp(nodeNames[iterate->nodeKind], "term") == 0 && iterate->numChildren > 1) {


                        /* variable */
                        if (strcmp(nodeNames[iterate->children[2]->children[0]->nodeKind], "var") == 0) {

                            for (int i = 0; i < NUM_REG; i++) {
                                if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[0]->val)) == 0) {
                                    fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                    break;

                                }
                            }
                            count += 1;

                        }
                        /* function */
                        else if (strcmp(nodeNames[iterate->children[2]->children[0]->nodeKind], "funcCallExpr") == 0) {
                            /* argument */
                            if (iterate->children[2]->children[0]->numChildren > 1) {
                                /* variable */
                                if (strcmp(nodeNames[iterate->children[2]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                                    for (int i = 0; i < NUM_REG; i++) {
                                        if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                            fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n", reg[i].regType, reg[i].num);
                                            break;

                                        }
                                    }
                                }
                                /* immediate */
                                else {
                                    value = iterate->children[2]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val;
                                    fprintf(filePointer, "\t\t\tli $a0, %d\n", value);

                                }
                            }
                            fprintf(filePointer, "\t\t\tjal %s\n", get_symbol_id(iterate->children[2]->children[0]->children[0]->val));
                            fprintf(filePointer, "\t\t\tmove $s%d, $v0\n", count);
                            count += 1;    
                        }
                        /* immediate */
                        else {
                            fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, iterate->children[2]->children[0]->val);
                            count += 1;


                        }

                        if (strcmp(ops[iterate->children[1]->val], "*") == 0) {
                            fprintf(filePointer, "\t\t\tmul $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                        } 

                        else if (strcmp(ops[iterate->children[1]->val], "/") == 0) {
                            fprintf(filePointer, "\t\t\tdiv $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                        } 
                    }
                    /* when we get to the addExpr stages, there is an add (or subtraction) on every stage */
                    if (strcmp(nodeNames[iterate->nodeKind], "addExpr") == 0 && iterate->numChildren > 1) {
                        /* multiplication/division is involved */
                        if (iterate->children[2]->numChildren > 1) {

                            int saveCount = count-1; /* so we can do addOp at the end */
                            tree* mulIterate = iterate;
                            mulIterate = iterate->children[2];

                            while (strcmp(nodeNames[mulIterate->nodeKind], "integer") != 0 && strcmp(nodeNames[mulIterate->nodeKind], "identifier") != 0) {
                                mulIterate = mulIterate->children[0];
                            } 

                            while(strcmp(nodeNames[mulIterate->nodeKind], "addExpr") != 0) {


                                /* initial */
                                if (strcmp(nodeNames[mulIterate->nodeKind], "integer") == 0) {
                                    fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, mulIterate->val);
                                    count += 1;


                                }
                                /* initial is variable */
                                else if (strcmp(nodeNames[mulIterate->nodeKind], "identifier") == 0) {
                                    /* function call */

                                    if (strcmp(nodeNames[mulIterate->parent->nodeKind], "funcCallExpr") == 0) {
                                        
                                        /* argument */
                                        if (mulIterate->parent->numChildren > 1) {
                                            /* variable */
                                            if (strcmp(nodeNames[mulIterate->parent->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                                                for (int i = 0; i < NUM_REG; i++) {
                                                    if (strcmp(reg[i].var, get_symbol_id(mulIterate->parent->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                                        fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n", reg[i].regType, reg[i].num);
                                                        break;

                                                    }
                                                }
                                            }
                                            /* immediate */
                                            else {
                                                value = mulIterate->parent->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val;
                                                fprintf(filePointer, "\t\t\tli $a0, %d\n", value);

                                            }
                                        }
                                        fprintf(filePointer, "\t\t\tjal %s\n", get_symbol_id(mulIterate->val));
                                        /* final assignment */

                                        fprintf(filePointer, "\t\t\tmove $s%d, $v0\n", count);

                                        count += 1;
                                    }
                                    else {
                                    for (int i = 0; i < NUM_REG; i++) {
                                        if (strcmp(reg[i].var, get_symbol_id(mulIterate->val)) == 0) {
                                            fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                            break;
                                        }
                                    }
                                    count += 1;
                                    }
                                }

                                

                                /* following */
                                if (strcmp(nodeNames[mulIterate->nodeKind], "term") == 0 && mulIterate->numChildren > 1) {
                                    /* variable */
                                    if (strcmp(nodeNames[mulIterate->children[2]->children[0]->nodeKind], "var") == 0) {
                                        for (int i = 0; i < NUM_REG; i++) {
                                            if (strcmp(reg[i].var, get_symbol_id(mulIterate->children[2]->children[0]->children[0]->val)) == 0) {
                                                fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                                break;

                                            }
                                        }
                                        count += 1;
                                    }
                                /* function */
                                else if (strcmp(nodeNames[mulIterate->children[2]->children[0]->nodeKind], "funcCallExpr") == 0) {
                                    /* argument */
                                    if (mulIterate->children[2]->children[0]->numChildren > 1) {
                                        /* variable */
                                        if (strcmp(nodeNames[mulIterate->children[2]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                                            for (int i = 0; i < NUM_REG; i++) {
                                                if (strcmp(reg[i].var, get_symbol_id(mulIterate->children[2]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                                    fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n", reg[i].regType, reg[i].num);
                                                    break;

                                                }
                                            }
                                        }
                                        /* immediate */
                                        else {
                                            value = mulIterate->children[2]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val;
                                            fprintf(filePointer, "\t\t\tli $a0, %d\n", value);

                                        }
                                    }
                                    fprintf(filePointer, "\t\t\tjal %s\n", get_symbol_id(mulIterate->children[2]->children[0]->children[0]->val));
                                    fprintf(filePointer, "\t\t\tmove $s%d, $v0\n", count);
                                    count += 1;    
                                }
                                    /* immediate */
                                    else {
                                        fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, mulIterate->children[2]->children[0]->val);
                                        count += 1;
                                    }


                                    if (strcmp(ops[mulIterate->children[1]->val], "*") == 0) {
                                        fprintf(filePointer, "\t\t\tmul $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                                    } 

                                    else if (strcmp(ops[mulIterate->children[1]->val], "/") == 0) {
                                        fprintf(filePointer, "\t\t\tdiv $s%d, $s%d, $s%d\n", count-1, count-1, count-2);

                                    } 
                                }

                                mulIterate = mulIterate->parent;
                            }
                            /* adding the total multiply to the old add */
                            if (strcmp(ops[mulIterate->children[1]->val], "+") == 0) {
                                fprintf(filePointer, "\t\t\tadd $s%d, $s%d, $s%d\n", saveCount+1, count-1, saveCount);
                            } 

                            else if (strcmp(ops[mulIterate->children[1]->val], "-") == 0) {
                                fprintf(filePointer, "\t\t\tsub $s%d, $s%d, $s%d\n", saveCount+1, count-1, saveCount);
                            } 
                            /* we don't need the temp registers we used for the multiplication so we can just reuse where we were in saveCount + 2
                            (since we need 2 for the instruction done just above, as well as another for the next instruction we do) */
                            count = saveCount + 2;

                        }
                        /* normal addition/subtraction */
                        else {
                            /* variable */
                            if (strcmp(nodeNames[iterate->children[2]->children[0]->children[0]->nodeKind], "var") == 0) {
                                for (int i = 0; i < NUM_REG; i++) {
                                    if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[0]->children[0]->val)) == 0) {
                                        fprintf(filePointer, "\t\t\tmove $s%d, $%s%d\n", count, reg[i].regType, reg[i].num);
                                        break;

                                    }
                                }
                                count += 1;

                            }
                            /* function */
                            else if (strcmp(nodeNames[iterate->children[2]->children[0]->children[0]->nodeKind], "funcCallExpr") == 0) {
                                /* argument */
                                if (iterate->children[2]->children[0]->children[0]->numChildren > 1) {
                                    /* variable */
                                    if (strcmp(nodeNames[iterate->children[2]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->nodeKind], "var") == 0) {
                                        for (int i = 0; i < NUM_REG; i++) {
                                            if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->val)) == 0) {
                                                fprintf(filePointer, "\t\t\tmove $a0, $%s%d\n", reg[i].regType, reg[i].num);
                                                break;

                                            }
                                        }
                                    }
                                    /* immediate */
                                    else {
                                        printf("true");
                                        value = iterate->children[2]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0]->children[0]->children[0]->val;
                                        fprintf(filePointer, "\t\t\tli $a0, %d\n", value);
                                    }
                                }
                                fprintf(filePointer, "\t\t\tjal %s\n", get_symbol_id(iterate->children[2]->children[0]->children[0]->children[0]->val));
                                fprintf(filePointer, "\t\t\tmove $s%d, $v0\n", count);
                                count += 1;    
                            }
                            /* immediate */
                            else {
                                fprintf(filePointer, "\t\t\tli $s%d, %d\n", count, iterate->children[2]->children[0]->children[0]->val);
                                count += 1;
                            }


                            if (strcmp(ops[iterate->children[1]->val], "+") == 0) {
                                fprintf(filePointer, "\t\t\tadd $s%d, $s%d, $s%d\n", count-1, count-1, count-2);
                            } 

                            else if (strcmp(ops[iterate->children[1]->val], "-") == 0) {
                                fprintf(filePointer, "\t\t\tsub $s%d, $s%d, $s%d\n", count-1, count-1, count-2);
                            } 
                        }

                    }



                    iterate = iterate->parent;
                }
                /* final assignment */
                for (int i = 0; i < NUM_REG; i++) {
                    if (strcmp(reg[i].var, get_symbol_id(iterate->parent->children[0]->children[0]->val)) == 0) {

                        fprintf(filePointer, "\t\t\tmove $%s%d, $s%d\n", reg[i].regType, reg[i].num, count-1);
                        count = 0;
                        break;

                    }
                }

            }
        }

    }

    if (strcmp(nodeName, "condStmt") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");

        tree* iterate = node;
        while (strcmp(nodeNames[iterate->nodeKind], "integer") != 0 && strcmp(nodeNames[iterate->nodeKind], "identifier") != 0) {
            iterate = iterate->children[0];
        } 
        /* immediate */
        if (strcmp(nodeNames[iterate->nodeKind], "integer") == 0) {
            fprintf(filePointer, "\t\t\tli $s0, %d\n", iterate->val);

        }
        /* variable */
        else if (strcmp(nodeNames[iterate->nodeKind], "identifier") == 0) {
            for (int i = 0; i < NUM_REG; i++) {
                if (strcmp(reg[i].var, get_symbol_id(iterate->val)) == 0) {
                    fprintf(filePointer, "\t\t\tmove $s0, $%s%d\n", reg[i].regType, reg[i].num);
                    break;

                }
            }
        }
        /* iterate from bottom of the stack to get our values to compare */
        while(strcmp(nodeNames[iterate->nodeKind], "condStmt") != 0) {

            if (strcmp(nodeNames[iterate->nodeKind], "expression") == 0 && iterate->numChildren > 1) {
                if (strcmp(nodeNames[iterate->children[2]->children[0]->children[0]->nodeKind], "var") == 0) {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[0]->val)) == 0) {
                            fprintf(filePointer, "\t\t\tmove $s1, $%s%d\n", reg[i].regType, reg[i].num);
                            break;

                        }
                    }

                }
                else {
                    fprintf(filePointer, "\t\t\tli $s1, %d\n", iterate->children[2]->children[0]->children[0]->children[0]->val);
                }
            }

            iterate = iterate->parent;


        }
        /* else exists, we want send people to different places based on if there's an else or not */
        if (node->numChildren > 2) {
            if (strcmp(ops[iterate->children[0]->children[1]->val], "<") == 0) {
                fprintf(filePointer, "\t\t\tbgt $s0, $s1, else%d\n", ifCount);
                } 

            else if (strcmp(ops[iterate->children[0]->children[1]->val], ">") == 0) {
                fprintf(filePointer, "\t\t\tblt $s0, $s1, else%d\n", ifCount);
            } 
            else if (strcmp(ops[iterate->children[0]->children[1]->val], "==") == 0) {
                fprintf(filePointer, "\t\t\tbne $s0, $s1, else%d\n", ifCount);
            } 

        }
        else {
            if (strcmp(ops[iterate->children[0]->children[1]->val], "<") == 0) {
                fprintf(filePointer, "\t\t\tbgt $s0, $s1, endI%d\n", ifCount);
                } 

            else if (strcmp(ops[iterate->children[0]->children[1]->val], ">") == 0) {
                fprintf(filePointer, "\t\t\tblt $s0, $s1, endI%d\n", ifCount);
            } 
            else if (strcmp(ops[iterate->children[0]->children[1]->val], "==") == 0) {
                fprintf(filePointer, "\t\t\tbne $s0, $s1, endI%d\n", ifCount);
            } 

                    
        }
        fprintf(filePointer, "begI%d:\n", ifCount);




    }

    if (strcmp(nodeName, "loopStmt") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");
        tree* iterate = node;

        /* if there is an immediate, save it to a permenent $t register so we don't need to worry about it getting overwritten in the loop */
        while (strcmp(nodeNames[iterate->nodeKind], "integer") != 0 && strcmp(nodeNames[iterate->nodeKind], "identifier") != 0) {
            iterate = iterate->children[0];
        } 
        /* immediate */
        if (strcmp(nodeNames[iterate->nodeKind], "integer") == 0) {
            for (int i = 0; i < NUM_REG; i++) {
                if (reg[i].free) {
                    /* t0 - t9 */
                    if (i < 10) {
                        reg[i].regType = "t";
                        reg[i].num = i;
                    }
                    /* v1 */
                    else if (i == 10) {
                        reg[i].regType = "v";
                        reg[i].num = 1;                    
                    }
                    /* a1 - a3 */
                    else if (i > 10) {
                        reg[i].regType = "a";
                        reg[i].num = i-10;                    
                    }
                    /* helps the condition statement identify it and free the variable after use, with a while loop there can 
                    only be one immediate */
                    reg[i].var = "temp";


                    reg[i].free = 0;

                    fprintf(filePointer, "\t\t\tli $%s%d, %d\n", reg[i].regType, reg[i].num, iterate->val);

                    break;
                }
            }

        }

        /* getting the second value, again we're only looking for immediates to turn into variables for comparing over several loops */
        while(strcmp(nodeNames[iterate->nodeKind], "loopStmt") != 0) {

            if (strcmp(nodeNames[iterate->nodeKind], "expression") == 0 && iterate->numChildren > 1) {
                /* if not a variable */
                if (strcmp(nodeNames[iterate->children[2]->children[0]->children[0]->nodeKind], "var") != 0) {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (reg[i].free) {
                            /* t0 - t9 */
                            if (i < 10) {
                                reg[i].regType = "t";
                                reg[i].num = i;
                            }
                            /* v1 */
                            else if (i == 10) {
                                reg[i].regType = "v";
                                reg[i].num = 1;                    
                            }
                            /* a1 - a3 */
                            else if (i > 10) {
                                reg[i].regType = "a";
                                reg[i].num = i-10;                    
                            }
                            /* helps the condition statement identify it and free the variable after use, with a while loop there can 
                            only be one immediate */
                            reg[i].var = "temp";


                            reg[i].free = 0;

                            fprintf(filePointer, "\t\t\tli $%s%d, %d\n", reg[i].regType, reg[i].num, iterate->children[2]->children[0]->children[0]->children[0]->val);

                            break;
                        }
                    }

                }
               
            }

            iterate = iterate->parent;


        }

        fprintf(filePointer, "\t\t\tj WTest%d\n", whileCount);
        fprintf(filePointer, "begW%d:\n", whileCount);




    }



    int i, j;



    for (i = 0; i < node->numChildren; i++)  {

        exportASM(getChild(node, i), nestLevel + 1);
    }


    if (strcmp(nodeName, "program") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");
    }

    if (strcmp(nodeName, "funDecl") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");
        if (strcmp(get_symbol_id(node->children[0]->children[1]->val), "main") != 0 ) {
            fprintf(filePointer, "\t\t\tjr $ra\n");
        }
        else {
            fprintf(filePointer, "\t\t\tli $v0, 10\n\t\t\tsyscall\n");
        }

    }



    /* used to place things like jumps and addresses in their right place for things like if statements */
    if (strcmp(nodeName, "statement") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");


        if (strcmp(nodeNames[node->parent->nodeKind], "condStmt") == 0) {
            /* end of if statement */
            if (node->parent->numChildren > 2) {
                /* compares addresses since they're both statements, we need to know so we can put things in the right place */
                if (node->parent->children[1] == node) {
                    fprintf(filePointer, "\t\t\tj endI%d\n", ifCount);
                    fprintf(filePointer, "else%d:\n", ifCount);

                }
                if (node->parent->children[2] == node) {
                    fprintf(filePointer, "endI%d:\n", ifCount);
                    ifCount += 1;

                }
            }
            /* no else */
            else {
                fprintf(filePointer, "endI%d:\n", ifCount);
                ifCount += 1;

            }



            
        }
    }

    if (strcmp(nodeName, "loopStmt") == 0) {
        FILE *filePointer = fopen("mips.asm", "ab");
        int reg1;
        int reg2;



        tree* iterate = node;
        while (strcmp(nodeNames[iterate->nodeKind], "integer") != 0 && strcmp(nodeNames[iterate->nodeKind], "identifier") != 0) {
            iterate = iterate->children[0];
        } 
        /* immediate */
        if (strcmp(nodeNames[iterate->nodeKind], "integer") == 0) {
            for (int i = 0; i < NUM_REG; i++) {
                if (strcmp(reg[i].var, "temp") == 0) {
                    reg1 = i;
                    break;

                }
            }

        }
        /* variable */
        else if (strcmp(nodeNames[iterate->nodeKind], "identifier") == 0) {
            for (int i = 0; i < NUM_REG; i++) {
                if (strcmp(reg[i].var, get_symbol_id(iterate->val)) == 0) {
                    reg1 = i;
                    break;

                }
            }
        }
        /* iterate from bottom of the stack to get our values to compare */
        while(strcmp(nodeNames[iterate->nodeKind], "loopStmt") != 0) {

            if (strcmp(nodeNames[iterate->nodeKind], "expression") == 0 && iterate->numChildren > 1) {
                if (strcmp(nodeNames[iterate->children[2]->children[0]->children[0]->nodeKind], "var") == 0) {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, get_symbol_id(iterate->children[2]->children[0]->children[0]->val)) == 0) {
                            reg2 = i;
                            break;

                        }
                    }

                }
                else {
                    for (int i = 0; i < NUM_REG; i++) {
                        if (strcmp(reg[i].var, "temp") == 0) {
                            reg2 = i;
                            break;

                        }
                    }
                }
            }

            iterate = iterate->parent;


        }
        fprintf(filePointer, "WTest%d:\n", whileCount);


        if (strcmp(ops[iterate->children[0]->children[1]->val], "<") == 0) {
            fprintf(filePointer, "\t\t\tblt $%s%d, $%s%d, begW%d\n", reg[reg1].regType, reg[reg1].num, reg[reg2].regType, reg[reg2].num, whileCount);
            } 

        else if (strcmp(ops[iterate->children[0]->children[1]->val], ">") == 0) {
            fprintf(filePointer, "\t\t\tbgt $%s%d, $%s%d, begW%d\n", reg[reg1].regType, reg[reg1].num, reg[reg2].regType, reg[reg2].num, whileCount);
        } 
        else if (strcmp(ops[iterate->children[0]->children[1]->val], "==") == 0) {
            fprintf(filePointer, "\t\t\tbeq $%s%d, $%s%d, begW%d\n", reg[reg1].regType, reg[reg1].num, reg[reg2].regType, reg[reg2].num, whileCount);
        } 

        /* free the temp variable so we can use it again */
        for (int i = 0; i < NUM_REG; i++) {
            if (strcmp(reg[i].var, "temp") == 0) {
                reg[i].free = 1;
                break;
            }
        }

                    




    }
   


}

