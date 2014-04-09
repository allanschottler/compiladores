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

void NOD_AddChild( Node * parent, Node * child )
{	
	if( parent && child )
	{
		Node * currChild = parent->child;
		
		if( currChild )
		{
			while( currChild->next )
				currChild = currChild->next;
			
			currChild->next = child;
			child->prev = currChild;
			child->parent = currChild->parent;
		}
		else
		{
			parent->child = child;
			child->parent = parent;
		}
	}
}

struct ast
{
	Node * root;
	Node * current;
	List * tokens;
};

Ast * AST_New( List * tokens )
{
	Ast * ast = ( Ast* )malloc( sizeof( Ast ) );
	ast->root = NOD_New( A_PROGRAM, NULL );
	ast->current = ast->root;
	ast->tokens = tokens;
	
	return ast;
}

void AST_Delete( Ast * ast )
{
	NOD_Delete( ast->root );
	free( ast );
}

void AST_Build( List * tokens )
{

}

Node * AST_BranchFunction( Ast * ast )
{
	Node * root = NOD_New( A_FUNCTION, NULL );
		
	Node * id = AST_BranchID( Ast * ast );
	NOD_AddChild( root, id );
	
	Node * decl = AST_BranchDeclvar( Ast * ast );
	NOD_AddChild( root, decl );
	
	Node * type = AST_BranchType( Ast * ast );
	NOD_AddChild( root, type );
	
	Node * block = AST_BranchBlock( Ast * ast );
	NOD_AddChild( root, block );
	
	Node * ret = AST_BranchReturn( Ast * ast );
	NOD_AddChild( root, ret );
	
	return root;
}




















