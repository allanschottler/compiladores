#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "token.h"


typedef struct node Node;

struct node
{
	int type;
	char * value;
	Node * parent;
	Node * next;
	Node * prev;
	Node * child;
};

Node * NOD_New( int type, char * value )
{
	Node * node = ( Node* )malloc( sizeof( Node ) );
	node->type = type;
	node->value = NULL;
	
	if( value )	
		node->value = strdup( value );	
	
	node->parent = NULL;
	node->next = NULL;
	node->prev = NULL;
	node->child = NULL;
	
	return node;
}

void NOD_Delete( Node * node )
{
	
}

struct ast
{
	Node * root;
	Node * current;
};

Ast * AST_New()
{
	Ast * ast = ( Ast* )malloc( sizeof( Ast ) );
	ast->root = NOD_New( A_PROGRAM, NULL );
	ast->current = ast->root;
	
	return ast;
}

void AST_Delete( Ast * ast )
{
	NOD_Delete( ast->root );
	free( ast );
}

void AST_AddChild( Ast * ast, int type, char * value )
{
//	Node * newNode = NOD_New( type, value );
	
	if( ast->current )
	{
		Node * currChild = ast->current->child;
		
		while( currChild ) 
			currChild = currChild->next;
		
		
	}
}

void AST_Build( List * tokens )
{

}
