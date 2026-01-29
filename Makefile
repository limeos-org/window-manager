# ---
# Common
# ---

CC = clang
CFLAGS = -Wall -Wextra -g -MMD -MP

INTERNAL_LIBS = $(shell pkg-config --libs limeos-common-lib)
EXTERNAL_DEPS = x11 xcomposite xi xrandr xfixes cairo dbus-1
EXTERNAL_LIBS = $(shell pkg-config --libs $(EXTERNAL_DEPS))
LIBS = $(INTERNAL_LIBS) $(EXTERNAL_LIBS)

CFLAGS += $(shell pkg-config --cflags $(EXTERNAL_DEPS))

# ---
# Build
# ---

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

TARGET = $(BIN_DIR)/limeos-window-manager

SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

INCLUDES = $(shell find $(SRC_DIR) -type d -exec printf "-I{} " \;)
CFLAGS += $(INCLUDES)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# ---
# Setup
# ---

setup:
	@echo "[" > compile_commands.json
	@first=1; for src in $(SOURCES); do \
		[ $$first -eq 0 ] && echo "," >> compile_commands.json; \
		first=0; \
		echo "{\"directory\":\"$(CURDIR)\",\"file\":\"$$src\",\"arguments\":[\"$(CC)\",$(foreach f,$(CFLAGS),\"$(f)\",)\"-c\",\"$$src\"]}" >> compile_commands.json; \
	done
	@echo "]" >> compile_commands.json
	@echo "Generated compile_commands.json"

# ---
# Other
# ---

.PHONY: all clean setup
