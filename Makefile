CC = cc
CFLAGS = -ansi -pedantic -Wall -Wextra -g
LDFLAGS = -lm

TGT = taffy
SRC = $(shell find -type f -name "*.c")
OBJ = $(SRC:./src/%.c=./build/%.o)

all: $(dir ${OBJ}) ${TGT}

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all ./taffy tests/test.taf

$(dir ${OBJ}):
	mkdir -p $(dir ${OBJ})

build/%.o: src/%.c
		${CC} -c ${CFLAGS} $< -o $@

${TGT}: ${OBJ}
		${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
		rm -fr build/
		rm ${TGT}