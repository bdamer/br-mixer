CC=g++
CFLAGS=-g -Wall -std=c++0x
LDFLAGS=-lm `libpng-config --ldflags`
TARGET=bin/brmixer
SRC=./src/

all: $(TARGET)

clean:
	rm -f *.o brmixer

$(TARGET): brmixer.o mixfile.o shpfile.o datfile.o trefile.o vqafile.o
	$(CC) $(CFLAGS) -o $(TARGET) brmixer.o mixfile.o shpfile.o datfile.o trefile.o vqafile.o $(LDFLAGS)

brmixer.o: $(SRC)brmixer.cpp
	$(CC) $(CFLAGS) -c $(SRC)brmixer.cpp

mixfile.o: $(SRC)mixfile.cpp
	$(CC) $(CFLAGS) -c $(SRC)mixfile.cpp

shpfile.o: $(SRC)shpfile.cpp
	$(CC) $(CFLAGS) -c $(SRC)shpfile.cpp

datfile.o: $(SRC)datfile.cpp
	$(CC) $(CFLAGS) -c $(SRC)datfile.cpp

trefile.o: $(SRC)trefile.cpp
	$(CC) $(CFLAGS) -c $(SRC)trefile.cpp
	
vqafile.o: $(SRC)vqafile.cpp
	$(CC) $(CFLAGS) -c $(SRC)vqafile.cpp

