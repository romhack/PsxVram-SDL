# --- Project details -------------------------------------------------

PROJ    := PsxVram-SDL
TARGET  := $(PROJ)

OBJS    := $(PROJ).o

# --- Build defines ---------------------------------------------------

CC      := gcc
LD      := gcc

CFLAGS :=-Wall

ifeq ($(OS),Windows_NT)
	CFLAGS +=-mwindows
	SDLFLAGS +=-lmingw32
endif 

SDLFLAGS +=-lSDL2main -lSDL2

ifeq ($(MODE),DEBUG)
	CFLAGS	+=-O0 -ggdb 	
else	
	CFLAGS  +=-O3 -s 
endif

.PHONY : debug build clean run 

# --- Build -----------------------------------------------------------
# Build process starts here!

build: $(TARGET).exe


# Compile (step 1)

$(TARGET).exe: $(TARGET).c
	$(CC)  $(CFLAGS) $(SDLFLAGS) -c $(TARGET).c 
	$(CC) -o $(TARGET).exe $(OBJS) $(CFLAGS) $(SDLFLAGS)
	


#-----Debug------------------------------------------------------------
debug:
	gdb $(PROJ).exe


# --- Clean -----------------------------------------------------------

clean : 

	@rm -fv *.exe
	@rm -fv *.o

# --- Run -----------------------------------------------------------
# Run app
run: 
	@./$(PROJ).exe

lindent:
	lindent $(TARGET).c
	

#EOF
