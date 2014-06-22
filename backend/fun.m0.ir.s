.data
id: .string "bla"
zz: .string "blo"
.text
.globl f
.globl g
.globl h
.globl i
.globl j
.f:
	pushl %ebp
	movl %esp, %ebp
	movl x %ecx
	movl $0 %ecx
	ret
	movl %ebp, %esp
	popl %ebp
	ret
.g:
	pushl %ebp
	movl %esp, %ebp
	movl x %ecx
	movl $0 %ecx
	movl y %ecx
	movl $0 %ecx
.L1:
	movl $t9 %ecx
	movl x %ecx
	movl 10 %ecx
	cmp %ecx %ecx
	jl .J0
	movl $0 %ecx
	jmp .J1
.J0:
	movl $1 %ecx
.J1:
	movl $t9 %ecx
	cmpl $1 %ecx
	jne .L2
	movl 1 %ecx
	movl 1 %ecx
	addl %ecx %ecx
	movl $t4 %ecx
	movl %ecx %ecx
	movl x %ecx
	addl %ecx %eax
	movl $t3 %ecx
	movl %eax %ecx
	movl %ecx $t3
	movl x %ecx
	addl %ecx %eax
	movl %ecx $t3
	movl $t2 %ecx
	movl %eax %ecx
	movl %ecx $t2
	movl 3 %ecx
	addl %ecx %eax
	movl %ecx $t2
	movl $t1 %ecx
	movl %eax %ecx
	movl %ecx $t1
	movl 9 %ecx
	addl %ecx %eax
	movl %ecx $t1
	movl $t0 %ecx
	movl %eax %ecx
	movl %ecx $t0
	movl i %ecx
	movsbl $0 %ecx
	movl %ecx $t0
	movl y %ecx
	movl %ecx %esi
	movl %ecx $t0
	movl v %ecx
	addl %ecx %esi
	movl %ecx $t0
	movl i %ecx
	movsbl %ecx (%esi)
	movl %eax %esi
	movl %ecx $t0
	movl v %ecx
	addl %ecx %esi
	movb %al (%esi)
	movl $t0 %eax
	movl %eax %eax
	movl $9 %eax
	movl %ecx $t0
	movl g %ecx
	movl g %ecx
	movl %ecx %eax
	movl %ecx g
	movl y %ecx
	addl %eax %ecx
	movl %ecx %eax
	jmp .L1
.L2:
	movl z %ecx
	movl id %ecx
	movl id %ecx
	movl %ecx %ecx
	ret
	movl %ebp, %esp
	popl %ebp
	ret
.h:
	pushl %ebp
	movl %esp, %ebp
	movl x %ecx
	movl $0 %ecx
	movl y %ecx
	movl $0 %ecx
	movl 0 %ecx
	cmpl $1 %ecx
	jeq .L3
	movl i %ecx
	movl $0 %ecx
	movl y %ecx
	movl %ecx %esi
	imul $4 %esi
	movl v %ecx
	addl %ecx %esi
	movl i %ecx
	movl (%esi) %ecx
	movl x %ecx
	movl %ecx %esi
	imul $4 %esi
	movl v %ecx
	addl %ecx %esi
	movl %eax (%esi)
	movl x %ecx
	pushl $R2
.L3:
	ret
	movl %ebp, %esp
	popl %ebp
	ret
.i:
	pushl %ebp
	movl %esp, %ebp
	movl x %ecx
	movl $0 %ecx
	movl y %ecx
	movl $0 %ecx
	addl %eax %eax
	movl $t5 %ecx
	movl %eax %ecx
	movl $t5 %eax
	movl %eax %eax
	movl %eax $t5
	movl %ecx $t5
	movl 12 %ecx
	movl %ecx %eax
	movl %ebp, %esp
	popl %ebp
	ret
.j:
	pushl %ebp
	movl %esp, %ebp
	movl a %ecx
	movl $0 %ecx
	movl b %ecx
	movl $0 %ecx
	movl c %ecx
	movl $0 %ecx
	cmp %eax %eax
	je .J2
	movl $0 %eax
	jmp .J3
.J2:
	movl $1 %eax
.J3:
	cmp %eax %eax
	jne .J4
	movl $0 %eax
	jmp .J5
.J4:
	movl $1 %eax
.J5:
	cmp %eax %eax
	jg .J6
	movl $0 %eax
	jmp .J7
.J6:
	movl $1 %eax
.J7:
	cmp %eax %eax
	jl .J8
	movl $0 %eax
	jmp .J9
.J8:
	movl $1 %eax
.J9:
	cmp %eax %eax
	jge .J10
	movl $0 %eax
	jmp .J11
.J10:
	movl $1 %eax
.J11:
	cmp %eax %eax
	jle .J12
	movl $0 %eax
	jmp .J13
.J12:
	movl $1 %eax
.J13:
	addl %eax %eax
	movl %eax %eax
	subl %ebx %ebx
	movl a %ecx
	movl %ebx %ecx
	imul %ebx %ebx
	movl %ecx a
	movl a %ecx
	movl %ebx %ecx
	idiv %ebx %ebx
	movl %ecx a
	movl a %ecx
	movl %ebx %ecx
	movl %ecx a
	movl a %ecx
	negl %ebx
	movl %ebx %ecx
	ret
	movl %ebp, %esp
	popl %ebp
	ret
