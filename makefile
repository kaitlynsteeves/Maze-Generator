all: maze mazep

maze: 
	gcc maze.c -o maze

mazep:
	gcc maze.c -o mazep -fopenmp -DPARALLEL

clean:
	rm -f maze mazep