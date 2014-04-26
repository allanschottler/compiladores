#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "ast.h"


Ast * PAR_ExpandBlock( Parser * par );
Ast * PAR_ExpandVar( Parser * par );
Ast * PAR_ExpandCall( Parser * par );
Ast * PAR_ExpandExp( Parser * par );

struct parser
{
    List * tokens;
    Ast * ast;
    Token * lastMatched;
};

int PAR_Peek( Parser * par )
{
    return LIS_Peek( par->tokens, &TOK_GetType );
}

void PAR_Match( Parser * par, int type )
{
    Token * lastMatched = LIS_Match( par->tokens, type, &TOK_IsType, &TOK_MatchError );
    
    if( TOK_GetType( lastMatched ) != T_NL )
        par->lastMatched = lastMatched;
}

void PAR_Error( Parser * par, const char * expected, const char * tip )
{
    Token * curr = ( Token* )LIS_GetCurrent( par->tokens );
    
    if( curr )
    {
    	fprintf( stderr, "!Syntax Error [line %d]: expected %s, received \'%s\' instead.\nTIP: %s\n", TOK_GetLine( curr ), expected, TOK_GetText( curr ), tip );
    }
    else
    {
    	fprintf( stderr, "!Syntax Error: expected %s, received EOF instead.\nTIP: %s\n", expected, tip );
    }
    
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

Ast * PAR_ExpandBaseType( Parser * par )
{
    //printf("PAR: BaseType\n");
    Ast * ast = AST_New();
    int peeked = PAR_Peek( par );
        
    if( peeked == T_STRING )
    {
        PAR_Match( par, T_STRING );
        AST_AppendChildNode( ast, A_TYPE, "string", TOK_GetLine( par->lastMatched ) );
    }
    else if( peeked == T_CHAR )
    {
        PAR_Match( par, T_CHAR );
        AST_AppendChildNode( ast, A_TYPE, "char", TOK_GetLine( par->lastMatched ) );
    }
    else if( peeked == T_INT )
    {
        PAR_Match( par, T_INT );
        AST_AppendChildNode( ast, A_TYPE, "int", TOK_GetLine( par->lastMatched ) );        
    }
    else if( peeked == T_BOOL )
    {
        PAR_Match( par, T_BOOL );
        AST_AppendChildNode( ast, A_TYPE, "bool", TOK_GetLine( par->lastMatched ) );        
    }
    else
    {
        PAR_Error( par, "string/char/int/bool", "Did you declare base type?" );
    }
    
    return ast;
}

Ast * PAR_ExpandType( Parser * par )
{
    //printf("PAR: Type\n");
    Ast * ast = AST_New();
    
    AST_AppendChildNode( ast, A_TYPE, NULL, TOK_GetLine( par->lastMatched ) );
        
    while( PAR_Peek( par ) == T_OSBRACKET )
    {
        PAR_Match( par, T_OSBRACKET );
        PAR_Match( par, T_CSBRACKET );
        AST_AppendChildNode( ast, A_TYPE, "[]", TOK_GetLine( par->lastMatched ) );        
    }
    
    AST_AppendChildTree( ast, PAR_ExpandBaseType( par ) );
    
    return ast;
}

Ast * PAR_ExpandDeclVar( Parser * par )
{
    //printf("PAR: DeclVar\n");
    Ast * ast = AST_New();
        
    PAR_Match( par, T_COLON );
    AST_AppendChildNode( ast, A_DECLVAR, NULL, TOK_GetLine( par->lastMatched ) );
    AST_AppendChildTree( ast, PAR_ExpandType( par ) );
    
    return ast;
}

Ast * PAR_ExpandGlobal( Parser * par )
{
    //printf("PAR: Global\n");
    Ast * ast;
    Token * matchedId;
    
    PAR_Match( par, T_ID );
    matchedId = par->lastMatched;
    
    ast = PAR_ExpandDeclVar( par );
    AST_PrependChildNode( ast, A_ID, TOK_GetText( matchedId ), TOK_GetLine( matchedId ) );
    
    PAR_Match( par, T_NL );
    
    return ast;
}

Ast * PAR_ExpandParam( Parser * par )
{
    //printf("PAR: Param\n");
    Ast * ast;
    Token * matchedId;
        
    PAR_Match( par, T_ID ); 
    matchedId = par->lastMatched;
       
    ast = PAR_ExpandDeclVar( par );
    AST_PrependChildNode( ast, A_ID, TOK_GetText( matchedId ), TOK_GetLine( matchedId ) );
    
    return ast;
}

Ast * PAR_ExpandParams( Parser * par )
{
    //printf("PAR: Params\n");    
    Ast * ast = AST_New();
    AST_AppendChildNode( ast, A_PARAMS, NULL, TOK_GetLine( par->lastMatched ) );
    
    AST_AppendChildTree( ast, PAR_ExpandParam( par ) );
        
    while( PAR_Peek( par ) == T_COMMA )
    {
        PAR_Match( par, T_COMMA );
        AST_AppendChildTree( ast, PAR_ExpandParam( par ) );       
    }
    
    return ast;
}

Ast * PAR_ExpandExpTerminal( Parser * par )
{
    Ast * ast = AST_New();    
    Token * matchedId;
    int peeked = PAR_Peek( par );
    
    if( peeked == T_ID )
    {
        PAR_Match( par, T_ID );
        matchedId = par->lastMatched;
        
        if( PAR_Peek( par ) == T_OCBRACKET )
        {
            AST_AppendChildTree( ast, PAR_ExpandCall( par ) );
        }
        else
        {
            AST_AppendChildTree( ast, PAR_ExpandVar( par ) );
        }  
        
        AST_PrependChildNode( ast, A_ID, TOK_GetText( matchedId ), TOK_GetLine( matchedId ) );  
    }
    else if( peeked == T_LITINT || peeked == T_LITSTRING )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, AST_TokenTypeToAst( peeked ), TOK_GetText( par->lastMatched ), TOK_GetLine( par->lastMatched ) );
    }
    else if( peeked == T_TRUE || peeked == T_FALSE )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, AST_TokenTypeToAst( peeked ), NULL, TOK_GetLine( par->lastMatched ) );
    }       
    else if( peeked == T_NEW )
    {
        PAR_Match( par, T_NEW );
        AST_AppendChildNode( ast, A_NEW, NULL, TOK_GetLine( par->lastMatched ) );
        
        PAR_Match( par, T_OSBRACKET );
        AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
        PAR_Match( par, T_CSBRACKET );
        AST_AppendChildTree( ast, PAR_ExpandType( par ) );
    }
    else if( peeked == T_OCBRACKET )
    {            
        PAR_Match( par, T_OCBRACKET );
        AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
        PAR_Match( par, T_CCBRACKET );                        
    }   
    else
    {
    	PAR_Error( par, "expression", "Missing expression?" );
    }
    
    return ast;
}

Ast * PAR_ExpandExpUnary( Parser * par )
{
    Ast * ast = AST_New();    
    int peeked;    
    
    peeked = PAR_Peek( par );
    
    if( peeked == T_NOT  )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, A_NOT, NULL, TOK_GetLine( par->lastMatched ) );        
            
        AST_AppendChildTree( ast, PAR_ExpandExpUnary( par ) );
    }
    else if( peeked == T_MINUS )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, A_NEGATIVE, NULL, TOK_GetLine( par->lastMatched ) );        
            
        AST_AppendChildTree( ast, PAR_ExpandExpUnary( par ) );
    }
    else
    {
        AST_AppendChildTree( ast, PAR_ExpandExpTerminal( par ) );
    }
    
    return ast;
}

Ast * PAR_ExpandExpB( Parser * par )
{
    Ast * ast = AST_New();
    Ast * branch;
    int peeked;
    
    branch = PAR_ExpandExpUnary( par );
    peeked = PAR_Peek( par );
    
    if( peeked == T_ASTERISK || peeked == T_SLASH )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, AST_TokenTypeToAst( peeked ), NULL, TOK_GetLine( par->lastMatched ) );        
            
        AST_AppendChildTree( ast, PAR_ExpandExpB( par ) );
    }
    
    AST_PrependChildTree( ast, branch );
    
    return ast;
}

Ast * PAR_ExpandExpA( Parser * par )
{
    Ast * ast = AST_New();
    Ast * branch;
    int peeked;
    
    branch = PAR_ExpandExpB( par );
    peeked = PAR_Peek( par );
    
    if( peeked == T_PLUS || peeked == T_MINUS )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, AST_TokenTypeToAst( peeked ), NULL, TOK_GetLine( par->lastMatched ) );        
            
        AST_AppendChildTree( ast, PAR_ExpandExpA( par ) );
    }
    
    AST_PrependChildTree( ast, branch );
    
    return ast;
}

Ast * PAR_ExpandExpCmp( Parser * par )
{
    Ast * ast = AST_New();
    Ast * branch;
    int peeked;
    
    branch = PAR_ExpandExpA( par );
    peeked = PAR_Peek( par );
    
    if( peeked == T_EQ || peeked == T_NEQ || peeked == T_LARGER || peeked == T_LARGEREQ || peeked == T_SMALLER || peeked == T_SMALLEREQ )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, AST_TokenTypeToAst( peeked ), NULL, TOK_GetLine( par->lastMatched ) );        
            
        AST_AppendChildTree( ast, PAR_ExpandExpCmp( par ) );
    }
    
    AST_PrependChildTree( ast, branch );
    
    return ast;
}

Ast * PAR_ExpandExpAnd( Parser * par )
{
    Ast * ast = AST_New();
    Ast * branch;
    int peeked;
    
    branch = PAR_ExpandExpCmp( par );
    peeked = PAR_Peek( par );
    
    if( peeked == T_AND )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, AST_TokenTypeToAst( peeked ), NULL, TOK_GetLine( par->lastMatched ) );        
            
        AST_AppendChildTree( ast, PAR_ExpandExpAnd( par ) );
    }
    
    AST_PrependChildTree( ast, branch );
    
    return ast;
}

Ast * PAR_ExpandExpOr( Parser * par )
{
    Ast * ast = AST_New();
    Ast * branch;
    int peeked;
    
    branch = PAR_ExpandExpAnd( par );
    peeked = PAR_Peek( par );
    
    if( peeked == T_OR )
    {
        PAR_Match( par, peeked );
        AST_AppendChildNode( ast, AST_TokenTypeToAst( peeked ), NULL, TOK_GetLine( par->lastMatched ) );        
            
        AST_AppendChildTree( ast, PAR_ExpandExpOr( par ) );
    }
    
    AST_PrependChildTree( ast, branch );
    
    return ast;
}

Ast * PAR_ExpandExp( Parser * par )
{
    return PAR_ExpandExpOr( par );
}

Ast * PAR_ExpandExps( Parser * par )
{
    //printf("PAR: Exps\n");
    Ast * ast = AST_New();
    int peeked = PAR_Peek( par );
    
    AST_AppendChildNode( ast, A_ARGS, NULL, TOK_GetLine( par->lastMatched ) );
    
    while( peeked != T_CCBRACKET )
    {
        AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
        
        peeked = PAR_Peek( par );
        
        if( peeked == T_COMMA )
        {
            PAR_Match( par, T_COMMA );
        }
    }
    
    return ast;
}

Ast * PAR_ExpandVar( Parser * par )
{
    //printf("PAR: Var\n");
    Ast * ast = AST_New();
    
    AST_AppendChildNode( ast, A_VAR, NULL, TOK_GetLine( par->lastMatched ) );
        
    while( PAR_Peek( par ) == T_OSBRACKET )
    {
        PAR_Match( par, T_OSBRACKET );        
        AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
        PAR_Match( par, T_CSBRACKET );        
    }  
    
    return ast;  
}

Ast * PAR_ExpandCmdIf( Parser * par )
{
    //printf("PAR: CmdIf\n");
    Ast * ast = AST_New();
    int foundElse = 0;
    
    PAR_Match( par, T_IF );
    AST_AppendChildNode( ast, A_IF, NULL, TOK_GetLine( par->lastMatched ) );
    
    AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
    PAR_ExpandNewLine( par );
    AST_AppendChildTree( ast, PAR_ExpandBlock( par ) );   
    
    while( PAR_Peek( par ) == T_ELSE && !foundElse )
    {
        PAR_Match( par, T_ELSE );
        
        if( PAR_Peek( par ) == T_IF )
        {
            PAR_Match( par, T_IF );
            AST_AppendChildTree( ast, PAR_ExpandExp( par ) );            
        }
        else
        {
        	foundElse = 1;
        }
        
        PAR_ExpandNewLine( par );
        AST_AppendChildTree( ast, PAR_ExpandBlock( par ) );        
    }
    
    PAR_Match( par, T_END );
    
    return ast;
}

Ast * PAR_ExpandCmdWhile( Parser * par )
{
    //printf("PAR: CmdWhile\n");
    Ast * ast = AST_New();
    
    PAR_Match( par, T_WHILE );
    AST_AppendChildNode( ast, A_WHILE, NULL, TOK_GetLine( par->lastMatched ) );
    
    AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
    PAR_ExpandNewLine( par );
    AST_AppendChildTree( ast, PAR_ExpandBlock( par ) );
    PAR_Match( par, T_LOOP );    
    
    return ast;
}

Ast * PAR_ExpandCmdReturn( Parser * par )
{
    //printf("PAR: CmdReturn\n");
    Ast * ast = AST_New();
    
    PAR_Match( par, T_RETURN );
    AST_AppendChildNode( ast, A_RETURN, NULL, TOK_GetLine( par->lastMatched ) );    
    
    if( PAR_Peek( par ) != T_NL )
    {
        AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
    }
    
    PAR_ExpandNewLine( par );
    
    return ast;
}

Ast * PAR_ExpandCmdAssign( Parser * par )
{
    //printf("PAR: CmdAssign\n");
    Ast * ast = AST_New();
    Ast * subAst;
    Token * matchedId = par->lastMatched;
    
    AST_AppendChildNode( ast, A_ASSIGN, NULL, TOK_GetLine( par->lastMatched ) );    
    subAst = PAR_ExpandVar( par );
    AST_PrependChildNode( subAst, A_ID, TOK_GetText( matchedId ), TOK_GetLine( par->lastMatched ) );    
               
    AST_AppendChildTree( ast, subAst );
    PAR_Match( par, T_EQ );
    AST_AppendChildTree( ast, PAR_ExpandExp( par ) );
    
    return ast;
}

Ast * PAR_ExpandCall( Parser * par )
{
    //printf("PAR: Call\n");
    Ast * ast = AST_New();    
    
    PAR_Match( par, T_OCBRACKET );
    AST_AppendChildNode( ast, A_CALL, NULL, TOK_GetLine( par->lastMatched ) );    
    AST_AppendChildTree( ast, PAR_ExpandExps( par ) );
    PAR_Match( par, T_CCBRACKET );
    
    return ast;
}

Ast * PAR_ExpandCmd( Parser * par )
{
    //printf("PAR: Cmd\n");
    Ast * ast;
    
    PAR_ExpandNewLine( par );
    
    int peeked = PAR_Peek( par );
    
    if( peeked == T_IF )
    {
        ast = PAR_ExpandCmdIf( par );        
    }
    else if( peeked == T_WHILE )
    {
        ast = PAR_ExpandCmdWhile( par );    
    }
    else if( peeked == T_RETURN )
    {
        ast = PAR_ExpandCmdReturn( par );    
    }
    else
    {    	
        PAR_Error( par, "command", "Declare variables on block start." );
    }
    
    PAR_ExpandNewLine( par );
    
    return ast;
}

Ast * PAR_ExpandBlock( Parser * par )
{
    //printf("PAR: Block\n");
    Ast * ast = AST_New();
    Ast * subAst;
    Token * matchedId;    
    int foundID = 0;    
    int peeked = PAR_Peek( par );
    
    AST_AppendChildNode( ast, A_BLOCK, NULL, TOK_GetLine( par->lastMatched ) + 1 );
    
    //Declvar
    while( peeked == T_ID )
    {
        PAR_Match( par, T_ID );
        matchedId = par->lastMatched;
        
        peeked = PAR_Peek( par );
        
        if( peeked == T_COLON )
        {
            subAst = PAR_ExpandDeclVar( par );
            AST_PrependChildNode( subAst, A_ID, TOK_GetText( matchedId ), TOK_GetLine( matchedId ) );
            AST_AppendChildTree( ast, subAst );
            
            PAR_ExpandNewLine( par );
            peeked = PAR_Peek( par );
        }
        else
        {
        	foundID = 1;
            break;
        }
    }

    //CMDs
    if( foundID )
    {
    	peeked = PAR_Peek( par );    
    
    	if( peeked == T_EQ || peeked == T_OSBRACKET )
    	{
    	    subAst = PAR_ExpandCmdAssign( par );
    	    //AST_PrependChildNode( subAst, A_ID, TOK_GetText( matchedId ) );
            AST_AppendChildTree( ast, subAst );
            
    	    PAR_ExpandNewLine( par );
    	}
    	else if( peeked == T_OCBRACKET )
    	{
    	    subAst = PAR_ExpandCall( par );
    	    AST_PrependChildNode( subAst, A_ID, TOK_GetText( matchedId ), TOK_GetLine( matchedId ) );
            AST_AppendChildTree( ast, subAst );
            
    	    PAR_ExpandNewLine( par );
    	}
	}
        
    peeked = PAR_Peek( par );
    
    while( peeked != T_END && peeked != T_LOOP && peeked != T_ELSE )
    {        
		if( peeked == T_ID )
	    {
	    	PAR_Match( par, T_ID );
	    	matchedId = par->lastMatched;
	            
	        peeked = PAR_Peek( par );
	           
	        if( peeked == T_EQ || peeked == T_OSBRACKET )
	        {
	            subAst = PAR_ExpandCmdAssign( par );
        	    //AST_PrependChildNode( subAst, A_ID, TOK_GetText( matchedId ) );
                AST_AppendChildTree( ast, subAst );
                
        	    PAR_ExpandNewLine( par );
	        }
	        else if( peeked == T_OCBRACKET )
	        {
	            subAst = PAR_ExpandCall( par );
        	    AST_PrependChildNode( subAst, A_ID, TOK_GetText( matchedId ), TOK_GetLine( matchedId ) );
                AST_AppendChildTree( ast, subAst );
                
        	    PAR_ExpandNewLine( par );
	        }
	    }
	    else
	    {
	        AST_AppendChildTree( ast, PAR_ExpandCmd( par ) );
	    }
	       
	    peeked = PAR_Peek( par );
	}    
    
    PAR_ExpandNewLine( par );
    
    return ast;
}

Ast * PAR_ExpandFunction( Parser * par )
{
    //printf("PAR: Function\n");
    Ast * ast = AST_New();    
    
    PAR_Match( par, T_FUN );
    AST_AppendChildNode( ast, A_FUNCTION, NULL, TOK_GetLine( par->lastMatched ) );
    
    PAR_Match( par, T_ID );
    AST_AppendChildNode( ast, A_ID, TOK_GetText( par->lastMatched ), TOK_GetLine( par->lastMatched ) );
    
    PAR_Match( par, T_OCBRACKET );
    
    if( PAR_Peek( par ) == T_ID )
    { 
        AST_AppendChildTree( ast, PAR_ExpandParams( par ) );
    }
    
    PAR_Match( par, T_CCBRACKET );
    
    if( PAR_Peek( par ) != T_NL )
    {
        PAR_Match( par, T_COLON );
        AST_AppendChildTree( ast, PAR_ExpandType( par ) );
    }
    
    PAR_ExpandNewLine( par );
    AST_AppendChildTree( ast, PAR_ExpandBlock( par ) );
    PAR_Match( par, T_END );
    PAR_ExpandNewLine( par );
    
    return ast;
}

Ast * PAR_ExpandDecl( Parser * par )
{
    //printf("PAR: Decl\n");
    Ast * ast = NULL;
    int peeked = PAR_Peek( par );
    
    if( peeked == T_FUN )
    {
        ast = PAR_ExpandFunction( par );
    }
    else if( peeked == T_ID )
    {
        ast = PAR_ExpandGlobal( par );
    }
    else if( peeked == T_NL )
    {
        PAR_ExpandNewLine( par );
    }
    else
    {        
        PAR_Error( par, "function/global", "" );
    }
    
    return ast;
}

void PAR_ExpandProgram( Parser * par )
{
    //printf("PAR: Program\n");
    Ast * ast = AST_New();
    AST_AppendChildNode( ast, A_PROGRAM, NULL, 0 );
    
    PAR_ExpandNewLine( par );
    
    while( PAR_Peek( par ) != -1 )
    {
        AST_AppendChildTree( ast, PAR_ExpandDecl( par ) );
    }
    
    AST_AppendChildTree( par->ast, ast );
    fprintf( stdout, "Parsing successful!\n" );   
}

/********************************************************************/

Parser * PAR_New( List * tokens )
{
    Parser * par = ( Parser* )malloc( sizeof( Parser ) );
    par->tokens = tokens;
    par->ast = AST_New();
    par->lastMatched = NULL;
    
    return par;
}

void PAR_Delete( Parser * par )
{
    if( par )
    {
        LIS_Delete( par->tokens, &TOK_Delete );
        AST_Delete( par->ast );
        par->ast = NULL;  
        
        free( par );      
    }
}

void PAR_Execute( Parser * par )
{
    if( LIS_GetSize( par->tokens ) )
        PAR_ExpandProgram( par );
        
    AST_Dump( par->ast );
}

void PAR_DumpTokens( Parser * par )
{
    LIS_Dump( par->tokens, &TOK_Dump );
}
