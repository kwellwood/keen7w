
CPP   = g++
CC    = gcc
RM    = del
MAKE  = make
MKDIR = mkdir

KEEN_OBJ = obj/k7.o obj/graphicslib.o obj/map.o obj/player.o obj/menu.o obj/camera.o obj/scoreboard.o obj/item.o obj/sprite.o obj/enemy.o obj/portal.o obj/lift.o obj/shot.o obj/audio.o
TED_OBJ  = obj/ted.o obj/audio.o obj/camera.o obj/enemy.o obj/graphicslib.o obj/item.o obj/lift.o obj/map.o obj/player.o obj/portal.o obj/sprite.o obj/shot.o obj/tedmenu.o
ALMP3    = almp3/src/lib/libalmp3.a

CXXINCS = -Ialmp3/src/include -Iallegro4/addons/loadpng/include -Iallegro4/include
LIBS    = -Lalmp3/src/lib -Lallegro4/addons/loadpng/lib -Lallegro4/lib \
          -mwindows -lalmp3 -lloadpng -lalleg44.dll

# -g : include debugging info
CXXFLAGS = $(CXXINCS) -g -D__GTHREAD_HIDE_WIN32API -fexpensive-optimizations -O1

# -static : link against static runtime (gnat)
# -s      : strip debug symbols
LINKFLAGS = -static #-s

.PHONY: all clean prebuild keen ted $(BIN_GRAPHICS_FILES)
.SECONDARY: $(OBJ)

keen: bin/keen7.exe
ted: bin/ted.exe
all: keen ted media

clean:
	-$(RM) $(KEEN_OBJ) $(TED_OBJ) bin/keen7.exe bin/ted.exe
	$(MAKE) -C almp3 clean

bin/keen7.exe: prebuild $(KEEN_OBJ) $(ALMP3)
	$(CPP) $(KEEN_OBJ) -o "$@" $(LINKFLAGS) $(LIBS)

bin/ted.exe: prebuild $(TED_OBJ) $(ALMP3)
	$(CPP) $(TED_OBJ) -o "$@" $(LINKFLAGS) $(LIBS)

obj/%.o: src/%.cpp
	$(CPP) -c src/$*.cpp -o $@ $(CXXFLAGS)

$(ALMP3):
	$(MAKE) -C almp3

media: bin/kevin.dat

GRAPHICS_FILES := $(addprefix graphics/,$(shell cat bin/kevin.txt))
BIN_GRAPHICS_FILES := $(addprefix bin/,$(GRAPHICS_FILES))

bin/kevin.dat: bin/kevin.txt $(BIN_GRAPHICS_FILES)

bin/graphics/%.png:
	cd bin && dat -f -k -c0 -s0 -a -t PNG kevin.dat graphics/$*.png

prebuild:
	-@$(MKDIR) obj
	-copy allegro4\bin\dat.exe bin\dat.exe
