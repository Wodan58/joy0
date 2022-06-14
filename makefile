# makefile for Joy 

HDRS  =  globals.h
SRCS  =  interp.c  scan.c  utils.c  main.c
OBJS  =  interp.o  scan.o  utils.o  main.o
CC    =  cc

joy:		$(OBJS)
		$(CC)  $(OBJS)  -o joy

$(OBJS):	$(HDRS)

clean:
		rm  -f $(OBJS)
