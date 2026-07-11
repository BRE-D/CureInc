# ============================================================
#  CureInc - Makefile
#  Compiler : GCC (MinGW)
#  Raylib   : C:/raylib/raylib/src
#  Change RAYLIB_PATH below if your install location differs.
# ============================================================

RAYLIB_PATH = C:/raylib/raylib/src

CC      = gcc
CFLAGS  = -std=c2x -Wall -Wno-missing-braces -I$(RAYLIB_PATH)
LDFLAGS = -L$(RAYLIB_PATH) -lraylib -lopengl32 -lgdi32 -lwinmm -lm

SRC = src/main.c   \
      src/virus.c  \
      src/cure.c   \
      src/region.c \
      src/events.c \
      src/skills.c \
      src/ui.c

TARGET = CureInc.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	-rm -f $(TARGET)

.PHONY: all clean
