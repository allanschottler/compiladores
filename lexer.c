#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"

#define LEXER_DEFAULT_BUFSIZE 32

// Palavras reservadas
#define T_IF 		0
#define T_ELSE 		1
#define T_END 		2 
#define T_WHILE 	3
#define T_LOOP 		4
#define T_FUN 		5
#define T_RETURN 	6
#define T_NEW 		7
#define T_STRING 	8
#define T_INT 		9
#define T_CHAR 		10
#define T_BOOL 		11
#define T_TRUE 		12
#define T_FALSE 	13
#define T_AND 		14
#define T_OR 		15
#define T_NOT 		16

// Operadores e pontuação
#define T_OCBRACKET	17
#define T_CCBRACKET	18
#define T_COMMA		19
#define T_COLON		20
#define T_LARGER	21
#define T_SMALLER	22
#define T_LARGEREQ	23
#define T_SMALLEREQ	24
#define T_EQ		25
#define T_NEQ    	26
#define T_OSBRACKET	27
#define T_CSBRACKET	28
#define T_PLUS		29
#define T_MINUS		30
#define T_ASTERISK	31
#define T_SLASH		32

// Misc
#define T_COMMENT   777
#define T_ID        999
#define T_ERROR     666

// Key strings
#define S_DECIMAL   "1234567890"
#define S_HEXA      "1234567890abcdefABCDEF"
#define S_OPERATOR  ":><=+-*/" 
#define S_PUNCT     "(),[]" 
#define S_WHITESPC  " \t\n"
#define S_ESCAPE    " \t\n\""


struct token
{
    int type;
    int line;
    char * text;
};

struct lexer
{
    char * buffer;
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
        while( strchr( S_WHITESPC, ch ) ) 
        {
            ch = LEX_Get( lex );
            
            if( ch == '\n' ) 
            {
                lex->line++;
            }
            
            ch = LEX_Peek( lex );
        }
        
        // Comments
        if( ch == '/' ) 
        {
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            
            for( ;; ) 
            {
                ch = LEX_Get( lex );
                
                // Line comment
                if( ch == '/' ) 
                {
                    LEX_AddToBuffer( lex, ch );
                    
                    while( ch != '\n' ) 
                    {
                        ch = LEX_Get( lex );
                        LEX_AddToBuffer( lex, ch ); 
                    }
                    
                    Token * tok = TOK_New( lex, T_COMMENT );
                    lex->line++;
                    
                    return tok;
                }
                // Nested comment
                else if( ch == '*' )
                {
                    LEX_AddToBuffer( lex, ch );
                                   
                    for( ;; )
                    {
                        ch = LEX_Get( lex );
                        
                        if( ch == '\n' ) 
                        {
                            lex->line++;
                        }
                            
                        while( ch != '*' ) 
                        {
                            LEX_AddToBuffer( lex, ch );
                            
                            if( ch == '\n' )
                                lex->line++;                            
                            
                            ch = LEX_Get( lex );
                        }                        
                        
                        LEX_AddToBuffer( lex, ch );
                        ch = LEX_Peek( lex );
                        
                        if( ch == '/' )
                        {
                            ch = LEX_Get( lex );
                            LEX_AddToBuffer( lex, ch );
                            return TOK_New( lex, T_COMMENT );
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }
        
        if( ch == EOF ) 
            return NULL; 
            
        // String        
        if( ch == '"' ) 
        {
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            
            for( ;; ) 
            {
                ch = LEX_Get( lex );
                
                if( ch == EOF ) 
                {
                    return TOK_New( lex, T_ERROR );
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
                        return TOK_New( lex, T_ERROR );
                    }
                } 
                else if( ch == '"' ) 
                {
                    LEX_AddToBuffer( lex, ch );
                    return TOK_New( lex, T_STRING );
                } 
                else if( ch == '\n' ) 
                {
                    return TOK_New( lex, T_ERROR );
                } 
                else 
                {
                    LEX_AddToBuffer( lex, ch );
                }
            }
        }
        
        if( ch == EOF ) 
            return NULL; 
            
        // Number
        if( strchr( S_DECIMAL, ch ) ) 
        {
            bool isHexa = false;
            ch = LEX_Get( lex );
            
            LEX_AddToBuffer( lex, ch );
            
            // Trata hexadecimal
            if( ch == '0' )
            {
                ch = LEX_Peek( lex );
                
                if( ch == 'x' )
                {                    
                    ch = LEX_Get( lex );
                    LEX_AddToBuffer( lex, ch );
                    
                    isHexa = true;
                }
            }
            
            for( ;; ) 
            {
                ch = LEX_Peek( lex );
                
                if( ch == EOF || strchr( S_ESCAPE, ch ) || ( isHexa && !strchr( S_HEXA, ch ) ) || ( !isHexa && !strchr( S_DECIMAL, ch ) ) ) 
                {
                    if( isHexa )
                    {
                        int value = strtol( lex->buffer, NULL, 16 );
                        sprintf( lex->buffer, "%d", value );
                    }
                    
                    Token * tok = TOK_New( lex, T_INT );
                    
                    if( ch == '\n' )
                    {
                        ch = LEX_Get( lex );
                        lex->line++;
                    }
                    
                    return tok;
                }
                else 
                {
                    ch = LEX_Get( lex );
                    LEX_AddToBuffer( lex, ch );
                }
            }
        }
        
        if( ch == EOF ) 
            return NULL;       
        
        // Punctuation
        if( strchr( S_PUNCT, ch ) ) 
        {
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            
            if( lex->bufUsed > 0 ) 
            { 
                if( strcmp( lex->buffer, "(" ) == 0 ) 
                {
                    return TOK_New( lex, T_OCBRACKET );
                }
                if( strcmp( lex->buffer, ")" ) == 0 ) 
                {
                    return TOK_New( lex, T_CCBRACKET );
                }
                if( strcmp( lex->buffer, "," ) == 0 ) 
                {
                    return TOK_New( lex, T_COMMA );
                }
                if( strcmp( lex->buffer, "[" ) == 0 ) 
                {
                    return TOK_New( lex, T_OSBRACKET );
                }
                if( strcmp( lex->buffer, "]" ) == 0 ) 
                {
                    return TOK_New( lex, T_CSBRACKET );
                }
            }
        }
        
        // Operators
        while( strchr( S_OPERATOR, ch ) ) 
        {
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            ch = LEX_Peek( lex );
            
            if( ch == EOF ) 
                return NULL;            
        }
        
        if( lex->bufUsed > 0 ) 
        {
            if( strcmp( lex->buffer, ":" ) == 0 ) 
            {
                return TOK_New( lex, T_COLON );
            }
            if( strcmp( lex->buffer, ">" ) == 0 ) 
            {
                return TOK_New( lex, T_LARGER );
            }
            if( strcmp( lex->buffer, "<" ) == 0 ) 
            {
                return TOK_New( lex, T_SMALLER );
            }
            if( strcmp( lex->buffer, ">=" ) == 0 ) 
            {
                return TOK_New( lex, T_LARGEREQ );
            }
            if( strcmp( lex->buffer, "<=" ) == 0 ) 
            {
                return TOK_New( lex, T_SMALLEREQ );
            }
            if( strcmp( lex->buffer, "=" ) == 0 ) 
            {
                return TOK_New( lex, T_EQ );
            }
            if( strcmp( lex->buffer, "<>" ) == 0 ) 
            {
                return TOK_New( lex, T_NEQ );
            }            
            if( strcmp( lex->buffer, "+" ) == 0 ) 
            {
                return TOK_New( lex, T_PLUS );
            }
            if( strcmp( lex->buffer, "-" ) == 0 ) 
            {
                return TOK_New( lex, T_MINUS );
            }
            if( strcmp( lex->buffer, "*" ) == 0 ) 
            {
                return TOK_New( lex, T_ASTERISK );
            }
            if( strcmp( lex->buffer, "/" ) == 0 ) 
            {
                return TOK_New( lex, T_SLASH );
            }
        }
        
        // Keywords
        while( !strchr( S_ESCAPE, ch ) && !strchr( S_OPERATOR, ch ) && !strchr( S_PUNCT, ch ) ) 
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
                return TOK_New( lex, T_IF );
            }
            if( strcmp( lex->buffer, "else" ) == 0 ) 
            {
                return TOK_New( lex, T_ELSE );
            }
            if( strcmp( lex->buffer, "end" ) == 0 ) 
            {
                return TOK_New( lex, T_END );
            }
            if( strcmp( lex->buffer, "while" ) == 0 ) 
            {
                return TOK_New( lex, T_WHILE );
            }
            if( strcmp( lex->buffer, "loop" ) == 0 ) 
            {
                return TOK_New( lex, T_LOOP );
            }
            if( strcmp( lex->buffer, "fun" ) == 0 ) 
            {
                return TOK_New( lex, T_FUN );
            }
            if( strcmp( lex->buffer, "return" ) == 0 ) 
            {
                return TOK_New( lex, T_RETURN );
            }
            if( strcmp( lex->buffer, "new" ) == 0 ) 
            {
                return TOK_New( lex, T_NEW );
            }
            if( strcmp( lex->buffer, "string" ) == 0 ) 
            {
                return TOK_New( lex, T_STRING );
            }
            if( strcmp( lex->buffer, "int" ) == 0 ) 
            {
                return TOK_New( lex, T_INT );
            }
            if( strcmp( lex->buffer, "char" ) == 0 ) 
            {
                return TOK_New( lex, T_CHAR );
            }
            if( strcmp( lex->buffer, "bool" ) == 0 ) 
            {
                return TOK_New( lex, T_BOOL );
            }
            if( strcmp( lex->buffer, "true" ) == 0 ) 
            {
                return TOK_New( lex, T_TRUE );
            }
            if( strcmp( lex->buffer, "false" ) == 0 ) 
            {
                return TOK_New( lex, T_FALSE );
            }
            if( strcmp( lex->buffer, "and" ) == 0 ) 
            {
                return TOK_New( lex, T_AND );
            }
            if( strcmp( lex->buffer, "or" ) == 0 ) 
            {
                return TOK_New( lex, T_OR );
            }
            if( strcmp( lex->buffer, "not" ) == 0 ) 
            {
                return TOK_New( lex, T_NOT );
            }
        }
        
        char * tr = lex->buffer;
        int i = 0;
        
        // ID's
        if( tr[i] == '_' || ( ( tr[i] >= 'a' && tr[i] <= 'z' ) || ( tr[i] >= 'A' && tr[i] <= 'Z' ) ) )
        {
            for( i = 0; i < lex->bufUsed; ++i ) 
            {          
                if( tr[i] != '_' && ( tr[i] < 'a' || tr[i] > 'z' ) && ( tr[i] < 'A' || tr[i] > 'Z' ) && !strchr( S_DECIMAL, tr[i] ) )
                {                  
                    return TOK_New( lex, T_ID );      
                } 
            }
            
            return TOK_New( lex, T_ID );
        }
        
        if( ch == EOF ) 
            return NULL;
            
        return TOK_New( lex, T_ERROR ); 
    }
}
