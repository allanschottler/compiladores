#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "icr.h"
#include "list.h"

typedef struct entry Entry;

struct entry
{
    int operation;
    char * value1;
    char * value2;    
    char * result;
    char * label;
};

Entry * ETR_New( int op, char * v1, char * v2, char * result )//, char * label )
{
    Entry * entry = ( Entry* )malloc( sizeof( Entry ) );
    
    entry->operation = op;
    entry->value1 = v1 ? strdup( v1 ) : v1;
    entry->value2 = v2 ? strdup( v2 ) : v2;    
    entry->result = result ? strdup( result ) : result; 
    //entry->label = strdup( label );       
    
    return entry;     
}

void ETR_Delete( void * entry )
{
    Entry * e = ( Entry* )entry;
    
    free( e->value1 );
    free( e->value2 );
    free( e->result );  
    //free( e->label );
    free( e );          
}

void ETR_Dump( void * entry )
{
    Entry * e = ( Entry* )entry;
    
    if( e->operation == O_LABL )
        printf( "%s:\n", e->value1 );
    else    
        printf( "\t" );
    
    if( e->operation == O_IFF )
    {
        printf( "ifFalse %s goto %s\n", e->value1, e->result );
    }
    else if( e->operation == O_IFT )
    {
        printf( "ifTrue %s goto %s\n", e->value1, e->result );    
    }
    else if( e->operation == O_ASGN )
    {
        printf( "%s = %s\n", e->result, e->value1 );
    }
    else if( e->operation == O_CALL )
    {
        printf( "call %s\n", e->value1 );
    }
    else if( e->operation == O_GOTO )
    {
        printf( "goto %s\n", e->value1 );
    }
    else if( e->operation == O_PARM )
    {
        printf( "param %s\n", e->value1 );
    }
    else if( e->operation == O_ARG )
    {
        printf( "arg %s\n", e->value1 );
    }
    else if( e->operation == O_RET )
    {
        printf( "ret %s\n", e->value1 );
    }
    else if( e->operation == O_NEW )
    {
        printf( "%s = %s times %s\n", e->result, e->value2, e->value1 );
    }
    else if( e->operation == O_NOT )
    {
        printf( "not %s\n", e->value1 );
    }    
    else if( e->operation != O_LABL )
    {
        char * str = malloc( 4 );
        
        switch( e->operation )
	    {
	        case O_ADD:
	            sprintf( str, "+" );
	            break;
            case O_SUB:
                sprintf( str, "-" );
	            break;                
            case O_DIV:
                sprintf( str, "/" );
	            break;                
            case O_MUL:
                sprintf( str, "*" );
	            break;                
            case O_EQ:
                sprintf( str, "=" );
	            break;                
            case O_NEQ:
                sprintf( str, "<>" );
	            break;                
            case O_LRGR:
                sprintf( str, ">" );
	            break;                
            case O_SMLR:
                sprintf( str, "<" );
	            break;                
            case O_LRGRE:
                sprintf( str, ">=" );
	            break;                
            case O_SMLRE:
                sprintf( str, "<=" );
	            break;                
            case O_AND:
                sprintf( str, "and" );
	            break;                
            case O_OR:
                sprintf( str, "or" );
	            break;               
            default:
                printf( "wut: %d\n", e->operation );                
                assert( 0 );
        }
        printf( "%s = %s %s %s\n", e->result, e->value1, str, e->value2 );
    }
}

struct icr
{
    List * entries;   
};

/*************************************************************/

void ICR_GenerateBlock( Icr * icr, Ast * ast );
void ICR_GenerateCall( Icr * icr, Ast * ast );

static int tempUniqueId = 0;
static int labelUniqueId = 0;

static char * generateTemp()
{
    char * str = malloc( 10 );    
    sprintf( str, "$t%d", tempUniqueId++ );
    return str;
}

static char * generateLabel()
{
    char * str = malloc( 10 );    
    sprintf( str, "L%d", labelUniqueId++ );
    return str;
}

int ICR_AstTypeToIcr( int type )
{
    switch( type )
	{
	    case A_ADD:
	        return O_ADD;
        case A_SUBTRACT:
            return O_SUB;
        case A_DIVIDE:
            return O_DIV;
        case A_MULTIPLY:
            return O_MUL;
        case A_EQ:
            return O_EQ;
        case A_NEQ:
            return O_NEQ;
        case A_LARGER:
            return O_LRGR;
        case A_SMALLER:
            return O_SMLR;
        case A_LARGEREQ:
            return O_LRGRE;
        case A_SMALLEREQ:
            return O_SMLRE;
        case A_AND:
            return O_AND;
        case A_OR:
            return O_OR;
        default:
            assert( 0 );
            return -1;
    }
}

char * ICR_GenerateExpression( Icr * icr, Ast * ast )
{
    Ast * child;
    int type = AST_GetNodeType( ast );
    
    switch( type )
	{
	    case A_ADD:
        case A_SUBTRACT:
        case A_DIVIDE:
        case A_MULTIPLY:
        case A_EQ:
        case A_NEQ:
        case A_LARGER:
        case A_SMALLER:
        case A_LARGEREQ:
        case A_SMALLEREQ:
        case A_AND:
        case A_OR:
        {
            char * temp = generateTemp();
            
            child = AST_GetChild( ast );
            char * e1 = ICR_GenerateExpression( icr, child );
            
            child = AST_NextSibling( child );
            char * e2 = ICR_GenerateExpression( icr, child );
            
            int op = ICR_AstTypeToIcr( type );
            LIS_PushBack( icr->entries, ETR_New( op, e1, e2, temp ) );
            
            free( child );
            return temp;            
        }
            break;
        
        case A_NOT:
        {
            char * temp = generateTemp();
            
            child = AST_GetChild( ast );
            char * e = ICR_GenerateExpression( icr, child );
            LIS_PushBack( icr->entries, ETR_New( O_NOT, e, NULL, temp ) );
            
            free( child );
            return temp;            
        }
            break;
            
        case A_NEGATIVE:
        {
            char * temp = generateTemp();
            
            child = AST_GetChild( ast );
            char * e = ICR_GenerateExpression( icr, child );            
            LIS_PushBack( icr->entries, ETR_New( O_SUB, "0", e, temp ) );
            
            free( child );
            return temp;
        }
            break;
            
        case A_NEW:
        {
            Ast * child = AST_GetChild( ast );            
            char * temp = generateTemp();
            char * e = ICR_GenerateExpression( icr, child );
            free( child );
            LIS_PushBack( icr->entries, ETR_New( O_NEW, "0", e, temp ) );
            
            return temp;
        }
            break;
        
	    case A_VAR:
	    {
	        char * id;
	        
	        Ast * child;
            for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	        {
	            int type = AST_GetNodeType( child );
	            if( type == A_ID )
	            {
	                id = AST_GetNodeValue( child );
	            }
	            else
	            {
	                char * exp = ICR_GenerateExpression( icr, child );
	                char * temp = generateTemp();
	                
	                LIS_PushBack( icr->entries, ETR_New( O_ADD, id, exp, temp ) );
	                id = temp;
	            }
	        }
	        
	        return id;
	    }
	        break;
	        
	    case A_CALL:
	    {
	        char * id = AST_FindId( ast );
	        ICR_GenerateCall( icr, ast );
	        char * temp = generateTemp();
	        LIS_PushBack( icr->entries, ETR_New( O_ASGN, id, NULL, temp ) );
	        
	        return temp;
	    }
	        break;
	            
        case A_LITINT:
        case A_LITSTRING:        
        {
            char * e = malloc( 20 );
            e = strdup( AST_GetNodeValue( ast ) );
            
            return e;
        }
            break;
          
        case A_TRUE:
        {
            char * e = malloc( 6 );            
            sprintf( e, "true" );
            
            return e;
        }
            break;
            
        case A_FALSE:
        {
            char * e = malloc( 6 );            
            sprintf( e, "false" );
            
            return e;
        }
            break;
            
            break;    
        default:
            assert( 0 );
	}
	
	return NULL;
}

void ICR_GenerateParams( Icr * icr, Ast * ast )
{
    Ast * child;
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    char * exp = ICR_GenerateExpression( icr, child );
	    LIS_PushBack( icr->entries, ETR_New( O_PARM, exp, NULL, NULL ) );
	}
}

void ICR_GenerateCall( Icr * icr, Ast * ast )
{    
    char * id = AST_FindId( ast );
    Ast * child = AST_GetChild( ast );
    child = AST_NextSibling( child );
    ICR_GenerateParams( icr, child );
    free( child );
    LIS_PushBack( icr->entries, ETR_New( O_CALL, id, NULL, NULL ) );
}

void ICR_GenerateReturn( Icr * icr, Ast * ast )
{    
    Ast * child = AST_GetChild( ast );
    
    if( child )
    {
        char * exp = ICR_GenerateExpression( icr, child );
        LIS_PushBack( icr->entries, ETR_New( O_RET, exp, NULL, NULL ) );
    }
    else
    {
        LIS_PushBack( icr->entries, ETR_New( O_RET, NULL, NULL, NULL ) );
    }
    
    free( child );
}

void ICR_GenerateAssign( Icr * icr, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    char * id = AST_FindId( child );
    
    child = AST_NextSibling( child );
    char * exp = ICR_GenerateExpression( icr, child );
    
    LIS_PushBack( icr->entries, ETR_New( O_ASGN, exp, NULL, id ) );
    free( child );    
}

void ICR_GenerateWhile( Icr * icr, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    char * exp = ICR_GenerateExpression( icr, child );
    char * startLabel = generateLabel();
    char * endLabel = generateLabel();
    
    LIS_PushBack( icr->entries, ETR_New( O_LABL, startLabel, NULL, NULL ) );    
    LIS_PushBack( icr->entries, ETR_New( O_IFF, exp, NULL, endLabel ) );
    
    child = AST_NextSibling( child );
    ICR_GenerateBlock( icr, child );
    LIS_PushBack( icr->entries, ETR_New( O_GOTO, startLabel, NULL, NULL ) );
    LIS_PushBack( icr->entries, ETR_New( O_LABL, endLabel, NULL, NULL ) );
    
    free( child );
}

void ICR_GenerateIf( Icr * icr, Ast * ast )
{
    char * endLabel = generateLabel();
    char * label = generateLabel();
    int firstRun = 1;
    int hasElse = 0;
    Ast * child;    
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    int type = AST_GetNodeType( child );
	    if( type != A_BLOCK ) //If expression
	    {
	        if( !firstRun )
	        {
	            LIS_PushBack( icr->entries, ETR_New( O_GOTO, endLabel, NULL, NULL ) );
	            LIS_PushBack( icr->entries, ETR_New( O_LABL, label, NULL, NULL ) );
	            label = generateLabel();
	        }
	        
	        char * exp = ICR_GenerateExpression( icr, child );    
	        LIS_PushBack( icr->entries, ETR_New( O_IFF, exp, NULL, label ) );
	        
	        child = AST_NextSibling( child );
	        ICR_GenerateBlock( icr, child );        
	    }
	    else
	    {
	        LIS_PushBack( icr->entries, ETR_New( O_GOTO, endLabel, NULL, NULL ) );
	        LIS_PushBack( icr->entries, ETR_New( O_LABL, label, NULL, NULL ) );
	        ICR_GenerateBlock( icr, child );
	        
	        hasElse = 1;
	    }	    
	    
	    firstRun = 0;
	}
	
	// Creates a second label. How to fix?
	if( !hasElse )
	    LIS_PushBack( icr->entries, ETR_New( O_LABL, label, NULL, NULL ) );
           
    LIS_PushBack( icr->entries, ETR_New( O_LABL, endLabel, NULL, NULL ) );
}

void ICR_GenerateDeclaration( Icr * icr, Ast * ast )
{
    char * id = AST_FindId( ast );
    LIS_PushBack( icr->entries, ETR_New( O_ASGN, "0", NULL, id ) );
}

void ICR_GenerateBlock( Icr * icr, Ast * ast )
{
    Ast * child;    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    int type = AST_GetNodeType( child );
	    switch( type )
	    {
	        case A_DECLVAR:
	            ICR_GenerateDeclaration( icr, child );
	            break;
	            
	        case A_IF:
	            ICR_GenerateIf( icr, child );
	            break;
	            
	        case A_WHILE:
	            ICR_GenerateWhile( icr, child );
	            break;
	            
	        case A_ASSIGN:
	            ICR_GenerateAssign( icr, child );
	            break;
	            
	        case A_CALL:
	            ICR_GenerateCall( icr, child );
	            break;
	            
	        case A_RETURN:
	            ICR_GenerateReturn( icr, child );
	            break;
	            
	        default:
	            printf( "block: %d\n", type );
	            assert( 0 );
	            break;
	    }
	}
}

void ICR_GenerateArgs( Icr * icr, Ast * ast )
{
    Ast * child;    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    char * id = AST_FindId( child );
	    LIS_PushBack( icr->entries, ETR_New( O_ARG, id, NULL, NULL ) );
	}
}

void ICR_GenerateFunction( Icr * icr, Ast * ast )
{
    char * id = AST_FindId( ast );
    LIS_PushBack( icr->entries, ETR_New( O_LABL, id, NULL, NULL ) );
    
    Ast * child;    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    int type = AST_GetNodeType( child );
	    
	    if( type == A_PARAMS )
	        ICR_GenerateArgs( icr, child );
	        	        
	    else if( type == A_BLOCK )
            ICR_GenerateBlock( icr, child ); 
    }    
   
    LIS_PushBack( icr->entries, ETR_New( O_RET, NULL, NULL, NULL ) );
}

void ICR_GenerateGlobals( Icr * icr, Ast * ast )
{
    Ast * child;    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    if( AST_GetNodeType( child ) == A_DECLVAR )
        {
            ICR_GenerateDeclaration( icr, child );
        }
	}    
}

/*************************************************************/

Icr * ICR_New()
{
    Icr * icr = ( Icr* )malloc( sizeof( Icr ) );
    
    icr->entries = LIS_New();
    
    return icr;
}

void ICR_Delete( Icr * icr )
{
    LIS_Delete( icr->entries, &ETR_Delete );
    free( icr );
}

void ICR_Build( Icr * icr, Ast * ast )
{
    ICR_GenerateGlobals( icr, ast );
    
    Ast * child;    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    if( AST_GetNodeType( child ) == A_FUNCTION )
        {
            ICR_GenerateFunction( icr, child );
        }
	}   
	
	printf( "Generated intermediate code!\n" ); 
}

void ICR_Dump( Icr * icr )
{
    LIS_Dump( icr->entries, &ETR_Dump );
}

void ICR_WriteToFile( Icr * icr, char * path )
{
    FILE * fp = freopen( path, "w", stdout );
    if( !fp )
    {
        exit( EXIT_FAILURE );
    }
    
    ICR_Dump( icr );
    fclose( stdout );
}
