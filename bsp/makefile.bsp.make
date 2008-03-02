# bsp.make
#
HEADERS	= bsp.h GraphicsGems.h
OBJS	= bspAlloc.o bspCollide.o bspPartition.o \
bspTree.o bspUtility.o bspMemory.o
BSPMAIN = mainBsp.o

OPT	= -g -I../gems
LIBS	= -lm
BSP	= bsp
BSPLIB  = lib$(BSP).a
# ANSI-C: Use CC on Suns. Use cc -Aa on HPs.
CC	= gcc



$(BSP)	: $(BSPLIB) $(BSPMAIN)
	echo Compiling $@ ...
	$(CC) $(OPT) $(BSPMAIN) $(BSPLIB) $(LIBS) -o $(BSP)
$(BSPLIB): $(OBJS)
	echo Compiling $@ ...
	$(AR) cru $(BSPLIB)  $(OBJS)

bspAlloc.o	: $(HEADERS) bspAlloc.c
	echo Compiling $@ ...
	$(CC) $(OPT) -c bspAlloc.c
bspCollide.o	: $(HEADERS) bspCollide.c
	echo Compiling $@ ...
	$(CC) $(OPT) -c bspCollide.c
bspPartition.o	: $(HEADERS) bspPartition.c
	echo Compiling $@ ...
	$(CC) $(OPT) -c bspPartition.c
bspTree.o	: $(HEADERS) bspTree.c
	echo Compiling $@ ...
	$(CC) $(OPT) -c bspTree.c
bspUtility.o	: $(HEADERS) bspUtility.c
	echo Compiling $@ ...
	$(CC) $(OPT) -c bspUtility.c
bspMemory.o	: $(HEADERS) bspMemory.c
	echo Compiling $@ ...
	$(CC) $(OPT) -c bspMemory.c
$(BSPMAIN)	: $(HEADERS) mainBsp.c
	echo Compiling $@ ...
	$(CC) $(OPT) -c mainBsp.c
clean:
	echo clean $(BSP) project ...
	$(RM) *.o $(BSPLIB) $(BSP)
# bsp.make
