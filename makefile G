PROG = c-wire
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

SRC = main.c utils.c fichier.c avl.c usine.c arbre.c fuites.c
OBJ = $(SRC:.c=.o)
HEADERS = utils.h fichier.h avl.h usine.h arbre.h fuites.h

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(PROG)

dirs:
	mkdir -p output tests tmp
