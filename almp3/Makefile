#
# Configuration
#
# OS       : Name of the current operation system: 'OSX', 'Windows_NT'
# SHORT_OS : Short, lower case name of the OS: 'osx', 'win'
# IS_WIN   : 'True', ''
# IS_OSX   : 'True', ''
# EXT      : Extention of executable files
#
# MAKE     : Name of 'make' executable for recursive building
#
# Defined functions:
#
# rm       : Produces a command that can be executed to delete a file.
#

ifeq ($(OSTYPE), darwin)
    OS       := OSX
    SHORT_OS := osx
    IS_WIN   :=
    IS_OSX   := True
    EXT      :=
else
    OS       := Windows_NT
    SHORT_OS := win
    IS_WIN   := True
    IS_OSX   :=
    EXT      := .exe
endif

#
# Tools
#

MAKE := make

#
# Functions
#

rm      = $(if $(IS_WIN),del $(call slashes,$1) 2> nul,rm -rf $1)
slashes = $(subst $(if $(IS_WIN),/,\),\,$1)

OBJ_DIR = $(if $(IS_WIN),mingw32,linux)
TARGET  = $(if $(IS_WIN),MINGW32_STATIC,LINUX_STATIC)

#
# Targets
#

src/lib/libalmp3.a:
	@$(MAKE) -C src CC=gcc TARGET=$(TARGET) LIBDEST=lib/libalmp3.a alincdir=../../allegro4/include

# almp3's makefile is not called to clean up because it uses 'rm' which won't
# work in a Windows environment. The cleanup is trivial anyway.
clean:
	-@$(call rm, src/lib/libalmp3.a)
	-@$(call rm, src/obj/$(OBJ_DIR)/static/*.o)
