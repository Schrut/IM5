XLIBS = -L/usr/X11/lib -L/usr/X11R6/lib -lX11 -lXext -lpthread
GLLIBS = -L/usr/X11R6/lib -lglut -lGL -lGLU

script1 : script1.o
	gcc -o script1 script1.o $(GLLIBS) $(XLIBS) -lm
