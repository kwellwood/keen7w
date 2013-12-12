# Makefile for AllegroMP3 using mpglib part of mpg123


# select (uncomment) one target and comment DJGPP if you are not aiming
# for that platform
#TARGET=DJGPP_STATIC
#TARGET=MINGW32_STATIC
#TARGET=MINGW32_DLL
#TARGET=LINUX_STATIC
TARGET=SOLARIS_DYNAMIC

prefix = /opt/BCallegro
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
includedir = ${prefix}/include
docdir = ${prefix}/share/doc
infodir = ${prefix}/info
mandir = ${prefix}/man
libdir = ${exec_prefix}/lib
alincdir = .
allibdir = .

#CC = gcc
LFLAGS =
LIBIMP =

INSTALL = /bc/cbe/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_INFO = /usr/sfw/bin/install-info
#DESTDIR =



# DJGPP_STATIC
ifeq ($(TARGET),DJGPP_STATIC)
TARGETFLAGS=-Wall -O2 -march=pentium -fomit-frame-pointer -finline-functions -ffast-math
OBJDIR=obj/djgpp/static
LIBDEST=lib/djgpp/libalmp3.a
endif



# MINGW32_STATIC
ifeq ($(TARGET),MINGW32_STATIC)
LFLAGS=-mwindows
TARGETFLAGS=-Wall -O2 -march=pentium -fomit-frame-pointer -finline-functions -ffast-math
OBJDIR=obj/mingw32/static
LIBDEST=lib/mingw32/libalmp3.a
endif


# MINGW32_DLL
ifeq ($(TARGET),MINGW32_DLL)
LFLAGS=-mwindows -shared -L$(allibdir)
TARGETFLAGS=-Wall -O2 -march=pentium -fomit-frame-pointer -finline-functions -ffast-math
OBJDIR=obj/mingw32/dll
LIBIMP=lib/mingw32/libalmp3dll.a
LIBDEST=lib/mingw32/almp3.dll
ALMP3_DLL=1
ALMP3_DLL_EXPORTS=1
endif



# LINUX_STATIC
ifeq ($(TARGET),LINUX_STATIC)
TARGETFLAGS=-Wall -O2 -march=pentium -fomit-frame-pointer -finline-functions -ffast-math
OBJDIR=obj/linux/static
LIBDEST=lib/linux/libalmp3.a
endif

# SOLARIS_DYNAMIC
ifeq ($(TARGET),SOLARIS_DYNAMIC)
TARGETFLAGS=-O -xlibmopt -Kpic -G -I/opt/BCallegro/include -L/opt/BCallegro/lib -R/opt/BCallegro/lib
OBJDIR=obj/unix/dynamic
LIBDEST=lib/unix/libalmp3.so
endif


# setting object files, paths and compiler flags
vpath %.c decoder

vpath %.c src
vpath %.o $(OBJDIR)

CFLAGS=$(TARGETFLAGS) -Idecoder -Iinclude -I$(alincdir)
OBJECTS=layer2.o layer3.o interface.o decode_i386.o dct64_i386.o common.o tabinit.o almp3.o
OBJECTS2=$(addprefix $(OBJDIR)/,$(OBJECTS))



# making of the library

# MINGW32_DLL
ifeq ($(TARGET),MINGW32_DLL)
$(LIBDEST): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS2) $(LFLAGS) $(CFLAGS) -Wl,--out-implib=$(LIBIMP) -lalleg

# others
else
ifeq ($(TARGET),SOLARIS_DYNAMIC)
$(LIBDEST): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS2) $(LFLAGS) $(CFLAGS) -lalleg-4.2.0
else
$(LIBDEST): $(OBJECTS)
	ar rs $(LIBDEST) $(OBJECTS2)
endif
endif


# compiling of the library
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $(OBJDIR)/$@

install:
	$(INSTALL_DATA) -D README.txt $(DESTDIR)$(docdir)/almp3/README.txt
	$(INSTALL_DATA) -D CHANGES.txt $(DESTDIR)$(docdir)/almp3/CHANGES.txt
	$(INSTALL_DATA) -D docs/ALMP3.txt $(DESTDIR)$(docdir)/almp3/ALMP3.txt
	$(INSTALL_DATA) -D docs/AUTHORS.txt $(DESTDIR)$(docdir)/almp3/AUTHORS.txt
	$(INSTALL_DATA) -D docs/COPYING.txt $(DESTDIR)$(docdir)/almp3/COPYING.txt
	$(INSTALL_DATA) -D include/almp3.h $(DESTDIR)$(includedir)/almp3.h
	$(INSTALL) -m 755 -D $(LIBDEST) $(DESTDIR)$(libdir)/libalmp3.so

clean:
	rm -f $(OBJECTS2) $(LIBDEST) $(LIBIMP)
