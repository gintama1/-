/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
static int location = 0;
static int count = 0;
/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
	
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{
	int token = 0;
	switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case AssignK:
		  break;
        case ReadK:
			//token = st_lookup(t->attr.name, t->id_type, 1);
   //       if ( token== -1)
   //       /* not yet in table, so treat as new definition */
   //         st_insert(t->attr.name,t->id_type,t->lineno,location++,token);
   //       else
   //       /* already in table, so ignore location, 
   //          add line number of use only */ 
   //         st_insert(t->attr.name, t->id_type, t->lineno,0,token);
          break;
		case VarDecK:														
			if (t->len == 0) {												//变量声明
				token = st_lookup(t->child[1]->attr.name, t->child[1]->id_type, 0);
				t->len = 4;
				if ( token == -1)
					/* not yet in table, so treat as new definition */
					st_dec_insert(t->child[1]->attr.name, t->child[1]->id_type, t->child[1]->lineno,t->len, location++,token);
				else
				{
					fprintf(listing, "%d  变量名%s已经声明\n", t->child[1]->lineno, t->child[1]->attr.name);
				}
			}
			else {
				token = st_lookup(t->child[1]->attr.name, t->child[1]->id_type, 0);
				if ( token== -1)
					/* not yet in table, so treat as new definition */
					st_dec_insert(t->child[1]->attr.name, t->child[1]->id_type, t->child[1]->lineno,t->len, location++,token);
				else
				{
					fprintf(listing, "%d  数组名%s已经声明\n", t->child[1]->lineno, t->child[1]->attr.name);
				}
			}
			break;
		case CallK:													 //函数调用
			token = st_lookup(t->child[0]->attr.name, t->child[0]->id_type, 1);
			if (token == -1)
				/* not yet in table, so treat as new definition */
				st_insert(t->child[0]->attr.name, t->child[0]->id_type, t->child[0]->lineno, location++, token);
			else
				/* already in table, so ignore location,
				   add line number of use only */
				st_insert(t->child[0]->attr.name, t->child[0]->id_type, t->child[0]->lineno, 0, token);
			if (t->child[1] != NULL) {
				TreeNode *temp = t->child[1];
				int i = 1;
				while (temp != NULL) {
					switch (temp->kind.exp) {
					case VarK:fprintf(listing, "variable  %s\n", temp->child[0]->attr.name);
						temp->child[0]->declared = 2;						//声明为 可能是数组或变量
						break;
					case ConstK:
						//fprintf(listing, "Const  %d\n", temp->id_type);
						break;
					}
					i++;
					temp = temp->sibling;
					//printTree(temp);
				}
			}

			break;
	  case EndK:
			StackPop();
			break;
        default:
          break;
      }
      break;
	
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
		  if (t->declared == 0 && t->id_type == 0) {								//变量调用
			  token = st_lookup(t->attr.name, t->id_type, 1);
			  if (token == -1)
			  {
				  fprintf(listing, "%d  变量%s没有声明\n", t->lineno, t->attr.name);
			  }
			  else
				  /* already in table, so ignore location,
					 add line number of use only */
				  st_insert(t->attr.name, t->id_type, t->lineno, 0, token);
		  }
		  else if (t->declared == 2&&t->id_type==0) {										//函数调用时，变量名或数组调用
			  token = st_lookup(t->attr.name, t->id_type, 1);							//默认是变量
			  if ( token== -1)
			  {
				  
				if (st_lookup(t->attr.name, 1, 1) != -1) {									//数组的形式搜索								//以数组的形式搜索
				  t->id_type = 1;
				  token = st_lookup(t->attr.name, 1, 1);
				  st_insert(t->attr.name, t->id_type, t->lineno, 0, token);
				}
				else {
					fprintf(listing, "%d  变量%s没有声明\n", t->lineno, t->attr.name);
				}
			  }
			  else
				  /* already in table, so ignore location,
					 add line number of use only */
				  st_insert(t->attr.name,t->id_type ,t->lineno, 0,token);
		  }
          break;
	 
	  case VarK:					
		  if (t->child[1] != NULL) {														//数组的元素调用
			  token = st_lookup(t->child[0]->attr.name, t->child[0]->id_type, 1);
			  if ( token== -1)
			  {
				  fprintf(listing, "%d  数组名%s没有声明\n", t->child[0]->lineno, t->child[0]->attr.name);
			  }
			  else
				  /* already in table, so ignore location,
					 add line number of use only */
				  st_insert(t->child[0]->attr.name, t->child[0]->id_type, t->child[0]->lineno, 0,token);
		  }
		  break;
        default:
          break;
      }
      break;
	case FunK:												
		switch (t->kind.type)
		{
		case FuncDecK:															//函数声明
			CreateStackEle(t->child[1]->lineno);				//确定符号表的入口并生成表元素
			if (t->child[0]->type == Void) {				//Void返回类型
				//fprintf(listing, "void emmmm\n");
				CreateBucketHead(0);							//生成桶的头结点
			}						
			else {												//Int返回类型
				//fprintf(listing, "integer emmmm\n");
				CreateBucketHead(1);
			}
			token = st_lookup(t->child[1]->attr.name, t->child[1]->id_type, 0);
			if ( token== -1)
				/* not yet in table, so treat as new definition */
				st_insert(t->child[1]->attr.name, t->child[1]->id_type, t->child[1]->lineno, location++,token);
			else
				/* already in table, so ignore location,
				   add line number of use only */
				st_insert(t->child[1]->attr.name, t->child[1]->id_type,t->child[1]->lineno, 0,token);
			
			break;
		case CompK:
			if (t->declared != 1) {
				CreateStackEle(t->lineno);				//确定符号表的入口并生成表元素
				CreateBucketHead(-1);				//生成桶的头结点
			}
			break;
		case ParamArrK:
			token = st_lookup(t->child[1]->attr.name, t->child[1]->id_type, 0);
			if ( token== -1)
				/* not yet in table, so treat as new definition */
				st_insert(t->child[1]->attr.name, t->child[1]->id_type, t->child[1]->lineno, location++,token);
			else
				/* already in table, so ignore location,
				   add line number of use only */
				st_insert(t->child[1]->attr.name, t->child[1]->id_type, t->child[1]->lineno, 0,token);
			break;
		case ParamK:
			if (strcmp(t->attr.name, "void") != 0) {			//参数不为空
				token = st_lookup(t->child[1]->attr.name, t->child[1]->id_type, 0);
				if (token == -1)
					/* not yet in table, so treat as new definition */
					st_insert(t->child[1]->attr.name, t->child[1]->id_type, t->child[1]->lineno, location++, token);
				else
					/* already in table, so ignore location,
					   add line number of use only */
					st_insert(t->child[1]->attr.name, t->child[1]->id_type, t->child[1]->lineno, 0, token);
			}
			
			break;
		default:
			break;
		}

    default:
      break;
  }
  //printSymTab(listing);
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
	CreateHead();					//生成栈头节点
	CreateStackEle(-1);				//确定符号表的入口并生成表元素
	CreateBucketHead(-1);				//生成桶的头结点
  traverse(syntaxTree,insertNode,nullProc);
  ClearStack();
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}


//检查返回函数是否匹配
void  CheckReturnType(TreeNode * tree, TreeNode *temp)
{
	int i;
	while (tree != NULL) {
		if (tree->nodekind == StmtK)
		{
			switch (tree->kind.stmt) {
			case ReturnK:
				if (tree->type != temp->type) {
					fprintf(listing, "line %d  返回类型不匹配\n",tree->lineno);
				}
				else {
					count++;
				}
				break;
			}
		}
		for (i = 0; i < MAXCHILDREN; i++)
			CheckReturnType(tree->child[i], temp);
		tree = tree->sibling;
	}

}








/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");
		  // LET GT GET
          if ((t->attr.op == EQ) || (t->attr.op == LT)||(t->attr.op==LET) || 
			  (t->attr.op == GT) || (t->attr.op == GET) || (t->attr.op == NEQ))
            t->type = Boolean;
          else
            t->type = Integer;
          break;
		case VarK:
		  t->type = Integer;
		  break;
        case ConstK:
			t->type = Integer;
			t->id_type = 0;
			break;
        case IdK:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
  case FunK:
	  switch (t->kind.type)
	  {
	  case FuncDecK:
		  TreeNode *temp = t->child[3];
		  CheckReturnType(temp,t);
		  if (t->type == Integer && count == 0) {
			  fprintf(listing, "%d  无int 返回值\n",temp->lineno);
			}
		  count = 0;
		  break;
	  default:
		  break;
	  }
	  break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
	  case WhileK:
		  if (t->child[0]->type == Integer)
			  typeError(t->child[0], "if test is not Boolean");
		  break;
        case AssignK:
          if (t->child[0]->type != t->child[1]->type)
            typeError(t->child[0],"type not equal");
          break;
        case WriteK:
			//fprintf(listing, "output()");
			if (t->child[1] == NULL || (t->child[1]->type != Integer) || (t->child[1]->type == Integer&& t->child[1]->sibling!=NULL)) {
				fprintf(listing, "%d output  argument is erro\n",t->lineno);
			}
           /* typeError(t->child[1],"write of non-integer value");*/
          break;
		case ReadK:
			if (t->child[1] != NULL) {
				fprintf(listing, "%d input  argument is erro\n", t->lineno);
			}
			break;
		case ReturnK:
			if (t->child[0] != NULL)t->type = t->child[0]->type;
			else t->type = Void;
			
			break;
        /*case RepeatK:
          if (t->child[1]->type == Integer)
            typeError(t->child[1],"repeat test is not Boolean");
          break;*/
		case CallK:												//函数的参数检查
			
			int index=findFunction(t->child[0]->attr.name);
			
			if(index==-1){ fprintf(listing, "function %s not find\n", t->child[0]->attr.name); }
			else {
				int return_type = getReturnType(index);
				if (return_type == 0) {
					t->id_type = -2; 
					t->type = Void;
					//fprintf(listing, "return type is Void\n");
				}
				else { 
					t->id_type = 0; 
					t->type = Integer;
					//fprintf(listing, "return type is Integer\n");
				}
				//fprintf(listing, "table %d \n", index);
				if (t->child[1] == NULL)fprintf(listing, "no argument\n");
				else {
					TreeNode *temp = t->child[1];
					int i = 1;
					while (temp != NULL) {
						switch (temp->kind.exp) {
						case VarK:
							//fprintf(listing, "variable  %d\n", temp->child[0]->id_type);
							if(!compare(index, i, temp->child[0]->id_type))fprintf(listing, "variable  %s 匹配失败\n", temp->child[0]->attr.name);
							break;
						case ConstK:
							//fprintf(listing, "Const  %d\n",temp->id_type);
							if (!compare(index, i, temp->id_type)) { fprintf(listing, "const  %d 匹配失败\n", temp->attr.val); }
							break;
						case CallK:
							if (!compare(index, i, temp->id_type)) { fprintf(listing, "call 匹配失败\n"); }
							break;
						}
						i++;
						temp = temp->sibling;
						//printTree(temp);
					}

				}
			}
			break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
