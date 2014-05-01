#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "symtable.h"


void errorLexer( Token * tok )
{
    fprintf( stderr, "!Lexing Error [line %d]: Unidentified Token \'%s\'.\n", TOK_GetLine( tok ), TOK_GetText( tok ) );
    
    exit( EXIT_FAILURE );
}

int main( int argc, char * argv[] )
{
	if( argc < 2 )
	{
	    printf( "Not enough arguments.\n" );
		return EXIT_FAILURE;
	}
		
	Lexer * lex = LEX_New();
	List * tokens = LIS_New();	
	
	freopen( argv[1], "r", stdin );
	
    for( ;; ) 
    {
        Token * tok = LEX_NextToken( lex );
        
        if( !tok )         
            break;
            
        if( TOK_GetType( tok ) == T_ERROR )
        {
            LIS_Delete( tokens, &TOK_Delete );
            LEX_Delete( lex );
            errorLexer( tok );
        }
        
        if( TOK_GetType( tok ) == T_COMMENT )
        {
            TOK_Delete( tok );
            continue;
        }              

        LIS_PushBack( tokens, tok );
    }
    
    LEX_Delete( lex );
    
    Parser * par = PAR_New();    
    PAR_Execute( par, tokens );
    
    Ast * ast = PAR_GetAst( par );
    SymTable * syt = SYT_New();
    SYT_Build( syt, ast );    
        
    AST_Dump( ast );
    
    SYT_Delete( syt );
    PAR_Delete( par );    
    //AST_Delete( ast );
    
	return EXIT_SUCCESS;
}
