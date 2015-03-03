CC=g++
CFLAGS=-g -Wall -std=c++0x
LDFLAGS=-lm `libpng-config --ldflags`
TARGET=brmixer
SRC=./src/

all: $(TARGET)

clean:
	rm -f *.o brmixer

$(TARGET): brmixer.o mixfile.o shpfile.o
	$(CC) $(CFLAGS) -o $(TARGET) brmixer.o mixfile.o shpfile.o $(LDFLAGS)

brmixer.o: $(SRC)brmixer.cpp
	$(CC) $(CFLAGS) -c $(SRC)brmixer.cpp

mixfile.o: $(SRC)mixfile.cpp
	$(CC) $(CFLAGS) -c $(SRC)mixfile.cpp

shpfile.o: $(SRC)shpfile.cpp
	$(CC) $(CFLAGS) -c $(SRC)shpfile.cpp

