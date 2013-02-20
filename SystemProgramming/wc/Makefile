### This Makefile was written for GNU Make. ###
CC      = gcc
CFLAGS  = -pipe -O3 -Wall -Wextra -pedantic -fno-ident
LDFLAGS = -pipe -O3 -s
TARGET  = mywc
OBJS    = mywc.o


.PHONY : all
all : $(TARGET)

$(TARGET) : $(OBJS)


.PHONY : clean
clean :
	$(RM) $(TARGET) $(OBJS)

.PHONY : objclean
objclean :
	$(RM) $(OBJS)
