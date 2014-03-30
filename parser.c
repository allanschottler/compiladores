#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "list.h"


struct parser
{
    List * tokens;
};

int PAR_Peek( Parser * par )
{
    return LIS_Peek( par->tokens, &TOK_GetType );
}

void PAR_Match( Parser * par, int type )
{
    LIS_Match( par->tokens, type, &TOK_IsType, &TOK_MatchError );
}

void PAR_Error( Parser * par, const char * expected )
{
    Token * curr = ( Token* )LIS_GetCurrent( par->tokens );
    
    fprintf( stderr, "!Syntax Error [line %d]: expected %s, received %s instead.\n", TOK_GetLine( curr ), expected, TOK_GetText( curr ) );
    
    exit( EXIT_FAILURE );
} 

/********************************************************************/

void PAR_ExpandNewLine( Parser * par )
{    
    if( PAR_Peek( par ) == T_NL )
    {
        PAR_Match( par, T_NL );        
        PAR_ExpandNewLine( par );
    }
}

void PAR_ExpandBaseType( Parser * par )
{
    printf("PAR: BaseType\n");    
    if( PAR_Peek( par ) == T_STRING )
    {
        PAR_Match( par, T_STRING );
    }
    else if( PAR_Peek( par ) == T_CHAR )
    {
        PAR_Match( par, T_CHAR );
    }
    else if( PAR_Peek( par ) == T_INT )
    {
        PAR_Match( par, T_INT );
    }
    else if( PAR_Peek( par ) == T_BOOL )
    {
        PAR_Match( par, T_BOOL );
    }
    else
    {
        PAR_Error( par, "string/char/int/bool" );
    }
}

void PAR_ExpandType( Parser * par )
{
    printf("PAR: Type\n");
    if( PAR_Peek( par ) == T_OSBRACKET )
    {
        PAR_Match( par, T_OSBRACKET );
        PAR_Match( par, T_CSBRACKET );
        PAR_ExpandType( par );
    }
    else
    {
        PAR_ExpandBaseType( par );        
    } 
}

void PAR_ExpandDeclVar( Parser * par )
{
    printf("PAR: DeclVar\n");
    PAR_Match( par, T_ID );
    PAR_Match( par, T_COLON );
    PAR_ExpandType( par );
}

void PAR_ExpandGlobal( Parser * par )
{
    printf("PAR: Global\n");
    PAR_ExpandDeclVar( par );
    PAR_Match( par, T_NL );
}

void PAR_ExpandParam( Parser * par );

void PAR_ExpandParams( Parser * par )
{

}

void PAR_ExpandBlock( Parser * par )
{
    PAR_ExpandNewLine( par );
}

void PAR_ExpandFunction( Parser * par )
{
    printf("PAR: Function\n");
    PAR_Match( par, T_FUN );
    PAR_Match( par, T_ID );
    PAR_Match( par, T_OCBRACKET );
    //PAR_ExpandParams( par );
    PAR_Match( par, T_CCBRACKET );
    
    if( PAR_Peek( par ) != T_NL )
    {
        PAR_Match( par, T_COLON );
        PAR_ExpandType( par );
    }
    
    PAR_ExpandNewLine( par );
    //PAR_ExpandBlock( par );
    PAR_Match( par, T_END );
    PAR_ExpandNewLine( par );
}

void PAR_ExpandDecl( Parser * par )
{
    printf("PAR: Decl\n");
    if( PAR_Peek( par ) == T_FUN )
    {
        PAR_ExpandFunction( par );
    }
    else if( PAR_Peek( par ) == T_ID )
    {
        PAR_ExpandGlobal( par );
    }
    else if( PAR_Peek( par ) == T_NL )
    {
        PAR_ExpandNewLine( par );
    }
    else
    {        
        PAR_Error( par, "function/global" );
    }
}

void PAR_ExpandProgram( Parser * par )
{
    printf("PAR: Program\n");
    PAR_ExpandNewLine( par );
    
    while( PAR_Peek( par ) != -1 )
    {
        PAR_ExpandDecl( par );
    }
}

void PAR_ExpandCmd( Parser * par );

void PAR_ExpandCmdIf( Parser * par );

void PAR_ExpandCmdWhile( Parser * par );

void PAR_ExpandCmdAssign( Parser * par );

void PAR_ExpandReturn( Parser * par );

void PAR_ExpandCall( Parser * par );

void PAR_ExpandExps( Parser * par );

void PAR_ExpandExp( Parser * par );

void PAR_ExpandVar( Parser * par );

/********************************************************************/

Parser * PAR_New()
{
    Parser * par = ( Parser* )malloc( sizeof( Parser ) );
    par->tokens = LIS_New();
    
    return par;
}

void PAR_Delete( Parser * par )
{
    LIS_Delete( par->tokens, &TOK_Delete );
}

void PAR_Push( Parser * par, Token * tok )
{
    LIS_PushBack( par->tokens, tok );
}

void PAR_Execute( Parser * par )
{
    if( LIS_GetSize( par->tokens ) )
        PAR_ExpandProgram( par );
}

void PAR_DumpTokens( Parser * par )
{
    LIS_Dump( par->tokens, &TOK_Dump );
}
