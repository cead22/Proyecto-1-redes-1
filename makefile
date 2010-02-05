CC = gcc
OBJ = edolab.o remote.o
PROG = edolab remote

all: $(PROG)

edolab: edolab.o
	$(CC) edolab.c -o edolab
remote: remote.o
	$(CC) remote.c -o remote 
edolab.o: edolab.c
	$(CC) -c edolab.c
remote.o: remote.c
	$(CC) -c remote.c
clean:
	$(RM) $(OBJ)