CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -Iinclude

SRC_DIR = src
BUILD_DIR = build

# sourced for the main application
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

TARGET = pomodoro
DIST_DIR = dist

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# rule to compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(DIST_DIR)
.PHONY: clean

install: $(TARGET)
	@mkdir -p $(DIST_DIR)/sounds
	cp $(TARGET) $(DIST_DIR)/
	cp -r sounds/*.mp3 $(DIST_DIR)/sounds/

run: install
	./$(DIST_DIR)/$(TARGET)