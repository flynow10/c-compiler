	.attribute	4, 16
	.attribute	5, "rv32i2p1"
	.file	"board.c"
	.text
	.globl	indexToCoord                    # -- Begin function indexToCoord
	.p2align	2
	.type	indexToCoord,@function
indexToCoord:                           # @indexToCoord
# %bb.0:                                # %entry
	addi	sp, sp, -32
	sw	ra, 28(sp)                      # 4-byte Folded Spill
	sw	s0, 24(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 32
	sw	a0, -20(s0)
	lw	a0, -20(s0)
	srai	a1, a0, 31
	srli	a1, a1, 29
	add	a1, a0, a1
	andi	a1, a1, -8
	sub	a0, a0, a1
	sw	a0, -16(s0)
	lw	a0, -20(s0)
	srai	a1, a0, 31
	srli	a1, a1, 29
	add	a0, a0, a1
	srai	a0, a0, 3
	sw	a0, -12(s0)
	lw	a0, -16(s0)
	lw	a1, -12(s0)
	lw	ra, 28(sp)                      # 4-byte Folded Reload
	lw	s0, 24(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 32
	ret
.Lfunc_end0:
	.size	indexToCoord, .Lfunc_end0-indexToCoord
                                        # -- End function
	.globl	coordToIndex                    # -- Begin function coordToIndex
	.p2align	2
	.type	coordToIndex,@function
coordToIndex:                           # @coordToIndex
# %bb.0:                                # %entry
	addi	sp, sp, -16
	sw	ra, 12(sp)                      # 4-byte Folded Spill
	sw	s0, 8(sp)                       # 4-byte Folded Spill
	addi	s0, sp, 16
                                        # kill: def $x12 killed $x11
                                        # kill: def $x12 killed $x10
	sw	a1, -12(s0)
	sw	a0, -16(s0)
	lw	a0, -16(s0)
	slli	a0, a0, 3
	lw	a1, -12(s0)
	add	a0, a0, a1
	lw	ra, 12(sp)                      # 4-byte Folded Reload
	lw	s0, 8(sp)                       # 4-byte Folded Reload
	addi	sp, sp, 16
	ret
.Lfunc_end1:
	.size	coordToIndex, .Lfunc_end1-coordToIndex
                                        # -- End function
	.globl	createBoard                     # -- Begin function createBoard
	.p2align	2
	.type	createBoard,@function
createBoard:                            # @createBoard
# %bb.0:                                # %entry
	addi	sp, sp, -16
	sw	ra, 12(sp)                      # 4-byte Folded Spill
	sw	s0, 8(sp)                       # 4-byte Folded Spill
	addi	s0, sp, 16
	li	a0, 264
	call	malloc
	sw	a0, -12(s0)
	lw	a1, -12(s0)
	li	a0, 4
	sw	a0, 0(a1)
	lw	a0, -12(s0)
	lw	ra, 12(sp)                      # 4-byte Folded Reload
	lw	s0, 8(sp)                       # 4-byte Folded Reload
	addi	sp, sp, 16
	ret
.Lfunc_end2:
	.size	createBoard, .Lfunc_end2-createBoard
                                        # -- End function
	.globl	createMove                      # -- Begin function createMove
	.p2align	2
	.type	createMove,@function
createMove:                             # @createMove
# %bb.0:                                # %entry
	addi	sp, sp, -32
	sw	ra, 28(sp)                      # 4-byte Folded Spill
	sw	s0, 24(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 32
	sw	a0, -12(s0)
	sw	a1, -16(s0)
	sw	a2, -20(s0)
	li	a0, 8
	call	malloc
	sw	a0, -24(s0)
	lw	a0, -12(s0)
	andi	a0, a0, 63
	lw	a1, -16(s0)
	andi	a1, a1, 63
	slli	a1, a1, 6
	or	a0, a0, a1
	lw	a1, -20(s0)
	slli	a1, a1, 12
	or	a0, a0, a1
	lw	a1, -24(s0)
	sh	a0, 0(a1)
	lw	a1, -24(s0)
	li	a0, 0
	sw	a0, 4(a1)
	lw	a0, -24(s0)
	lw	ra, 28(sp)                      # 4-byte Folded Reload
	lw	s0, 24(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 32
	ret
.Lfunc_end3:
	.size	createMove, .Lfunc_end3-createMove
                                        # -- End function
	.globl	makeMove                        # -- Begin function makeMove
	.p2align	2
	.type	makeMove,@function
makeMove:                               # @makeMove
# %bb.0:                                # %entry
	addi	sp, sp, -48
	sw	ra, 44(sp)                      # 4-byte Folded Spill
	sw	s0, 40(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 48
	sw	a0, -12(s0)
	sw	a1, -16(s0)
	lw	a0, -12(s0)
	lw	a0, 260(a0)
	lw	a1, -16(s0)
	sw	a0, 4(a1)
	lw	a0, -16(s0)
	lw	a1, -12(s0)
	sw	a0, 260(a1)
	lw	a0, -16(s0)
	lhu	a0, 0(a0)
	andi	a0, a0, 63
	sw	a0, -20(s0)
	lw	a0, -16(s0)
	lh	a0, 0(a0)
	slli	a0, a0, 20
	srli	a0, a0, 26
	sw	a0, -24(s0)
	lw	a0, -20(s0)
	call	indexToCoord
	sw	a1, -28(s0)
	sw	a0, -32(s0)
	lw	a0, -24(s0)
	call	indexToCoord
	sw	a1, -36(s0)
	sw	a0, -40(s0)
	lw	a0, -12(s0)
	lw	a1, -32(s0)
	slli	a1, a1, 5
	add	a0, a0, a1
	lw	a1, -28(s0)
	slli	a1, a1, 2
	add	a0, a0, a1
	lw	a0, 0(a0)
	sw	a0, -44(s0)
	lw	a0, -12(s0)
	lw	a1, -32(s0)
	slli	a1, a1, 5
	add	a0, a0, a1
	lw	a1, -28(s0)
	slli	a1, a1, 2
	add	a1, a0, a1
	li	a0, 0
	sw	a0, 0(a1)
	lw	a0, -44(s0)
	lw	a1, -12(s0)
	lw	a2, -40(s0)
	slli	a2, a2, 5
	add	a1, a1, a2
	lw	a2, -36(s0)
	slli	a2, a2, 2
	add	a1, a1, a2
	sw	a0, 0(a1)
	lw	ra, 44(sp)                      # 4-byte Folded Reload
	lw	s0, 40(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 48
	ret
.Lfunc_end4:
	.size	makeMove, .Lfunc_end4-makeMove
                                        # -- End function
	.globl	generateMoves                   # -- Begin function generateMoves
	.p2align	2
	.type	generateMoves,@function
generateMoves:                          # @generateMoves
# %bb.0:                                # %entry
	addi	sp, sp, -64
	sw	ra, 60(sp)                      # 4-byte Folded Spill
	sw	s0, 56(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 64
	sw	a0, -12(s0)
	li	a2, 0
	sw	a2, -60(s0)                     # 4-byte Folded Spill
	mv	a0, a2
	mv	a1, a2
	call	createMove
	mv	a1, a0
	lw	a0, -60(s0)                     # 4-byte Folded Reload
	sw	a1, -16(s0)
	lw	a1, -16(s0)
	sw	a1, -20(s0)
	sw	a0, -24(s0)
	j	.LBB5_1
.LBB5_1:                                # %for.cond
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB5_3 Depth 2
	lw	a1, -24(s0)
	li	a0, 7
	blt	a0, a1, .LBB5_10
	j	.LBB5_2
.LBB5_2:                                # %for.body
                                        #   in Loop: Header=BB5_1 Depth=1
	li	a0, 0
	sw	a0, -28(s0)
	j	.LBB5_3
.LBB5_3:                                # %for.cond1
                                        #   Parent Loop BB5_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lw	a1, -28(s0)
	li	a0, 7
	blt	a0, a1, .LBB5_8
	j	.LBB5_4
.LBB5_4:                                # %for.body3
                                        #   in Loop: Header=BB5_3 Depth=2
	lw	a0, -12(s0)
	lw	a1, -24(s0)
	slli	a1, a1, 5
	add	a0, a0, a1
	lw	a1, -28(s0)
	slli	a1, a1, 2
	add	a0, a0, a1
	lw	a0, 0(a0)
	bnez	a0, .LBB5_6
	j	.LBB5_5
.LBB5_5:                                # %if.then
                                        #   in Loop: Header=BB5_3 Depth=2
	lw	a0, -24(s0)
	sw	a0, -36(s0)
	lw	a0, -28(s0)
	sw	a0, -32(s0)
	lw	a0, -24(s0)
	sw	a0, -44(s0)
	lw	a0, -28(s0)
	addi	a0, a0, 1
	sw	a0, -40(s0)
	lw	a1, -32(s0)
	lw	a0, -36(s0)
	call	coordToIndex
	sw	a0, -64(s0)                     # 4-byte Folded Spill
	lw	a1, -40(s0)
	lw	a0, -44(s0)
	call	coordToIndex
	mv	a1, a0
	lw	a0, -64(s0)                     # 4-byte Folded Reload
	li	a2, 0
	call	createMove
	sw	a0, -48(s0)
	lw	a0, -48(s0)
	lw	a1, -20(s0)
	sw	a0, 4(a1)
	lw	a0, -48(s0)
	sw	a0, -20(s0)
	j	.LBB5_6
.LBB5_6:                                # %if.end
                                        #   in Loop: Header=BB5_3 Depth=2
	j	.LBB5_7
.LBB5_7:                                # %for.inc
                                        #   in Loop: Header=BB5_3 Depth=2
	lw	a0, -28(s0)
	addi	a0, a0, 1
	sw	a0, -28(s0)
	j	.LBB5_3
.LBB5_8:                                # %for.end
                                        #   in Loop: Header=BB5_1 Depth=1
	j	.LBB5_9
.LBB5_9:                                # %for.inc12
                                        #   in Loop: Header=BB5_1 Depth=1
	lw	a0, -24(s0)
	addi	a0, a0, 1
	sw	a0, -24(s0)
	j	.LBB5_1
.LBB5_10:                               # %for.end14
	li	a0, 0
	sw	a0, -52(s0)
	j	.LBB5_11
.LBB5_11:                               # %for.cond16
                                        # =>This Inner Loop Header: Depth=1
	lw	a1, -52(s0)
	li	a0, 7
	blt	a0, a1, .LBB5_14
	j	.LBB5_12
.LBB5_12:                               # %for.body18
                                        #   in Loop: Header=BB5_11 Depth=1
	lw	a1, -52(s0)
	li	a2, 0
	mv	a0, a2
	call	createMove
	sw	a0, -56(s0)
	lw	a0, -56(s0)
	lw	a1, -20(s0)
	sw	a0, 4(a1)
	lw	a0, -56(s0)
	sw	a0, -20(s0)
	j	.LBB5_13
.LBB5_13:                               # %for.inc22
                                        #   in Loop: Header=BB5_11 Depth=1
	lw	a0, -52(s0)
	addi	a0, a0, 1
	sw	a0, -52(s0)
	j	.LBB5_11
.LBB5_14:                               # %for.end24
	lw	a0, -16(s0)
	lw	ra, 60(sp)                      # 4-byte Folded Reload
	lw	s0, 56(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 64
	ret
.Lfunc_end5:
	.size	generateMoves, .Lfunc_end5-generateMoves
                                        # -- End function
	.ident	"clang version 23.0.0git (https://github.com/llvm/llvm-project.git 2430410b7d879fce3db76c21bb8c60ed22abd0b5)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym indexToCoord
	.addrsig_sym coordToIndex
	.addrsig_sym malloc
	.addrsig_sym createMove
