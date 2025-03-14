CC=gcc
CFLAGS=-Wall -I./include -I./src
LDFLAGS=-lreadline

SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin

# Fuentes
SOURCES=$(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/cli/*.c $(SRC_DIR)/cli/commands/*.c $(SRC_DIR)/db/*.c $(SRC_DIR)/utils/*.c)
OBJECTS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Objetivo principal
TARGET=$(BIN_DIR)/nql_cli

all: $(TARGET)

# Crear directorios necesarios
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: $(TARGET)
	$(TARGET)

# Regla para ejecutar con Valgrind
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose $(TARGET)

# Debug con símbolos para depuración
debug: CFLAGS += -g
debug: clean $(TARGET)

.PHONY: all clean run valgrind debug
