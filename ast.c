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
	int line;
	Node * parent;
	Node * next;
	Node * prev;
	Node * child;
};

Node * ASN_New( int type, char * value, int line )
{
	Node * node = ( Node* )malloc( sizeof( Node ) );
	node->type = type;
	node->line = line;
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
    if( node )
    {
	    Node * next = node->child;
	    Node * toDelete;
	
	    while( next )
	    {
	        toDelete = next;
	        next = next->next;
	        ASN_Delete( toDelete );
	    }	
        
        if( node->next )
        {
            if( node->prev )
            {
                node->next->prev = node->prev;
                node->prev->next = node->next;
            }
            else
            {
                node->next->prev = NULL;
                
                if( node->parent )
                    node->parent->child = node->next;
            }
        }
        else
        {
            if( node->prev )
            {
                node->prev->next = NULL;            
            }
            else
            {  
                if( node->parent )          
                    node->parent->child = NULL;
            }
        }
		
	    if( node->value )
	        free( node->value );
	        
	    free( node );
	}
}

char * ASN_ToString( int type )
{
    char * str = ( char* )malloc( 16 * sizeof( char ) );
    
    switch( type )
    {
        case A_PROGRAM:
            strcpy( str, "Program" );
            break;
        
        case A_TYPE:
            strcpy( str, "Type" );        
            break;
            
        case A_IF:
            strcpy( str, "If" );        
            break;
                        
        case A_WHILE:
            strcpy( str, "While" );        
            break;
            
        case A_DECLVAR:
            strcpy( str, "Declvar" );        
            break;
            
        case A_ASSIGN:
            strcpy( str, "Assign" );        
            break;
            
        case A_CALL:
            strcpy( str, "Call" );        
            break;
            
        case A_FUNCTION:
            strcpy( str, "Function" );        
            break;
            
        case A_RETURN:
            strcpy( str, "Return" );        
            break;
            
        case A_BLOCK:
            strcpy( str, "Block" );        
            break;
            
        case A_ID:
            strcpy( str, "ID" );        
            break;
            
        case A_VAR:
            strcpy( str, "Var" );        
            break;
            
        case A_ARGS:
            strcpy( str, "Args" );        
            break;
            
        case A_PARAMS:
            strcpy( str, "Params" );        
            break;            
            
        case A_ADD:
            strcpy( str, "+" );        
            break;
                    
        case A_SUBTRACT:
            strcpy( str, "-" );        
            break;
        
        case A_DIVIDE:
            strcpy( str, "/" );        
            break;
            
        case A_MULTIPLY:
            strcpy( str, "*" );        
            break;
            
        case A_EQ:
            strcpy( str, "=" );        
            break;
            
        case A_NEQ:
            strcpy( str, "<>" );        
            break;
            
        case A_LARGER:
            strcpy( str, ">" );        
            break;
            
        case A_SMALLER:
            strcpy( str, "<" );        
            break;
            
        case A_LARGEREQ:
            strcpy( str, ">=" );        
            break;
            
        case A_SMALLEREQ:
            strcpy( str, "<=" );        
            break;
            
        case A_AND:
            strcpy( str, "And" );        
            break;
            
        case A_OR:
            strcpy( str, "Or" );        
            break;
            
        case A_NOT:
            strcpy( str, "Not" );        
            break;
            
        case A_NEGATIVE:
            strcpy( str, "Negative" );        
            break;
            
        case A_NEW:
            strcpy( str, "New" );        
            break; 
            
        case A_LITINT:
            strcpy( str, "Integer" );        
            break;
            
        case A_LITSTRING:
            strcpy( str, "String" );        
            break;  
            
        case A_TRUE:
            strcpy( str, "True" );        
            break;
            
        case A_FALSE:
            strcpy( str, "False" );        
            break;                                                
                                                                                                                                                            
    }
    
    return str;
}

void ASN_Dump( Node * node, int depth )
{
    char * str = ASN_ToString( node->type );
    printf("%s %s @%d\n", str, node->value, node->line );
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
    if( ast )
    {
	    ASN_Delete( ast->root );	
    	free( ast );    	
    }
}

void AST_PrependChildTree( Ast * parent, Ast * child )
{
    if( parent && child )
    {
        Node * childNode = child->root;
        
        if( parent->root && childNode )
        {	
	        Node * currChild = parent->root->child;
		
	        if( currChild )
	        {
		        currChild->prev = childNode;
		        childNode->next = currChild;
		        childNode->parent = currChild->parent;
		        currChild->parent->child = childNode;		        
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
	
	free( child );
}

void AST_AppendChildTree( Ast * parent, Ast * child )
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
	
	free( child );
}

void AST_PrependChildNode( Ast * parent, int type, char * text, int line )
{
    if( !parent )
        return;
    
    Node * child = ASN_New( type, text, line );
    
    if( parent->root )
    {	
        Node * currChild = parent->root->child;
	
        if( currChild )
        {
            currChild->prev = child;
            child->next = currChild;
            child->parent = currChild->parent;
            currChild->parent->child = child;
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

void AST_AppendChildNode( Ast * parent, int type, char * text, int line )
{
    if( !parent )
        return;
        
    Node * child = ASN_New( type, text, line );
        
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

int AST_TokenTypeToAst( int tokenType )
{
    switch( tokenType )
    {        
        case T_LARGER:
            return A_LARGER;
            
        case T_SMALLER:
            return A_SMALLER;
            
        case T_LARGEREQ:
            return A_LARGEREQ;
            
        case T_SMALLEREQ:
            return A_SMALLEREQ;
            
        case T_EQ:
            return A_EQ;
            
        case T_NEQ:
            return A_NEQ;
                    
        case T_PLUS:
            return A_ADD;
            
        case T_MINUS:
            return A_SUBTRACT;
            
        case T_ASTERISK:
            return A_MULTIPLY;
            
        case T_SLASH:
            return A_DIVIDE;            
            
        case T_AND:
            return A_AND;
            
        case T_OR:
            return A_OR;
            
        case T_NOT:
            return A_NOT;
            
        case T_LITINT:
            return A_LITINT;
            
        case T_LITSTRING:
            return A_LITSTRING;            
        
        case T_TRUE:
            return A_TRUE;
            
        case T_FALSE:
            return A_FALSE;
                                         
        default:
            return -1;                              
    }
}

void AST_Dump( Ast * ast )
{
    if( !ast )
        return;
        
    printf( "\n=======AST=======\n" );
    ASN_Dump( ast->root, 1 );
}

// I hate everything about this function, but it's saturday night
/*int AST_GetChildrenArray( Ast * ast, Ast *** outChildren )
{
    if( !ast )
        return 0;
    
    if( !ast->root )
        return 0;
            
    Node * currNode = ast->root->child;
    
    if( !currNode )
        return 0;
        
    int count = 1;
    
    while( currNode->next )
    {
        count++;
        currNode = currNode->next;
    }
    
    *outChildren = ( Ast** )malloc( count * sizeof( Ast* ) );
    currNode = ast->root->child;

    int i = 0;
    
    do    
    {
        Ast * child = AST_New();        
        child->root = currNode;
        ( *outChildren )[i++] = child;
        currNode = currNode->next;        
    }
    while( currNode );
    
    return count;
}

void AST_FreeChildrenArray( Ast ** children, int nChildren )
{
    int i;
    
    for( i = 0; i < nChildren; i++ )
    {
        free( children[i] );
    }   
    
    free( children );
}*/

int AST_GetType( Ast * ast )
{
    return ast->root->type;
}

char * AST_GetValue( Ast * ast )
{
    if( ast->root->value )
        return ast->root->value;
        
    return NULL;
}

int AST_GetLine( Ast * ast )
{
    return ast->root->line;
}

Ast * AST_GetChild( Ast * ast )
{
	if( !ast )
		return NULL;
		
	if( !ast->root )
		return NULL;
	
	if( !ast->root->child )
		return NULL;
		
	Ast * child = AST_New();
	child->root = ast->root->child;
		
	return child;
}

Ast * AST_NextSibling( Ast * ast )
{
	if( !ast )
		return NULL;
		
	if( !ast->root )
	{
	    free( ast );
		return NULL;
    }
		
	if( !ast->root->next )
	{
	    free( ast );
		return NULL;
	}
		
	Ast * sibling = AST_New();
	sibling->root = ast->root->next;
	
	free( ast );
	
	return sibling;
}

char * AST_FindId( Ast * ast )
{
    int type = ast->root->type;
    
    if( type != A_FUNCTION && type != A_CALL && type != A_VAR && type != A_DECLVAR )
        return NULL;
        
    Node * child;
    
    for( child = ast->root->child; child; child = child->next )
    {
        if( child->type == A_ID )
            return child->value;
    }
    
    return NULL;
}

char * AST_FindType( Ast * ast )
{
    int type = ast->root->type;
    
    if( type != A_FUNCTION && type != A_DECLVAR )
        return NULL;
        
    Node * child;
    
    for( child = ast->root->child; child; child = child->next )
    {
        if( child->type == A_TYPE )
        {            
            child = child->child;
            
            while( strcmp( child->value, "[]" ) == 0 )
                child = child->next;
                
            return child->value;
        }
    }
    
    return NULL;
}

