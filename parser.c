#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "list.h"


void PAR_ExpandBlock( Parser * par );
void PAR_ExpandVar( Parser * par );
void PAR_ExpandCall( Parser * par );

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

void PAR_Error( Parser * par, const char * expected, const char * tip )
{
    Token * curr = ( Token* )LIS_GetCurrent( par->tokens );
    
    fprintf( stderr, "!Syntax Error [line %d]: expected %s, received \'%s\' instead.\nTIP: %s\n", TOK_GetLine( curr ), expected, TOK_GetText( curr ), tip );
    
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
    int peeked = PAR_Peek( par );
        
    if( peeked == T_STRING )
    {
        PAR_Match( par, T_STRING );
    }
    else if( peeked == T_CHAR )
    {
        PAR_Match( par, T_CHAR );
    }
    else if( peeked == T_INT )
    {
        PAR_Match( par, T_INT );
    }
    else if( peeked == T_BOOL )
    {
        PAR_Match( par, T_BOOL );
    }
    else
    {
        PAR_Error( par, "string/char/int/bool", "Did you declare base type?" );
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
    PAR_Match( par, T_COLON );
    PAR_ExpandType( par );
}

void PAR_ExpandGlobal( Parser * par )
{
    printf("PAR: Global\n");
    PAR_Match( par, T_ID );
    PAR_ExpandDeclVar( par );
    PAR_Match( par, T_NL );
}

void PAR_ExpandParam( Parser * par )
{
    printf("PAR: Param\n");
    PAR_Match( par, T_ID );
    PAR_ExpandDeclVar( par );
}

void PAR_ExpandParams( Parser * par )
{
    printf("PAR: Params\n");    
    PAR_ExpandParam( par );
        
    if( PAR_Peek( par ) == T_COMMA )
    {
        PAR_Match( par, T_COMMA );
        PAR_ExpandParams( par );       
    }
}

void PAR_ExpandExp( Parser * par )
{
    /*printf("PAR: Exp\n");
    int peeked = PAR_Peek( par );
    
    while( peeked != T_COMMA && peeked != T_CCBRACKET )
    {
        if( peeked == T_ID )
        {
            PAR_Match( par, T_ID );
            PAR_ExpandVar( par );
        }
        else if( peeked == T_LITINT )
        {
            PAR_Match( par, T_LITINT );
        }
        else if( peeked == T_LITSTRING )
        {
            PAR_Match( par, T_LITSTRING );
        }
        else if( peeked == T_TRUE )
        {
            PAR_Match( par, T_TRUE );
        }
        else if( peeked == T_FALSE )
        {
            PAR_Match( par, T_FALSE );
        }
        else if( peeked == T_NEW )
        {
            PAR_Match( par, T_NEW );
        }
    }*/
}

void PAR_ExpandExps( Parser * par )
{
    printf("PAR: Exps\n");
    int peeked = PAR_Peek( par );
    
    while( peeked != T_CCBRACKET )
    {
        PAR_ExpandExp( par );
        
        peeked = PAR_Peek( par );
        
        if( peeked == T_COMMA )
        {
            PAR_Match( par, T_COMMA );
        }
    }
}

void PAR_ExpandVar( Parser * par )
{
    printf("PAR: Var\n");    
    while( PAR_Peek( par ) == T_OSBRACKET )
    {
        PAR_Match( par, T_OSBRACKET );
        PAR_ExpandExp( par );
        PAR_Match( par, T_CSBRACKET );        
    }
}

void PAR_ExpandCmdIf( Parser * par )
{
    printf("PAR: CmdIf\n");
    PAR_Match( par, T_IF );
    PAR_ExpandExp( par );
    PAR_ExpandNewLine( par );
    PAR_ExpandBlock( par );   
    
    if( PAR_Peek( par ) == T_ELSE )
    {
        PAR_Match( par, T_ELSE );
        
        if( PAR_Peek( par ) != T_NL )
        {
            PAR_Match( par, T_IF );
            PAR_ExpandExp( par );            
        }
        
        PAR_ExpandNewLine( par );
        PAR_ExpandBlock( par );        
    }
    
    PAR_Match( par, T_END );
}

void PAR_ExpandCmdWhile( Parser * par )
{
    printf("PAR: While\n");
}

void PAR_ExpandCmdReturn( Parser * par )
{
    printf("PAR: Return\n");
}

void PAR_ExpandCmdAssign( Parser * par )
{
    printf("PAR: CmdAssign\n");       
    PAR_ExpandVar( par );
    PAR_Match( par, T_EQ );
    PAR_ExpandExp( par );
}

void PAR_ExpandCall( Parser * par )
{
    printf("PAR: Call\n");
    PAR_Match( par, T_OCBRACKET );
    PAR_ExpandExps( par );
    PAR_Match( par, T_CCBRACKET );
}

void PAR_ExpandCmd( Parser * par )
{
    printf("PAR: Cmd\n");
    int peeked = PAR_Peek( par );
    
    if( peeked == T_IF )
    {
        PAR_ExpandCmdIf( par );        
    }
    else if( peeked == T_WHILE )
    {
        PAR_ExpandCmdWhile( par );    
    }
    else if( peeked == T_RETURN )
    {
        PAR_ExpandCmdReturn( par );    
    }
    else
    {
        PAR_Error( par, "command", "Declare variables on block start." );
    }
    
    PAR_ExpandNewLine( par );
}

void PAR_ExpandBlock( Parser * par )
{
    printf("PAR: Block\n");    
    int peeked = PAR_Peek( par );
    
    //Declvar
    while( peeked == T_ID )
    {
        PAR_Match( par, T_ID );
        
        peeked = PAR_Peek( par );
        
        if( peeked == T_COLON )
        {
            PAR_ExpandDeclVar( par );
            PAR_ExpandNewLine( par );
            peeked = PAR_Peek( par );
        }
        else
        {
            break;
        }
    }

    //CMDs
    peeked = PAR_Peek( par );    
    
    if( peeked == T_EQ || peeked == T_OSBRACKET )
    {
        PAR_ExpandCmdAssign( par );
        PAR_ExpandNewLine( par );
    }
    else if( peeked == T_OCBRACKET )
    {
        PAR_ExpandCall( par );
        PAR_ExpandNewLine( par );
    }
        
    peeked = PAR_Peek( par );
    
    while( peeked != T_END && peeked != T_LOOP && peeked != T_ELSE )
    {        
        if( peeked == T_ID )
        {
            PAR_Match( par, T_ID );
            
            peeked = PAR_Peek( par );
            
            if( peeked == T_EQ || peeked == T_OSBRACKET )
            {
                PAR_ExpandCmdAssign( par );
                PAR_ExpandNewLine( par );
            }
            else if( peeked == T_OCBRACKET )
            {
                PAR_ExpandCall( par );
                PAR_ExpandNewLine( par );
            }
        }
        else
        {
            PAR_ExpandCmd( par );
        }
        
        peeked = PAR_Peek( par );
    }
    
    PAR_ExpandNewLine( par );
}

void PAR_ExpandFunction( Parser * par )
{
    printf("PAR: Function\n");
    PAR_Match( par, T_FUN );
    PAR_Match( par, T_ID );
    PAR_Match( par, T_OCBRACKET );
    
    if( PAR_Peek( par ) == T_ID )
    { 
        PAR_ExpandParams( par );
    }
    
    PAR_Match( par, T_CCBRACKET );
    
    if( PAR_Peek( par ) != T_NL )
    {
        PAR_Match( par, T_COLON );
        PAR_ExpandType( par );
    }
    
    PAR_ExpandNewLine( par );
    PAR_ExpandBlock( par );
    PAR_Match( par, T_END );
    PAR_ExpandNewLine( par );
}

void PAR_ExpandDecl( Parser * par )
{
    printf("PAR: Decl\n");
    int peeked = PAR_Peek( par );
    
    if( peeked == T_FUN )
    {
        PAR_ExpandFunction( par );
    }
    else if( peeked == T_ID )
    {
        PAR_ExpandGlobal( par );
    }
    else if( peeked == T_NL )
    {
        PAR_ExpandNewLine( par );
    }
    else
    {        
        PAR_Error( par, "function/global", "" );
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
