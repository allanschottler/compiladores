#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"

#define S_NONE		0
#define S_BOOL		1
#define S_CHAR		2
#define S_INT		3
#define S_STRING	4


typedef struct symtable SymTable;


SymTable * SYT_New();

void SYT_Delete( SymTable * syt );

void SYT_Build( SymTable * syt, Ast * ast );

#endif
