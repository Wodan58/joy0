# makefile for Joy

HDRS  =  globals.h
SRCS  =  interp.c  scan.c  utils.c  main.c
OBJS  =  interp.o  scan.o  utils.o  main.o
# Use CC environment variable
CFLAGS = -O3 -Wall -Wextra -Wpedantic -Werror -Wno-char-subscripts -Wno-int-conversion -Wno-old-style-definition

joy:	$(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJS):$(HDRS)

clean:
	rm -f $(OBJS)
