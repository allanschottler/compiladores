#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "list.h"


typedef struct parser Parser;


Parser * PAR_New( List * tokens );

void PAR_Delete( Parser * par );

void PAR_Execute( Parser * par );

void PAR_DumpTokens( Parser * par );

#endif
