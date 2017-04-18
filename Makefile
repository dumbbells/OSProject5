CC=gcc
CFLAGS=-g
DEPS = includes.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: oss userProcess

oss: parent.o  rscMgmt.o system.o
	gcc -o $@ $^ $(CFLAGS)

userProcess: child.o rscMgmt.o system.o
	gcc -o $@ $^ $(CFLAGS)
	rm *.o

clean:
	rm -f *.o oss userProcess a.out
