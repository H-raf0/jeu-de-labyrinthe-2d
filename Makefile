# Define directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
AUDIO_DIR = $(SRC_DIR)/audio
GRAPHICS_DIR = $(SRC_DIR)/graphics
UI_DIR = $(SRC_DIR)/ui
INCLUDE_DIR = include
BUILD_DIR = build
BUILD_CORE = $(BUILD_DIR)/core
BUILD_AUDIO = $(BUILD_DIR)/audio
BUILD_GRAPHICS = $(BUILD_DIR)/graphics
BUILD_UI = $(BUILD_DIR)/ui

# Compiler and flags
CFLAGS = -I$(INCLUDE_DIR) -Wall -Wextra -g
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lm

# Source files
SOURCES = $(SRC_DIR)/main_app.c \
          $(CORE_DIR)/jeu.c \
          $(CORE_DIR)/laby.c \
          $(CORE_DIR)/structures.c \
          $(AUDIO_DIR)/audio.c \
          $(GRAPHICS_DIR)/effetSDL.c \
          $(GRAPHICS_DIR)/labySDL.c \
          $(UI_DIR)/drawing.c \
          $(UI_DIR)/endscreen.c

# Object files (mirror source structure in build/)
OBJECTS = $(BUILD_DIR)/main_app.o \
          $(BUILD_CORE)/jeu.o \
          $(BUILD_CORE)/laby.o \
          $(BUILD_CORE)/structures.o \
          $(BUILD_AUDIO)/audio.o \
          $(BUILD_GRAPHICS)/effetSDL.o \
          $(BUILD_GRAPHICS)/labySDL.o \
          $(BUILD_UI)/drawing.o \
          $(BUILD_UI)/endscreen.o

# Target executable
TARGET = exec

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Create build directories and compile
$(BUILD_DIR)/main_app.o: $(SRC_DIR)/main_app.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_CORE)/%.o: $(CORE_DIR)/%.c
	@mkdir -p $(BUILD_CORE)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_AUDIO)/%.o: $(AUDIO_DIR)/%.c
	@mkdir -p $(BUILD_AUDIO)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_GRAPHICS)/%.o: $(GRAPHICS_DIR)/%.c
	@mkdir -p $(BUILD_GRAPHICS)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_UI)/%.o: $(UI_DIR)/%.c
	@mkdir -p $(BUILD_UI)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean