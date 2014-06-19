#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "uthash.h"

#define N_REGS      6

#define BB_INS      0
#define BB_START    1
#define BB_END      2

// Variables and registers hash
typedef struct varhash VarHash;

struct varhash
{
    char * id;
    char * value;	
	UT_hash_handle hh;
};

// Next variable usage hash
typedef struct usagehash UsageHash;

struct usagehash
{
    char * id;
    int value;	
	UT_hash_handle hh;
};

typedef struct basicblock BasicBlock;

struct basicblock
{
    Instr * start;
    Instr * end;
    int size;
};

BasicBlock * BBL_New()
{
    BasicBlock * bbl = ( BasicBlock* )malloc( sizeof( BasicBlock ) );
    bbl->start = NULL;
    bbl->end = NULL;
    bbl->size = 0;
    
    return bbl;
}

void BBL_Delete( BasicBlock * bbl )
{
    free( bbl );
}

void BBL_Dump( BasicBlock * bbl, Function * func, int blockNum )
{    
    Instr * start;
    printf( "\nfun %s block %d size %d\n", func->name, blockNum, bbl->size );
    for( start = bbl->start; start != bbl->end->next; start = start->next )
    {
        Instr_dump( start, stdout );
        
        if( start->op != OP_LABEL && start->op != OP_GOTO && start->op != OP_CALL && start->op != OP_RET && start->op != OP_RET_VAL )
            printf( "\t%d %d %d\n", start->x.nextUsage, start->y.nextUsage, start->z.nextUsage );
                    
    }
}

struct assembler
{
    VarHash * varStates;
    UsageHash * varUsages;
};

/*
Constructor
*/
Assembler * ASM_New()
{
    Assembler * asm = ( Assembler* )malloc( sizeof( Assembler ) );
    
    asm->varStates = NULL;
    asm->varUsages = NULL;
    
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
void ASM_AddVar( Assembler * asm, char * name )
{
    if( !name )
        return;
    
    VarHash *h;
    
    // States
    HASH_FIND_STR( asm->varStates, name, h );
        
    if( !h )
    {
	    VarHash * h = ( VarHash* )malloc( sizeof( VarHash ) );
	    h->id = strdup( name );
	    h->value = NULL;	    
	
	    HASH_ADD_STR( asm->varStates, id, h );
	}
	else
	{
	    fprintf( stderr, "!Assembling Error: internal compiler error.\n");
	    exit( EXIT_FAILURE );
	}
	
	// Usages
	HASH_FIND_STR( asm->varUsages, name, h );
        
    if( !h )
    {
	    UsageHash * h = ( UsageHash* )malloc( sizeof( UsageHash ) );
	    h->id = strdup( name );
	    h->value = 0;	    
	
	    HASH_ADD_STR( asm->varUsages, id, h );
	}
	else
	{
	    fprintf( stderr, "!Assembling Error: internal compiler error.\n");
	    exit( EXIT_FAILURE );
	}
}

/*
Sets value of key name in hash
*/
void ASM_SetVarValue( Assembler * asm, char * name, char * value )
{
    if( !name )
        return;
        
    VarHash * h = ( VarHash* )malloc( sizeof( VarHash ) );
    VarHash * tmp;
    h->id = strdup( name );
    h->value = strdup( value );
    
    HASH_FIND_STR( asm->varStates, name, tmp );
        
    if( tmp )
    {        
        HASH_DEL( asm->varStates, tmp );	    
    }   
    
    HASH_ADD_STR( asm->varStates, id, h );
}

/*
Sets vars' next alive usage within instruction
*/
void ASM_SetVarUsage( Assembler * asm, char * name, int value )
{
    if( !name )
        return;
        
    UsageHash * h = ( UsageHash* )malloc( sizeof( UsageHash ) );
    UsageHash * tmp;
    h->id = strdup( name );
    h->value = value;
    
    HASH_FIND_STR( asm->varUsages, name, tmp );
        
    if( tmp )
    {        
        HASH_DEL( asm->varUsages, tmp );	    
    }   
    
    HASH_ADD_STR( asm->varUsages, id, h );
}

/*
Gets vars next alive usage within instruction
*/
int ASM_GetVarUsage( Assembler * asm, char * name )
{
    if( !name )
        return 0;
    
    UsageHash *h;
    HASH_FIND_STR( asm->varUsages, name, h );
        
    if( h )
    {	    
	    return h->value;
	}   
	
	return 0;
}

/*
Dumps hash entries for debugging
*/
/*void ASM_DumpLocalVars( Assembler * asm )
{
    VarHash *curr, *tmp;

    HASH_ITER( hh, asm->varStates, curr, tmp ) 
    {
        printf( "%s %s\n", curr->id, curr->value );
    }
}*/

/*
Clears states hash
*/
void ASM_ClearHashes( Assembler * asm )
{    
    HASH_CLEAR( hh, asm->varStates );
    HASH_CLEAR( hh, asm->varUsages );
}

/*
Adds registers and variables to hash
*/
void ASM_SetupHashes( Assembler * asm, Function * func )
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
    
    bbl->size = 0;
    
    if( !currIns )
        currIns = func->code;
        
    int type = getInstructionType( currIns );
    
    bbl->start = currIns;
    
    if( type == BB_START )
    {
        last = currIns;  
        currIns = currIns->next;
        bbl->size++;
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
        bbl->size++;
    }
    
    if( !currIns )    
        bbl->end = last;
        
    return ( currIns != NULL );
}

/*
Generates assembly code of given basic block
*/
void ASM_GenerateCode( Assembler * asm, BasicBlock * bbl )
{
        
}

void ASM_SetVarLiveness( Assembler * asm, Addr * var, int nextUsage, int thisUsage )
{
    if( (*var).type != AD_TEMP && (*var).type != AD_LOCAL && (*var).type != AD_GLOBAL )
    {
        (*var).nextUsage = -1;
        return;
    }

    (*var).nextUsage = nextUsage;
    ASM_SetVarUsage( asm, (*var).str, thisUsage );
}

/*
Tags addresses' next alive reference
*/
void ASM_SetupVarsLiveness( Assembler * asm, Instr * start, Instr * end, int depth  )
{
    if( start == end )
    {        
        ASM_SetVarLiveness( asm, &(start->x), 0, depth );
        ASM_SetVarLiveness( asm, &(start->y), 0, depth );
        ASM_SetVarLiveness( asm, &(start->z), 0, depth );       
                                    
        return;
    }
    
    ASM_SetupVarsLiveness( asm, start->next, end, depth+1 );
    
    int x = ASM_GetVarUsage( asm, start->x.str );
    int y = ASM_GetVarUsage( asm, start->y.str );
    int z = ASM_GetVarUsage( asm, start->z.str );
    
    ASM_SetVarLiveness( asm, &(start->x), x, 0 );
    ASM_SetVarLiveness( asm, &(start->y), y, depth );
    ASM_SetVarLiveness( asm, &(start->z), z, depth );
}

/*
Builds given function's basic blocks
*/
void ASM_BuildBlocks( Assembler * asm, Function * func )
{
    int blockNum = 0;
    int loop = 0;
    BasicBlock * bbl = BBL_New();
    
    do
    {        
        loop = ASM_NextBasicBlock( asm, func, bbl );
                
        ASM_SetupVarsLiveness( asm, bbl->start, bbl->end, 1 );        
        BBL_Dump( bbl, func, blockNum++ );
        ASM_GenerateCode( asm, bbl );
        ASM_ClearHashes( asm ); 
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
        //ASM_ClearHashes( asm );
        ASM_SetupHashes( asm, func );
        ASM_BuildBlocks( asm, func );
        func = func->next;
    }
}
