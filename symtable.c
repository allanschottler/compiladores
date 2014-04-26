#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtable.h"
#include "uthash.h"

#define SCOPE_SIZE 4

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

    HASH_FIND_STR( syt->current->symbols, idName, h );
    return ( h ? h->type : S_NONE );
}
 
int SYT_AddSymbol( SymTable * syt, char * idName, int type )
{
    Hash * h;
    HASH_FIND_STR( syt->current->symbols, idName, h );
    
    if( h == NULL ) 
    {
	    h = ( Hash* )malloc( sizeof( Hash ) );
	    h->id = ( char* )malloc( strlen( idName ) * sizeof( char ) );
	    strcpy( h->id, idName );
	    h->type = type;
	
	    HASH_ADD_STR( syt->current->symbols, id, h );
	    return 1;
	}
	
	return 0;
}

void SYT_Build( SymTable * syt, Ast * ast )
{
	syt->ast = ast;
	
	printf("open\n");
	SYT_OpenScope( syt );
	
		printf("add\n");
	if( SYT_AddSymbol( syt, "x", S_INT ) )
	{
	    printf("check\n");
	    printf("Added? %d\n", SYT_CheckSymbol( syt, "x" ) );
	}
	
	if( SYT_AddSymbol( syt, "y", S_BOOL ) )
	{
	    printf("check2\n");
	    printf("Added? %d\n", SYT_CheckSymbol( syt, "y" ) );
	}

	printf("close\n");	
	SYT_CloseScope( syt );	
}

void SYT_ProcessNode( SymTable * syt )
{

}


