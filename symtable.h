#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"


typedef struct symtable SymTable;


SymTable * SYT_New();

void SYT_Delete( SymTable * syt );

void SYT_Build( SymTable * syt, Ast * ast );

#endif
