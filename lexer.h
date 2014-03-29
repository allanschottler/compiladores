#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct lexer Lexer;


Lexer * LEX_New();

void LEX_Delete( Lexer * lex );

Token * LEX_NextToken( Lexer * lex );

#endif
