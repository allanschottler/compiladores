#ifndef LIST_H
#define LIST_H


typedef struct list List;


List * LIS_New();

void LIS_Delete( List * list, void (*pfuncDelete)( void * ) );

void LIS_PushBack( List * list, void * info );

void * LIS_Match( List * list, int info, int (*pfuncCmp)( void *, int ), void (*pfuncError)( void *, int ) );

int LIS_Peek( List * list, int (*pfuncIntEval)( void * ) );

int LIS_GetSize( List * list );

void * LIS_GetCurrent( List * list );

void LIS_Dump( List * list, void (*pfuncDump)( void * ) );

#endif
