SPARROW_FOLDER = ../sparrow3d

#==stuff linked to
STATIC = -Wl,-rpath=$(SPARROW_FOLDER) -Wl,-Bstatic -lsparrowNet -Wl,-Bdynamic
DYNAMIC =  -lSDL_net -lSDL
#==global Flags. Even on the gp2x with 16 kb Cache, -O3 is much better then -Os
CFLAGS = -O3 -fsingle-precision-constant -fPIC
# Testtweaks: -fgcse-lm -fgcse-sm -fcched-spec-load -fmodulo-sched -funsafe-loop-optimizations -Wunsafe-loop-optimizations -fgcse-las -fgcse-after-reload -fvariable-expansion-in-unroller -ftracer -fbranch-target-load-optimize
GENERAL_TWEAKS = -ffast-math
#==PC==
CPP = gcc -g -DX86CPU $(GENERAL_TWEAKS)
SDL = `sdl-config --cflags`

ifdef TARGET
include $(SPARROW_FOLDER)/target-files/$(TARGET).mk
BUILD = ./build/$(TARGET)/fusilli
SPARROW_LIB = $(SPARROW_FOLDER)/build/$(TARGET)/sparrow3d
ifeq ($(TARGET),win32)
	#the BUILDING_DLL is important for the linking...
	STATIC = $(SPARROW_LIB)/libsparrowNet_static.a -DBUILDING_DLL
endif
else
TARGET = "Default (change with make TARGET=otherTarget. See All targets with make targets)"
BUILD = .
SPARROW_LIB = $(SPARROW_FOLDER)
endif
LIB += -L$(SPARROW_LIB)
INCLUDE += -I$(SPARROW_FOLDER)



all: fusilli
	@echo "=== Built for Target "$(TARGET)" ==="

targets:
	@echo "The targets are the same like for sparrow3d. :P"

fusilli: fusilli.c makeBuildDir
	$(CPP) $(CFLAGS) fusilli.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC) -o $(BUILD)/fusilli$(SUFFIX)

makeBuildDir:
	 @if [ ! -d $(BUILD:/fusilli=/) ]; then mkdir $(BUILD:/fusilli=/);fi
	 @if [ ! -d $(BUILD) ]; then mkdir $(BUILD);fi

clean:
	rm -f *.o
	rm -f fusilli

oclean:
	rm -f *.o
