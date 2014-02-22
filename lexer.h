#ifndef LEXER_H
#define LEXER_H

typedef struct token Token;

typedef struct lexer Lexer;


Token * TOK_New( Lexer * lex, int type );

void TOK_Delete( Token * tok );

void TOK_Dump( Token * tok );


Lexer * LEX_New();

void LEX_Delete( Lexer * lex );

char LEX_Peek( Lexer * lex );

char LEX_Get( Lexer * lex );

void LEX_AddToBuffer( Lexer * lex, char c );

Token * LEX_NextToken( Lexer * lex );

#endif
