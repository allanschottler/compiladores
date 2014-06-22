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
    char * value[32];
    int size;	
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
	    h->size = 0;
	    int i;
	    for( i = 0; i < 32; i++ )
    	    h->value[i] = NULL;
	    
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
    h->size = 0;   
    
    HASH_FIND_STR( asm->varStates, name, tmp );
        
    if( tmp )
    {  
        if( value ) 
        {   
            h->value[h->size++] = strdup( value );
        }        
        
        HASH_DEL( asm->varStates, tmp );	    
    }   
    
    HASH_ADD_STR( asm->varStates, id, h );
}

/*
Adds value of key name in hash
*/
void ASM_AddVarValue( Assembler * asm, char * name, char * value )
{
    if( !name )
        return; 
    
    VarHash * h = ( VarHash* )malloc( sizeof( VarHash ) );
    VarHash * tmp;
    h->id = strdup( name );
    h->size = 0;    
    
    HASH_FIND_STR( asm->varStates, name, tmp );
        
    if( tmp )
    {      
        int i;  
        for( i = 0; i < tmp->size; i++ )
        {
            h->value[h->size++] = strdup( tmp->value[i] );            
        }
        
        h->value[h->size++] = strdup( value );
        
        HASH_DEL( asm->varStates, tmp );	    
    }   
    
    HASH_ADD_STR( asm->varStates, id, h );
}

/*
Gets vars value
*/
int ASM_GetVarValues( Assembler * asm, char * name, char *** out )
{
    if( !name )
        return 0;
    
    VarHash *h;
    HASH_FIND_STR( asm->varStates, name, h );
        
    if( h )
    {	
        *out = malloc( 32 * sizeof( char* ) );
        int i;
        for( i = 0; i < h->size; i++ )
        {
            (*out)[i] = strdup( h->value[i] );
        }
	    return h->size;
	}   
	
	return 0;
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

void ASM_UpdateLoad( Assembler * asm, char * reg, char * var )
{
    ASM_SetVarValue( asm, reg, var );
    ASM_AddVarValue( asm, var, reg );    
}

void ASM_UpdateStore( Assembler * asm, char * var )
{
    ASM_AddVarValue( asm, var, var );    
}

void ASM_UpdateOperation( Assembler * asm, char * regx, char * varx )
{
    ASM_SetVarValue( asm, regx, varx );
    ASM_SetVarValue( asm, varx, regx );
    
    VarHash * tmp, * s;
    HASH_ITER( hh, asm->varStates, s, tmp ) 
    {
        if( s->id[0] != '$' )
        {
            int i;
            for( i = 0; i < s->size; i++ )
            {
                if( strcmp( regx, s->value[i] ) == 0 )
                {
                    free( s->value[i] );
                    
                    // Shift everything left. Why am I not using c++ again?
                    int j;
                    for( j = i; j < s->size - 1; j++ )
                    {
                        s->value[j] = s->value[j+1];
                    }
                    
                    s->value[s->size] = NULL;
                    s->size--;
                }
            }
        }
    }        
}

static char * keyToRegister( char * key, int isByte )
{
    char * str = malloc( 5 );
    
    if( strcmp( key, "$R0" ) == 0 )
    {
        if( !isByte )
            sprintf( str, "%%eax" );        
        else
            sprintf( str, "%%al" );        
    }
    else if( strcmp( key, "$R1" ) == 0 )
    {
        if( !isByte )
            sprintf( str, "%%ebx" );        
        else
            sprintf( str, "%%bl" );        
    }
    else if( strcmp( key, "$R2" ) == 0 )
    {
        if( !isByte )
            sprintf( str, "%%ecx" );        
        else
            sprintf( str, "%%cl" );        
    }
    else if( strcmp( key, "$R3" ) == 0 )
    {
        if( !isByte )
            sprintf( str, "%%edx" );        
        else
            sprintf( str, "%%dl" );        
    }
    else
    {
        printf( "SOMETHING IS WRONG, FIX MEEEEEE!\n" );
        return 0;
    }
    
    return str;
}

char * ASM_FindRegisterForAddress( Assembler * asm, Addr a )
{
    if( !a.str )
        return NULL;
        
    int i;
    char * emptyReg = NULL;
    char ** valueA;
    int sizeA = ASM_GetVarValues( asm, a.str, &valueA );
    
    if( sizeA )
    {       
        for( i = 0; i < N_REGS/2; i++ )
        {
            char * reg = malloc( 5 );
            sprintf( reg, "$R%d", i );
            
            char ** valueReg;
            int sizeReg = ASM_GetVarValues( asm, reg, &valueReg );
            
            if( !sizeReg )
            {
                if( !emptyReg )
                    emptyReg = reg;
                    
                continue;
            }
            
            if( strcmp( valueA[0], valueReg[0] ) == 0 )
            {
                /*free( valueA );
                free( valueReg );*/
                free( emptyReg );
                return reg;
            }
            
            free( reg );
            //free( valueReg );
        }
        
        if( emptyReg ) 
        {
            //free( valueA );   
            return emptyReg;  
        }
    } 
    
    //Spill
    char ** valueReg;
    int sizeReg = ASM_GetVarValues( asm, "$R2", &valueReg );
    char * key = malloc( 5 );
    sprintf( key, "$R2" );
    char * reg = keyToRegister( key, 0 );
    
    if( sizeReg )
    {
        printf( "\tmovl %s %s\n", reg, valueReg[0] );
        ASM_UpdateStore( asm, valueReg[0] );
        
        int i;
        for( i = 0; i < sizeReg; i++ )
            free( valueReg[i] );
            
        free( valueReg );
    }
    
    printf( "\tmovl %s %s\n", a.str, reg );
    ASM_UpdateLoad( asm, reg, a.str );    
    
    return key;
}

static int uniqueLabel = 0;

static char * generateLabel()
{
    char * str = malloc( 25 );
    sprintf( str, ".J%d", uniqueLabel++ );
    
    return str;
}

/*
Generates assembly code of given basic block
*/
void ASM_GenerateCode( Assembler * asm, BasicBlock * bbl )
{
    Instr * curr = bbl->start;
    
    while( curr != bbl->end->next )
    {
        switch( curr->op )
        {
            case OP_LABEL:
            {
                printf( "%s:\n", curr->x.str );
                break;
            }
                
		    case OP_GOTO:
		    {
		        printf( "\tjmp %s\n", curr->x.str );
		        break;
		    }
		       
		    case OP_PARAM:
		    {	
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );	
		        char * rx = keyToRegister( x, 0 );
		        
		        printf( "\tpushl %s\n", x );
		        free( x );
		        free( rx );
		        break;
		    }
		    
		    case OP_RET_VAL:
		    {
			    char ** ret;
			    int sizeReg = ASM_GetVarValues( asm, "$R0", &ret );
		        char * eax = keyToRegister( "$R0", 0 );
			    
			    if( sizeReg )
			    {			        
			        printf( "\tmovl %s %s\n", eax, ret[0] );
			        ASM_UpdateStore( asm, ret[0] );			        
			    }
			    
			    char * x = ASM_FindRegisterForAddress( asm, curr->x );
			    char * rx = keyToRegister( x, 0 );
			    printf( "\tmovl %s %s\n", rx, eax );
			    
			    free( eax );
			    free( x );
			    free( rx );
			    
			    break;
		    }
		    
		    // instructions with x and y
		    case OP_IF:
		    {
		        char * cond = ASM_FindRegisterForAddress( asm, curr->x );
		        char * rcond = keyToRegister( cond, 0 );
		        	        
		        printf( "\tcmpl $1 %s\n", rcond );
		        printf( "\tjeq %s\n", curr->y.str );
		        free( cond );
		        free( rcond );		        
		        break;
		    }
		    
		    case OP_IF_FALSE:
		    {
		        char * cond = ASM_FindRegisterForAddress( asm, curr->x );;
		       	char * rcond = keyToRegister( cond, 0 );
		       		        
		        printf( "\tcmpl $1 %s\n", rcond );
		        printf( "\tjne %s\n", curr->y.str );
		        free( cond );
		        free( rcond );
		        break;
		    }
		    
		    case OP_SET:
		    {
	            char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
	            
		        if( curr->y.type == AD_NUMBER )
		        {
		            printf( "\tmovl $%s %s\n", curr->y.str, rx );
		        }
		        else
		        {		            
	                char * y = ASM_FindRegisterForAddress( asm, curr->y );
	                char * ry = keyToRegister( y, 0 );
	                
	                ASM_UpdateLoad( asm, y, curr->y.str );
	                printf( "\tmovl %s %s\n", curr->y.str, ry );
		            printf( "\tmovl %s %s\n", ry, rx );
		            free( y );
		            free( ry );
		        }		        
		        
		        free( x );
		        free( rx );
		        break;
		    }
		    
		    case OP_SET_BYTE:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
	            
		        if( curr->y.type == AD_NUMBER )
		        {
		            printf( "\tmovsbl $%s %s\n", curr->y.str, rx );
		        }
		        else
		        {		            
	                char * y = ASM_FindRegisterForAddress( asm, curr->y );
	                char * ry = keyToRegister( y, 1 );
	                
	                ASM_UpdateLoad( asm, y, curr->y.str );
	                printf( "\tmovsbl %s %s\n", curr->y.str, ry );
		            printf( "\tmovsbl %s %s\n", ry, rx );
		            free( y );
		            free( ry );
		        }		        
		        
		        free( x );
		        free( rx );
		        break;
		    }
		    
		    case OP_NEG:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        printf( "\tnegl %s\n", ry );
		        printf( "\tmovl %s %s\n", ry, rx );
		        free( x );
		        free( rx );
		        free( y );
		        free( ry );
		    }
		    
		    case OP_NEW:
		    case OP_NEW_BYTE:
		    case OP_CALL:
		    {
			    
			    break;
		    }
		    
		    // instruction with x, y and z
		    case OP_SET_IDX:
		    {
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
		        printf( "\tmovl %s %%esi\n", rz );
		        printf( "\timul $4 %%esi\n" );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        printf( "\taddl %s %%esi\n", ry );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        printf( "\tmovl (%%esi) %s\n", rx );
		        free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    case OP_SET_IDX_BYTE:
		    {
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
		        printf( "\tmovl %s %%esi\n", rz );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        printf( "\taddl %s %%esi\n", ry );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        printf( "\tmovsbl %s (%%esi)\n", rx );
		        free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    case OP_IDX_SET:
		    {
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        printf( "\tmovl %s %%esi\n", ry );
		        printf( "\timul $4 %%esi\n" );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        printf( "\taddl %s %%esi\n", rx );
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
		        printf( "\tmovl %s (%%esi)\n", rz );
		        free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    case OP_IDX_SET_BYTE:
		    {
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        printf( "\tmovl %s %%esi\n", ry );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        printf( "\taddl %s %%esi\n", rx );
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 1 );
		        printf( "\tmovb %s (%%esi)\n", rz );
		        free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    case OP_NE:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
	            char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
	            printf( "\tcmp %s %s\n", ry, rz );
	            char * label = generateLabel();
	            char * endLabel = generateLabel();	            
	            printf( "\tjne %s\n", label );
	            printf( "\tmovl $0 %s\n", rx );
	            printf( "\tjmp %s\n", endLabel );
	            printf( "%s:\n", label );
	            printf( "\tmovl $1 %s\n", rx );	
	            printf( "%s:\n", endLabel );
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        free( label );
		        free( endLabel );
		        break;            
            }
            
		    case OP_EQ:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
	            char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
	            printf( "\tcmp %s %s\n", ry, rz );
	            char * label = generateLabel();
	            char * endLabel = generateLabel();	            
	            printf( "\tje %s\n", label );
	            printf( "\tmovl $0 %s\n", rx );
	            printf( "\tjmp %s\n", endLabel );
	            printf( "%s:\n", label );
	            printf( "\tmovl $1 %s\n", rx );	
	            printf( "%s:\n", endLabel );
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        free( label );
		        free( endLabel );
		        break;            
            }
            
		    case OP_LT:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
	            char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
	            printf( "\tcmp %s %s\n", ry, rz );
	            char * label = generateLabel();
	            char * endLabel = generateLabel();	            
	            printf( "\tjl %s\n", label );
	            printf( "\tmovl $0 %s\n", rx );
	            printf( "\tjmp %s\n", endLabel );
	            printf( "%s:\n", label );
	            printf( "\tmovl $1 %s\n", rx );	
	            printf( "%s:\n", endLabel );
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        free( label );
		        free( endLabel );
		        break;            
            }
            
		    case OP_GT:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
	            char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
	            printf( "\tcmp %s %s\n", ry, rz );
	            char * label = generateLabel();
	            char * endLabel = generateLabel();	            
	            printf( "\tjg %s\n", label );
	            printf( "\tmovl $0 %s\n", rx );
	            printf( "\tjmp %s\n", endLabel );
	            printf( "%s:\n", label );
	            printf( "\tmovl $1 %s\n", rx );	
	            printf( "%s:\n", endLabel );
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        free( label );
		        free( endLabel );
		        break;            
            }
            
		    case OP_LE:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
	            char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
	            printf( "\tcmp %s %s\n", ry, rz );
	            char * label = generateLabel();
	            char * endLabel = generateLabel();	            
	            printf( "\tjle %s\n", label );
	            printf( "\tmovl $0 %s\n", rx );
	            printf( "\tjmp %s\n", endLabel );
	            printf( "%s:\n", label );
	            printf( "\tmovl $1 %s\n", rx );	
	            printf( "%s:\n", endLabel );
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        free( label );
		        free( endLabel );
		        break;            
            }
            
		    case OP_GE:
		    {
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
	            char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
	            printf( "\tcmp %s %s\n", ry, rz );
	            char * label = generateLabel();
	            char * endLabel = generateLabel();	            
	            printf( "\tjge %s\n", label );
	            printf( "\tmovl $0 %s\n", rx );
	            printf( "\tjmp %s\n", endLabel );
	            printf( "%s:\n", label );
	            printf( "\tmovl $1 %s\n", rx );	
	            printf( "%s:\n", endLabel );
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        free( label );
		        free( endLabel );
		        break;            
            }
		    
		    case OP_ADD:
		    {
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
		        printf( "\taddl %s %s\n", rz, ry );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
	            printf( "\tmovl %s %s\n", ry, rx );
	            ASM_UpdateOperation( asm, x, curr->x.str );	            
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    case OP_SUB:
		    {
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
		        printf( "\tsubl %s %s\n", rz, ry );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
	            printf( "\tmovl %s %s\n", ry, rx );
	            ASM_UpdateOperation( asm, x, curr->x.str );	            
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    case OP_DIV:
		    {
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
		        printf( "\tidiv %s %s\n", rz, ry );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
	            printf( "\tmovl %s %s\n", ry, rx );
	            ASM_UpdateOperation( asm, x, curr->x.str );
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    case OP_MUL:
		    {
		        char * y = ASM_FindRegisterForAddress( asm, curr->y );
	            char * ry = keyToRegister( y, 0 );
		        char * z = ASM_FindRegisterForAddress( asm, curr->z );
	            char * rz = keyToRegister( z, 0 );
		        printf( "\timul %s %s\n", rz, ry );
		        char * x = ASM_FindRegisterForAddress( asm, curr->x );
	            char * rx = keyToRegister( x, 0 );
	            printf( "\tmovl %s %s\n", ry, rx );
	            ASM_UpdateOperation( asm, x, curr->x.str );	            
	            free( x );
		        free( rx );
		        free( y );
		        free( ry );
		        free( z );
		        free( rz );
		        break;
		    }
		    
		    // instruction with no args
		    case OP_RET:
		    {
		        printf( "\tret\n" );
			    break;
		    }
        }
        
        curr = curr->next;
    }
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
    int loop = 0;
    BasicBlock * bbl = BBL_New();
    
    printf( ".%s:\n", func->name );
    printf( "\tpushl %%ebp\n" );
    printf( "\tmovl %%esp, %%ebp\n" );
    
    do
    {        
        loop = ASM_NextBasicBlock( asm, func, bbl );
                
        ASM_SetupVarsLiveness( asm, bbl->start, bbl->end, 1 );        
        ASM_GenerateCode( asm, bbl );
        ASM_ClearHashes( asm ); 
        //BBL_Dump( bbl, func, blockNum++ );
    }
    while( loop );
    
    printf( "\tmovl %%ebp, %%esp\n" );    
    printf( "\tpopl %%ebp\n" );
    printf( "\tret\n" );    
}

/*
Builds assembly code
*/
void ASM_Build( Assembler * asm, IR * ir, char * filepath )
{
    FILE * fp = freopen( filepath, "w", stdout );
    
    printf( ".data\n" );
    
    String * str = ir->strings;
    while( str )
    {
        printf( "%s: .string %s\n", str->name, str->value );
        str = str->next;
    }
        
    printf( ".text\n" );
    
    Function * func = ir->functions;
    while( func )
    {
        printf( ".globl %s\n", func->name );
        func = func->next;
    }
    
    func = ir->functions;
    while( func )
    {
        //ASM_ClearHashes( asm );
        ASM_SetupHashes( asm, func );
        ASM_BuildBlocks( asm, func );
        func = func->next;
    }
    
    fclose( fp );
}
