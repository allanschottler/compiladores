
#include <stdio.h>
#include <stdlib.h>

#include "ir.h"
#include "assembler.h"

extern FILE* yyin;
extern int yyparse();
extern int yydebug;

extern IR* ir;

int main(int argc, char** argv) {
	int err;
	//yydebug = 1;
	if (argc < 2) {
		fprintf(stderr, "Uso: %s arquivo.m0.ir\n", argv[0]);
		exit(1);
	}
	yyin = fopen(argv[1], "r");
	err = yyparse();
	fclose(yyin);
	if (err != 0) {
		fprintf(stderr, "Error reading input file.\n");
		exit(1);
	}
	
	char filepath[100];
	sprintf( filepath, "%s.s", argv[1] );
	
	Assembler * asm = ASM_New();
	ASM_Build( asm, ir, filepath );
	
	return 0;
}

