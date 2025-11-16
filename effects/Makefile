SRC ?= effetmain.c effetSDL.c bSDL.c
EXE := exec

CC := gcc
CFLAGS := -Wall -Wextra -MMD -Og -g $(shell sdl2-config --cflags) -fsanitize=address,undefined
# CFLAGS := -Wall -Wextra -MMD -O2 $(shell sdl2-config --cflags)  # version optimisée

LDFLAGS := -fsanitize=address,undefined -lSDL2_image -lSDL2_ttf -lSDL2_gfx -lm $(shell sdl2-config --libs)

OBJ := $(patsubst %.c, build/%.o, $(SRC))
DEP := $(OBJ:.o=.d)

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(EXE)

cleanall:
	rm -rf $(EXE) *.d *.o build

-include $(DEP)
