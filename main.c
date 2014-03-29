#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"


void errorLexer( Token * tok )
{
    fprintf( stderr, "!Lexing Error [line %d]: Unidentified Token %s.\n", TOK_GetLine( tok ), TOK_GetText( tok ) );
    
    exit( EXIT_FAILURE );
}

int main( int argc, char * argv[] )
{
	if( argc < 2 )
	{
	    printf( "Not enough arguments.\n" );
		return;
	}
		
	Lexer * lex = LEX_New();
	Parser * par = PAR_New();
	
	freopen( argv[1], "r", stdin );
	
    for( ;; ) 
    {
        Token * tok = LEX_NextToken( lex );
        
        if( !tok )         
            break;
            
        if( TOK_GetType( tok ) == T_ERROR )
        {
            PAR_Delete( par );
            LEX_Delete( lex );
            errorLexer( tok );
        }
        
        if( TOK_GetType( tok ) == T_COMMENT )
        {
            TOK_Delete( tok );
            continue;
        }      

        PAR_Push( par, tok );
    }

    PAR_Execute( par );
    
    PAR_Delete( par );
    LEX_Delete( lex );
   
	return 0;
}
