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

void LIS_Match( List * list, void * info, int (*pfuncCmp)( void *, void * ), void (*pfuncError)( void *, void * ) )
{
    if( (*pfuncCmp)( info, list->current->info ) )
        LIS_Advance( list );
    else
        (*pfuncError)( info, list->current->info );
}

void * LIS_Peek( List * list )
{
    return list->current->next->info;
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
