TARGET = QUEENS
OBJS = queens.o
#CFLAGS = -O0 -g -ggdb -w  -Wall
CFLAGS = -O3 -w -g -g -DNDEBUG -fno-builtin
#LDFLAGS = -lpthread 
#LDFLAGS = ../../Pthread/libmypthread.a
WRAPPER_OBJ = ../../Apps/mcsim_wrapper.o
CC = gcc

$(TARGET): $(OBJS) $(WRAPPER_OBJ)
	$(CC) -static $(OBJS) $(WRAPPER_OBJ) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

$(WRAPPER_OBJ): %.o: %.c
	gcc -O0 -g -c $< -o $@

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(OBJS) $(TARGET) pin*.log
