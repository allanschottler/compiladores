#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "list.h"
#include "ast.h"

typedef struct parser Parser;


Parser * PAR_New();

void PAR_Delete( Parser * par );

void PAR_Execute( Parser * par, List * tokens );

void PAR_DumpTokens( Parser * par );

Ast * PAR_GetAst( Parser * par );

#endif
