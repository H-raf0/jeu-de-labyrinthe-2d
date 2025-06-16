# Define the compiler
CC = gcc

# Define C compiler flags
CFLAGS:=-Wall -Wextra -MMD -Og -g $(sdl2-config --cflags) -fsanitize=address,undefined

# Specify the linker flags
LDFLAGS:=-fsanitize=address,undefined -lSDL2_image -lSDL2_ttf -lSDL2_gfx -lm -lSDL2
LDFLAGS2 = -lSDL2 -lSDL2_image -lm

# Specify the source files
PROG_SOURCES = prog.c
PROG2_SOURCES = prog2.c
SPRITE_SOURCES = sprite.c

# Define the object files (replace .c with .o)
PROG_OBJECTS = $(PROG_SOURCES:.c=.o)
PROG2_OBJECTS = $(PROG2_SOURCES:.c=.o)
SPRITE_OBJECTS = $(SPRITE_SOURCES:.c=.o)

# Define the final executable names
PROG_EXECUTABLE = prog
PROG2_EXECUTABLE = prog2
SPRITE_EXECUTABLE = sprite

# Default target
all: $(PROG_EXECUTABLE) $(PROG2_EXECUTABLE) $(SPRITE_EXECUTABLE)

# Link the object files to create the prog executable
$(PROG_EXECUTABLE): $(PROG_OBJECTS)
	$(CC) $(PROG_OBJECTS) -o $@ $(LDFLAGS)

# Link the object files to create the serpent executable
$(PROG2_EXECUTABLE): $(PROG2_OBJECTS)
	$(CC) $(PROG2_OBJECTS) -o $@ $(LDFLAGS)

# Link the object files to create the serpent executable
$(SPRITE_EXECUTABLE): $(SPRITE_OBJECTS)
	$(CC) $(SPRITE_OBJECTS) -o $@ $(LDFLAGS2)

# Compile each .c file into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build files
clean:
	rm -f $(PROG_OBJECTS) $(PROG_EXECUTABLE) $(PROG2_EXECUTABLE) $(PROG2_OBJECTS) $(SPRITE_EXECUTABLE) $(SPRITE_OBJECTS)