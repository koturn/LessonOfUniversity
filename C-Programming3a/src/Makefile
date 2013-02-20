ifeq ($(OS),Windows_NT)
    SUFFIX = .exe
else
    SUFFIX =
endif
CC      = gcc
LDLIBS  = -lm
MACROS  = -DOPTIMIZE
CFLAGS  = -pipe -O3 -Wall -W -Wextra $(MACROS) $(ENCODE)
LDFLAGS = -pipe -O3 -s
TARGET  = group03$(SUFFIX)
LIBDIR  = lib
OBJS    = main.o $(LIBDIR)/data_handler.o
SRCS    = $(OBJS:%.o=%.c)


%.exe :
	$(CC) $(LDFLAGS) $(filter %.c %.o, $^) $(LDLIBS) -o $@


all : $(TARGET)

$(TARGET) : $(OBJS)

main.o : main.c $(LIBDIR)/data_handler.h

$(LIBDIR)/data_handler.o : $(LIBDIR)/data_handler.c $(LIBDIR)/data_handler.h


.PHONY : clean objclean
clean :
	$(RM) $(TARGET) $(OBJS)
objclean :
	$(RM) $(OBJS)
