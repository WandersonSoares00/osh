CFLAGS ?= -Wall -Wno-format-security

CC ?= cc

SRCS = src/*.c

OBJ = bin

CHECK_READLINE := $(shell echo '#include <readline/readline.h>' | gcc -E - 1>/dev/null 2>/dev/null && echo "yes" || echo "no")

ifeq ($(CHECK_READLINE), yes)
    CFLAGS += -DHAVE_READLINE
    LDFLAGS += -lreadline
endif

all: $(OBJ) cshell

cshell: $(SRCS)
	$(CC) $(CFLAGS) -o $(OBJ)/cshell $(SRCS) $(LDFLAGS)

run: cshell
	./$(OBJ)/cshell

$(OBJ):
	mkdir -v $(OBJ)

clean:
	rm -rf $(OBJ)/cshell

