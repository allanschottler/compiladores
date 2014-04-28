#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtable.h"
#include "uthash.h"

#define SCOPE_SIZE 4

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
	scope->scopes = ( Scope** )malloc( SCOPE_SIZE * sizeof( Scope* ) );
	scope->nScopes = 0;
	scope->maxScopes = SCOPE_SIZE;
	scope->symbols = NULL;
	
	int i;
	
	for( i = 0; i < SCOPE_SIZE; i++ )
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
	
	HASH_ITER( hh, scope->symbols, h, tmp ) 
	{
        HASH_DEL( scope->symbols, h );
        free( h );
    }
    
	free( scope->scopes );
	free( scope );
}

void SCO_AppendScope( Scope * parent, Scope * child )
{
	if( !parent )
		return;
		
	if( !child )
		return;
		
	child->parent = parent;
	
	if( parent->nScopes == parent->maxScopes )
	{
		parent->maxScopes *= 2;
		parent->scopes = realloc( parent->scopes, parent->maxScopes );
	}
	
	parent->scopes[parent->nScopes++] = child;
}

struct symtable
{
	Ast * ast;
	Scope * root;
	Scope * current;
};


SymTable * SYT_New()
{
	SymTable * syt = ( SymTable* )malloc( sizeof( SymTable ) );
	syt->ast = 0;
	syt->root = 0;
	syt->current = 0;
	
	return syt;
}

void SYT_Delete( SymTable * syt )
{
	SCO_Delete( syt->root );
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
    
    //Check parent scopes
    HASH_FIND_STR( syt->current->symbols, idName, h );
    return ( h ? h->type : S_NONE );
}
 
int SYT_AddSymbol( SymTable * syt, char * idName, int type )
{
    if( !idName )
        return 0;
        
    if( type == S_NONE )
        return 0;
        
    Hash * h;
    HASH_FIND_STR( syt->current->symbols, idName, h );
    
    if( h == NULL ) 
    {
	    h = ( Hash* )malloc( sizeof( Hash ) );
	    h->id = ( char* )malloc( strlen( idName ) * sizeof( char ) );
	    strcpy( h->id, idName );
	    h->type = type;
	
	    HASH_ADD_STR( syt->current->symbols, id, h ); //This works cuz macros
	    return 1;
	}
	
	return 0;
}

void SYT_VisitFunction( SymTable * syt, Ast * ast )
{

}

void SYT_VisitDeclaration( SymTable * syt, Ast * ast )
{

}

void SYT_VisitAssign( SymTable * syt, Ast * ast )
{

}

void SYT_VisitCall( SymTable * syt, Ast * ast )
{

}

void SYT_ProcessNode( SymTable * syt, Ast * ast )
{
    switch( AST_GetType( ast ) )
    {
        case A_FUNCTION:
            SYT_VisitFunction( syt, ast );
    }
}

void SYT_Build( SymTable * syt, Ast * ast )
{
	syt->ast = ast;
	
	Ast ** children;
	int nChildren = AST_GetChildrenArray( ast, &children );
	int i;
	
	// Global scope
	SYT_OpenScope( syt );
	
	for( i = 0; i < nChildren; i++ )
	{
	    switch( AST_GetType( children[i] ) )
	    {
	        case A_FUNCTION:
	            {
	                Ast ** funcChildren;
	                int nFChildren = AST_GetChildrenArray( children[i], &funcChildren );                
	                char * name = AST_FindId( funcChildren, nFChildren );
	                char * strType = AST_FindType( funcChildren, nFChildren );
	                int type = S_NONE;
	                
	                if( strcmp( strType, "char" ) == 0 )
	                    type = S_CHAR;
	                else if( strcmp( strType, "int" ) == 0 )
	                    type = S_INT;
	                else if( strcmp( strType, "bool" ) == 0 )
	                    type = S_BOOL;
	                else if( strcmp( strType, "string" ) == 0 )
	                    type = S_STRING;
	                    
	                if( !SYT_AddSymbol( syt, name, type ) )
	                {
	                    errorSymTable( ERROR_REDEFINED, name, AST_GetLine( children[i] ) );
	                }
	            }
	            
	            break;
	            
	        case A_DECLVAR:
	            break;
	            
            default:
	            errorSymTable( ERROR_UNKNOWN, NULL, 0 );
	    }
	}
		
    //SYT_ProcessNode( syt, ast );
    SYT_CloseScope( syt );
}

