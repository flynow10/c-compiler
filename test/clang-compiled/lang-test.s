	.attribute	4, 16
	.attribute	5, "rv32i2p1"
	.file	"lang-test.c"
	.text
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   # @main
# %bb.0:                                # %entry
	addi	sp, sp, -32
	sw	ra, 28(sp)                      # 4-byte Folded Spill
	sw	s0, 24(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 32
	li	a2, 0
	sw	a2, -12(s0)
	sw	a0, -16(s0)
	sw	a1, -20(s0)
	li	a0, 7
	sw	a0, -24(s0)
	li	a0, 3
	sw	a0, -28(s0)
	lw	a0, -24(s0)
	lw	a1, -28(s0)
	add	a0, a0, a1
	lw	ra, 28(sp)                      # 4-byte Folded Reload
	lw	s0, 24(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 32
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        # -- End function
	.ident	"clang version 23.0.0git (https://github.com/llvm/llvm-project.git 2430410b7d879fce3db76c21bb8c60ed22abd0b5)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
