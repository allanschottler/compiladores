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
#define O_AND   13
#define O_OR    14
#define O_NOT   15  

#define O_ASGN  16
#define O_CALL  17
#define O_GOTO  18
#define O_LABL  19
#define O_PARM  20
#define O_ARG   21
#define O_RET   22
#define O_NEW   23

// Intermediate Code Representation
typedef struct icr Icr;


Icr * ICR_New();

void ICR_Delete( Icr * icr );

void ICR_Build( Icr * icr, Ast * ast );

void ICR_Dump( Icr * icr );

#endif
