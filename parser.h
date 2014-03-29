#ifndef PARSER_H
#define PARSER_H

#include "token.h"


typedef struct parser Parser;


Parser * PAR_New();

void PAR_Delete( Parser * par );

void PAR_Execute( Parser * par );

void PAR_Push( Parser * par, Token * tok );

#endif
