/**
 * maze.c
 * generates a maze on a grid of size n*n from user input
**/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#ifdef PARALLEL
#include <omp.h> 
#endif

#define NUM_THREADS 4

typedef struct stackInfo{
    int x;
    int y;
    struct stackInfo *next;
    struct stackInfo *prev;
} Stack;

//Global variables to keep track of how many times each thread is used
int count1 = 0;
int count2 = 0;
int count3 = 0;
int count4 = 0;

//Maze functions
char **initMaze(int size);
void createMaze(char **ptr, int size);
void freeMaze(char **ptr, int size);

//Stack functions
Stack *initStack(int rank, int size);
void addToStack(Stack **stack, int row, int col);

//Other helper functions
int checkRowCol(int row, int col, int size);
void direction( int row, int col, int midRow, int midCol, int size, Stack **stack, char **maze, int rank);

int main(int argc, char* argv[]) {
    int size = 11; 	//default grid size
    int seed = 0; 	//default seed
    int argPtr = 0; //used to read args
    char **maze = NULL; //the maze

    if (argc > 1) {
        argPtr = 1;
        while(argPtr < argc) {
            if (strcmp(argv[argPtr], "-n") == 0) {
                sscanf(argv[argPtr+1], "%d", &size);
                argPtr += 2;
            } else  if (strcmp(argv[argPtr], "-s") == 0) {
                sscanf(argv[argPtr+1], "%d", &seed);
                argPtr += 2;
                srand(seed);
            } else {
                printf("USAGE: %s <-n size of maze> <-s seed>\n", argv[0]);
                exit(1);
            }
        }
    } 

    //Initialize the maze
    maze = initMaze(size);
    
    #ifdef PARALLEL
#   pragma omp parallel num_threads(NUM_THREADS) 
    #endif

    //Create the maze 
    createMaze(maze,size);
    
    //Display the maze
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            printf("%c ",maze[i][j]);
        }
        printf("\n");  
    }

    #ifdef PARALLEL
    printf("process 0 count: %d\n",count1);
    printf("process 1 count: %d\n",count2);
    printf("process 2 count: %d\n",count3);
    printf("process 3 count: %d\n",count4);
    #endif

    //Free the maze
    freeMaze(maze, size);

    return 0;
} 

//Initializes the maze array
char **initMaze(int size) {
    char **ptr = malloc(sizeof(char*)*size);

    for(int i = 0; i < size; i++) {
        ptr[i] = malloc(sizeof(char) * size);
        for(int j = 0; j < size; j++) {
            ptr[i][j] = '.'; //fill with '.'
        }
    }
    return ptr;
}

//Creates a maze - this is the critical section
void createMaze(char **maze, int size) {
    int row, col = 0;
    int rank = 0;

    //Counters for random number generator
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;

    #ifdef PARALLEL
    rank = omp_get_thread_num(); 
    #endif
    
    //Initialize the stack
    Stack *stack = initStack(rank,size);

    //Set our first values
    row = stack->x;
    col = stack->y;

    //While the stack is not empty.
    while(stack) {
        for(int i = 0; i<4; i++) {
            //Randomly visit each of the four neighbours.     
            switch(rand() % 4) {
                case 0: //Up
                        if(a!=1) {
                            direction((row-2), col, (row-1), col, size, &stack, maze, rank);
                            a=1;
                        } else {
                            i--;
                        }
                        break;
                case 1: //Right
                        if(b!=1) {
                            direction(row, (col+2), row, (col+1), size, &stack, maze, rank);
                            b=1;
                        } else {
                            i--;
                        }
                        break;
                case 2: //Down
                        if(c!=1) {
                            direction((row+2), col, (row+1), col, size, &stack, maze, rank); 
                            c=1;
                        } else {
                            i--;
                        }
                        break;
                case 3: //Left
                        if(d!=1) {
                            direction(row, (col-2), row, (col-1), size, &stack, maze, rank);
                            d=1;
                        } else {
                            i--;
                        }
            }    
        }
        //Reset counters for next loop
        a = b = c = d = 0;
        
        //Pop an item out of the stack.
        row = stack->x;
        col = stack->y;
        free(stack->next);
        stack = stack->prev;
    }
    free(stack);
}

//Frees the maze array
void freeMaze(char **ptr, int size) {
    for(int i = 0; i < size; i++) {
        free(ptr[i]);
    }
    free(ptr);
}

//Initializes a stack depending on rank's location
Stack *initStack(int rank, int size) {
    Stack *stack = malloc(sizeof(Stack));

    //Pick a starting location and put it in the stack. 
    switch(rank) {
                //(1,1)
        case 0: stack->x = 1;
                stack->y = 1;
                break;

                //(1, (size-2))
        case 1: stack->x = 1;
                stack->y = size-2;
                break;

                //((size-2), 1)
        case 2: stack->x = size-2;
                stack->y = 1;
                break;

                //((size-2), (size-2))
        case 3: stack->x = size-2;
                stack->y = size-2;
    }

    stack->prev = NULL;
    stack->next = NULL;

    return(stack);
}

//Adds a new item to the stack
void addToStack(Stack **stack, int row, int col) {
    (*stack)->next = malloc(sizeof(Stack));
    (*stack)->next->prev = (*stack);

    (*stack) = (*stack)->next;
    (*stack)->x = row;
    (*stack)->y = col;
    (*stack)->next = NULL;
}

//Checks to make sure that the row and column are valid to prevent accessing out of bounds areas
int checkRowCol(int row, int col, int size) {
    if(row < 1 || row > (size - 1)) {
        return 1; //returns 1 if invalid
    } 

    if(col < 1 || col > (size - 1)) {
        return 1; //returns 1 if invalid
    }

    return 0; //returns 0 on sucess
}

//Calls functions - used to differentiate between different directions
void direction(int row, int col, int midRow, int midCol, int size, Stack **stack, char **maze, int rank) {
    if(checkRowCol(row, col, size) == 0) {
        if(maze[row][col] == '.') {
            //If the neighbour has not been claimed already (it contains a '.') then claim it 
            maze[row][col] = rank + '0';
            //and the intervening unclaimed space by putting the thread rank in those two array elements. 
            maze[midRow][midCol] = rank + '0';
            //Push each of the successfully visited neighbours onto the stack. 
            addToStack(stack,row,col);
            //keep track of how many times each process has been used
            switch(rank) {
                case 0: count1++; break;
                case 1: count2++; break;
                case 2: count3++; break;
                case 3: count4++; break;
            }
        }
    }
}
