CC = ~/Code/OpenSource/llvm-project/build/bin/clang
LDFLAGS = -march=rv32i -mabi=ilp32
CFLAGS = -target riscv32 -march=rv32i
CODE_DIR = test-code
BUILD_DIR = clang-compiled
objects = board.o fib.o lang-test.o
asm = fib.s lang-test.s board.s

all: $(objects) $(asm)

make-asm: $(asm)

$(objects): %.o: $(CODE_DIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $(BUILD_DIR)/$@

$(asm): %.s: $(CODE_DIR)/%.c
	$(CC) $(CFLAGS) -S $^ -o $(BUILD_DIR)/$@

build:
	mkdir -p $@