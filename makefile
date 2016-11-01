CFLAGS=-Wall -Wpedantic -ansi -g -O3
LIBS=-lcurses -lm -lc
LDFLAGS=$(LIBS)
SOURCES=e.c editor.c editor_functions.c buffer.c list.c utils.c #getdelim.c
OBJECTS=e.o editor.o editor_functions.o buffer.o list.o utils.o #getdelim.o
EXECUTABLE=e

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(OBJECTS) 
	    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	    $(CC) -c $(CFLAGS) $< -o $@

clean:
	rm *.o $(EXECUTABLE) vgcore*
