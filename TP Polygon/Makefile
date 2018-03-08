EXE:=main

all: clean $(EXE)

clean:
	rm -f $(EXE) && rm -f *.o

$(EXE): *.c
	gcc -Wall -o $(EXE) $(EXE).c -lglut -lGLU -lGL -lm 

run: all
	./$(EXE)

