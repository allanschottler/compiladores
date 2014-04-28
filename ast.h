#ifndef AST_H
#define AST_H

#include "list.h"

/*****************************************************************
DOCUMENTACAO:

Abaixo está especificado como estão organizados os filhos de
cada nó. Seguindo uma notação parecida com EBNF, [ x ] significa 
que x é opcional; { x } significa que x pode aparecer zero ou 
mais vezes. "Terminal" indica que o nó possui um valor final e
portanto não possui filhos. Entre parenteses estão comentários
relevantes.
OBS: Utiliza-se EXP (embora não haja um nó do tipo EXP) para
expressar composição de "terminals" da parte "Expressions". 
*****************************************************************/

// Branches
#define A_PROGRAM	0   // {FUNCTION} {DECLVAR}
#define A_TYPE		1   // terminal | {TYPE}
#define A_IF		2   // EXP BLOCK {EXP BLOCK} [BLOCK]  (1o EXP-BLOCK é IF, EXP-BLOCK opcionais são ELSE IF e o ultimo bloco é ELSE opcional)
#define A_WHILE		3   // EXP BLOCK
#define A_DECLVAR	4   // ID TYPE
#define A_ASSIGN	5   // VAR EXP
#define A_CALL		6   // ID {EXP}
#define A_FUNCTION	7   // ID {DECLVAR} [TYPE]
#define A_RETURN	8   // EXP
#define A_BLOCK		9   // {DECLVAR} {ASSIGN | CALL | CMD} (CMD sendo IF, WHILE, RETURN)
#define A_ID		10  // terminal
#define A_VAR       11  // ID [EXP]
#define A_ARGS      12  // {EXP}
#define A_PARAMS    13  // {DECLVAR}

// Expressions
#define A_ADD       14  // EXP EXP
#define A_SUBTRACT  15  // EXP EXP  
#define A_DIVIDE    16  // EXP EXP
#define A_MULTIPLY  17  // EXP EXP
#define A_EQ        18  // EXP EXP
#define A_NEQ       19  // EXP EXP
#define A_LARGER    20  // EXP EXP
#define A_SMALLER   21  // EXP EXP
#define A_LARGEREQ  22  // EXP EXP
#define A_SMALLEREQ 23  // EXP EXP
#define A_AND       24  // EXP EXP
#define A_OR        25  // EXP EXP
#define A_NOT       26  // EXP
#define A_NEGATIVE  27  // EXP
#define A_NEW       28  // ID EXP TYPE
#define A_LITINT    29  // terminal
#define A_LITSTRING 30  // terminal
#define A_TRUE      31  // terminal
#define A_FALSE     32  // terminal


typedef struct ast Ast;


Ast * AST_New();

void AST_Delete( Ast * ast );

void AST_PrependChildTree( Ast * parent, Ast * child );

void AST_AppendChildTree( Ast * parent, Ast * child );

void AST_PrependChildNode( Ast * parent, int type, char * text, int line );

void AST_AppendChildNode( Ast * parent, int type, char * text, int line );

int AST_TokenTypeToAst( int tokenType );

void AST_Dump( Ast * ast );

/*int AST_GetChildrenArray( Ast * ast, Ast *** children );

void AST_FreeChildrenArray( Ast ** children, int nChildren );*/

int AST_GetType( Ast * ast );

char * AST_GetValue( Ast * ast );

int AST_GetLine( Ast * ast );

Ast * AST_GetChild( Ast * ast );

Ast * AST_NextSibling( Ast * ast );

char * AST_FindId( Ast * ast );

char * AST_FindType( Ast * ast );

#endif
