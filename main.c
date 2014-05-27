#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "symtable.h"
#include "icr.h"


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
	
	FILE* fp = freopen( argv[1], "r", stdin );
	if( !fp )
	{
	    printf( "File doesn't exist.\n" );
		return EXIT_FAILURE;
	}
	
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
    PAR_Delete( par );    
    
    SymTable * syt = SYT_New();
    SYT_Build( syt, ast );
    SYT_Delete( syt );
    
    AST_Dump( ast );
        
    Icr * icr = ICR_New();    
    ICR_Build( icr, ast );
    
    char path[64];
    sprintf( path, "%s.ic", argv[1] ); 
    ICR_WriteToFile( icr, path );   
    
	return EXIT_SUCCESS;
}
