#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol.h"

#define MAX_PARAMS 16

struct symbol
{    
	int type;
	int ptrType;
	int * paramsType;
	int * paramsPtrType;
	int nParams;
	int scopeId;
};

Symbol * SYM_New( int type, int ptrType )
{
    Symbol * s = ( Symbol* )malloc( sizeof( Symbol ) );    
    
    s->type = type;
    s->ptrType = ptrType;
    s->paramsType = NULL;
    s->paramsPtrType = NULL;
    s->nParams = 0;
    s->scopeId = 0;
    
    return s;
}

void SYM_Delete( Symbol * sym )
{
    if( !sym )
        return;
        
    if( sym->nParams )
    {
        free( sym->paramsType );
        free( sym->paramsPtrType );
    }
    
    free( sym );    
}

void SYM_PushParam( Symbol * sym, int type, int ptrType )
{
    if( !sym->paramsType )
    {
        sym->paramsType = ( int* )malloc( MAX_PARAMS * sizeof( int ) );
        sym->paramsPtrType = ( int* )malloc( MAX_PARAMS * sizeof( int ) );
    }   
    
    sym->paramsType[sym->nParams] = type;
    sym->paramsPtrType[sym->nParams] = ptrType;
    sym->nParams++;
}

int SYM_CompareParams( Symbol * sym1, Symbol * sym2 )
{
    if( sym1->nParams != sym2->nParams )
        return 0;
        
    if( sym1->nParams == 0 && sym2->nParams == 0 )
        return 1;
            
    int i;
    
    for( i = 0; i < sym1->nParams; i++ )
    {
        if( sym1->paramsPtrType[i] != sym2->paramsPtrType[i] )
            return 0;
            
        if( sym1->paramsType[i] != sym2->paramsType[i] )
        {
            if( ( sym1->paramsType[i] == S_CHAR && sym2->paramsType[i] == S_INT ) || ( sym2->paramsType[i] == S_CHAR && sym1->paramsType[i] == S_INT ) )
                return 1;
                
            return 0;
        }            
    }
    
    return 1;
}

int SYM_StringToType( char * str )
{
    if( !str )
        return S_VOID;
        
    if( strcmp( str, "char" ) == 0 )
        return S_CHAR;
    else if( strcmp( str, "int" ) == 0 )
        return S_INT;
    else if( strcmp( str, "bool" ) == 0 )
        return S_BOOL;
    else if( strcmp( str, "string" ) == 0 )
        return S_STRING;
        
    return S_VOID;
}

char * SYM_SymbolToString( Symbol * sym )
{
    char * str = ( char* )malloc( 64 * sizeof( char ) );
    str[0] = 0;
    
    if( sym )
    {
        if( sym->nParams )
        {
            char params[3];
            sprintf( params, "%d", sym->nParams );
            strcat( str, "function(" );
            strcat( str, params );
            strcat( str, ") : " );
        }
        
        if( sym->ptrType )
        {
            char ptrType[4];
            
            sprintf( ptrType, "%d", sym->ptrType );
            strcat( str, ptrType );
            strcat( str, "*" );
        }
        
        switch( sym->type )
        {
            case S_VOID:
                strcat( str, "void" );
                break;
                
            case S_INT:
                strcat( str, "int" );
                break;

            case S_BOOL:
                strcat( str, "bool" );
                break;            
                
            case S_STRING:
                strcat( str, "string" );
                break;
                
            case S_CHAR:
                strcat( str, "char" );
                break;                        
        }
    }
    
    return str;    
}

int SYM_GetType( Symbol * sym )
{
    return sym->type;
}

int SYM_GetPtrType( Symbol * sym )
{
    return sym->ptrType;
}

int SYM_GetScopeId( Symbol * sym )
{
    return sym->scopeId;
}

void SYM_SetScopeId( Symbol * sym, int id )
{
    sym->scopeId = id;
}
