# Variables
CFLAGS=-Wall -Wextra $(shell pkg-config --cflags raylib)
LIBS=$(shell pkg-config --libs raylib) -lglfw -lfftw3 -lm -ldl -lpthread
OBJ=main.o procesado_audio.o global.o interfaz.o
EXEC=ejecutable
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

# Reglas
all: clean $(EXEC)

$(EXEC): $(OBJ)
	@{ gcc $(CFLAGS) -o $@ $^ $(LIBS) > /dev/null; } || { echo "Error compiling $(EXEC)"; exit 1; }

%.o: %.c
	@{ gcc $(CFLAGS) -c $< -o $@ > /dev/null; } || { echo "Error compiling $<"; exit 1; }

clean:
	@rm -f $(OBJ) $(EXEC)

install: $(EXEC)
	@{ mkdir -p $(BINDIR) && cp $(EXEC) $(BINDIR); } || { echo "Error installing $(EXEC)"; exit 1; }

uninstall:
	@{ rm -f $(BINDIR)/$(EXEC); } || { echo "Error uninstalling $(EXEC)"; exit 1; }

.PHONY: all clean install uninstall
