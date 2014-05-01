#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtable.h"
#include "symbol.h"
#include "uthash.h"

#define MAX_SCOPES 4

#define ERROR_UNDECLARED    0
#define ERROR_REDEFINED     1
#define ERROR_UNKNOWN       2

static void errorSymbol( int error, char * name, int line )
{
    switch( error )
    {
        case ERROR_UNDECLARED:
            fprintf( stderr, "!Symbol Error [line %d]: Undeclared identifier \'%s\'.\n", line, name );
            break;
            
        case ERROR_REDEFINED:
            fprintf( stderr, "!Symbol Error [line %d]: Redefinition of identifier \'%s\'.\n", line, name );
            break;
            
        case ERROR_UNKNOWN:
            fprintf( stderr, "!Symbol Error: Internal error.\n" );
            break;
            
        default:
            break;
    }
    
    free( name );
    
    exit( EXIT_FAILURE );
}

static void errorTyping( int assert, const char * error, int line )
{
    if( assert )
        return;
        
    fprintf( stderr, "!Typing Error [line %d]: %s\n", line, error );
    
    exit( EXIT_FAILURE );
}

typedef struct hash Hash;

struct hash
{
    char * id;
	Symbol * symbol;
	UT_hash_handle hh;
};

typedef struct scope Scope;

struct scope
{
	Scope * parent;
	Scope ** scopes;	
	int nScopes;
	int maxScopes;
	int nSymbols;
	Hash * symbols;	
};

Scope * SCO_New()
{
	Scope * scope = ( Scope* )malloc( sizeof( Scope ) );
	scope->parent = NULL;
	scope->scopes = ( Scope** )malloc( MAX_SCOPES * sizeof( Scope* ) );
	scope->nScopes = 0;
	scope->maxScopes = MAX_SCOPES;
	scope->nSymbols = 0;
	scope->symbols = NULL;
	
	int i;
	
	for( i = 0; i < MAX_SCOPES; i++ )
		scope->scopes[i] = NULL;
	
	return scope;
}

void SCO_Delete( Scope * scope )
{
	if( !scope )
		return;
		
	int i;
	Hash *h, *tmp;
	
	for( i = 0; i < scope->nScopes; i++ )
	{
		SCO_Delete( scope->scopes[i] );		
	}

	free( scope->scopes );
	
	HASH_ITER( hh, scope->symbols, h, tmp ) 
	{
        HASH_DEL( scope->symbols, h );
        free( h );
    }    
	
	free( scope );
}

void SCO_AppendScope( Scope * parent, Scope * child )
{
	if( !parent )
	{
	    free( child );
		return;
	}
		
	if( !child )
		return;
		
	child->parent = parent;
	
	if( parent->nScopes == parent->maxScopes )
	{
		parent->maxScopes *= 2;
		parent->scopes = realloc( parent->scopes, parent->maxScopes * sizeof( Scope* ) );
	}
	
	parent->scopes[parent->nScopes++] = child;
}

void SYT_ProcessNode( SymTable * syt, Ast * ast );
void SYT_VisitExpression( SymTable * syt, Ast * ast );

struct symtable
{
	Ast * ast;
	Scope * root;
	Scope * current;
};


SymTable * SYT_New()
{
	SymTable * syt = ( SymTable* )malloc( sizeof( SymTable ) );
	syt->ast = NULL;
	syt->root = SCO_New();
	syt->current = syt->root;
	
	return syt;
}

void SYT_Delete( SymTable * syt )
{
	SCO_Delete( syt->root );
	free( syt );
}

void SYT_OpenScope( SymTable * syt )
{
	Scope * newScope = SCO_New();

	SCO_AppendScope( syt->current, newScope );
	syt->current = newScope;
}

void SYT_CloseScope( SymTable * syt )
{
	if( !syt->current )
		return;
		
	syt->current = syt->current->parent;
}

int SYT_CheckSymbol( SymTable * syt, char * idName, Symbol ** s )
{
	Hash * h;
	Scope * current = syt->current;
    
    do
    {
        HASH_FIND_STR( current->symbols, idName, h );
        
        if( h )
        {
            *s = h->symbol;
            return SYM_GetType( h->symbol ); 
        }           
        
        current = current->parent;
    }
    while( current );
    
    return 0;
}
 
int SYT_AddSymbol( SymTable * syt, char * idName, Symbol * s )
{
    if( !idName )
        return 0;
    
    Symbol * garbage;
    int check = SYT_CheckSymbol( syt, idName, &garbage );
    
    if( !check ) 
    {
	    Hash * h = ( Hash* )malloc( sizeof( Hash ) );
	    h->id = idName;
	    h->symbol = s;
	
	    HASH_ADD_STR( syt->current->symbols, id, h ); //This works cuz macros
	    syt->current->nSymbols++;
	    
	    return 1;
	}
	
	return 0;
}

void SYT_VisitDeclaration( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    char * name = AST_GetNodeValue( child );
    int ptrType;	                
    int type = SYM_StringToType( AST_FindType( ast, &ptrType ) );
    Symbol * s = SYM_New( type, ptrType );    
    
    if( !SYT_AddSymbol( syt, name, s ) )	                
        errorSymbol( ERROR_REDEFINED, name, AST_GetNodeLine( ast ) );
        
    AST_Annotate( child, s );
    AST_Annotate( ast, s );
}

void SYT_VisitID( SymTable * syt, Ast * ast )
{
    char * name = AST_GetNodeValue( ast );
    Symbol * s;
    int type = SYT_CheckSymbol( syt, name, &s );
    
    if( !type )	                
        errorSymbol( ERROR_UNDECLARED, name, AST_GetNodeLine( ast ) );
        
    AST_Annotate( ast, s );
}

void SYT_VisitAssign( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    SYT_VisitExpression( syt, child );
    
    Symbol * s1 = AST_GetNodeAnnotation( child );
    int type1 = SYM_GetType( s1 );
    int ptrType1 = SYM_GetPtrType( s1 );
    
    child = AST_NextSibling( child );
    SYT_VisitExpression( syt, child );    
        
    Symbol * s2 = AST_GetNodeAnnotation( child );
    int type2 = SYM_GetType( s2 );
    int ptrType2 = SYM_GetPtrType( s2 );
    
    free( child );
    
    errorTyping( ( type1 == type2 && ptrType1 == ptrType2 ), "Expression does not evaluate to variable\'s type.", AST_GetNodeLine( ast ) );        
}

void SYT_VisitCall( SymTable * syt, Ast * ast )
{
    char * name = AST_FindId( ast );
    Symbol * s;
    int type = SYT_CheckSymbol( syt, name, &s );
    
    if( !type )	                
        errorSymbol( ERROR_UNDECLARED, name, AST_GetNodeLine( ast ) );
    
    Ast * args = AST_GetChild( ast );
    args = AST_NextSibling( args );
    SYT_VisitExpression( syt, args );
    
    Symbol * s1;
    Symbol * s2 = AST_GetNodeAnnotation( args );
        
    int garbage = SYT_CheckSymbol( syt, name, &s1 );
    
    free( args );
    
    errorTyping( SYM_CompareParams( s1, s2 ), "Expression does not match function parameter type.", AST_GetNodeLine( ast ) );    
    
    AST_Annotate( ast, SYM_New( type, 0 ) );
}

void SYT_VisitIf( SymTable * syt, Ast * ast )
{
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        if( AST_GetNodeType( child ) == A_BLOCK )
            continue;
        
        SYT_VisitExpression( syt, child );    
        
        Symbol * s = AST_GetNodeAnnotation( child );
        int expType = SYM_GetType( s );
        int expPtrType = SYM_GetPtrType( s );
        
        errorTyping( ( expType == S_BOOL && expPtrType == 0 ), "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) );        
    }    
}

void SYT_VisitWhile( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    SYT_VisitExpression( syt, child );
    
    Symbol * s = AST_GetNodeAnnotation( child );
    int expType = SYM_GetType( s );
    int expPtrType = SYM_GetPtrType( s );    
    
    errorTyping( ( expType == S_BOOL && expPtrType == 0 ), "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) );
            
    free( child );
}
 
void SYT_VisitReturn( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    Symbol * s = SYM_New( S_VOID, 0 );
    
    if( child )
    {
        free( s );
        SYT_VisitExpression( syt, child );
        s = AST_GetNodeAnnotation( child );
    }
    
    AST_Annotate( ast, s );
    
    free( child );
}
           
void SYT_VisitParams( SymTable * syt, Ast * ast )
{
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {        
        SYT_VisitDeclaration( syt, child );
    }    
}

void SYT_VisitLitString( SymTable * syt, Ast * ast )
{
    AST_Annotate( ast, SYM_New( S_STRING, 0 ) );
}

void SYT_VisitLitInt( SymTable * syt, Ast * ast )
{
    AST_Annotate( ast, SYM_New( S_INT, 0 ) );
}

void SYT_VisitLitBool( SymTable * syt, Ast * ast )
{
    AST_Annotate( ast, SYM_New( S_BOOL, 0 ) );
}

void SYT_VisitVar( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    Symbol * s = AST_GetNodeAnnotation( child );
    int type = SYM_GetType( s );
    int ptrType = SYM_GetPtrType( s );    
    Symbol * s2;
    int count = 0;    
    
    child = AST_NextSibling( child );
    
    while( child )
    {        
        count++;     
        
        s2 = AST_GetNodeAnnotation( child );
        int expType = SYM_GetType( s2 );
        int expPtrType = SYM_GetPtrType( s2 );
        
        errorTyping( ( expType == S_INT && expPtrType == 0 ), "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );        
        
        child = AST_NextSibling( child );
    }   
    
    errorTyping( count <= ptrType, "Expression does not match expected pointer dimension.", AST_GetNodeLine( ast ) );
    
    AST_Annotate( ast, SYM_New( type, ptrType - count ) );
}

void SYT_VisitArgs( SymTable * syt, Ast * ast )
{
    Symbol * sym = SYM_New( S_VOID, 0 );
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        Symbol * s = AST_GetNodeAnnotation( child );
        int type = SYM_GetType( s );
        int ptrType = SYM_GetPtrType( s );
        
        SYM_PushParam( sym, type, ptrType );
    }
    
    AST_Annotate( ast, sym );
}

void SYT_VisitNew( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    Symbol * s = AST_GetNodeAnnotation( child );
    int type = SYM_GetType( s );
    int ptrType = SYM_GetPtrType( s );
    
    errorTyping( ( type == S_INT && ptrType == 0 ), "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );    
        
    child = AST_NextSibling( child );
    type = SYM_StringToType( AST_FindType( ast, &ptrType ) );
    
    AST_Annotate( ast, SYM_New( type, ptrType + 1 ) );
}

void SYT_VisitNot( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    Symbol * s = AST_GetNodeAnnotation( child );
    int type = SYM_GetType( s );
    int ptrType = SYM_GetPtrType( s );
    
    free( child );
    
    errorTyping( ( type == S_BOOL && ptrType == 0 ), "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) );    
    
    AST_Annotate( ast, SYM_New( S_BOOL, 0 ) );
}

void SYT_VisitNegative( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    Symbol * s = AST_GetNodeAnnotation( child );
    int type = SYM_GetType( s );
    int ptrType = SYM_GetPtrType( s );    
    
    int line = AST_GetNodeLine( child );
    free( child );
    
    errorTyping( ( type == S_INT && ptrType == 0 ), "Expression does not evaluate to type \'int\'.", line );    
    
    AST_Annotate( ast, SYM_New( S_INT, 0 ) );
}

void SYT_VisitArithmitic( SymTable * syt, Ast * ast )
{
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        Symbol * s = AST_GetNodeAnnotation( child );
        int type = SYM_GetType( s );
        int ptrType = SYM_GetPtrType( s );
        
        errorTyping( ( ( type == S_INT || type == S_CHAR ) && ptrType == 0 ), "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
    }
    
    AST_Annotate( ast, SYM_New( S_INT, 0 ) );
}

void SYT_VisitLogic( SymTable * syt, Ast * ast )
{
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        Symbol * s = AST_GetNodeAnnotation( child );
        int type = SYM_GetType( s );
        int ptrType = SYM_GetPtrType( s );
        
        errorTyping( ( type == S_BOOL && ptrType == 0 ), "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) ); 
    }
    
    AST_Annotate( ast, SYM_New( S_BOOL, 0 ) );
}

void SYT_VisitEqual( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    Symbol * s1 = AST_GetNodeAnnotation( child );
    int type1 = SYM_GetType( s1 );
    int ptrType1 = SYM_GetPtrType( s1 );
    
    child = AST_NextSibling( child );    
    Symbol * s2 = AST_GetNodeAnnotation( child );
    int type2 = SYM_GetType( s2 );
    int ptrType2 = SYM_GetPtrType( s2 );        
    
    errorTyping( ( type1 == type2 && ptrType1 == ptrType2 ), "Expressions not matching type.", AST_GetNodeLine( child ) );
    
    AST_Annotate( ast, SYM_New( S_BOOL, 0 ) );
}

void SYT_VisitComparison( SymTable * syt, Ast * ast )
{
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        Symbol * s = AST_GetNodeAnnotation( child );
        int type = SYM_GetType( s );
        int ptrType = SYM_GetPtrType( s );        
        
        errorTyping( ( type == S_INT && ptrType == 0 ), "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );        
    }
    
    AST_Annotate( ast, SYM_New( S_BOOL, 0 ) );
}

void SYT_VisitExpression( SymTable * syt, Ast * ast )
{
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
        SYT_VisitExpression( syt, child );    
    
    int nodeType = AST_GetNodeType( ast );
    
    switch( nodeType )
    {
        case A_ID:
            SYT_VisitID( syt, ast );
            break;
        
        case A_LITSTRING:
            SYT_VisitLitString( syt, ast );            
            break;
        
        case A_LITINT:
            SYT_VisitLitInt( syt, ast );            
            break;
            
        case A_TRUE:
        case A_FALSE:
            SYT_VisitLitBool( syt, ast ); 
            break;
        
        case A_VAR:
            SYT_VisitVar( syt, ast );
            break;
        
        case A_CALL:
            SYT_VisitCall( syt, ast );  //Error if return type == void
            break;
            
        case A_ARGS:
            SYT_VisitArgs( syt, ast );
            break;
            
        case A_NEW:
            SYT_VisitNew( syt, ast );
            break;
                
        case A_NOT:
            SYT_VisitNot( syt, ast );
            break;
        
        case A_NEGATIVE:
            SYT_VisitNegative( syt, ast );
            break;
                         
        case A_ADD:
        case A_SUBTRACT:
        case A_MULTIPLY:
        case A_DIVIDE:
            SYT_VisitArithmitic( syt, ast );
            break;
            
        case A_AND:
        case A_OR:
            SYT_VisitLogic( syt, ast );
            break;
            
        case A_EQ:
            SYT_VisitEqual( syt, ast );
            break;
            
        case A_NEQ:
        case A_LARGER:
        case A_SMALLER:
        case A_LARGEREQ:
        case A_SMALLEREQ:
            SYT_VisitComparison( syt, ast );
            break;                      
    }
}

void SYT_VisitFunction( SymTable * syt, Ast * ast )
{
    char * name = AST_FindId( ast );
    int ptrType;	                
    int type = SYM_StringToType( AST_FindType( ast, &ptrType ) );
    Symbol * s = SYM_New( type, ptrType );
    Ast * params = AST_GetChild( ast );
    params = AST_NextSibling( params );
    
    if( AST_GetNodeType( params ) == A_PARAMS )
    {
        Ast * child;
        
        for( child = AST_GetChild( params ); child; child = AST_NextSibling( child ) )
	    {
	        if( AST_GetNodeType( child ) == A_DECLVAR )
	        {
	            int paramPtrType;	                
                int paramType = SYM_StringToType( AST_FindType( child, &paramPtrType ) );
                
                SYM_PushParam( s, paramType, paramPtrType );
	        }
	    }
	}
	
	free( params );
	
    if( !SYT_AddSymbol( syt, name, s ) )
        errorSymbol( ERROR_REDEFINED, name, AST_GetNodeLine( ast ) );
}

void SYT_VisitGlobals( SymTable * syt, Ast * ast )
{
    Ast * child;	
	
	for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    switch( AST_GetNodeType( child ) )
	    {
	        case A_FUNCTION:
	            SYT_VisitFunction( syt, child );
	            break;
	            
	        case A_DECLVAR:
	            SYT_VisitDeclaration( syt, child );
	            break;
	            
	        default:
	            errorSymbol( ERROR_UNKNOWN, NULL, 0 );	  
	            break;
	    } 
	}
}

int SYT_AssertReturns( SymTable * syt, Ast * ast, int type, int ptrType )
{
    Ast * child;
    int foundReturn = 0;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    if( AST_GetNodeType( child ) == A_RETURN )
	    {
	        Symbol * s = AST_GetNodeAnnotation( child );
            int returnType = SYM_GetType( s );
            int returnPtrType = SYM_GetPtrType( s );        
            
            errorTyping( ( type == returnType && ptrType == returnPtrType ), "Return expression does not evaluate to function return type.", AST_GetNodeLine( child ) );
            
            foundReturn = 1;   
	    }
	    
	    if( SYT_AssertReturns( syt, child, type, ptrType ) )
	        foundReturn = 1;
	}
	
	return foundReturn;
}

void SYT_ProcessNode( SymTable * syt, Ast * ast )
{
    switch( AST_GetNodeType( ast ) )
    {
        case A_FUNCTION:
        case A_BLOCK:
            SYT_OpenScope( syt );
            break;
            
        case A_ID:
            SYT_VisitID( syt, ast );
            break;
            
        case A_PARAMS:
            SYT_VisitParams( syt, ast );
            break;
                
        case A_DECLVAR:
            SYT_VisitDeclaration( syt, ast );
            break;
            
        case A_ASSIGN:
            SYT_VisitAssign( syt, ast );
            break;
            
        case A_CALL:
            SYT_VisitCall( syt, ast ); //Error if return type != void
            break;
            
        case A_IF:
            SYT_VisitIf( syt, ast );
            break;
            
        case A_WHILE:
            SYT_VisitWhile( syt, ast );
            break;
            
        case A_RETURN:
            SYT_VisitReturn( syt, ast );
            break;                    
    }
    
    Ast * child;
	
	for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    if( !AST_GetNodeAnnotation( child ) )	
	        SYT_ProcessNode( syt, child );
	}	
	
	if( AST_GetNodeType( ast ) == A_FUNCTION )
	{
	    SYT_CloseScope( syt );
	    
	    Symbol * s;
	    int type = SYT_CheckSymbol( syt, AST_FindId( ast ), &s );     
	    
	    if( type != S_VOID )
	    {
	        errorTyping( SYT_AssertReturns( syt, ast, type, SYM_GetPtrType( s ) ), "No return expression for function", AST_GetNodeLine( ast ) );
	    }
	    else
	    {
	        int hasReturn = SYT_AssertReturns( syt, ast, type, SYM_GetPtrType( s ) ); //optional
	    }
	}
	    
	if( AST_GetNodeType( ast ) == A_BLOCK )
	    SYT_CloseScope( syt );
}

void SYT_Build( SymTable * syt, Ast * ast )
{
	syt->ast = ast;	
	
	SYT_VisitGlobals( syt, ast );
	
	Ast * child;
		
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
        if( AST_GetNodeType( child ) != A_DECLVAR )
        {
            SYT_ProcessNode( syt, child );
        }
    }
}

