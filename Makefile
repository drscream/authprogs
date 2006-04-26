PROG = authprogs
OBJS = safe_string.o str.o log.o authprogs.o

CC = gcc
CFLAGS= -Wall
LDFLAGS =

.c.o:
	$(CC) -c $(CFLAGS) $*.c

$(PROG) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) $(LIBS)

clean :
	rm -f *.o $(PROG) 

