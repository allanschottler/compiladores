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

void TOK_Delete( void * tok )
{
    Token * t = (Token*)tok;
    free( t->text );
    free( t );
}

int TOK_Compare( Token * tok1, Token * tok2 )
{
    return ( tok1->type == tok2->type );
}

void TOK_MatchError( Token * expected, Token * received )
{
    fprintf( stderr, "!Syntax Error [line %d]: expected Token type %d, received Token type %d instead.\n", received->line, expected->type, received->type );
    
    exit( EXIT_FAILURE );
}

void TOK_Dump( void * tok )
{
    Token * t = (Token*)tok;
    printf( "%d\n", t->type );
    printf( "%s @line %d\n", t->text, t->line );
}

int TOK_GetType( Token * tok )
{
    return tok->type;
}

char * TOK_GetText( Token * tok )
{
    return tok->text;
}

int TOK_GetLine( Token * tok )
{
    return tok->line;
}
