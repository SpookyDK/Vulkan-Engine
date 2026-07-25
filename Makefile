CC       := gcc
CFLAGS   := -Wall -Wextra -Wshadow -Wdouble-promotion -Wnull-dereference -Wstack-usage=2048 -g
INCLUDES := -isystem include/tiny_obj_c -isystem include
DEFS     := -DTOBJ_ENABLE_FILE_IO
LIBS     := -lvulkan -lglfw -lm

# Enable -fanalyzer if 'analyze=1' is passed
ifeq ($(analyze),1)
    CFLAGS += -fanalyzer
endif

OBJS   := main.o tiny_obj_c.o tobj_tess.o
TARGET := vulkan_test

# Set the default target to show help + build
.DEFAULT_GOAL := default

default: help $(TARGET)

$(TARGET): $(OBJS)
	@echo "==> Linking $(TARGET)..."
	@$(CC) $(OBJS) -o $@ $(LIBS)
	@echo "==> Build successful!"

# Main code (analyzed if analyze=1)
main.o: main.c
	@echo "==> Compiling main.c..."
	@$(CC) -c $< $(INCLUDES) $(DEFS) $(CFLAGS) -o $@

# Third-party code (built quietly without warnings/analyzer)
tiny_obj_c.o: include/tiny_obj_c/tiny_obj_c.c
	@echo "==> Compiling tiny_obj_c.c..."
	@$(CC) -c $< $(INCLUDES) $(DEFS) -o $@

tobj_tess.o: include/tiny_obj_c/tobj_tess.c
	@echo "==> Compiling tobj_tess.c..."
	@$(CC) -c $< $(INCLUDES) -o $@

# Quick Help Menu
help:
	@echo "---------------------------------------------------------"
	@echo " Vulkan Test Build System"
	@echo "---------------------------------------------------------"
	@echo "  make              - Build project (fast mode)"
	@echo "  make analyze=1    - Build project with -fanalyzer enabled"
	@echo "  make run          - Build and run executable"
	@echo "  make memtest      - Run with Valgrind leak detection"
	@echo "  make clean        - Delete object files & executable"
	@echo "  make help         - Show this menu"
	@echo "---------------------------------------------------------"

run: $(TARGET)
	./$(TARGET)

memtest: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=definite,indirect --suppressions=vulkan.supp ./$(TARGET)

clean:
	@rm -f $(OBJS) $(TARGET)
	@echo "==> Cleaned up object files and binary."

.PHONY: default help run memtest clean
