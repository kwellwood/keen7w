
CPP   = g++
CC    = gcc
RM    = del
MAKE  = make
MKDIR = mkdir

KEEN_OBJ = obj/k7.o obj/graphicslib.o obj/map.o obj/player.o obj/menu.o obj/camera.o obj/scoreboard.o obj/item.o obj/sprite.o obj/enemy.o obj/portal.o obj/lift.o obj/shot.o obj/audio.o
TED_OBJ  = obj/ted.o obj/audio.o obj/camera.o obj/enemy.o obj/graphicslib.o obj/item.o obj/lift.o obj/map.o obj/player.o obj/portal.o obj/sprite.o obj/shot.o obj/tedmenu.o
ALMP3    = almp3/src/lib/libalmp3.a

INCS    = 
CXXINCS = -I"allegro4/include" -I"almp3/src/include"
LIBS    = -L"allegro4/lib" -L"almp3/src/lib" -mwindows -lalmp3 -lalleg -s 

CFLAGS   = $(INCS) -fexpensive-optimizations -O1
CXXFLAGS = $(CXXINCS) -D__GTHREAD_HIDE_WIN32API -g -fexpensive-optimizations -O1

.PHONY: all clean prebuild keen ted
.SECONDARY: $(OBJ)

keen: bin/keen7.exe
ted: bin/ted.exe
all: keen ted

clean:
	-$(RM) $(KEEN_OBJ) $(TED_OBJ) bin/keen7.exe bin/ted.exe
	$(MAKE) -C almp3 clean

bin/keen7.exe: prebuild $(KEEN_OBJ) $(ALMP3)
	$(CPP) $(KEEN_OBJ) -o "$@" $(LIBS)

bin/ted.exe: prebuild $(TED_OBJ) $(ALMP3)
	$(CPP) $(TED_OBJ) -o "$@" $(LIBS)

obj/%.o: src/%.cpp
	$(CPP) -c src/$*.cpp -o $@ $(CXXFLAGS)

$(ALMP3):
	$(MAKE) -C almp3

prebuild:
	-@$(MKDIR) obj
