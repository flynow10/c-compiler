	.attribute	4, 16
	.attribute	5, "rv32i2p1"
	.file	"fib.c"
	.text
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	addi	sp, sp, -48
	sw	ra, 44(sp)                      # 4-byte Folded Spill
	sw	s0, 40(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 48
	li	a2, 0
	sw	a2, -44(s0)                     # 4-byte Folded Spill
	sw	a2, -12(s0)
	sw	a0, -16(s0)
	sw	a1, -20(s0)
	lui	a0, %hi(.L.str)
	addi	a0, a0, %lo(.L.str)
	call	print_string
	lw	a0, -44(s0)                     # 4-byte Folded Reload
	sw	a0, -28(s0)
	li	a1, 1
	sw	a1, -32(s0)
	sw	a0, -36(s0)
	j	.LBB0_1
.LBB0_1:                                # %for.cond
                                        # =>This Inner Loop Header: Depth=1
	lw	a1, -36(s0)
	li	a0, 9
	blt	a0, a1, .LBB0_4
	j	.LBB0_2
.LBB0_2:                                # %for.body
                                        #   in Loop: Header=BB0_1 Depth=1
	lw	a0, -32(s0)
	sw	a0, -40(s0)
	lw	a0, -28(s0)
	lw	a1, -32(s0)
	add	a0, a0, a1
	sw	a0, -32(s0)
	lw	a0, -40(s0)
	sw	a0, -28(s0)
	lw	a0, -32(s0)
	call	print_int
	lui	a0, %hi(.L.str.1)
	addi	a0, a0, %lo(.L.str.1)
	call	print_string
	j	.LBB0_3
.LBB0_3:                                # %for.inc
                                        #   in Loop: Header=BB0_1 Depth=1
	lw	a0, -36(s0)
	addi	a0, a0, 1
	sw	a0, -36(s0)
	j	.LBB0_1
.LBB0_4:                                # %for.end
	lw	a0, -12(s0)
	lw	ra, 44(sp)                      # 4-byte Folded Reload
	lw	s0, 40(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 48
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.globl	print_string                    # -- Begin function print_string
	.p2align	2
	.type	print_string,@function
print_string:                           # @print_string
# %bb.0:                                # %entry
	addi	sp, sp, -16
	sw	ra, 12(sp)                      # 4-byte Folded Spill
	sw	s0, 8(sp)                       # 4-byte Folded Spill
	addi	s0, sp, 16
	sw	a0, -12(s0)
	lw	a1, -12(s0)
	#APP
	li	a7, 4
	mv	a0, a1
	ecall

	#NO_APP
	lw	ra, 12(sp)                      # 4-byte Folded Reload
	lw	s0, 8(sp)                       # 4-byte Folded Reload
	addi	sp, sp, 16
	ret
.Lfunc_end1:
	.size	print_string, .Lfunc_end1-print_string
                                        # -- End function
	.globl	print_int                       # -- Begin function print_int
	.p2align	2
	.type	print_int,@function
print_int:                              # @print_int
# %bb.0:                                # %entry
	addi	sp, sp, -16
	sw	ra, 12(sp)                      # 4-byte Folded Spill
	sw	s0, 8(sp)                       # 4-byte Folded Spill
	addi	s0, sp, 16
	sw	a0, -12(s0)
	lw	a1, -12(s0)
	#APP
	li	a7, 1
	mv	a0, a1
	ecall

	#NO_APP
	lw	ra, 12(sp)                      # 4-byte Folded Reload
	lw	s0, 8(sp)                       # 4-byte Folded Reload
	addi	sp, sp, 16
	ret
.Lfunc_end2:
	.size	print_int, .Lfunc_end2-print_int
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"Fibonacci numbers:\n"
	.size	.L.str, 20

	.type	.L.str.1,@object                # @.str.1
.L.str.1:
	.asciz	"\n"
	.size	.L.str.1, 2

	.ident	"clang version 23.0.0git (https://github.com/llvm/llvm-project.git 2430410b7d879fce3db76c21bb8c60ed22abd0b5)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym print_string
	.addrsig_sym print_int
