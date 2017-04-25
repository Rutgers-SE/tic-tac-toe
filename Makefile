CC = clang
CFLAGS = -g

CL_OBJECTS = cl.o comm.o
GS_OBJECTS = gs.o comm.o

all: gs cl

gs: $(GS_OBJECTS)
cl: $(CL_OBJECTS)

comm.o: comm.c
	$(CC) $(CFLAGS) -c comm.c

gs.o: gs.c
	$(CC) $(CFLAGS) -c gs.c

cl.o: cl.c
	$(CC) $(CFLAGS) -c cl.c
