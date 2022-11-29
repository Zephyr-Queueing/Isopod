CC=g++
CFLAGS=-std=c++11 -Iext -Isrc -Lext/message_v1.0.1.a
EDIR=ext
BDIR=bin
ODIR=obj
SDIR=src
TDIR=test

all: clean mount install $(BDIR)/main

$(BDIR)/main: $(ODIR)/main.o $(EDIR)/Message.h
	$(CC) $(CFLAGS) -o $(BDIR)/main $(ODIR)/main.o $(EDIR)/message.a

$(ODIR)/main.o: $(SDIR)/main.cpp $(EDIR)/Message.h
	$(CC) $(CFLAGS) -o $(ODIR)/main.o -c $(SDIR)/main.cpp

install:
	wget -P $(EDIR) https://github.com/Zephyr-Queueing/Quartz-Model/releases/download/v1.0.4/message.a;
	wget -P $(EDIR) https://github.com/Zephyr-Queueing/Quartz-Model/releases/download/v1.0.4/Message.h;
	wget -P $(EDIR) https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

mount:
	mkdir $(EDIR) $(BDIR) $(ODIR)

clean:
	rm -rf $(EDIR) $(BDIR) $(ODIR)
