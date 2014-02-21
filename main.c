#include <stdio.h>

#include "lexer.h"

int main( int argc, char * argv[] )
{
	if( argc < 2 )
	{
	    printf( "Not enough arguments.\n" );
		return;
	}
		
	Lexer * lex = LEX_New();
	
	freopen( argv[1], "r", stdin );
	
    for( ;; ) 
    {
        Token * tok = LEX_NextToken( lex );
        
        if( !tok )         
            break;       

        TOK_Dump( tok );
        TOK_Delete( tok );
    }

    LEX_Delete( lex );
   
	return 0;
}
