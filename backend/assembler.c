#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"

#define N_REGS 3

typedef struct varhash VarHash;

struct varhash
{
    char * id;
    char * value;	
	UT_hash_handle hh;
};

struct assembler
{
    VarHash * varStates;
};

Assembler * ASM_New()
{
    Assembler * asm = ( Assembler* )malloc( sizeof( Assembler ) );
    
    asm->varStates = NULL;
    
    return asm;
}

void ASM_Delete( Assembler * asm )
{

}

int ASM_AddVar( Assembler * asm, char * name )
{
    if( !name )
        return 0;
    
    HASH_FIND_STR( current->symbols, idName, h );
        
    if( !h )
    {
	    VarHash * h = ( VarHash* )malloc( sizeof( VarHash ) );
	    h->id = name;
	    h->value = NULL;	    
	
	    HASH_ADD_STR( asm->varStates, id, h ); //This works cuz macros
	    
	    return 1;
	}
	else
	{
	    printf( stderr, "!Assembling Error: internal compiler error.\n");
	    exit( EXIT_FAILURE );
	}
	
	return 0;
}

void ASM_SetupLocalVars( Assembler * asm, Function * func )
{
    int i;
    for( i = 0; i < N_REGS; i++ )
    {
        char * str = malloc( 5 );
        sprintf( str, "$R%d", i );
        ASM_AddVar( asm, str );
    }
    
    Variable * it = asm->ir->function
}

void ASM_Build( Assembler * asm, IR * ir )
{

}
