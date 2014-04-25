#include <stdio.h>
#include <stdlib.h>

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
	
	for( i = 0; i < scope->nScopes; i++ )
	{
		SCO_Delete( scope->scopes[i] );
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

void SYT_Build( SymTable * syt, Ast * ast )
{
	syt->ast = ast;
}

void SYT_ProcessNode( SymTable * syt )
{

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

int SYT_CheckSymbol( SymTable * syt, const char * id )
{
	//std::map<char *, int>::iterator it = syt->current->symbols.find( id );
	
	//return ( *it == syt->current->symbols.begin() ? S_NONE : *it );
	HASH_FIND_
}
 
void SYT_AddSymbol( SymTable * syt, char * id, int type )
{
	struct hash * h = malloc( sizeof( struct hash ) );
	strcpy( h->id, id );
	h->type = type;
	
	HASH_ADD_STR( hash, id, h );
}
