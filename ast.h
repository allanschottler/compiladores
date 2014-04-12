#ifndef AST_H
#define AST_H

#include "list.h"

// Branches
#define A_PROGRAM	0
#define A_TYPE		1
#define A_IF		2
#define A_WHILE		3
#define A_DECLVAR	4
#define A_ASSIGN	5
#define A_CALL		6
#define A_FUNCTION	7
#define A_RETURN	8
#define A_BLOCK		9
#define A_ID		10
#define A_VAR       11
#define A_ARGS      12

typedef struct ast Ast;


Ast * AST_New();

void AST_Delete( Ast * ast );

void AST_AddChildTree( Ast * parent, Ast * child );

void AST_AddChildNode( Ast * parent, int type, char * text );

void AST_Dump( Ast * ast );

#endif
