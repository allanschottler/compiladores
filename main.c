#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "list.h"


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
	List * list = LIS_New();
	
	freopen( argv[1], "r", stdin );
	
    for( ;; ) 
    {
        Token * tok = LEX_NextToken( lex );
        
        if( !tok )         
            break;
            
        if( TOK_GetType( tok ) == T_ERROR )
        {
            LIS_Delete( list, &TOK_Delete );
            LEX_Delete( lex );
            errorLexer( tok );
        }      

        LIS_PushBack( list, tok );
    }

    LIS_Dump( list, &TOK_Dump );
    
    LIS_Delete( list, &TOK_Delete );
    LEX_Delete( lex );
   
	return 0;
}
