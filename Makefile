# --- Project details -------------------------------------------------

PROJ    := PsxVram-SDL
TARGET  := $(PROJ)

OBJS    := $(PROJ).o

# --- Build defines ---------------------------------------------------

CC      := gcc
LD      := gcc


ifeq ($(MODE),DEBUG)
	CFLAGS	:= -ggdb -O0 -Wall	
else	
	CFLAGS  := -O3 -s -Wall -mwindows
endif

SDLFLAGS := -lmingw32 -lSDL2main -lSDL2
#LINKERFLAGS := -Wl,-Map,foo.map 



.PHONY : debug build clean run 

# --- Build -----------------------------------------------------------
# Build process starts here!

build: $(TARGET).exe


# Compile (step 1)

$(TARGET).exe: $(TARGET).c
	$(CC)  $(CFLAGS) $(SDLFLAGS) -c $(TARGET).c 
	$(CC) -o $(TARGET).exe $(OBJS) $(CFLAGS) $(SDLFLAGS) $(LINKERFLAGS)
	


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
