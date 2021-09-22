PRGNAME     = game.elf
CC			= gcc

SRCDIR		= ./source ./source/sdl12
VPATH		= $(SRCDIR)
SRC_C		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
SRC_CP		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJ_C		= $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJ_CP		= $(notdir $(patsubst %.cpp, %.o, $(SRC_CP)))
OBJS		= $(OBJ_C) $(OBJ_CP)

CFLAGS		= -O0 -g3 -Wall -Wextra 
CFLAGS		+= -Isource -Isource/sdl12 -I/usr/include/libxml2


LDFLAGS     = -lc -lgcc -lm -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lxml2 -ltidy -lexpat

# Rules to make executable
$(PRGNAME): $(OBJS)  
	$(CC) $(CFLAGS) -o $(PRGNAME) $^ $(LDFLAGS)

$(OBJ_C) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PRGNAME) *.o
