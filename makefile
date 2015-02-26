CC=g++
CFLAGS=-g -Wall -std=c++0x
TARGET=brmixer
SRC=./src/

all: $(TARGET)

$(TARGET): brmixer.o mixfile.o
	$(CC) $(CFLAGS) -o $(TARGET) brmixer.o mixfile.o

brmixer.o: $(SRC)brmixer.cpp
	$(CC) $(CFLAGS) -c $(SRC)brmixer.cpp

mixfile.o: $(SRC)mixfile.cpp
	$(CC) $(CFLAGS) -c $(SRC)mixfile.cpp

