#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "token.h"


// Abract Syntax Node
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

Node * ASN_New( int type, char * value )
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

void ASN_Delete( Node * node )
{
	/*int isFirstChild = ( node->prev == NULL );
	Node * currChild = node->child;
	
	if( currChild )
	{
	    while( currChild->next )
		    currChild = currChild->next;
		    
		while( currChild->prev )
		{
		    Node * toDelete = currChild;
		    currChild = currChild->prev;
		    
		    NOD_Delete( toDelete );
		}
	}*/
}

char * ASN_ToString( int type )
{
    char * str = ( char* )malloc( 16 * sizeof( char ) );
    
    switch( type )
    {
        case 0:
            strcpy( str, "Program" );
            break;
        
        case 1:
            strcpy( str, "Type" );        
            break;
            
        case 2:
            strcpy( str, "If" );        
            break;
                        
        case 3:
            strcpy( str, "While" );        
            break;
            
        case 4:
            strcpy( str, "Declvar" );        
            break;
            
        case 5:
            strcpy( str, "Assign" );        
            break;
            
        case 6:
            strcpy( str, "Call" );        
            break;
            
        case 7:
            strcpy( str, "Function" );        
            break;
            
        case 8:
            strcpy( str, "Return" );        
            break;
            
        case 9:
            strcpy( str, "Block" );        
            break;
            
        case 10:
            strcpy( str, "ID" );        
            break;
            
        case 11:
            strcpy( str, "Var" );        
            break;
            
        case 12:
            strcpy( str, "Args" );        
            break;
            
        case 13:
            strcpy( str, "Params" );        
            break;               
                                                                                                                                                            
    }
    
    return str;
}

void ASN_Dump( Node * node, int depth )
{
    char * str = ASN_ToString( node->type );
    printf("VALUE: %s TYPE: %s\n", node->value, str );
    free( str );
    
    Node * currChild = node->child;
    
    if( currChild )
    {
        do
        {
            int i;
            for( i = 0; i < depth; i++ )
                printf("\t");
                
            ASN_Dump( currChild, depth + 1 );
            currChild = currChild->next;
        } 
        while( currChild );
    }
}

// Abract Syntax Tree
struct ast
{
	Node * root;
	Node * current;
	List * tokens;
};

Ast * AST_New()
{
	Ast * ast = ( Ast* )malloc( sizeof( Ast ) );
	ast->root = NULL;
	ast->current = NULL;	
	
	return ast;
}

void AST_Delete( Ast * ast )
{
	ASN_Delete( ast->root );
	free( ast );
}

void AST_AddChildTree( Ast * parent, Ast * child )
{
    if( parent && child )
    {
        Node * childNode = child->root;
        
        if( parent->root && childNode )
        {	
	        Node * currChild = parent->root->child;
		
	        if( currChild )
	        {
		        while( currChild->next )
			        currChild = currChild->next;
		
		        currChild->next = childNode;
		        childNode->prev = currChild;
		        childNode->parent = currChild->parent;
	        }
	        else
	        {
		        parent->root->child = childNode;
		        childNode->parent = parent->root;
	        }
	    }
	    else
	    {
	        parent->root = childNode;
	    }
	}
}

void AST_AddChildNode( Ast * parent, int type, char * text )
{
    if( parent )
    {
        Node * child = ASN_New( type, text );
        
        if( parent->root )
        {	
	        Node * currChild = parent->root->child;
		
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
		        parent->root->child = child;
		        child->parent = parent->root;
	        }
	    }
	    else
	    {
	        parent->root = child;
	    }	
	}
}

void AST_Dump( Ast * ast )
{
    printf("\n=======AST=======\n");
    ASN_Dump( ast->root, 1 );
}














