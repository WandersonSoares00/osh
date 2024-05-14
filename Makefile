CFLAGS ?= -Wall -Wno-format-security

CC ?= cc

SRCS = src/*.c

OBJ = bin

all: $(OBJ) cshell

cshell: $(SRCS)
	$(CC) $(CFLAGS) -o $(OBJ)/cshell $(SRCS)

run: cshell
	./$(OBJ)/cshell

$(OBJ):
	mkdir -v $(OBJ)

clean:
	rm -rf $(OBJ)/cshell

