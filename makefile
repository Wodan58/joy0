# makefile for Joy 

HDRS  =  globals.h
SRCS  =  interp.c  scan.c  utils.c  main.c
OBJS  =  interp.o  scan.o  utils.o  main.o
CC    =  gcc -O3 -Wall -Wextra -Wpedantic -Werror

joy:		$(OBJS)
		$(CC)  $(OBJS)  -o joy

$(OBJS):	$(HDRS)

clean:
		rm  -f $(OBJS)
