%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<../src/tree.h>
#include<../src/strtab.h>
#include<stdbool.h>

extern int yylineno;
#define YYDEBUG_LEXER_TEXT yytext
yydebug=1;

enum nodeTypes {PROGRAM, DECLLIST, DECL, VARDECL, TYPESPEC, FUNDECL,
                FORMALDECLLIST, FORMALDECL, FUNBODY, LOCALDECLLIST,
                STATEMENTLIST, STATEMENT, COMPOUNDSTMT, ASSIGNSTMT,
                CONDSTMT, LOOPSTMT, RETURNSTMT, EXPRESSION, RELOP,
                ADDEXPR, ADDOP, TERM, MULOP, FACTOR, FUNCCALLEXPR,
                ARGLIST, INTEGER, IDENTIFIER, VAR, ARRAYDECL, CHAR,
                FUNCTYPENAME};

enum opType {ADD, SUB, MUL, DIV, LT, LTE, EQ, GTE, GT, NEQ};

char* scope = "";
int operation = 0;
int currOperator = ADD;
bool legalIndex = false;
int argCount = 0;
char* storeId;
struct argInfo argTypes[10];
char* argNames[10][10];

bool prevDeclared = false;
int mismatchChecking[10];
int mismatchCount = 0;
bool err = true;
bool errorMessage = false;

%}

%union
{
    int value;
    struct treenode *node;
    char *strval;
}

/*Add token declarations below. The type <value> indicates that the associated token will be of a value type such as integer, float etc., and <strval> indicates that the associated token will be of string type.*/
%token <strval> ID STRCONST CHARCONST
%token <value> INTCONST
/*Add the rest of the tokens below.*/
%token <value> KWD_IF KWD_ELSE KWD_WHILE KWD_INT KWD_STRING KWD_CHAR KWD_RETURN KWD_VOID

%token <value> OPER_ADD OPER_SUB OPER_MUL OPER_DIV OPER_LT OPER_GT OPER_GTE OPER_LTE OPER_EQ OPER_NEQ OPER_ASGN

%token <value> OPER_AT OPER_INC OPER_DEC OPER_AND OPER_OR OPER_NOT OPER_MOD

/* brackets & parens */
%token <value> LSQ_BRKT RSQ_BRKT LCRLY_BRKT RCRLY_BRKT LPAREN RPAREN

/* punctuation */
%token <value> COMMA SEMICLN 


%token <value> ILLEGAL_TOK ERROR





/* Declate NTs as of type node. Provided below is one example.*/
%type <node> program declList decl varDecl typeSpecifier funDecl formalDeclList formalDecl funBody localDeclList
%type <node> statementList statement compoundStmt assignStmt condStmt loopStmt returnStmt expression
%type <node> relop addExpr addop term mulop factor funcCallExpr argList var funcTypeName






/*start takes the most general structure*/
%start program

%%
    program         : declList
                    {
			            tree* programNode = maketree(PROGRAM);
                        ST_insert("program", "global", VOID_TYPE, FUNCTION);

			            addChild(programNode, $1);
			            $$ = programNode;
			            ast = programNode;
                    }
                    ;

    declList        : decl
                    {
                        tree* declListNode = maketree(DECLLIST);
                        ST_insert("declList", "global", VOID_TYPE, ARRAY);

			            addChild(declListNode, $1);
			            $$ = declListNode;
                    }
                    | declList decl
                    {
			            tree* declListNode = maketree(DECLLIST);
                        ST_insert("declList", "global", VOID_TYPE, ARRAY);
                        if ($1 != NULL) { addChild(declListNode, $1); }
			            addChild(declListNode, $2);
			            $$ = declListNode;
                    }
                    ;

    decl            : varDecl
                    {
		                tree* declNode = maketree(DECL);
                        ST_insert("decl", "global", VOID_TYPE, SCALAR);

			            addChild(declNode, $1);
			            $$ = declNode;
                    }
                    | funDecl
                    {
			            tree* declNode = maketree(DECL);
                        ST_insert("decl", "global", VOID_TYPE, FUNCTION);

			            addChild(declNode, $1);
			            $$ = declNode;
                    }
                    ;
    
    varDecl         : typeSpecifier ID LSQ_BRKT INTCONST RSQ_BRKT SEMICLN
                    {

                        if (ST_lookup($2, "local") != -1) { yyerror("Symbol declared multiple times."); }
			            tree* varDeclNode = maketree(VARDECL);
                    	int index = ST_insert_size($2, "local", $1->val, ARRAY, $4);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);
			            tree* intConstNode = maketreeWithVal(INTEGER, $4);

                        if ($4 <= 0) {yyerror("Array variable declared with size of zero or less."); }

			            addChild(varDeclNode, $1);
			            addChild(varDeclNode, idNode);

			            $$ = varDeclNode;
                    }
                    | typeSpecifier ID SEMICLN
                    {
                        if (ST_lookup($2, "local") != -1) { yyerror("Symbol declared multiple times."); }
                        tree* varDeclNode = maketree(VARDECL);
                        int index = ST_insert($2, "local", $1->val, SCALAR);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);
                    	addChild(varDeclNode, $1);
                        addChild(varDeclNode, idNode);

                        ST_insert("varDecl", "global", VOID_TYPE, SCALAR);


                    	$$ = varDeclNode;

                    }
                    ;

    typeSpecifier   : KWD_INT
                    {
                        tree* intNode = maketreeWithVal(TYPESPEC, INT_TYPE);
                        ST_insert("intNode", "global", INT_TYPE, FUNCTION);
                        $$ = intNode;

	                }
                    | KWD_CHAR
                    {
                        tree* charNode = maketreeWithVal(TYPESPEC, CHAR_TYPE);
                        ST_insert("charNode", "global", CHAR_TYPE, FUNCTION);
                        $$ = charNode;
	                }
                    | KWD_VOID
                    {
                        tree* voidNode = maketreeWithVal(TYPESPEC, VOID_TYPE);
                        ST_insert("voidNode", "global", VOID_TYPE, FUNCTION);
                        $$ = voidNode;
                    }
                    ;

    funDecl         : funcTypeName LPAREN formalDeclList RPAREN funBody
                    {
			            tree* funDeclNode = maketree(FUNDECL);
	

                        strTable[ST_lookup(storeId, "global")].parameters = argCount;
                        for (int i = 0; i < argCount; i++) {
                            strTable[ST_lookup(storeId, "global")].argTypes[i].data_type = argTypes[i].data_type;
                            strTable[ST_lookup(storeId, "global")].argTypes[i].symbol_type = argTypes[i].symbol_type;
                        }
                        argCount = 0;

			            addChild(funDeclNode, $1);
                        addChild(funDeclNode, $3);
                        addChild(funDeclNode, $5);
			            $$ = funDeclNode;
                    }
                    | funcTypeName LPAREN RPAREN funBody
                    {
			            tree* funDeclNode = maketree(FUNDECL);
                        tree* idNode = maketreeWithVal(IDENTIFIER, $2);		
                        tree* lParenNode = maketreeWithVal(CHAR, $3);
                        tree* rParenNode = maketreeWithVal(CHAR, $4);	
                        ST_insert("funDecl", "global", VOID_TYPE, FUNCTION);

			            addChild(funDeclNode, $1);
                        addChild(funDeclNode, $4);
			            $$ = funDeclNode;
                    }
                    ;

    funcTypeName    : typeSpecifier ID
                    {
                        if (ST_lookup($2, "global") != -1) { yyerror("Symbol declared multiple times."); }

                        tree* funcTypeNameNode = maketree(FUNCTYPENAME);

                    	int index = ST_insert_func($2, "global", $1->val, 0);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);
                        storeId = $2;

                        addChild(funcTypeNameNode, $1);

                    	addChild(funcTypeNameNode, idNode);


                        $$ = funcTypeNameNode; 
                    }
    
    formalDeclList  : formalDecl
                    {
			            tree* formalDeclListNode = maketree(FORMALDECLLIST);
                        ST_insert("formalDeclList", "global", VOID_TYPE, ARRAY);



                        addChild(formalDeclListNode, $1);
			            $$ = formalDeclListNode;
                    }
                    | formalDecl COMMA formalDeclList
                    {
			            tree* formalDeclListNode = maketree(FORMALDECLLIST);
                        tree* commaNode = maketreeWithVal(CHAR, $2);
                        ST_insert("formalDeclList", "global", VOID_TYPE, ARRAY);


                        addChild(formalDeclListNode, $1);
                        if ($3 != NULL) { addChild(formalDeclListNode, $3); }
                        $$ = formalDeclListNode;
                    }
                    ;

    formalDecl      : typeSpecifier ID
                    {
			            tree* formalDeclNode = maketree(FORMALDECL);
                    	int index = ST_insert($2, "global", $1->val, scope);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);		
                        
                        argTypes[argCount].data_type = $1->val;
                        argTypes[argCount].symbol_type = SCALAR;
                        
                        strcpy(argNames[argCount], $2);
                        

                        argCount += 1;


			            addChild(formalDeclNode, $1);
			            addChild(formalDeclNode, idNode);
			            $$ = formalDeclNode;
                    }
                    | typeSpecifier ID LSQ_BRKT RSQ_BRKT
                    {
			            tree* formalDeclNode = maketree(FORMALDECL);
                    	int index = ST_insert($2, "global", $1->val, scope);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);	
                        argTypes[argCount].data_type = $1->val;
                        argTypes[argCount].symbol_type = ARRAY;

                        argCount += 1;


			            addChild(formalDeclNode, $1);
			            addChild(formalDeclNode, idNode);

			            $$ = formalDeclNode;
                    }
                    ;
    
    funBody         : LCRLY_BRKT localDeclList statementList RCRLY_BRKT
                    {
			            tree* funBodyNode = maketree(FUNBODY);
                        tree* lCrlyBrktNode = maketreeWithVal(CHAR, $1);
                        tree* rCrlyBrktNode = maketreeWithVal(CHAR, $4);
                        ST_insert("funBody", "local", VOID_TYPE, FUNCTION);

                        if ($2 != NULL) { addChild(funBodyNode, $2); } 
                        if ($3 != NULL) { addChild(funBodyNode, $3); } 

			            $$ = funBodyNode;
                    }
                    | LCRLY_BRKT RCRLY_BRKT
                    {
                        tree* funBodyNode = maketree(FUNBODY);
                        ST_insert("funBody", "local", VOID_TYPE, FUNCTION);
                        $$ = funBodyNode;
                    }

                    ;
                
    localDeclList   :
                    {
			            $$ = NULL;
                    }
                    | varDecl localDeclList
                    {
                        tree* localDeclListNode = maketree(LOCALDECLLIST);
                        ST_insert("localDeclList", "local", VOID_TYPE, ARRAY);

			            addChild(localDeclListNode, $1);
                    	if ($2 != NULL) { addChild(localDeclListNode, $2); }

                        $$ = localDeclListNode;
                    }
                    ;
                
    statementList   :
                    {
                        $$ = NULL;
                    }
                    | statement statementList
                    {
                        tree* statementListNode = maketree(STATEMENTLIST);
                        ST_insert("statementList", "local", VOID_TYPE, ARRAY);

			            addChild(statementListNode, $1);
                        if ($2 != NULL) { addChild(statementListNode, $2); }

                        $$ = statementListNode;
                    }
                    ;
    
    statement       : compoundStmt
                    {
			            tree* statementNode = maketree(STATEMENT);
                        ST_insert("statement", "local", VOID_TYPE, FUNCTION);

			            addChild(statementNode, $1);
			            $$ = statementNode;
                    }
                    | assignStmt
                    {
			            tree* statementNode = maketree(STATEMENT);
                        ST_insert("statement", "local", VOID_TYPE, FUNCTION);
                        
			            addChild(statementNode, $1);
			            $$ = statementNode;
                    }
                    | condStmt
                    {
			            tree* statementNode = maketree(STATEMENT);
                         ST_insert("statement", "local", VOID_TYPE, FUNCTION);
                       
			            addChild(statementNode, $1);
			            $$ = statementNode;
                    }
                    | loopStmt
                    {
			            tree* statementNode = maketree(STATEMENT);
                        ST_insert("statement", "local", VOID_TYPE, FUNCTION);
                        
			            addChild(statementNode, $1);
			            $$ = statementNode;
                    }
                    | returnStmt
                    {
			            tree* statementNode = maketree(STATEMENT);
                        ST_insert("statement", "local", VOID_TYPE, FUNCTION);
                        
			            addChild(statementNode, $1);
			            $$ = statementNode;
                    }
                    ;
    
    compoundStmt    : LCRLY_BRKT statementList RCRLY_BRKT
                    {
			            tree* compoundStmtNode = maketree(COMPOUNDSTMT);
                        tree* lCrlyBrktNode = maketreeWithVal(CHAR, $1);
                        tree* rCrlyBrktNode = maketreeWithVal(CHAR, $3);  
                        ST_insert("compoundStmt", "local", VOID_TYPE, FUNCTION);

                        addChild(compoundStmtNode, $2);
			            $$ = compoundStmtNode;
                    }
                    ;
    
    assignStmt      : var OPER_ASGN expression SEMICLN
                    {
			            tree* assignStmtNode = maketree(ASSIGNSTMT);
                        tree* operAsgnNode = maketreeWithVal(CHAR, $2);
                        tree* semiClnNode =  maketreeWithVal(CHAR, $4);

                        

                        if (mismatchChecking[0] == INT_TYPE) {
                            for (int i = 1; i < mismatchCount; i++) {
                                if (mismatchChecking[i] != INT_TYPE && mismatchChecking[i] != CHAR_TYPE) {
                                    yyerror("Type mismatch in assignment.");
                                }
                            }
                        }

                        else if (mismatchChecking[0] == CHAR_TYPE) {
                            for (int i = 1; i < mismatchCount; i++) {
                                if (mismatchChecking[i] != CHAR_TYPE) {
                                    yyerror("Type mismatch in assignment.");
                                }
                            }
                        }

                        mismatchCount = 0;

                        ST_insert("assignStmt", "local", VOID_TYPE, FUNCTION);

                        
			            addChild(assignStmtNode, $1);
			            addChild(assignStmtNode, $3);
			            $$ = assignStmtNode;
                    }
                    | expression SEMICLN
                    {
                        tree* assignStmtNode = maketree(ASSIGNSTMT);
                        tree* semiClnNode =  maketreeWithVal(CHAR, $2);

                        mismatchCount = 0;


                        ST_insert("assignStmt", "local", VOID_TYPE, FUNCTION);

                        
			            addChild(assignStmtNode, $1);
			            $$ = assignStmtNode;
                    }
                    ;


    
    condStmt        : KWD_IF LPAREN expression RPAREN statement
                    {
			            tree* condStmtNode = maketree(CONDSTMT);
                        tree* ifNode =  maketreeWithVal(CHAR, $1);
                        tree* lParenNode =  maketreeWithVal(CHAR, $2);
                        tree* rParenNode =  maketreeWithVal(CHAR, $4);
                        ST_insert("condStmt", "local", VOID_TYPE, FUNCTION);

			            addChild(condStmtNode, $3);
                        addChild(condStmtNode, $5);
			            $$ = condStmtNode;
                    }
                    | KWD_IF LPAREN expression RPAREN statement KWD_ELSE statement
                    {
			            tree* condStmtNode = maketree(CONDSTMT);
                        tree* ifNode =  maketreeWithVal(CHAR, $1);
                        tree* lParenNode =  maketreeWithVal(CHAR, $2);
                        tree* rParenNode =  maketreeWithVal(CHAR, $4);
                        tree* elseNode = maketreeWithVal(CHAR, $6);
                        ST_insert("condStmt", "local", VOID_TYPE, FUNCTION);


			            addChild(condStmtNode, $3);
                        addChild(condStmtNode, $5);
                        addChild(condStmtNode, $7);
			            $$ = condStmtNode;
                    }
                    ;

    loopStmt        : KWD_WHILE LPAREN expression RPAREN statement
                    {
			            tree* loopStmtNode = maketree(LOOPSTMT);
                        tree* whileNode =  maketreeWithVal(CHAR, $1);
                        tree* lParenNode =  maketreeWithVal(CHAR, $2);
                        tree* rParenNode =  maketreeWithVal(CHAR, $4);
                        ST_insert("loopStmt", "local", VOID_TYPE, FUNCTION);

			            addChild(loopStmtNode, $3);
                        addChild(loopStmtNode, $5);
                        $$ = loopStmtNode;
                    }
                    ;

    returnStmt      : KWD_RETURN SEMICLN
                    {
			            tree* returnStmtNode = maketree(RETURNSTMT);
                        tree* returnNode =  maketreeWithVal(CHAR, $1);
                        tree* semiClnNode =  maketreeWithVal(CHAR, $2);
                        ST_insert("returnStmt", "local", VOID_TYPE, FUNCTION);

                        $$ = returnStmtNode;
                    }
                    | KWD_RETURN expression SEMICLN
                    {
			            tree* returnStmtNode = maketree(RETURNSTMT);
                        tree* returnNode =  maketreeWithVal(CHAR, $1);
                        tree* semiClnNode =  maketreeWithVal(CHAR, $3);
                        ST_insert("returnStmt", "local", VOID_TYPE, FUNCTION);

                        addChild(returnStmtNode, $2);
                        $$ = returnStmtNode;
                    }
                    ;
    
    var             : ID
                    {
                        if (ST_lookup($1, "local") == -1) {
                            for (int i = 0; i < argCount; i++) {
                                if (strcmp(argNames[i], $1) == 0) { err = false; }
                            }
                            if (err) { yyerror("Undeclared variable"); }
                        }
                        err = true;

			            tree* varNode = maketree(VAR);
			            int index = ST_insert($1, "local", strTable[ST_lookup($1, "local")].data_type, strTable[ST_lookup($1, "local")].symbol_type);
                    
                        mismatchChecking[mismatchCount] = strTable[ST_lookup($1, "local")].data_type;
                        mismatchCount += 1;

                        if (strTable[ST_lookup($1, "local")].data_type != INT_TYPE) {
                            legalIndex = false;
                        }
                        else { legalIndex = true; }
                        

                        argTypes[argCount].data_type = strTable[ST_lookup($1, "local")].data_type;
                        argTypes[argCount].symbol_type = strTable[ST_lookup($1, "local")].symbol_type;

                        tree* idNode = maketreeWithVal(IDENTIFIER, index);
			            addChild(varNode, idNode);
                        $$ = varNode;
                    }
                    | ID LSQ_BRKT addExpr RSQ_BRKT
                    {
			            tree* varNode = maketree(VAR);
                        int index = ST_insert($1, "local", INT_TYPE, scope);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);
                        tree* lswBrktNode =  maketreeWithVal(CHAR, $2);
                        tree* rswBrktNode =  maketreeWithVal(CHAR, $4);

                        if (strTable[ST_lookup($1, "local")].symbol_type == SCALAR) {
                            yyerror("Non-array identifier used as an array.");
                        }
                        else {
                            if (!legalIndex) { yyerror("Array indexed using non-integer expression."); }
                            else {
                                if (operation >= strTable[ST_lookup($1, "local")].size) { yyerror("Statically sized array indexed with constant, out-of-bounds expression."); }
                                operation = 0;
                                currOperator = ADD;
                            }
                        }

                        argTypes[argCount].data_type = strTable[ST_lookup($1, "local")].data_type;
                        argTypes[argCount].symbol_type = strTable[ST_lookup($1, "local")].symbol_type;



                        addChild(varNode, idNode);
                        addChild(varNode, $3);
                        $$ = varNode;
                    }
                    ;

    expression      : addExpr
                    {
			            tree* expressionNode = maketree(EXPRESSION);
                        ST_insert("expression", "local", VOID_TYPE, FUNCTION);

                        addChild(expressionNode, $1);
                        $$ = expressionNode;
                    }
                    | expression relop addExpr
                    {
			            tree* expressionNode = maketree(EXPRESSION);
                        ST_insert("expression", "local", VOID_TYPE, FUNCTION);

                        if ($1 != NULL) { addChild(expressionNode, $1); }

                        addChild(expressionNode, $2);
                        addChild(expressionNode, $3);

                        $$ = expressionNode;
                    }
                    ;

    relop           : OPER_LTE
                    {

                        currOperator = LTE;
                        tree* lteopNode = maketreeWithVal(RELOP, LTE);
                        ST_insert("relop,<=", "local", INT_TYPE, FUNCTION);
                        $$ = lteopNode;

                    }
                    | OPER_LT
                    {
                        currOperator = LT;
                        tree* ltopNode = maketreeWithVal(RELOP, LT);
                        ST_insert("relop,<", "local", INT_TYPE, FUNCTION);
                        $$ = ltopNode;
                    }
                    | OPER_GT
                    {
                        currOperator = GT;
                        tree* gtopNode = maketreeWithVal(RELOP, GT);
                        ST_insert("relop,>", "local", INT_TYPE, FUNCTION);
                        $$ = gtopNode;
                    }
                    | OPER_GTE
                    {
                        currOperator = GTE;
                        tree* gteopNode = maketreeWithVal(RELOP, GTE);
                        ST_insert("relop,>", "local", INT_TYPE, FUNCTION);
                        $$ = gteopNode;
                    }
                    | OPER_EQ
                    {
                        currOperator = EQ;
                        tree* eqeopNode = maketreeWithVal(RELOP, EQ);
                        ST_insert("relop,==", "local", INT_TYPE, FUNCTION);
                        $$ = eqeopNode;
                    }
                    | OPER_NEQ
                    {
                        currOperator = NEQ;
                        tree* neqeopNode = maketreeWithVal(RELOP, NEQ);
                        ST_insert("relop,!=", "local", INT_TYPE, FUNCTION);
                        $$ = neqeopNode;
                    }
                    ;

    addExpr         : term
                    {
                        tree* addExprNode = maketree(ADDEXPR);
                        ST_insert("addExpr", "local", INT_TYPE, FUNCTION);

                        addChild(addExprNode, $1);
                        $$ = addExprNode;

                    }
                    | addExpr addop term
                    {
                        tree* addExprNode = maketree(ADDEXPR);
                        ST_insert("addExpr", "local", INT_TYPE, FUNCTION);

                        addChild(addExprNode, $1);
                        addChild(addExprNode, $2);
                        addChild(addExprNode, $3);

                        $$ = addExprNode;
                    }
                    ;

    addop           : OPER_ADD
                    {
                        currOperator = ADD;
                        tree* addopNode = maketreeWithVal(ADDOP, ADD);
                        ST_insert("addop,+", "local", INT_TYPE, FUNCTION);
                        $$ = addopNode;

                    }
                    | OPER_SUB
                    {
                        currOperator = SUB;
                        tree* addopNode = maketreeWithVal(ADDOP, SUB);
                        ST_insert("addop,-", "local", INT_TYPE, FUNCTION);
                        $$ = addopNode;
                    }
                    ;
    
    term            : factor
                    {
                        tree* termNode = maketree(TERM);
                        ST_insert("term", "local", VOID_TYPE, FUNCTION);

                        addChild(termNode, $1);
                        $$ = termNode;

                    }
                    | term mulop factor
                    {
                        tree* termNode = maketree(TERM);
                        ST_insert("term", "local", VOID_TYPE, FUNCTION);

                        addChild(termNode, $1);
                        addChild(termNode, $2);
                        addChild(termNode, $3);
                        $$ = termNode;
                    }
                    ;

    mulop           : OPER_MUL
                    {
                        currOperator = MUL;
                        tree* mulopNode = maketreeWithVal(MULOP, MUL);
                        ST_insert("mulop,*", "local", INT_TYPE, FUNCTION);
                        $$ = mulopNode;
                    }
                    | OPER_DIV
                    {
                        currOperator = DIV;
                        tree* mulopNode = maketreeWithVal(MULOP, DIV);
                        ST_insert("mulop,/", "local", INT_TYPE, FUNCTION);
                        $$ = mulopNode;
                    }
                    ;

    factor          : LPAREN expression RPAREN
                    {
                        tree* factorNode = maketree(FACTOR);
                        tree* lParenNode = maketreeWithVal(CHAR, $1);
                        tree* rParenNode = maketreeWithVal(CHAR, $3);
                        ST_insert("factor", "local", INT_TYPE, FUNCTION);

                        addChild(factorNode, $2);
                        $$ = factorNode;
                    }
                    | var
                    {
                        tree* factorNode = maketree(FACTOR);
                        ST_insert("factor", "local", VOID_TYPE, FUNCTION);

                        addChild(factorNode, $1);
                        $$ = factorNode;
                    }
                    | funcCallExpr
                    {
                        tree* factorNode = maketree(FACTOR);
                        ST_insert("factor", "local", VOID_TYPE, FUNCTION);
                       
                        addChild(factorNode, $1);
                        $$ = factorNode;
                    }
                    | INTCONST
                    {
                        tree* factorNode = maketree(FACTOR);
                        tree* intNode = maketreeWithVal(INTEGER, $1);
                        ST_insert("factor", "local", INT_TYPE, SCALAR);

                        if (currOperator == ADD) {operation += $1;}
                        if (currOperator == SUB) {operation -= $1;}
                        if (currOperator == MUL) {operation *= $1;}
                        if (currOperator == DIV) {operation /= $1;}

                        mismatchChecking[mismatchCount] = INT_TYPE;
                        mismatchCount += 1;

                        legalIndex = true;

                        addChild(factorNode, intNode);
                        $$ = factorNode;
                    }
                    | CHARCONST
                    {
                        tree* factorNode = maketree(FACTOR);
                        tree* charNode = maketreeWithVal(CHAR, $1);
                        ST_insert("factor", "local", CHAR_TYPE, SCALAR);

                        mismatchChecking[mismatchCount] = CHAR_TYPE;
                        mismatchCount += 1;
                        
                        legalIndex = false;

                        addChild(factorNode, charNode);
                        $$ = factorNode;
                    }
                    | STRCONST
                    {
                        tree* factorNode = maketree(FACTOR);
                        tree* strNode = maketreeWithVal(CHAR, $1);
                        ST_insert("factor", "local", CHAR_TYPE, SCALAR);

                        legalIndex = false;


                        addChild(factorNode, strNode);
                        $$ = factorNode;
                    }
                    ;

    funcCallExpr    : ID LPAREN argList RPAREN
                    {
                        tree* funcCallExprNode = maketree(FUNCCALLEXPR);
                        int index = ST_insert($1, "local", INT_TYPE, FUNCTION);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);

                        if (ST_lookup($1, "global") == -1 && strcmp($1, "output") != 0) {
                            yyerror("Undefined function");
                        }
                        else {
                            if (argCount > strTable[ST_lookup($1, "global")].parameters && strcmp($1, "output") != 0) { yyerror("Too many arguments provided in function call."); }
                            else if (argCount < strTable[ST_lookup($1, "global")].parameters) { yyerror("Too few arguments provided in function call."); }
                            else {
                                for (int i = 0; i < argCount; i++) {
                                    if (argTypes[i].data_type != strTable[ST_lookup($1, "global")].argTypes[i].data_type) {
                                        yyerror("Argument type mismatch in function call.");
                                        break;
                                    }
                                    if (argTypes[i].symbol_type != strTable[ST_lookup($1, "global")].argTypes[i].symbol_type) {
                                        yyerror("Argument type mismatch in function call.");
                                        break;
                                    }
                                }

                            }
                        }

                        argCount = 0;

                        ST_insert("funcCallExpr", "local", VOID_TYPE, FUNCTION);

                        addChild(funcCallExprNode, idNode);
                        addChild(funcCallExprNode, $3);
                        $$ = funcCallExprNode;
                    }
                    | ID LPAREN RPAREN
                    {


                        tree* funcCallExprNode = maketree(FUNCCALLEXPR);
                        int index = ST_insert($1, "local", INT_TYPE, FUNCTION);
                        tree* idNode = maketreeWithVal(IDENTIFIER, index);

                        if (ST_lookup($1, "global") == -1 && strcmp($1, "output") != 0) {
                            yyerror("Undefined function");
                        }

                        if (argCount != strTable[ST_lookup($1, "global")].parameters) { yyerror("Too few arguments provided in function call."); }

                        ST_insert("funcCallExpr", "local", VOID_TYPE, FUNCTION);

                        addChild(funcCallExprNode, idNode);
                        $$ = funcCallExprNode;
                    }
                    ;

    argList         : expression
                    {
                        tree* argListNode = maketree(ARGLIST);
                        ST_insert("argList", "local", VOID_TYPE, ARRAY);

                        argCount += 1;

                        addChild(argListNode, $1);
                        $$ = argListNode;
                    }
                    | argList COMMA expression
                    {
                        tree* argListNode = maketree(ARGLIST);
                        tree* commaNode = maketreeWithVal(CHAR, $2);
                        ST_insert("argList", "local", VOID_TYPE, ARRAY);

                        argCount += 1;

                        addChild(argListNode, $1);
                        addChild(argListNode, $3);
                        $$ = argListNode;
                    }
                    ;

%%

int yywarning(char * msg){
    printf("warning: line %d: %s\n", yylineno, msg);

    return 0;
}

int yyerror(char * msg){
    printf("error: line %d: %s\n", yylineno, msg);
    errorMessage = true;

    return 0;
}
