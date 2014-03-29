#ifndef LIST_H
#define LIST_H

typedef struct list List;


List * LIS_New();

void LIS_Delete( List * list, void (*pfuncDelete)( void * ) );

void LIS_PushBack( List * list, void * info );

void LIS_Advance( List * list );

void LIS_Match( List * list, void * info, int (*pfuncCmp)( void *, void * ), void (*pfuncError)( void *, void * ) );

void * LIS_Peek( List * list );

void LIS_Dump( List * list, void (*pfuncDump)( void * ) );

#endif
