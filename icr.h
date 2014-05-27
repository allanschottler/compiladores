#ifndef ICR_H
#define ICR_H

#include "ast.h"

// Operations
#define O_IFT   1
#define O_IFF   2
#define O_ADD   3
#define O_SUB   4
#define O_MUL   5
#define O_DIV   6
#define O_EQ    7
#define O_NEQ   8
#define O_LRGR  9
#define O_SMLR  10
#define O_LRGRE 11
#define O_SMLRE 12
//#define O_AND   13
//#define O_OR    14


#define O_ASGN  13
#define O_CALL  14
#define O_GOTO  15
#define O_LABL  16
#define O_PARM  17
//#define O_ARG   21
#define O_RET   18
#define O_NEW   19
#define O_FUN   20
//#define O_ARRAY 25

// Intermediate Code Representation
typedef struct icr Icr;


Icr * ICR_New();

void ICR_Delete( Icr * icr );

void ICR_Build( Icr * icr, Ast * ast );

void ICR_Dump( Icr * icr );

void ICR_WriteToFile( Icr * icr, char * path );

#endif
