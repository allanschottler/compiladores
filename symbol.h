#ifndef SYMBOL_H
#define SYMBOL_H

#define S_VOID		1
#define S_BOOL		2
#define S_CHAR		3
#define S_INT		4
#define S_STRING	5


typedef struct symbol Symbol;


Symbol * SYM_New( int type, int ptrType );

void SYM_Delete( Symbol * sym );

void SYM_PushParam( Symbol * sym, int type, int ptrType );

int SYM_CompareParams( Symbol * sym1, Symbol * sym2 );

int SYM_StringToType( char * str );

char * SYM_SymbolToString( Symbol * sym );

int SYM_GetType( Symbol * sym );

int SYM_GetPtrType( Symbol * sym );

int SYM_GetScopeId( Symbol * sym );

void SYM_SetScopeId( Symbol * sym, int id );

#endif
