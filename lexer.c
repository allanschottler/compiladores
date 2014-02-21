#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"

// Misc
#define LEXER_DEFAULT_BUFSIZE 32
#define ERROR 666

// Palavras reservadas
#define R_IF 		(1<<0)
#define R_ELSE 		(1<<1)
#define R_END 		(1<<2) 
#define R_WHILE 	(1<<3)
#define R_LOOP 		(1<<4)
#define R_FUN 		(1<<5)
#define R_RETURN 	(1<<6)
#define R_NEW 		(1<<7)
#define R_STRING 	(1<<8)
#define R_INT 		(1<<9)
#define R_CHAR 		(1<<10)
#define R_BOOL 		(1<<11)
#define R_TRUE 		(1<<12)
#define R_FALSE 	(1<<13)
#define R_AND 		(1<<14)
#define R_OR 		(1<<15)
#define R_NOT 		(1<<16)

// Operadores e pontuação
#define O_OCBRACKET	(1<<17)
#define O_CCBRACKET	(1<<18)
#define O_COMMA		(1<<19)
#define O_COLON		(1<<20)
#define O_LARGER	(1<<21)
#define O_SMALLER	(1<<22)
#define O_LARGEREQ	(1<<23)
#define O_SMALLEREQ	(1<<24)
#define O_EQ		(1<<25)
#define O_ABRACKETS	(1<<26)
#define O_OSBRACKET	(1<<27)
#define O_CSBRACKET	(1<<28)
#define O_PLUS		(1<<29)
#define O_MINUS		(1<<30)
#define O_ASTERISK	(1<<31)
#define O_SLASH		(1<<32)

struct token
{
   	int type;
   	int line;
   	char* text;
};

struct lexer
{
	char* buffer;
   	int bufSize;
  	int bufUsed;
   	int ch;
   	int line;
   	bool peeked;
};


Token * TOK_New( Lexer * lex, int type )
{
	Token* tok = malloc( sizeof( Token ) );
	tok->type = type;
	tok->text = strdup( lex->buffer );
	tok->line = lex->line;
	lex->bufUsed = 0;
	lex->buffer[0] = '\0';
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


Lexer * LEX_New()
{
	Lexer* lex = malloc( sizeof( Lexer ) );
	lex->bufSize = LEXER_DEFAULT_BUFSIZE;
	lex->bufUsed = 0;
	lex->buffer = malloc( lex->bufSize );
	lex->buffer[0] = '\0';
	lex->line = 1;
	return lex;
}

void LEX_Delete( Lexer * lex )
{
	free( lex->buffer );
	free( lex );
}

char LEX_Peek( Lexer * lex )
{
	if( lex->peeked )	
		return lex->ch;
	
    char c = getchar();
    lex->ch = c;
    lex->peeked = true;
    return c;
}

char LEX_Get( Lexer * lex )
{
    if( lex->peeked )
    {
        lex->peeked = false;
        return lex->ch;
    }
    
    char c = getchar();
    lex->ch = c;
    return c;
}

void LEX_AddToBuffer( Lexer * lex, char c )
{
    lex->buffer[lex->bufUsed] = c;
    lex->bufUsed++;
    
    if( lex->bufUsed == lex->bufSize )
    {
        lex->bufSize *= 2;
        lex->buffer = realloc( lex->buffer, lex->bufSize );
    }
    
    lex->buffer[lex->bufUsed] = '\0';
}

Token * LEX_NextToken( Lexer * lex )
{
    for( ;; ) 
    {
        char ch = LEX_Peek( lex );

        // Whitespace
        // [ \t\n]*
        while( strchr( " \t\n", ch ) ) 
        {
            ch = LEX_Get( lex );
            
            if( ch == '\n' ) 
            {
                lex->line++;
            }
            
            ch = LEX_Peek( lex );
        }

        // String
        // \"([^\n"\\]|\\["\\])*\"
        if( ch == '"' ) 
        {
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            
            for( ;; ) 
            {
                ch = LEX_Get( lex );
                if( ch == EOF ) 
                {
                    return TOK_New( lex, ERROR );
                } 
                else if( ch == '\\' ) 
                {
                    ch = LEX_Get( lex );
                    if( ch == '\\' || ch == '"' ) 
                    {
                        LEX_AddToBuffer( lex, ch );
                    } 
                    else 
                    {
                        return TOK_New( lex, ERROR );
                    }
                } 
                else if( ch == '"' ) 
                {
                    LEX_AddToBuffer( lex, ch );
                    return TOK_New( lex, R_STRING );
                } 
                else if( ch == '\n' ) 
                {
                    return TOK_New( lex, ERROR );
                } 
                else 
                {
                    LEX_AddToBuffer( lex, ch );
                }
            }
        }

        if( ch == EOF ) 
        {
            return NULL;
        }

        // Keywords
        while( !strchr( " \t\n\"", ch ) ) 
        {
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            ch = LEX_Peek( lex );
            
            if( ch == EOF ) 
                return NULL;            
        }

        if( lex->bufUsed > 0 ) 
        {
            if( strcmp( lex->buffer, "if" ) == 0 ) 
            {
                return TOK_New( lex, R_IF );
            }
            if( strcmp( lex->buffer, "else" ) == 0 ) 
            {
                return TOK_New( lex, R_ELSE );
            }
            if( strcmp( lex->buffer, "end" ) == 0 ) 
            {
                return TOK_New( lex, R_END );
            }
            if( strcmp( lex->buffer, "while" ) == 0 ) 
            {
                return TOK_New( lex, R_WHILE );
            }
            if( strcmp( lex->buffer, "loop" ) == 0 ) 
            {
                return TOK_New( lex, R_LOOP );
            }
            if( strcmp( lex->buffer, "fun" ) == 0 ) 
            {
                return TOK_New( lex, R_FUN );
            }
            if( strcmp( lex->buffer, "return" ) == 0 ) 
            {
                return TOK_New( lex, R_RETURN );
            }
            if( strcmp( lex->buffer, "new" ) == 0 ) 
            {
                return TOK_New( lex, R_NEW );
            }
            if( strcmp( lex->buffer, "string" ) == 0 ) 
            {
                return TOK_New( lex, R_STRING );
            }
            if( strcmp( lex->buffer, "int" ) == 0 ) 
            {
                return TOK_New( lex, R_INT );
            }
            if( strcmp( lex->buffer, "char" ) == 0 ) 
            {
                return TOK_New( lex, R_CHAR );
            }
            if( strcmp( lex->buffer, "bool" ) == 0 ) 
            {
                return TOK_New( lex, R_BOOL );
            }
            if( strcmp( lex->buffer, "true" ) == 0 ) 
            {
                return TOK_New( lex, R_TRUE );
            }
            if( strcmp( lex->buffer, "false" ) == 0 ) 
            {
                return TOK_New( lex, R_FALSE );
            }
            if( strcmp( lex->buffer, "and" ) == 0 ) 
            {
                return TOK_New( lex, R_AND );
            }
            if( strcmp( lex->buffer, "or" ) == 0 ) 
            {
                return TOK_New( lex, R_OR );
            }
            if( strcmp( lex->buffer, "not" ) == 0 ) 
            {
                return TOK_New( lex, R_NOT );
            }
            
            return TOK_New( lex, ERROR );
        }
    }
}
