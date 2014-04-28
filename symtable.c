#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtable.h"
#include "uthash.h"

#define MAX_SCOPE_SIZE 4

#define ERROR_UNDECLARED    0
#define ERROR_REDEFINED     1
#define ERROR_UNKNOWN       2

static void errorSymTable( int error, char * name, int line )
{
    switch( error )
    {
        case ERROR_UNDECLARED:
            fprintf( stderr, "!Typing Error [line %d]: Undeclared identifier \'%s\'.\n", line, name );
            break;
            
        case ERROR_REDEFINED:
            fprintf( stderr, "!Typing Error [line %d]: Redefinition of identifier \'%s\'.\n", line, name );
            break;
            
        case ERROR_UNKNOWN:
            fprintf( stderr, "!Typing Error: Internal error.\n" );
            break;
            
        default:
            break;
    }
    
    free( name );
    
    exit( EXIT_FAILURE );
}

typedef struct hash Hash;

struct hash
{
	char * id;
	int type;
	UT_hash_handle hh;
};

typedef struct scope Scope;

struct scope
{
	Scope * parent;
	Scope ** scopes;	
	int nScopes;
	int maxScopes;
	Hash * symbols;	
};

Scope * SCO_New()
{
	Scope * scope = ( Scope* )malloc( sizeof( Scope ) );
	scope->parent = NULL;
	scope->scopes = ( Scope** )malloc( MAX_SCOPE_SIZE * sizeof( Scope* ) );
	scope->nScopes = 0;
	scope->maxScopes = MAX_SCOPE_SIZE;
	scope->symbols = NULL;
	
	int i;
	
	for( i = 0; i < MAX_SCOPE_SIZE; i++ )
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

int SYT_CheckSymbol( SymTable * syt, const char * idName )
{
	Hash * h;
	Scope * current = syt->current;
    
    do
    {
        HASH_FIND_STR( current->symbols, idName, h );
        
        if( h )
            return 1;            
        
        current = current->parent;
    }
    while( current );
    
    return 0;
}
 
int SYT_AddSymbol( SymTable * syt, char * idName, int type )
{
    if( !idName )
        return 0;
    
    if( !SYT_CheckSymbol( syt, idName ) ) 
    {
	    Hash * h = ( Hash* )malloc( sizeof( Hash ) );
	    h->id = idName;
	    h->type = type;
	
	    HASH_ADD_STR( syt->current->symbols, id, h ); //This works cuz macros
	    return 1;
	}
	
	return 0;
}

void SYT_VisitID( SymTable * syt, Ast * ast )
{
    char * name = AST_GetValue( ast );
    
    if( !SYT_CheckSymbol( syt, name ) )	                
        errorSymTable( ERROR_UNDECLARED, name, AST_GetLine( ast ) );
}

void SYT_VisitDeclaration( SymTable * syt, Ast * ast )
{
    char * name = AST_FindId( ast );	                
    int type = SYT_StringToType( AST_FindType( ast ) );

    if( !SYT_AddSymbol( syt, name, type ) )	                
        errorSymTable( ERROR_REDEFINED, name, AST_GetLine( ast ) );
}

void SYT_VisitAssign( SymTable * syt, Ast * ast )
{
    Ast * var = AST_GetChild( ast );
    char * name = AST_FindId( var );
    
    free( var );

    if( !SYT_CheckSymbol( syt, name ) )	                
        errorSymTable( ERROR_UNDECLARED, name, AST_GetLine( ast ) );
}

void SYT_VisitCall( SymTable * syt, Ast * ast )
{
    char * name = AST_FindId( ast );

    if( !SYT_CheckSymbol( syt, name ) )	                
        errorSymTable( ERROR_UNDECLARED, name, AST_GetLine( ast ) );
}

void SYT_VisitFunction( SymTable * syt, Ast * ast )
{
    SYT_OpenScope( syt );
    
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        switch( AST_GetType( ast ) )
        {
            case A_DECLVAR:
                SYT_VisitDeclaration( syt, child );
                break;
                
            case A_BLOCK:
                {
                    Ast * child2;
                    
                    for( child2 = AST_GetChild( ast ); child2; child2 = AST_NextSibling( child2 ) )                    
                        SYT_ProcessNode( syt, child2 );                    
                }
                break;
                
            default:
                break;                
        }
    }
    
    SYT_CloseScope( syt );
}

void SYT_ProcessNode( SymTable * syt, Ast * ast )
{
    switch( AST_GetType( ast ) )
    {
        case A_BLOCK:
            SYT_OpenScope( syt );
            break;
            
        case A_ID:
            SYT_VisitID( syt, ast );
            break;
            
        case A_FUNCTION:
            SYT_VisitFunction( syt, ast );
            break;
            
        case A_DECLVAR:
            SYT_VisitDeclaration( syt, ast );
            break;
            
        case A_ASSIGN:
            SYT_VisitAssign( syt, ast );
            break;
            
        case A_CALL:
            SYT_VisitCall( syt, ast );
            break;
    }
    
    Ast * child;
	
	for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )	
	    SYT_ProcessNode( syt, child );	
	
	if( AST_GetType( ast ) == A_BLOCK )
	    SYT_CloseScope( syt );
}

int SYT_StringToType( char * str )
{
    if( !str )
        return S_NONE;
        
    if( strcmp( str, "char" ) == 0 )
        return S_CHAR;
    else if( strcmp( str, "int" ) == 0 )
        return S_INT;
    else if( strcmp( str, "bool" ) == 0 )
        return S_BOOL;
    else if( strcmp( str, "string" ) == 0 )
        return S_STRING;
        
    return S_NONE;
}

void SYT_Build( SymTable * syt, Ast * ast )
{
	syt->ast = ast;	
	
	Ast * child;
	
	for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
        if( AST_GetType( child ) != A_FUNCTION && AST_GetType( child ) != A_DECLVAR )
            errorSymTable( ERROR_UNKNOWN, NULL, 0 );	        

        char * name = AST_FindId( child );	                
        int type = SYT_StringToType( AST_FindType( child ) );

        if( !SYT_AddSymbol( syt, name, type ) )	                
            errorSymTable( ERROR_REDEFINED, name, AST_GetLine( child ) );
	}
		
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
        if( AST_GetType( child ) != A_DECLVAR )
        {
            SYT_ProcessNode( syt, child );
        }
    }
}

