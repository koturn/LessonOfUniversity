CC      = gcc
MACRO   = -DOPTIMIZE
# ENCODE  = -finput-charset=utf-8 -fexec-charset=utf-8
CFLAGS  = -pipe -O3 -Wall -Wextra $(MACRO) $(ENCODE)
LDFLAGS = -pipe -O3 -s
LIBDIR  = lib
TARGET  = server   client
OBJS    = server.o client.o $(LIBDIR)/numgame.o $(LIBDIR)/netutil.o


.PHONY : all
all : $(TARGET)

server   : server.o $(LIBDIR)/numgame.o $(LIBDIR)/netutil.o

server.o : server.c $(LIBDIR)/numgame.h $(LIBDIR)/netutil.h $(LIBDIR)/macro.h

client   : client.o $(LIBDIR)/numgame.o $(LIBDIR)/netutil.o

client.o : client.c $(LIBDIR)/numgame.h $(LIBDIR)/netutil.h $(LIBDIR)/macro.h

$(LIBDIR)/numgame.o : $(LIBDIR)/numgame.c $(LIBDIR)/numgame.h $(LIBDIR)/macro.h

$(LIBDIR)/netutil.o  : $(LIBDIR)/netutil.c  $(LIBDIR)/netutil.h  $(LIBDIR)/macro.h


.PHONY : clean
clean :
	$(RM) $(TARGET) $(OBJS) *~ $(LIBDIR)/*~

.PHONY : objclean
objclean :
	$(RM) $(OBJS) *~ $(LIBDIR)/*~
