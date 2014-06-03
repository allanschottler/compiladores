#include "ir.h"

typedef struct assembler Assembler;


Assembler * ASM_New();

void ASM_Delete( Assembler * asm );

void ASM_Build( Assembler * asm, IR * ir );
