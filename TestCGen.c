/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#pragma warning(disable:4996)
/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int Offset = 0;

/* prototype for internal recursive code generator */
static void cGen1(TreeNode * tree);
static int index = 0;
static int label = 1;
static char* newtemp() {
	char *m = malloc(10);
	sprintf(m, "t%d", index);
	index++;
	return m;
}
static char* newlabel() {
	char *m = malloc(10);
	sprintf(m, "lab%d", label);
	label++;
	return m;
}

/* Procedure genStmt generates code at a statement node */
static void genStmt1(TreeNode * tree)
{
	char * string = malloc(50);
	TreeNode * p1, *p2, *p3 ,*p4;
	char *label1, *label2;
	int savedLoc1, savedLoc2, currentLoc;
	int loc;
	switch (tree->kind.stmt) {

	case IfK:
		
		p1 = tree->child[0];
		p2 = tree->child[1];
		p3 = tree->child[2];
		/* generate code for test expression */
		label1 = newlabel();
		label2 = "";
		if (p3 != NULL) {
			label2 = newlabel();
		}
		
		cGen1(p1);
		printf("if_false %s  goto %s\n", p1->token, label1);
		
		/* recurse on then part */
		cGen1(p2);
		if (p3 != NULL) {
			printf("goto %s\n", label2);
		}
		printf("%s\n", label1);
		/* recurse on else part */
		if (p3 != NULL) {
			cGen1(p3);
			printf("%s\n", label2);
		}
		break; /* if_k */
	case ElseK:
		cGen1(tree->child[0]);
		break;
	case WhileK:
		label1 = newlabel();
		label2 = newlabel();

		p1 = tree->child[0];
		p2 = tree->child[1];
		printf("%s\n", label1);
		cGen1(p1);
		printf("if_false %s  goto %s\n", p1->token, label2);
		cGen1(p2);
		printf("goto %s\n", label1);
		printf("%s\n", label2);
		break; /* repeat */

	case AssignK:
		
		cGen1(tree->child[1]);
		tree->token = malloc(10);
		tree->token = tree->child[1]->token;

		if (tree->child[0]->child[1] == NULL) {		//普通变量
			sprintf(string, "%s = %s", tree->child[0]->child[0]->attr.name, tree->child[1]->token);
			emitComment(string);
			printf("%s\n", string);
		}
		else {										//数组变量
			cGen1(tree->child[0]->child[1]);
			char *temp=newtemp();
			char *temp1 = newtemp();
			printf("%s = %s * 4\n",temp, tree->child[0]->child[1]->token);
			printf("%s = &%s + %s\n", temp1, tree->child[0]->child[0]->attr.name,temp);
			printf("*%s = %s\n", temp1, tree->child[1]->token);
			sprintf(string, "%s[%s] = %s", tree->child[0]->child[0]->attr.name,
											tree->child[0]->child[1]->token,
											tree->child[1]->token);
			emitComment(string);
			//printf("%s\n", string);
		}
		break; /* assign_k */
	case ReturnK:
		if (tree->child[0] != NULL) {
			cGen1(tree->child[0]);
			printf("return %s\n", tree->child[0]->token);
		}
		else {
			printf("return \n");
		}
		break;
	case CallK:
		printf("begin_args\n");
		cGen1(tree->child[1]);
		TreeNode *temp = tree->child[1];
		while (temp != NULL) {
			printf("args %s\n", temp->token);
			temp = temp->sibling;
		}
		tree->token = malloc(10);
		tree->token = newtemp();
		
		printf("%s = call %s\n",tree->token,tree->child[0]->attr.name);
		
		break;
	case ReadK:
		
		tree->token = malloc(10);
		tree->token = newtemp();
		printf("%s = input() \n", tree->token);

		break;
	case WriteK:
		cGen1(tree->child[1]);
		printf("output( %s )\n", tree->child[1]->token);
		break;
	
	default:
		break;
	}
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp1(TreeNode * tree)
{
	int loc;
	TreeNode * p1, *p2;
	char * string = malloc(50);
	switch (tree->kind.exp) {

	case ConstK:
		
		tree->token = malloc(10);
		sprintf(tree->token, "%d", tree->attr.val);
		
		break; /* ConstK */

	case IdK:
		tree->token = tree->attr.name;
		
		break; /* IdK */
	case VarK:
		tree->token = malloc(10);
		
		if (tree->child[1] == NULL) {		//普通变量
			tree->token = malloc(10);
			sprintf(tree->token, "%s", tree->child[0]->attr.name);
		}
		else {										//数组变量
			cGen1(tree->child[1]);
			tree->token = malloc(10);
			char *temp = newtemp();
			char *temp1 = newtemp();
			printf("%s = %s * 4\n", temp, tree->child[1]->token);
			printf("%s = &%s + %s\n", temp1, tree->child[0]->attr.name, temp);
			//printf("*%s = %s\n", temp1, tree->child[1]->token);			
			sprintf(string, "*%s",temp1);
			sprintf(tree->token, string);
			
		}
		break;
	case ArrK:
		
		cGen1(tree->child[0]);
		tree->token = malloc(10);
		tree->token = tree->child[0]->token;
		
		break;
	case OpK:
		if (TraceCode) emitComment("-> Op");
		p1 = tree->child[0];
		p2 = tree->child[1];
		/* gen code for ac = left arg */
		cGen1(p1);
		
		cGen1(p2);
		
		switch (tree->attr.op) {
		case PLUS:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s + %s", tree->token,p1->token, p2->token);
			printf("%s \n", string);
			
			emitComment(string);
			break;
		case MINUS:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s - %s", tree->token, p1->token, p2->token);
			printf("%s \n", string);

			emitComment(string);
			break;
		case TIMES:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s * %s", tree->token, p1->token, p2->token);
			printf("%s \n", string);
			emitComment(string);
			
			break;
		case OVER:

			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s / %s", tree->token, p1->token, p2->token);
			printf("%s \n", string);

			emitComment(string);

			break;
		case LT:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s < %s", tree->token, p1->token, p2->token);
			printf("%s\n", string);
			break;
		case LET:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s <= %s", tree->token, p1->token, p2->token);
			printf("%s\n", string);
		break;		
		case EQ:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s == %s", tree->token, p1->token, p2->token);
			printf("%s\n", string);
			break;
		case NEQ:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s != %s", tree->token, p1->token, p2->token);
			printf("%s\n", string);
			break;
		case GT:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s > %s", tree->token, p1->token, p2->token);
			printf("%s\n", string);
			break;
		case GET:
			tree->token = malloc(10);
			tree->token = newtemp();
			sprintf(string, "%s = %s >= %s", tree->token, p1->token, p2->token);
			printf("%s\n", string);
			break;
		
		default:
			emitComment("BUG: Unknown operator");
			break;
		} /* case op */
		if (TraceCode)  emitComment("<- Op");
		break; /* OpK */

	default:
		break;
	}
} /* genExp */
/* genFunction*/
static void genFun1(TreeNode * tree) {
	TreeNode * p1, *p2;
	switch (tree->kind.type) {
	case FuncDecK:
		//emitComment("function declars");
		printf("entry %s\n", tree->child[1]->attr.val);
		cGen1(tree->child[3]);
		break;
	case CompK:
		
		p1 = tree->child[0];
		p2 = tree->child[1];
		cGen1(p1);
		cGen1(p2);

		break;

	}
}
/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen1(TreeNode * tree)
{
	if (tree != NULL)
	{
		switch (tree->nodekind) {
		case StmtK:
			genStmt1(tree);
			break;
		case ExpK:
			
			genExp1(tree);
			break;
		case FunK:
			genFun1(tree);
		default:
			break;
		}
		cGen1(tree->sibling);
	}
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen1(TreeNode * syntaxTree, char * codefile)
{
	char * s = malloc(strlen(codefile) + 7);
	strcpy(s, "File: ");
	strcat(s, codefile);
	emitComment("TINY Compilation to TM Code");
	emitComment(s);
	/* generate standard prelude */
	emitComment("Standard prelude:");
	emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
	emitRM("ST", ac, 0, ac, "clear location 0");
	emitComment("End of standard prelude.");
	/* generate code for TINY program */
	cGen1(syntaxTree);
	/* finish */
	emitComment("End of execution.");
	emitRO("HALT", 0, 0, 0, "");
}
