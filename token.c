#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "token.h"

struct token
{
    int type;
    int line;
    char * text;
};

Token * TOK_New( char * text, int type, int line )
{
    Token* tok = malloc( sizeof( Token ) );
    tok->type = type;
    tok->text = strdup( text );
    tok->line = line;

    return tok;
}

void TOK_Delete( Token * tok )
{
    free( tok->text );
    free( tok );
}

void TOK_Dump( Token * tok )
{
    printf( "%d\n", tok->type );
    printf( "%s @line %d\n", tok->text, tok->line );
}
