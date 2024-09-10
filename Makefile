CFLAGS ?= -Wall -Wno-format-security

CC ?= cc

SRCS = src/*.c

OBJ = bin

CHECK_READLINE := $(shell echo '#include <readline/readline.h>' | gcc -E - 1>/dev/null 2>/dev/null && echo "yes" || echo "no")

ifeq ($(CHECK_READLINE), yes)
    CFLAGS += -DHAVE_READLINE
    LDFLAGS += -lreadline
endif

all: $(OBJ) osh

osh: $(SRCS)
	$(CC) $(CFLAGS) -o $(OBJ)/osh $(SRCS) $(LDFLAGS)

run: osh
	./$(OBJ)/osh

$(OBJ):
	mkdir -v $(OBJ)

clean:
	rm -rf $(OBJ)/osh

