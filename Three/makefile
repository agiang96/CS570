# makefile for:
# cs570 Program 3
# Department of Computer Science
# San Diego State University
#
# This is the make file for the CS570 p3 assignment.
# Your program is linked with the pthread library as suggested in man sem_open

CC = gcc -g -O -Wall -Wpointer-arith -Wcast-qual -Wwrite-strings -Wmissing-prototypes -Wnested-externs
TO_RM = p3 p3robot p3main.o p3helper.o p3robot.o

all:	p3 p3robot

p3:	p3main.o 
	${CC} -o p3 p3main.o -pthread
# The previous line MUST begin with a TAB!

p3robot:        p3robot.o p3helper.o 
	$(CC) -o p3robot p3robot.o p3helper.o -pthread

p3main.o:	p3main.c p3.h
	$(CC) -c p3main.c

p3robot.o:	p3robot.c p3.h
	$(CC) -c p3robot.c

p3helper.o:	p3helper.c p3.h
	$(CC) -c p3helper.c

clean:
	rm -f ${TO_RM}
