#Author CARIOU Adrien
DEBUG=no
CC=g++
ifeq ($(DEBUG),yes)
	CFLAGS= -Wall  -g 	
	LDFLAGS= 
else
	CFLAGS= -Wall 
	LDFLAGS= 
endif
EXEC=main
SRC= main.cpp pima.cpp
OBJ=$(SRC:.cpp=.o)

COMPILE = $(CC) $(CFLAGS) $(LDFLAGS) -o $@ -L/usr/X11R6/lib -lm -lpthread -lX11

all: $(EXEC)
ifeq ($(DEBUG),yes)
	@echo "*Debug mode*"
else
	@echo "*Release mode*"
endif


main: $(OBJ) 
	@$(COMPILE) $^

main.o: main.cpp

%.o: %.cpp
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: all clean mrproper 

clean:
	@rm -rf *.o


mrproper: clean
	@rm -rf $(EXEC)
	@rm -rf *~





