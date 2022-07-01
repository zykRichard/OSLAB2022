	.file	"co.c"
	.text
	.globl	T_NUM
	.bss
	.align 4
	.type	T_NUM, @object
	.size	T_NUM, 4
T_NUM:
	.zero	4
	.section	.rodata
.LC0:
	.string	"That's OK!"
	.text
	.type	stack_switch_call, @function
stack_switch_call:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$40, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	leaq	.LC0(%rip), %rdi
	call	puts@PLT
	movq	-24(%rbp), %rcx
	movq	-32(%rbp), %rdx
	movq	-40(%rbp), %rax
	movq	%rcx, %rbx
#APP
# 13 "co.c" 1
	movq %rbx, %rsp; movq %rax, %rdi; jmp *%rdx
# 0 "" 2
#NO_APP
	nop
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	stack_switch_call, .-stack_switch_call
	.globl	current
	.bss
	.align 8
	.type	current, @object
	.size	current, 8
current:
	.zero	8
	.text
	.globl	co_start
	.type	co_start, @function
co_start:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movl	$4352, %edi
	call	malloc@PLT
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-8(%rbp), %rax
	movq	-32(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-8(%rbp), %rax
	movq	-40(%rbp), %rdx
	movq	%rdx, 24(%rax)
	movq	-8(%rbp), %rax
	movl	$1, 32(%rax)
	movq	-8(%rbp), %rax
	movq	$0, 40(%rax)
	movq	current(%rip), %rax
	testq	%rax, %rax
	jne	.L3
	movq	-8(%rbp), %rax
	movq	%rax, current(%rip)
	movq	current(%rip), %rax
	movq	current(%rip), %rdx
	movq	%rdx, 48(%rax)
	jmp	.L4
.L3:
	movq	current(%rip), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, 48(%rax)
	movq	current(%rip), %rax
	movq	48(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 48(%rax)
	movq	-8(%rbp), %rax
	movq	%rax, current(%rip)
.L4:
	movl	T_NUM(%rip), %eax
	addl	$1, %eax
	movl	%eax, T_NUM(%rip)
	movq	-8(%rbp), %rax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	co_start, .-co_start
	.globl	co_wait
	.type	co_wait, @function
co_wait:
.LFB8:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	jmp	.L7
.L8:
	movl	$0, %eax
	call	co_yield
.L7:
	movq	-8(%rbp), %rax
	movl	32(%rax), %eax
	cmpl	$4, %eax
	jne	.L8
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	co_wait, .-co_wait
	.globl	co_yield
	.type	co_yield, @function
co_yield:
.LFB9:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	current(%rip), %rax
	addq	$56, %rax
	movq	%rax, %rdi
	call	_setjmp@PLT
	endbr64
	movl	%eax, -12(%rbp)
	cmpl	$0, -12(%rbp)
	jne	.L14
	movq	current(%rip), %rax
	movq	%rax, -8(%rbp)
	movq	current(%rip), %rax
	movq	48(%rax), %rax
	movq	%rax, current(%rip)
	movq	current(%rip), %rax
	movq	24(%rax), %rax
	movq	%rax, %rsi
	movq	current(%rip), %rax
	movq	16(%rax), %rax
	movq	current(%rip), %rdx
	leaq	256(%rdx), %rcx
	movq	%rsi, %rdx
	movq	%rax, %rsi
	movq	%rcx, %rdi
	call	stack_switch_call
	movq	current(%rip), %rax
	movl	$4, 32(%rax)
	movq	-8(%rbp), %rax
	addq	$56, %rax
	movl	$1, %esi
	movq	%rax, %rdi
	call	longjmp@PLT
.L14:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	co_yield, .-co_yield
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
