/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#pragma warning(disable:4996)
/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
int start_time=0;
int end_time=0;
void printToken( TokenType token, const char* tokenString )
{ switch (token)
  { case IF:
    case THEN:
	case INT:
	case VOID:
    case ELSE:
    case END:
    case REPEAT:
    case UNTIL:
    case READ:
    case WRITE:
	case RETURN:
	case WHILE:
      fprintf(listing,
         "reserved word: %s\n",tokenString);
      break;
    case ASSIGN: fprintf(listing,"=\n"); break;
    case LT: fprintf(listing,"<\n"); break;
	case LET: fprintf(listing, "<=\n"); break;
	case GT: fprintf(listing, ">\n"); break;
	case GET: fprintf(listing, ">=\n"); break;
	case EQ: fprintf(listing, "==\n"); break;
    case NEQ: fprintf(listing,"!=\n"); break;
    case LPAREN: fprintf(listing,"(\n"); break;
    case RPAREN: fprintf(listing,")\n"); break;
	case LP: fprintf(listing, "[\n"); break;
	case RP: fprintf(listing, "]\n"); break;
	case SLP: fprintf(listing, "{\n"); break;
	case SRP: fprintf(listing, "}\n"); break;
    case SEMI: fprintf(listing,";\n"); break;
    case PLUS: fprintf(listing,"+\n"); break;
    case MINUS: fprintf(listing,"-\n"); break;
    case TIMES: fprintf(listing,"*\n"); break;
    case OVER: fprintf(listing,"/\n"); break;
    case ENDFILE: fprintf(listing,"EOF\n"); break;
    case NUM:
      fprintf(listing,
          "NUM, val= %s\n",tokenString);
      break;
    case ID:
      fprintf(listing,
          "ID, name= %s\n",tokenString);
      break;
    case ERROR:
      fprintf(listing,
          "ERROR: %s\n",tokenString);
      break;
    default: /* should never happen */
      fprintf(listing,"Unknown token: %d\n",token);
  }
}


/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
	t->len = 0;//数组长度
  }
  return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
	t->id_type = 0;			//默认id为变量名
	t->declared = 0;		//默认不是声明
  }
  return t;
}


/* Function newExpNode creates a new function
 * node for syntax tree construction
 */
TreeNode * newFuncNode(FunKind kind)
{
	TreeNode * t = (TreeNode *)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = FunK;
		t->lineno = lineno;
		t->kind.type = kind;
		t->type = Void;
	}
	return t;
}



/* Function newExpNode creates a new declare type
 * node for syntax tree construction
 */
TreeNode * newTypeNode(TypeKind kind)
{
	TreeNode * t = (TreeNode *)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = TypeK;
		t->kind.type = kind;
		
		t->lineno = lineno;
		t->type = Void;
	}
	return t;
}
/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if (tree->nodekind==StmtK)
    { switch (tree->kind.stmt) {
        case IfK:
          fprintf(listing,"If\n");
          break;
		case ElseK:
			fprintf(listing, "Else\n");
			break;
      
		case WhileK:
			fprintf(listing, "While\n");
			break;
		case EndK:
			fprintf(listing, "Component End\n");
			break;
		case ReturnK:
			fprintf(listing, "Return\n");
			/*if (tree->child[0] != NULL)tree->type = tree->child[0]->type;
			else tree->type = Void;*/
			break;
        case AssignK:
          fprintf(listing,"Assign \n");
          break;
		case VarDecK:
			tree->child[1]->declared = 1;
			
			if (tree->len == 0) {
				
				fprintf(listing, "Variable declare\n");
			}
			else {
				tree->child[1]->id_type = 1;
				fprintf(listing, "Array declare len is %d\n", tree->len);
				tree->len = 4 * tree->len;
				
				
			}
			break;
		case CallK:
			tree->child[0]->id_type = 2;					//变量名类型为函数类型
			if (strcmp(tree->child[0]->attr.name , "input")==0) {		//特殊的函数input，output
				fprintf(listing, "input \n");
				tree->kind.stmt = ReadK;
				tree->type = Integer;
			}
			else if (strcmp(tree->child[0]->attr.name, "output") == 0) {
				fprintf(listing, "output \n");
				tree->kind.stmt = WriteK;
				tree->type = Void;
			}
			else {
				fprintf(listing, "Call \n");
			}
			


			break;
        /*case ReadK:
          fprintf(listing,"Read: %s\n",tree->attr.name);
          break;
        case WriteK:
          fprintf(listing,"Write\n");
          break;*/
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { switch (tree->kind.exp) {
        case OpK:
          fprintf(listing,"Op: ");
          printToken(tree->attr.op,"\0");
          break;
        case ConstK:
          fprintf(listing,"Const: %d\n",tree->attr.val);
          break;
        case IdK:
			
          fprintf(listing,"Id: %s \n",tree->attr.name);
          break;
		
		case ArrK:
			fprintf(listing, "Array  at : \n");
			break;
		case VarK:
			fprintf(listing, "Variable \n");
			if (tree->child[1] != NULL) {					//如果不为空则表明是数组名
				tree->child[0]->id_type = 1;
			}
			break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
	else if (tree->nodekind == TypeK) {
		switch (tree->kind.type) {
		case IntK:
			tree->type = Integer;
			fprintf(listing, "Kind: int \n");
			break;
		case VoidK:
			tree->type = Void;
			fprintf(listing, "Kind: void \n");
			break;
		default:
			fprintf(listing, "Unknown TypeNode kind\n");
			break;
		}
	}
	else if (tree->nodekind == FunK) {
		switch (tree->kind.type) {

		case FuncDecK:
			fprintf(listing, " Function declare %d \n",tree->lineno);
			
			tree->child[1]->id_type = 2;					//变量名改为函数名
			tree->child[1]->declared = 1;
			tree->child[3]->declared = 1;					//函数体已经压入
			switch (tree->child[0]->kind.type) {
			case IntK:
				tree->type = Integer;
				break;
			case VoidK:
				tree->type = Void;
				break;
			}



			break;
		case CompK:
			fprintf(listing, " Comppound-stmt \n");
			break;
		case ParamK:
			if (tree->attr.name != "void") {
				tree->child[1]->declared = 1;
				tree->child[1]->id_type = 3;
				tree->attr.name = tree->child[1]->attr.name;
				fprintf(listing, " Params \n");
			}
			else
				fprintf(listing, " Params is null\n");
			break;
		case ParamArrK:
			fprintf(listing, " Params Array\n");
			tree->child[1]->id_type = 4;
			tree->child[1]->declared = 1;
			break;
		default:
			fprintf(listing, "Unknown TypeNode kind\n");
			break;
		}
	}
    else fprintf(listing,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
