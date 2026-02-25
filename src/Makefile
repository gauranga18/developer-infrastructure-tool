CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.o utils.o deploy.o help.o config.o clone.o init.o build-path-deployment.o prepare-deploy-workspace.o repo.o log.o

# The final executable
forge: $(OBJ)
	$(CC) $(OBJ) -o forge

# How to turn any .c file into a .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up files
clean:
	rm -f *.o forge
