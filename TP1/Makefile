EXE:=script1

all: clean $(EXE)

clean:
	rm -f $(EXE) && rm -f *.o

$(EXE): *.c
	gcc -Wall -o $(EXE) script1.c -lglut -lGLU -lGL -lm

run: all
	./$(EXE)


