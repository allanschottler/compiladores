#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "uthash.h"

#define N_REGS      6

#define BB_INS      0
#define BB_START    1
#define BB_END      2

typedef struct varhash VarHash;

struct varhash
{
    char * id;
    char * value;	
	UT_hash_handle hh;
};

typedef struct basicblock BasicBlock;

struct basicblock
{
    Instr * start;
    Instr * end;
};

BasicBlock * BBL_New()
{
    BasicBlock * bbl = ( BasicBlock* )malloc( sizeof( BasicBlock ) );
    bbl->start = NULL;
    bbl->end = NULL;
    
    return bbl;
}

void BBL_Delete( BasicBlock * bbl )
{
    free( bbl );
}

struct assembler
{
    VarHash * varStates;
};

/*
Constructor
*/
Assembler * ASM_New()
{
    Assembler * asm = ( Assembler* )malloc( sizeof( Assembler ) );
    
    asm->varStates = NULL;
    
    return asm;
}

/*
Destructor
*/
void ASM_Delete( Assembler * asm )
{

}

/*
Creates an entry in vars hash
*/
int ASM_AddVar( Assembler * asm, char * name )
{
    if( !name )
        return 0;
    
    VarHash *h;
    HASH_FIND_STR( asm->varStates, name, h );
        
    if( !h )
    {
	    VarHash * h = ( VarHash* )malloc( sizeof( VarHash ) );
	    h->id = strdup( name );
	    h->value = NULL;	    
	
	    HASH_ADD_STR( asm->varStates, id, h ); //This works cuz macros
	    
	    return 1;
	}
	else
	{
	    fprintf( stderr, "!Assembling Error: internal compiler error.\n");
	    exit( EXIT_FAILURE );
	}
	
	return 0;
}

/*
Clears states hash
*/
void ASM_ClearLocalVars( Assembler * asm )
{
    VarHash *curr, *tmp;

    HASH_ITER( hh, asm->varStates, curr, tmp ) 
    {
        HASH_DEL( asm->varStates, curr );
        free( curr );
    }
}

/*
Dumps hash entries for debugging
*/
void ASM_DumpLocalVars( Assembler * asm )
{
    VarHash *curr, *tmp;

    HASH_ITER( hh, asm->varStates, curr, tmp ) 
    {
        printf( "%s %s\n", curr->id, curr->value );
    }
}

/*
Adds registers and variables to hash
*/
void ASM_SetupLocalVars( Assembler * asm, Function * func )
{
    int i;
    char * str = malloc( 64 );
    
    for( i = 0; i < N_REGS; i++ )
    {
        sprintf( str, "$R%d", i );
        ASM_AddVar( asm, str );
    }
    
    Variable * it = func->locals;
    while( it )
    {
        sprintf( str, "%s", it->name );
        ASM_AddVar( asm, str );
        it = it->next;
    }
    
    it = func->temps;
    while( it )
    {
        sprintf( str, "%s", it->name );
        ASM_AddVar( asm, str );
        it = it->next;
    }
    
    free( str );
}

/*
Return 1 if ins is a basic block end instruction
*/
static int getInstructionType( Instr * ins )
{
    switch( ins->op )
    {
	    case OP_GOTO:
	    case OP_IF:
	    case OP_IF_FALSE:
            return BB_END;
            
        case OP_LABEL:
            return BB_START;
                            
        default:
            return BB_INS;
    }
}

/*
Returns[in] function's next basic block start and end instructions
Returns[out] 0 if function reached end 
*/
int ASM_NextBasicBlock( Assembler * asm, Function * func, BasicBlock * bbl )
{
    int foundEnd = 0;
    static Instr * currIns = NULL;
    Instr * last;    
        
    if( !currIns )
        currIns = func->code;
        
    int type = getInstructionType( currIns );
    
    bbl->start = currIns;
    
    if( type == BB_START )
    {
        last = currIns;  
        currIns = currIns->next;
    }            
        
    while( currIns && !foundEnd )
    {
        type = getInstructionType( currIns );
        
        if( type != BB_INS )
        {
            foundEnd = 1;
            
            if( type == BB_END )
            {
                bbl->end = currIns;
            }
            else
            {
                bbl->end = last;
                break;
            }
        }
        
        last = currIns;  
        currIns = currIns->next;
    }
    
    if( !currIns )    
        bbl->end = last;
        
    return ( currIns != NULL );
}

/*
Generates assembly code of given basic block
*/
void ASM_GenerateBlock( Assembler * asm, BasicBlock * bbl )
{

}

/*
Tags addresses next alive reference
*/
void ASM_SetupVarsLiveness( Assembler * asm, BasicBlock * bbl )
{

}

/*
Builds given function's basic blocks
*/
void ASM_BuildBlocks( Assembler * asm, Function * func )
{
    int blockNum = 0;
    BasicBlock * bbl = BBL_New();
    int loop = 0;
    
    do
    {        
        loop = ASM_NextBasicBlock( asm, func, bbl );
    
        ASM_SetupVarsLiveness( asm, bbl );
        ASM_GenerateBlock( asm, bbl );
    }
    while( loop );    
}

/*
Builds assembly code
*/
void ASM_Build( Assembler * asm, IR * ir )
{
    Function * func = ir->functions;
    while( func )
    {
        ASM_ClearLocalVars( asm );
        ASM_SetupLocalVars( asm, func );
        ASM_BuildBlocks( asm, func );
        func = func->next;
    }
}
