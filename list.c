#include <stdlib.h>

#include "list.h"


typedef struct node Node;

struct node
{
    void * info;
    Node * next;
    Node * prev;
};

Node * NOD_New( void * info )
{
    Node * node = ( Node* )malloc( sizeof( Node ) );
    node->info = info;
    node->next = NULL;
    node->prev = NULL;
    
    return node;
}

void NOD_Delete( Node * node, void (*pfuncDelete)( void * ) )
{
    if( !node )
        return;
        
    NOD_Delete( node->next, pfuncDelete );
    
    if( node->prev )
        node->prev->next = NULL;
    
    (*pfuncDelete)( node->info );    
    free( node );
}

void LIS_Advance( List * list );

struct list
{
    int size;
    Node * first;
    Node * last;
    Node * current;
};

List * LIS_New()
{
    List * list = ( List* )malloc( sizeof( List ) );
    list->first = NULL;
    list->last = NULL;
    list->current = NULL;
    list->size = 0;
    
    return list;
}

void LIS_Delete( List * list, void (*pfuncDelete)( void * ) )
{
    NOD_Delete( list->first, pfuncDelete );
    free( list );
}

void LIS_PushBack( List * list, void * info )
{
    if( list->size )
    {
        list->last->next = NOD_New( info );
        list->last = list->last->next;
    }
    else
    {
        list->first = NOD_New( info );
        list->last = list->current = list->first;       
    }
    
    list->size++;
}

void LIS_Advance( List * list )
{
    list->current = list->current->next;
}

void LIS_Match( List * list, int info, int (*pfuncCmp)( void *, int ), void (*pfuncError)( void *, int ) )
{
	if( list->current == NULL )
    	(*pfuncError)( NULL, info );
    	
    if( (*pfuncCmp)( list->current->info, info ) )
        LIS_Advance( list );
    else
        (*pfuncError)( list->current->info, info );
        
    
}

int LIS_Peek( List * list, int (*pfuncIntEval)( void * ) )
{
    if( list->current )
        return (*pfuncIntEval)( list->current->info );
        
    return -1;
}

int LIS_GetSize( List * list )
{
    return list->size;
}

void * LIS_GetCurrent( List * list )
{
	if( list->current )
	    return list->current->info;
	    
	return NULL;
}

void LIS_Dump( List * list, void (*pfuncDump)( void * ) )
{
    list->current = list->first;
    
    while( list->current != NULL )
    {
        (*pfuncDump)( list->current->info );
        list->current = list->current->next;
    }
}
