# Makefile for the smash program
CC = g++ 
CFLAGS = -g -Wall -pthread
CCLINK = $(CC)
OBJS = main.o bank.o atm.o account.o
RM = rm -f
# Creating the  executable
Bank: $(OBJS)
	$(CCLINK) $(CFLAGS) -o Bank $(OBJS)
# Creating the object files
account.o: account.cpp account.h
atm.o: atm.cpp atm.h bank.h account.h
bank.o: bank.cpp bank.h account.h
main.o: main.cpp account.h bank.h atm.h
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*

