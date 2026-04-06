CC = gcc

# Debug build (with symbols, for development)
debug: CFLAGS = -Wall -Wextra -g
debug: forge

# Release build (optimized, stripped, for production)
release: CFLAGS = -Wall -Wextra -O2 -ffunction-sections -fdata-sections -flto
release: LDFLAGS = -Wl,--gc-sections -s -flto
release: forge

OBJ = main.o utils.o deploy.o help.o config.o clone.o init.o \
      build-path-deployment.o prepare-deploy-workspace.o repo.o \
      log.o state.o templates.o progress.o remote.o cleanup.o bundle.o diff.o

# Default build (debug)
all: debug

# The final executable
forge: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o forge

# How to turn any .c file into a .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up files
clean:
	rm -f *.o forge

.PHONY: all debug release clean