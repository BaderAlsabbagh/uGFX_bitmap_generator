CC=gcc
CFLAGS=-I.
DEPS = libbmp.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: main.o libbmp.o 
	$(CC) -o bw_convert.exe main.o libbmp.o 

test: bw_convert.exe 128x64.bmp
	.\bw_convert.exe -i 128x64.bmp -o BWimage.bmp > output.h

clean:
	del *.o *.exe