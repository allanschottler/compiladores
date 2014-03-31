#ifndef TOKEN_H
#define TOKEN_H

// Palavras reservadas
#define T_IF 		0
#define T_ELSE 		1
#define T_END 		2 
#define T_WHILE 	3
#define T_LOOP 		4
#define T_FUN 		5
#define T_RETURN 	6
#define T_NEW 		7
#define T_STRING 	8
#define T_INT 		9
#define T_CHAR 		10
#define T_BOOL 		11
#define T_TRUE 		12
#define T_FALSE 	13
#define T_AND 		14
#define T_OR 		15
#define T_NOT 		16

// Operadores e pontuação
#define T_OCBRACKET	17
#define T_CCBRACKET	18
#define T_COMMA		19
#define T_COLON		20
#define T_LARGER	21
#define T_SMALLER	22
#define T_LARGEREQ	23
#define T_SMALLEREQ	24
#define T_EQ		25
#define T_NEQ    	26
#define T_OSBRACKET	27
#define T_CSBRACKET	28
#define T_PLUS		29
#define T_MINUS		30
#define T_ASTERISK	31
#define T_SLASH		32

// Valores
#define T_LITSTRING	33
#define T_LITINT	34

// Misc
#define T_COMMENT   777
#define T_ID        999
#define T_ERROR     666
#define T_NL        333


typedef struct token Token;

/***** CALLBACKS *****/

void TOK_Delete( void * tok );

int TOK_IsType( void * tok, int type );

void TOK_MatchError( void * received, int expected );

int TOK_GetType( void * tok );

/*********************/

Token * TOK_New( char * text, int type, int line );

char * TOK_GetText( Token * tok );

int TOK_GetLine( Token * tok );

void TOK_Dump( void * tok );

#endif
