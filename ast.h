#ifndef AST_H
#define AST_H

#include "list.h"

// Branches
#define A_PROGRAM	0   // {FUNCTION} {DECLVAR}
#define A_TYPE		1   // terminal | {TYPE}
#define A_IF		2   // EXP BLOCK {EXP BLOCK} {BLOCK}
#define A_WHILE		3   // EXP BLOCK
#define A_DECLVAR	4   // ID TYPE
#define A_ASSIGN	5
#define A_CALL		6
#define A_FUNCTION	7
#define A_RETURN	8
#define A_BLOCK		9
#define A_ID		10
#define A_VAR       11
#define A_ARGS      12

// Expressions
#define A_ADD       13
#define A_SUBTRACT  14
#define A_DIVIDE    15
#define A_MULTIPLY  16
#define A_EQ        17
#define A_NEQ       18
#define A_LARGER    19
#define A_SMALLER   20
#define A_LARGEREQ  21
#define A_SMALLEREQ 22
#define A_AND       23
#define A_OR        24
#define A_NOT       25
#define A_NEGATIVE  26
#define A_NEW       27
#define A_LITINT    28
#define A_LITSTRING 29
#define A_TRUE      30
#define A_FALSE     31


typedef struct ast Ast;


Ast * AST_New();

void AST_Delete( Ast * ast );

void AST_PrependChildTree( Ast * parent, Ast * child );

void AST_AppendChildTree( Ast * parent, Ast * child );

void AST_PrependChildNode( Ast * parent, int type, char * text, int line );

void AST_AppendChildNode( Ast * parent, int type, char * text, int line );

int AST_TokenTypeToAst( int tokenType );

void AST_Dump( Ast * ast );

#endif
