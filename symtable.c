#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtable.h"
#include "uthash.h"

#define MAX_SCOPE_SIZE 4

#define ERROR_UNDECLARED    0
#define ERROR_REDEFINED     1
#define ERROR_UNKNOWN       2

static void errorSymbol( int error, char * name, int line )
{
    switch( error )
    {
        case ERROR_UNDECLARED:
            fprintf( stderr, "!Symbol Error [line %d]: Undeclared identifier \'%s\'.\n", line, name );
            break;
            
        case ERROR_REDEFINED:
            fprintf( stderr, "!Symbol Error [line %d]: Redefinition of identifier \'%s\'.\n", line, name );
            break;
            
        case ERROR_UNKNOWN:
            fprintf( stderr, "!Symbol Error: Internal error.\n" );
            break;
            
        default:
            break;
    }
    
    free( name );
    
    exit( EXIT_FAILURE );
}

static void errorTyping( int assert, const char * error, int line )
{
    if( assert )
        return;
        
    fprintf( stderr, "!Typing Error [line %d]: %s\n", line, error );
}

typedef struct hash Hash;

struct hash
{
	char * id;
	int type;
	int ptrType;
	UT_hash_handle hh;
};

typedef struct scope Scope;

struct scope
{
	Scope * parent;
	Scope ** scopes;	
	int nScopes;
	int maxScopes;
	Hash * symbols;	
};

Scope * SCO_New()
{
	Scope * scope = ( Scope* )malloc( sizeof( Scope ) );
	scope->parent = NULL;
	scope->scopes = ( Scope** )malloc( MAX_SCOPE_SIZE * sizeof( Scope* ) );
	scope->nScopes = 0;
	scope->maxScopes = MAX_SCOPE_SIZE;
	scope->symbols = NULL;
	
	int i;
	
	for( i = 0; i < MAX_SCOPE_SIZE; i++ )
		scope->scopes[i] = NULL;
	
	return scope;
}

void SCO_Delete( Scope * scope )
{
	if( !scope )
		return;
		
	int i;
	Hash *h, *tmp;
	
	for( i = 0; i < scope->nScopes; i++ )
	{
		SCO_Delete( scope->scopes[i] );		
	}

	free( scope->scopes );
	
	HASH_ITER( hh, scope->symbols, h, tmp ) 
	{
        HASH_DEL( scope->symbols, h );
        free( h );
    }    
	
	free( scope );
}

void SCO_AppendScope( Scope * parent, Scope * child )
{
	if( !parent )
	{
	    free( child );
		return;
	}
		
	if( !child )
		return;
		
	child->parent = parent;
	
	if( parent->nScopes == parent->maxScopes )
	{
		parent->maxScopes *= 2;
		parent->scopes = realloc( parent->scopes, parent->maxScopes * sizeof( Scope* ) );
	}
	
	parent->scopes[parent->nScopes++] = child;
}

void SYT_ProcessNode( SymTable * syt, Ast * ast );
void SYT_VisitExpression( SymTable * syt, Ast * ast );

struct symtable
{
	Ast * ast;
	Scope * root;
	Scope * current;
};


SymTable * SYT_New()
{
	SymTable * syt = ( SymTable* )malloc( sizeof( SymTable ) );
	syt->ast = NULL;
	syt->root = SCO_New();
	syt->current = syt->root;
	
	return syt;
}

void SYT_Delete( SymTable * syt )
{
	SCO_Delete( syt->root );
	free( syt );
}

void SYT_OpenScope( SymTable * syt )
{
	Scope * newScope = SCO_New();

	SCO_AppendScope( syt->current, newScope );
	syt->current = newScope;
}

void SYT_CloseScope( SymTable * syt )
{
	if( !syt->current )
		return;
		
	syt->current = syt->current->parent;
}

int SYT_CheckSymbol( SymTable * syt, const char * idName, int * outPtr )
{
	Hash * h;
	Scope * current = syt->current;
    
    do
    {
        HASH_FIND_STR( current->symbols, idName, h );
        
        if( h )
        {
            *outPtr = h->ptrType;
            return h->type; 
        }           
        
        current = current->parent;
    }
    while( current );
    
    return 0;
}
 
int SYT_AddSymbol( SymTable * syt, char * idName, int type, int ptrType )
{
    if( !idName )
        return 0;
    
    int garbage;
    int check = SYT_CheckSymbol( syt, idName, &garbage );
    
    if( !check ) 
    {
	    Hash * h = ( Hash* )malloc( sizeof( Hash ) );
	    h->id = idName;
	    h->type = type;
	    h->ptrType = ptrType;
	
	    HASH_ADD_STR( syt->current->symbols, id, h ); //This works cuz macros
	    return 1;
	}
	
	return 0;
}

void SYT_VisitDeclaration( SymTable * syt, Ast * ast )
{
    char * name = AST_FindId( ast );
    int ptrType;	                
    int type = SYT_StringToType( AST_FindType( ast, &ptrType ) );

    if( !SYT_AddSymbol( syt, name, type, ptrType ) )	                
        errorSymbol( ERROR_REDEFINED, name, AST_GetNodeLine( ast ) );
}

void SYT_VisitID( SymTable * syt, Ast * ast )
{
    char * name = AST_GetNodeValue( ast );
    int ptrType;
    int type = SYT_CheckSymbol( syt, name, &ptrType );
    
    if( !type )	                
        errorSymbol( ERROR_UNDECLARED, name, AST_GetNodeLine( ast ) );
        
    AST_Annotate( ast, type, ptrType );
}

void SYT_VisitAssign( SymTable * syt, Ast * ast )
{
    Ast * var = AST_GetChild( ast );
    char * name = AST_FindId( var );
    int ptrType;
    int type = SYT_CheckSymbol( syt, name, &ptrType );
    
    free( var );
    
    if( !type )	                
        errorSymbol( ERROR_UNDECLARED, name, AST_GetNodeLine( ast ) );
}

void SYT_VisitCall( SymTable * syt, Ast * ast )
{
    char * name = AST_FindId( ast );
    int ptrType;
    int type = SYT_CheckSymbol( syt, name, &ptrType );
    
    if( !type )	                
        errorSymbol( ERROR_UNDECLARED, name, AST_GetNodeLine( ast ) );
}

void SYT_VisitFunction( SymTable * syt, Ast * ast )
{
    SYT_OpenScope( syt );
    
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        switch( AST_GetNodeType( ast ) )
        {
            case A_DECLVAR:
                SYT_VisitDeclaration( syt, child );
                break;
                
            case A_BLOCK:
                {
                    Ast * child2;
                    
                    for( child2 = AST_GetChild( ast ); child2; child2 = AST_NextSibling( child2 ) )                    
                        SYT_ProcessNode( syt, child2 );                    
                }
                break;
                
            default:
                break;                
        }
    }
    
    SYT_CloseScope( syt );
}

void SYT_VisitLitString( SymTable * syt, Ast * ast )
{
    AST_Annotate( ast, S_STRING, 0 );
}

void SYT_VisitLitInt( SymTable * syt, Ast * ast )
{
    AST_Annotate( ast, S_INT, 0 );
}

void SYT_VisitLitBool( SymTable * syt, Ast * ast )
{
    AST_Annotate( ast, S_BOOL, 0 );
}

void SYT_VisitVar( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    int ptrType;
    int type = AST_GetNodeAnnotation( ast, &ptrType );    
    int count = 0;    
    
    child = AST_NextSibling( child );
    
    while( child )
    {        
        count++;     
        
        int expPtrType;        
        int expType = AST_GetNodeAnnotation( child, &expPtrType );
        
        errorTyping( expType == S_INT, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
        errorTyping( expPtrType == 0, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
        
        child = AST_NextSibling( child );
    }   
    
    errorTyping( count <= ptrType, "Expression does not match expected pointer dimension.", AST_GetNodeLine( ast ) );
    
    AST_Annotate( ast, type, ptrType - count );
}

void SYT_VisitNew( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    int ptrType;
    int type = AST_GetNodeAnnotation( child, &ptrType );
    
    errorTyping( type == S_INT, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
    errorTyping( ptrType == 0, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
        
    child = AST_NextSibling( child );    
    type = AST_GetNodeAnnotation( child, &ptrType );
    
    AST_Annotate( ast, type, ptrType + 1 );
}

void SYT_VisitNot( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    int ptrType;
    int type = AST_GetNodeAnnotation( child, &ptrType );
    
    free( child );
    
    errorTyping( type == S_BOOL, "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) );
    errorTyping( ptrType == 0, "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) );
    
    AST_Annotate( ast, S_BOOL, 0 );
}

void SYT_VisitNegative( SymTable * syt, Ast * ast )
{
    Ast * child = AST_GetChild( ast );
    int ptrType;
    int type = AST_GetNodeAnnotation( child, &ptrType );
    
    free( child );
    
    errorTyping( type == S_INT, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
    errorTyping( ptrType == 0, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
    
    AST_Annotate( ast, S_INT, 0 );
}

void SYT_VisitArithmitic( SymTable * syt, Ast * ast )
{
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        int ptrType;
        int type = AST_GetNodeAnnotation( child, &ptrType );
        
        errorTyping( ( type == S_INT || type == S_CHAR ), "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
        errorTyping( ptrType == 0, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
        
    }
    
    AST_Annotate( ast, S_INT, 0 );
}

void SYT_VisitLogic( SymTable * syt, Ast * ast )
{
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        int ptrType;
        int type = AST_GetNodeAnnotation( child, &ptrType );
        
        errorTyping( type == S_BOOL, "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) );
        errorTyping( ptrType == 0, "Expression does not evaluate to type \'bool\'.", AST_GetNodeLine( child ) );
        
    }
    
    AST_Annotate( ast, S_BOOL, 0 );
}

void SYT_VisitComparison( SymTable * syt, Ast * ast )
{
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
    {
        int ptrType;
        int type = AST_GetNodeAnnotation( child, &ptrType );
        
        errorTyping( type == S_INT, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
        errorTyping( ptrType == 0, "Expression does not evaluate to type \'int\'.", AST_GetNodeLine( child ) );
        
    }
    
    AST_Annotate( ast, S_BOOL, 0 );
}

void SYT_VisitExpression( SymTable * syt, Ast * ast )
{
    Ast * child;
    
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
        SYT_VisitExpression( syt, child );    
    
    int nodeType = AST_GetNodeType( ast );
    
    switch( nodeType )
    {
        case A_ID:
            SYT_VisitID( syt, ast );
            break;
        
        case A_LITSTRING:
            SYT_VisitLitString( syt, ast );            
            break;
        
        case A_LITINT:
            SYT_VisitLitInt( syt, ast );            
            break;
            
        case A_TRUE:
        case A_FALSE:
            SYT_VisitLitBool( syt, ast ); 
            break;
        
        case A_VAR:
            SYT_VisitVar( syt, ast );
            break;
        
        case A_NEW:
            SYT_VisitNew( syt, ast );
            break;
                
        case A_NOT:
            SYT_VisitNot( syt, ast );
            break;
        
        case A_NEGATIVE:
            SYT_VisitNegative( syt, ast );
            break;
                         
        case A_ADD:
        case A_SUBTRACT:
        case A_MULTIPLY:
        case A_DIVIDE:
            SYT_VisitArithmitic( syt, ast );
            break;
            
        case A_AND:
        case A_OR:
            SYT_VisitLogic( syt, ast );
            break;
            
        case A_EQ:
        case A_NEQ:
        case A_LARGER:
        case A_SMALLER:
        case A_LARGEREQ:
        case A_SMALLEREQ:
            SYT_VisitComparison( syt, ast );
            break;            
    }
}
void SYT_ProcessNode( SymTable * syt, Ast * ast )
{
    switch( AST_GetNodeType( ast ) )
    {
        case A_BLOCK:
            SYT_OpenScope( syt );
            break;
            
        case A_ID:
            SYT_VisitID( syt, ast );
            break;
            
        case A_FUNCTION:
            SYT_VisitFunction( syt, ast );
            break;
            
        case A_DECLVAR:
            SYT_VisitDeclaration( syt, ast );
            break;
            
        case A_ASSIGN:
            SYT_VisitAssign( syt, ast );
            break;
            
        case A_CALL:
            SYT_VisitCall( syt, ast );
            break;
            
        case A_IF:
            SYT_VisitIf( syt, ast );
            break;
            
        case A_WHILE:
            SYT_VisitWhile( syt, ast );
            break;
            
        default:
            errorSymbol( ERROR_UNKNOWN, NULL, 0 );
            break;            
    }
    
    Ast * child;
	
	for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
	    // Is this correct?
	    int garbage;
	    
	    if( AST_GetNodeAnnotation( child, garbage ) )	
	        SYT_ProcessNode( syt, child );
	}	
	
	if( AST_GetNodeType( ast ) == A_BLOCK )
	    SYT_CloseScope( syt );
}

int SYT_StringToType( char * str )
{
    if( !str )
        return S_NONE;
        
    if( strcmp( str, "char" ) == 0 )
        return S_CHAR;
    else if( strcmp( str, "int" ) == 0 )
        return S_INT;
    else if( strcmp( str, "bool" ) == 0 )
        return S_BOOL;
    else if( strcmp( str, "string" ) == 0 )
        return S_STRING;
        
    return S_NONE;
}

void SYT_Build( SymTable * syt, Ast * ast )
{
	syt->ast = ast;	
	
	Ast * child;
	
	for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
        if( AST_GetNodeType( child ) != A_FUNCTION && AST_GetNodeType( child ) != A_DECLVAR )
            errorSymbol( ERROR_UNKNOWN, NULL, 0 );	        

        char * name = AST_FindId( child );
        int ptrType;	                
        int type = SYT_StringToType( AST_FindType( child, &ptrType ) );

        if( !SYT_AddSymbol( syt, name, type, ptrType ) )	                
            errorSymbol( ERROR_REDEFINED, name, AST_GetNodeLine( child ) );
	}
		
    for( child = AST_GetChild( ast ); child; child = AST_NextSibling( child ) )
	{
        if( AST_GetNodeType( child ) != A_DECLVAR )
        {
            SYT_ProcessNode( syt, child );
        }
    }
}

