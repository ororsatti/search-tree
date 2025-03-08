CC=gcc
CFLAGS=-Wall -Wextra -ggdb
DEST_DIR=dest
EXEC_NAME=search
MODULES=radix.o

mainapp: main.o $(MODULES) $(DEST_DIR)
	$(CC) $(CFLAGS) main.o $(MODULES) -o $(DEST_DIR)/$(EXEC_NAME)
	rm *.o

%.o: %.c
	$(CC) $(CFLAGS) -c $^

$(DEST_DIR):
	mkdir $(DEST_DIR)
