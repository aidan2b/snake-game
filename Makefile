CC=gcc
CFLAGS=-Wall -Wextra
INCLUDES=-IC:/msys64/mingw64/include/SDL2/ -IC:/msys64/mingw64/include/SDL2_ttf/ # Include the SDL2 headers
LIBS=-LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf # Correct order of linking libraries
SRCS = snake.c
OBJS = $(SRCS:.c=.o)
MAIN = snake
SHARED_LIB = libsnake.so

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

shared: $(SRCS)
	$(CC) -shared -o $(SHARED_LIB) -fPIC $(CFLAGS) $(INCLUDES) $(SRCS) $(LIBS)

clean:
	del *.o
	del $(MAIN).exe
	del $(SHARED_LIB)
