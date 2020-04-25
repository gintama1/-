/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
	 
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
   { char * name;
     LineList lines;
     int memloc ; /* memory location for variable */
	 int id_type;			//id类型 -2:void变量 -1链接, 0:int变量，1:int数组，2函数，3形参变量，4形参数组，5 表
	 int total;										//总大小
	 int parent;									//调用它的入口index
	 int link;										//调用的函数入口
	 int len;										//长度
	 int return_type;								//函数返回类型
     struct BucketListRec * next;
	
   } * BucketList;

/* the  table */
static BucketList hashTable[SIZE];
static int index;												//表入口

//存放栈指针
typedef struct stack {
	BucketList element;
	struct stack * next;
	int offset;
	int index;
	int lineno;
}Stack;
static Stack *head;												//栈



void StackPush(Stack *temp) {					//压入
	temp->next = head->next;
	head->next = temp;
}
Stack * Peek() {								//查看第一个
	Stack * m = head->next;
	return m;
}
void ClearStack() {
	hashTable[0]->len = head->next->offset;
}
void StackPop() {					//弹出
	int temp = head->next->index;						//调用的函数入口
	int temp_lineno = head->next->lineno;
	hashTable[temp]->len = head->next->offset;
	if (head->next != NULL) {

		head->next = head->next->next;
		index = head->next->index;				//更改符号表的入口地址
	}
	//printf("index  %d\n", index);

	

	
	BucketList l = (BucketList)malloc(sizeof(struct BucketListRec));
	l->name = "Link";
	l->id_type = -1;
	l->link = temp;
	l->lines = (LineList)malloc(sizeof(struct LineListRec));
	l->lines->lineno = temp_lineno;
	l->memloc = l->link;
	l->lines->next = NULL;

	BucketList m = hashTable[index];
	while (m->next != NULL)m = m->next;
	m->next = l;
	l->next = NULL;
	
	


	
}

void CreateStackEle(int lineno) {
	int i = 0;
	for (; i < SIZE; i++) {
		if (hashTable[i] == NULL)break;
	}
	index = i;													//查找非空的栈入口
	Stack * w = (Stack *)malloc(sizeof(Stack));
	w->offset = 0;
	w->index = index;
	w->lineno = lineno;
	w->element = hashTable[index];
	w->next = NULL;
	StackPush(w);												//压入栈中
	
}


void CreateHead() {										//生成栈的头节点
	head = (Stack *)malloc(sizeof(Stack));
	head->element = NULL;
	head->next = NULL;
	head->offset = 0;
	head->index = -1;
	index = 0;										//符号表入口地址初始化
}


void CreateBucketHead(int return_type) {						//用return_type存放函数返回类型 0:void 1:int
	BucketList l = hashTable[index];
	l = (BucketList)malloc(sizeof(struct BucketListRec));
	if (index == 0) {
		l->parent = -1;
		char *name = (char *)malloc(50);
		sprintf(name, "%s %d", "table", index);
		l->name = name;
		l->id_type = 5;
		l->memloc = 12;
		l->next = NULL;
		l->lines = NULL;
		l->len = 0;
		l->total = 0;
		l->return_type = return_type;
		hashTable[index] = l;
		//printf("%s %d\n", l->name,index);
	}
	else {
		Stack * m = Peek();
		l->parent = m->next->index;			//调用的入口
		
		char *name = (char *)malloc(50);
		sprintf(name, "%s %d", "table",index);
		l->name = name;
		l->len = 0;
		l->total = 0;
		l->id_type = 5;
		l->memloc = 12;
		l->return_type = return_type;
		l->next = NULL;
		l->lines = NULL;
		hashTable[index] = l;
		//printf("%s %d\n", l->name, index);

	}
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int id_type,int lineno, int loc ,int token)
{ 
	int h =0;
	if (token != -1)h = token;
	else h = index;
  BucketList l =  hashTable[h];
  
  while ((l != NULL) && ((strcmp(name, l->name) != 0) || (id_type != l->id_type))) {
	  if ((strcmp(name, l->name) == 0) && l->id_type == 3&&id_type==0)break;
	  if ((strcmp(name, l->name) == 0) && l->id_type == 4 && id_type == 1)break;
	  l = l->next;
  }
   
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
	l->id_type = id_type;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
	/*l->next = hashTable[h]->next;
	hashTable[h]->next = l;*/
	
	BucketList m = hashTable[h];
	while (m->next != NULL)m = m->next;
	m->next = l;
	l->next = NULL;


    //l->next = hashTable[h];
    //hashTable[h] = l; 
  }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */



void st_dec_insert(char * name, int id_type, int lineno, int len,int loc, int token)
{
	int h = index;
	BucketList l = hashTable[h];

		l = (BucketList)malloc(sizeof(struct BucketListRec));
		l->name = name;
		l->id_type = id_type;
		l->lines = (LineList)malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->memloc = loc;
		l->len = len;
		l->lines->next = NULL;

		BucketList temp = hashTable[h];
		while (temp->next != NULL)temp = temp->next;
		temp->next = l;
		l->next = NULL;
		/*l->next = hashTable[h]->next;
		hashTable[h]->next = l;*/

		Stack *m = Peek();
		int a = m->offset;
		m->offset += l->len;
		l->len = a;

		
	
} /* st_dec_insert */








int st_parent_lookup(char * name, int type, int num)
{
	int h = num;
	BucketList l = hashTable[h]->next;
	while ((l != NULL) && ((strcmp(name, l->name) != 0) || (type != l->id_type))) {
		if ((strcmp(name, l->name) == 0) && l->id_type == 3)break;
		if ((strcmp(name, l->name) == 0) && l->id_type == 4 && type == 1)break;
		l = l->next;
	}
	if (l == NULL) {
		if (hashTable[h]->parent == -1) { return -1; }
		else { return st_parent_lookup(name, type, hashTable[h]->parent); }	//像父节点查找

	}
	else return h;															//返回含有该声明的table
}


/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name ,int type ,int tag)					//tag=0,声明 tag=1 调用	
{ int h = index;
  BucketList l =  hashTable[h]->next;
  while ((l != NULL) && ((strcmp(name, l->name) != 0) || (type != l->id_type))) {
	  if ((strcmp(name, l->name) == 0) && l->id_type == 3&&type==0)break;
	  if ((strcmp(name, l->name) == 0) && l->id_type == 4&&type==1)break;
	  l = l->next;
  }
  if (tag == 1) {															//调用
	  if (l == NULL) {
		  if (hashTable[h]->parent == -1) { return -1; }
		  else { return st_parent_lookup(name, type, hashTable[h]->parent); }	//向父节点查找

	  }
	  else return h;
  }
  else {																	//声明
	  if (l == NULL)return -1;
	  else return h;
  }
}


/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing,"Variable Name  Location  Type    Offset  Line Numbers \n");
  fprintf(listing,"-------------  --------	 ----   ------  ------------   \n");
  for (i=0;i<SIZE;++i)
  { if (hashTable[i] != NULL)
    { 
	  printf("\n\n");
	  BucketList l = hashTable[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-14s ",l->name);
		if (l->id_type != 5) {
			fprintf(listing, "%-8d  ", l->memloc);
			fprintf(listing, "%-6d  ", l->id_type);
		}
		else {
			fprintf(listing, "%60s   %6d  ", "total size:",l->len);
		}
		if (l->id_type == 1 || l->id_type == 0) {
			fprintf(listing, "%-5d  ", l->len);
		}
		else {
			fprintf(listing, "       ");
		}
        while (t != NULL)
        { fprintf(listing,"%0d ",t->lineno);
          t = t->next;
        }

		
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
} /* printSymTab */



int findFunction(char * name) {								//查找函数
	int num = 0;
	for (; hashTable[num] != NULL&&num < SIZE; num++) {
		if (strcmp(hashTable[num]->next->name,name) == 0&& hashTable[num]->next->id_type==2) {
			return num;
		}
	}
	return -1;
}
int compare(int num,int count,int target) {				//参数
	BucketList l = hashTable[num]->next;
	int i = 0;
	while (l != NULL&&i<count) {
		l = l->next;
		i++;
	}
	if (l == NULL)return 0;
	if ((l->id_type == 3 && target == 0) || (l->id_type == 4 && target == 1))return 1;
	return 0;
}
int getReturnType(int num) {							//获取函数返回类型
	return hashTable[num]->return_type;
}
