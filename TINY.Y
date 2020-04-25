/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedVal;
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */

%}

%token IF THEN ELSE END REPEAT UNTIL READ WRITE RETURN WHILE 
%token INT VOID
%token ID NUM 
%token ASSIGN EQ LT PLUS MINUS TIMES OVER LPAREN RPAREN SEMI LET GT GET LP RP SLP SRP COMMA NEQ
%token ERROR 

%% /* Grammar for TINY */
program     : declaration-list
                 { savedTree = $1;} 
            ;
declaration-list    : declaration-list declaration
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | declaration  { $$ = $1; }
            ;
declaration : var-declaration { $$ = $1; }
            | fun-declaration { $$ = $1; }
			;
var-declaration: type-specifier id SEMI{
			$$=newStmtNode(VarDecK);							//普通变量声明节点
			$$->child[0]=$1;
			$$->child[1]=$2;
			}
			|type-specifier id LP NUM{
				savedVal = atoi(tokenString);	
			} 
			RP SEMI{		
			$$=newStmtNode(VarDecK);							//数组变量声明节点
			$$->child[0]=$1;
			$$->child[1]=$2;
			$$->len=savedVal;
			
		
			}
			;
type-specifier :INT{											//变量类型节点
				 $$=newTypeNode(IntK);
				}
				|VOID{
				$$=newTypeNode(VoidK);
				}
				;
id: ID{
			$$ = newExpNode(IdK);
            $$->attr.name =copyString(tokenString);
		}
		;
fun-declaration:type-specifier id LPAREN params RPAREN compound-stmt{
					$$=newFuncNode(FuncDecK);			
					$$->child[0]=$1;
					$$->child[1]=$2;
					$$->child[2]=$4;
					$$->child[3]=$6;
					
				}
				;
				
params:  params-list{$$=$1;}
		|VOID {$$=newFuncNode(ParamK);
			   $$->attr.name="void";}							//普通参数节点
		;
		
params-list:params-list COMMA param{
			YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
			}
			|param{
					$$=$1;
			}
			;
			
param :type-specifier id{
			$$=newFuncNode(ParamK);							//普通参数节点
			$$->child[0]=$1;
			$$->child[1]=$2;
		}
		|type-specifier id LP RP{	
		$$=newFuncNode(ParamArrK);							//参数数组节点		
		$$->child[0]=$1;
		$$->child[1]=$2;
		}
		;
compound-stmt:SLP local-declarations statement-list end{
			$$=newFuncNode(CompK);					
			$$->child[0]=$2;
			$$->child[1]=$3;
			$$->child[2]=$4;
		}
		;
		
local-declarations: local-declarations var-declaration{
					YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
					}
					|{ $$ = NULL; }
					;
					
statement-list:statement-list statement{
					YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;} 
                     else $$ = $2;
				}
				|{ $$ = NULL; }
				;
statement:  expression-stmt{$$=$1;}
			|compound-stmt{$$=$1;}
			|selection-stmt{$$=$1;}
			|iteration-stmt{$$=$1;}
			|return-stmt{$$=$1;}
			;
expression-stmt:expression SEMI{
				$$=$1;
				}
				|SEMI{$$=NULL;}
				;
selection-stmt:IF LPAREN expression RPAREN statement {
				   $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
				   
				}
				|IF  LPAREN expression RPAREN statement else-stmt
				{
				   $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = $6;
				  
				};
else-stmt:ELSE statement{
				$$ = newStmtNode(ElseK);
				$$->child[0] =$2;
				
				}
iteration-stmt:WHILE LPAREN expression RPAREN statement{
				  $$ = newStmtNode(WhileK);
				  $$->child[0] = $3;
                  $$->child[1] = $5;
				 
				};
end: SRP{
	$$=newStmtNode(EndK);
}
return-stmt: RETURN SEMI
			  {$$ = newStmtNode(ReturnK);}
			  | RETURN expression SEMI
			  {$$ = newStmtNode(ReturnK);
				$$->child[0]=$2;}
			  ;
expression :var ASSIGN expression{

			$$ = newStmtNode(AssignK);
				$$->child[0]=$1;
                $$->child[1]=$3;
			}
			|simple-expression{
			$$=$1;
			};
			
			
			
var:	id arrs{
			$$ = newExpNode(VarK);				//调用变量节点或数组地址节点
			
			$$->child[0]=$1;
			$$->child[1]=$2;
			}
arrs:	LP expression RP{
			$$ = newExpNode(ArrK);				//调用数组元素节点
			$$->child[0]=$2;
		}
		|{$$=NULL;}
		;
		
simple-expression: additive-expression LT additive-expression 
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LT;
                 }
            | additive-expression EQ additive-expression
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = EQ;
                 }
			| additive-expression LET additive-expression
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LET;
                 }
		    | additive-expression GT additive-expression
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = GT;
                 }
			| additive-expression GET additive-expression
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = GET;
                 }
			| additive-expression NEQ additive-expression
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = NEQ;
                 }
            | additive-expression { $$ = $1; }
            ;

additive-expression:additive-expression PLUS term 
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = PLUS;
                 }
            | additive-expression MINUS term
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = MINUS;
                 } 
            | term { $$ = $1; }
            ;
term : term TIMES factor 
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = TIMES;
                 }
            | term OVER factor
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = OVER;
                 }
            | factor { $$ = $1; }
            ;


factor      : LPAREN expression RPAREN
                 { $$ = $2; }
            | NUM
                 { $$ = newExpNode(ConstK);
                   $$->attr.val = atoi(tokenString);
                 }			
            |var{$$ =$1;}
			|call{$$=$1;}
            | error { $$ = NULL; }
            ;
 call: id LPAREN  args RPAREN{
		$$=newStmtNode(CallK);					//调用函数节点
		$$->child[0]=$1;
		$$->child[1]=$3;
		}
		;
args:args-list{$$=$1;}
	| { $$ = NULL; }
	;
	
args-list:args-list COMMA expression{
			YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
	      }
		  |expression{
					$$=$1;
		  }
		  ;
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

