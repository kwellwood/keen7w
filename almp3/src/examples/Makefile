# Makefile for AllegroMP3 examples


# select (uncomment) one target and commet DJGPP again if you are not
# aiming for that platform
#TARGET=DJGPP_STATIC
#TARGET=MINGW32_STATIC
#TARGET=LINUX_STATIC
TARGET=SOLARIS_DYNAMIC


#CC=cc
LFLAGS=-s



# DJGPP_STATIC
ifeq ($(TARGET),DJGPP_STATIC)
TARGETFLAGS=-Wall -O2 -march=pentium -fomit-frame-pointer -finline-functions -ffast-math
OUT=djgpp/example.exe
LIBS=../lib/djgpp/libalmp3.a -lalleg
OBJDIR=../obj/djgpp/examples
endif



# MINGW32_STATIC
ifeq ($(TARGET),MINGW32_STATIC)
LFLAGS=-s -mwindows
TARGETFLAGS=-Wall -O2 -march=pentium -fomit-frame-pointer -finline-functions -ffast-math
OUT=mingw32/example.exe
LIBS=../lib/mingw32/libalmp3.a -lalleg
OBJDIR=../obj/mingw32/examples
endif



# LINUX_STATIC
ifeq ($(TARGET),LINUX_STATIC)
TARGETFLAGS=-Wall -O2 -march=pentium -fomit-frame-pointer -finline-functions -ffast-math
OUT=linux/example
LIBS=../lib/linux/libalmp3.a `allegro-config --libs`
OBJDIR=../obj/linux/examples
endif


# SOLARIS_DYNAMIC
ifeq ($(TARGET),SOLARIS_DYNAMIC)
TARGETFLAGS=-O -xlibmopt `/opt/BCallegro/bin/allegro-config --cflags`
OUT=unix/example
LIBS=../lib/unix/libalmp3.so `/opt/BCallegro/bin/allegro-config --libs`
OBJDIR=../obj/unix/examples
endif

# setting compiler flags
CFLAGS=$(TARGETFLAGS) -I../include

# making of the example
$(OUT): example.o
	$(CC) $(LFLAGS) $(CFLAGS) $(OBJDIR)/$< -o $@ $(LIBS)



# compiling of the example
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $(OBJDIR)/$@



clean:
	rm -f $(OBJDIR)/example.o $(OUT)

