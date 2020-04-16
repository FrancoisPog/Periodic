CC=gcc
CFLAGS=-Wall -std=c99 -g 

all:

% : %.c 
	$(CC) $(CFLAGS)  -o $@ $<


clean : 
	rm -f *.o

mrproper : clean
	rm -f $(TARGET)