#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"

#define LEXER_DEFAULT_BUFSIZE 32

// Key strings
#define S_DECIMAL   "1234567890"
#define S_HEXA      "1234567890abcdefABCDEF"
#define S_OPERATOR  ":><=+-*/" 
#define S_PUNCT     "(),[]" 
#define S_WHITESPC  " \t\n"
#define S_ESCAPE    " \t\n\""


char LEX_Peek( Lexer * lex );

char LEX_Get( Lexer * lex );

void LEX_AddToBuffer( Lexer * lex, char c );

void LEX_ClearBuffer( Lexer * lex );

Token * LEX_AllocToken( Lexer * lex, int type );

struct lexer
{
    char * buffer;
    int bufSize;
    int bufUsed;
    int ch;
    int line;
    bool peeked;
};

Lexer * LEX_New()
{
    Lexer* lex = malloc( sizeof( Lexer ) );
    lex->bufSize = LEXER_DEFAULT_BUFSIZE;
    lex->bufUsed = 0;
    lex->buffer = malloc( lex->bufSize );
    lex->buffer[0] = '\0';
    lex->line = 1;
    lex->peeked = false;
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

void LEX_ClearBuffer( Lexer * lex )
{
    lex->bufUsed = 0;
    lex->buffer[0] = '\0';
}

Token * LEX_AllocToken( Lexer * lex, int type )
{
    Token * tok = TOK_New( lex->buffer, type, lex->line );
    LEX_ClearBuffer( lex );
    
    return tok;
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
                Token * tok = LEX_AllocToken( lex, T_NL );
                lex->line++;
                
                return tok;
            }
            
            ch = LEX_Peek( lex );
        }
        
        // Comments
        if( ch == '/' ) 
        {
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            char c = LEX_Peek( lex );
            
            if( c == '/' || c == '*' )
            {
                for( ;; ) 
                {
                    ch = LEX_Get( lex );
                    
                    // Line comment
                    if( ch == '/' ) 
                    {
                        LEX_AddToBuffer( lex, ch );
                        ch = LEX_Peek( lex );
                        
                        while( ch != '\n' && ch != EOF ) 
                        {
                            ch = LEX_Get( lex );
                            LEX_AddToBuffer( lex, ch );
                            ch = LEX_Peek( lex ); 
                        }
                        
                        Token * tok = LEX_AllocToken( lex, T_COMMENT );
                        //lex->line++;
                        
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
                                if( ch == EOF )
                                    return LEX_AllocToken( lex, T_COMMENT );
                                    
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
                                return LEX_AllocToken( lex, T_COMMENT );
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        
        if( ch == EOF ) 
            return NULL; 
            
        // String        
        if( ch == '"' ) 
        {
            ch = LEX_Get( lex );
            //LEX_AddToBuffer( lex, ch );
            
            for( ;; ) 
            {
                ch = LEX_Get( lex );
                
                if( ch == EOF ) 
                {
                    return LEX_AllocToken( lex, T_ERROR );
                } 
                else if( ch == '\\' ) 
                {
                    ch = LEX_Get( lex );
                    
                    if( ch == '\\' || ch == '"' ) 
                    {
                        LEX_AddToBuffer( lex, ch );
                    }
                    else if( ch == 'n' )
                    {
                        LEX_AddToBuffer( lex, '\n' );
                    } 
                    else 
                    {
                        return LEX_AllocToken( lex, T_ERROR );
                    }
                } 
                else if( ch == '"' ) 
                {
                    //LEX_AddToBuffer( lex, ch );
                    return LEX_AllocToken( lex, T_LITSTRING );
                } 
                else if( ch == '\n' ) 
                {
                    return LEX_AllocToken( lex, T_ERROR );
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
                    
                    return LEX_AllocToken( lex, T_LITINT );
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
                    return LEX_AllocToken( lex, T_OCBRACKET );
                }
                if( strcmp( lex->buffer, ")" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_CCBRACKET );
                }
                if( strcmp( lex->buffer, "," ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_COMMA );
                }
                if( strcmp( lex->buffer, "[" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_OSBRACKET );
                }
                if( strcmp( lex->buffer, "]" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_CSBRACKET );
                }
            }
        }
        
        // Operators
        if( strchr( S_OPERATOR, ch ) )
        {
            // Devido a leitura de '/' em comments, isso vem primeiro
            if( strcmp( lex->buffer, "/" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_SLASH );
            }
                
            ch = LEX_Get( lex );
            LEX_AddToBuffer( lex, ch );
            
            if( ch == '<' )
            {   
                char c = LEX_Peek( lex );
                
                if( c == '=' )
                {
                    ch = LEX_Get( lex );
                    LEX_AddToBuffer( lex, ch );
                    return LEX_AllocToken( lex, T_SMALLEREQ );
                }
                else if( c == '>' )
                {
                    ch = LEX_Get( lex );
                    LEX_AddToBuffer( lex, ch );
                    return LEX_AllocToken( lex, T_NEQ );
                }
                
                return LEX_AllocToken( lex, T_SMALLER );
            }            
            else if( ch == '>' )
            {
                char c = LEX_Peek( lex );
                
                if( c == '=' )
                {
                    ch = LEX_Get( lex );
                    LEX_AddToBuffer( lex, ch );
                    return LEX_AllocToken( lex, T_LARGEREQ );
                }
                
                return LEX_AllocToken( lex, T_LARGER );
            }
            else
            {
                if( strcmp( lex->buffer, ":" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_COLON );
                }
                if( strcmp( lex->buffer, "+" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_PLUS );
                }
                if( strcmp( lex->buffer, "-" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_MINUS );
                }
                if( strcmp( lex->buffer, "*" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_ASTERISK );
                } 
                if( strcmp( lex->buffer, "=" ) == 0 ) 
                {
                    return LEX_AllocToken( lex, T_EQ );
                }               
            }
            
            // Nao tratou algum char em S_OPERATOR
            return LEX_AllocToken( lex, T_ERROR );
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
                return LEX_AllocToken( lex, T_IF );
            }
            if( strcmp( lex->buffer, "else" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_ELSE );
            }
            if( strcmp( lex->buffer, "end" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_END );
            }
            if( strcmp( lex->buffer, "while" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_WHILE );
            }
            if( strcmp( lex->buffer, "loop" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_LOOP );
            }
            if( strcmp( lex->buffer, "fun" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_FUN );
            }
            if( strcmp( lex->buffer, "return" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_RETURN );
            }
            if( strcmp( lex->buffer, "new" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_NEW );
            }
            if( strcmp( lex->buffer, "string" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_STRING );
            }
            if( strcmp( lex->buffer, "int" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_INT );
            }
            if( strcmp( lex->buffer, "char" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_CHAR );
            }
            if( strcmp( lex->buffer, "bool" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_BOOL );
            }
            if( strcmp( lex->buffer, "true" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_TRUE );
            }
            if( strcmp( lex->buffer, "false" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_FALSE );
            }
            if( strcmp( lex->buffer, "and" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_AND );
            }
            if( strcmp( lex->buffer, "or" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_OR );
            }
            if( strcmp( lex->buffer, "not" ) == 0 ) 
            {
                return LEX_AllocToken( lex, T_NOT );
            }
        }
        
        char * tr = lex->buffer;
        int i = 0;
        
        // ID's
        if( tr[i] == '_' || ( ( tr[i] >= 'a' && tr[i] <= 'z' ) || ( tr[i] >= 'A' && tr[i] <= 'Z' ) ) )
        {
            for( i = 0; i < lex->bufUsed; ++i ) 
            {          
                if( !( tr[i] == '_' || ( ( tr[i] >= 'a' && tr[i] <= 'z' ) || ( tr[i] >= 'A' && tr[i] <= 'Z' ) || strchr( S_DECIMAL, tr[i] ) ) ) )
                {                  
                    return LEX_AllocToken( lex, T_ERROR );      
                } 
            }
            
            return LEX_AllocToken( lex, T_ID );
        }
        
        if( ch == EOF ) 
            return NULL;
            
        return LEX_AllocToken( lex, T_ERROR ); 
    }
}
