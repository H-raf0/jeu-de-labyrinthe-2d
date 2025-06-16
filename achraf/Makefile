# Define the compiler
CC = gcc

# Define C compiler flags
CFLAGS = -Wall -Wextra

# Specify the linker flags
LDFLAGS = -lSDL2 -lm
LDFLAGS2 = -lSDL2 -lSDL2_image -lm

# Specify the source files
PROG_SOURCES = prog.c
SERPENT_SOURCES = serpent.c
SPRITE_SOURCES = sprite.c

# Define the object files (replace .c with .o)
PROG_OBJECTS = $(PROG_SOURCES:.c=.o)
SERPENT_OBJECTS = $(SERPENT_SOURCES:.c=.o)
SPRITE_OBJECTS = $(SPRITE_SOURCES:.c=.o)

# Define the final executable names
PROG_EXECUTABLE = prog
SERPENT_EXECUTABLE = serpent
SPRITE_EXECUTABLE = sprite

# Default target
all: $(PROG_EXECUTABLE) $(SERPENT_EXECUTABLE) $(SPRITE_EXECUTABLE)

# Link the object files to create the prog executable
$(PROG_EXECUTABLE): $(PROG_OBJECTS)
	$(CC) $(PROG_OBJECTS) -o $@ $(LDFLAGS)

# Link the object files to create the serpent executable
$(SERPENT_EXECUTABLE): $(SERPENT_OBJECTS)
	$(CC) $(SERPENT_OBJECTS) -o $@ $(LDFLAGS)

# Link the object files to create the serpent executable
$(SPRITE_EXECUTABLE): $(SPRITE_OBJECTS)
	$(CC) $(SPRITE_OBJECTS) -o $@ $(LDFLAGS2)

# Compile each .c file into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build files
clean:
	rm -f $(PROG_OBJECTS) $(SERPENT_OBJECTS) $(SPRITE_OBJECTS) $(PROG_EXECUTABLE) $(SERPENT_EXECUTABLE) $(SPRITE_EXECUTABLE)