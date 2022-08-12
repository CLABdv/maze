//Credit to wikipedia:
//https://en.wikipedia.org/wiki/Maze_generation_algorithm
//Randomized Prim's algorithm

/*
TODO:
Make it so that one x-coordinate is two characters wide
Make it so that if  you do not have colours it calculates the maze but does not print it
*/

#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define BLANK ' '
struct coords
{
    int y;
    int x;
};

void finyx(int, struct coords *, struct coords *);
int finind(int *, struct coords *, struct coords *);
int getrandind(int);
void getedges(int, int *, struct coords *);
void createmaze(bool *, int *, struct coords *, int *);
void fwriteB(FILE *, bool *, int);

int main(int argc, char **argv)
{
    int i, j,opt, argst;
    //printbool, storebool, sleepbool
    bool stB = true, slB = false;
    char *path = malloc(sizeof(char)*15);
    path = NULL;
    struct timespec *slp = malloc(sizeof(struct timespec));
    while((opt = getopt(argc, argv, ":lt:hn"))!=-1)
    {
        switch(opt)
        {
            case 'h':
            endwin();
            printf("possible arguments:\n-t\tsleep time in nanoseconds between printing each \"block\"\n");
            printf("-n\tdo not store the maze\n");
            printf("-l\tchoose storage location of maze (by default /tmp/maze)\n");
            printf("-h\tdisplay this menu\n");
            return 0;
            case 'l':
            path = optarg;
            break;

            case 't':
            slB=true;
            unsigned long placeholder = atol(optarg);
            slp->tv_nsec=placeholder%(int)1e9;
            slp->tv_sec=(placeholder-slp->tv_nsec)/1e9;
            break;
            case 'n':
            stB = false;
            break;

            case '?':
            endwin();
            printf("Invalid option -%c\n",optopt);
            printf("-h for options\n");
            return 2;

            case ':':
            endwin();
            printf("Option -%c requires a value\n", optopt);
            printf("-h for help\n");
            return 1;

        }
    }
    

    
    //all possible positions on the terminal
    //seed randomizer, initiate stdscr, hide cursor, initiate colour pairs
    srand(time(0));
    initscr();
    //declare timer
    //start and elapsed
    struct timespec start, end;
    struct coords xy;
    struct coords pr;
    getmaxyx(stdscr, xy.y, xy.x);

    if (!has_colors)
    {
        endwin();
        printf("Your terminal does not support colours.\n");
        return 1;
    }
    curs_set(0);
    start_color();
    use_default_colors();
    init_color(COLOR_BLACK,0,0,0);
    init_pair(2, COLOR_BLACK, COLOR_BLACK);
    attron(COLOR_PAIR(2));
    for (i=0; i<xy.y; i++)
    {
        move(i,0);
        for (j=0; j<xy.x; j++)
        {
            addch('+');
        }
    }
    attroff(COLOR_PAIR(2));
    finyx(xy.x*(xy.y-1), &pr, &xy);
    mvaddch(pr.y, pr.x, BLANK);
    int *walls = malloc(sizeof(int)*xy.x*xy.y);
    bool *cells = calloc(sizeof(int),xy.x*xy.y);
    
    
    //bottom left corner
    cells[xy.x*(xy.y-1)]=true;
    int amwalls = 2;
    walls[0] = (xy.x*(xy.y-1))+1;
    walls[1] = (xy.x*(xy.y-1))-xy.x;

    //start timer
    clock_gettime(CLOCK_MONOTONIC,&start);
    //Reason for having an if at the start and two nearly identical loops is that a call to
    //nanosleep and refresh takes a ridicoulus amount of extra time
    if (slB)
    {
        while (amwalls > 0)
        {
            createmaze(cells, walls, (void *) &xy, &amwalls);
            nanosleep(slp, NULL);
            refresh();
        }
    }
    else
    {
        while (amwalls > 0)
            createmaze(cells, walls, (void *) &xy, &amwalls);
        refresh(); 
    }

    //get elapsed clockss
    clock_gettime(CLOCK_MONOTONIC,&end);
    //CLOCKS PER SEC IS A PLACEHOLDER
    //IT IS INCORRECT
    double time_taken = (double) (end.tv_sec-start.tv_sec)+ 1e-9*(end.tv_nsec-start.tv_nsec);
    //setting to NULL counters double free or corruption exception
    //aka trying to free something that you are not allowed to free.
    walls=NULL;
    slp = NULL;
    free(walls);
    free(slp);


    if (stB)
    {
        FILE *storage;
        if (path == NULL)
            path="/tmp/maze";
        storage = fopen(path, "w");
        if (storage != NULL)
        {
            fprintf(storage,"##Y: %d\n##X: %d\n",xy.y,xy.x);
            fwriteB(storage, cells, xy.x*xy.y);
        }
        fclose(storage);
    }
    cells=NULL;
    free(cells);
    path=NULL;
    free(path);
    //wait for input to end window
    getch();
    endwin();
    printf("maze was created in %lf seconds\n",time_taken);
    return 0;
}

//helper functions

//find wanted y and x from the index
void finyx(int index, struct coords *want, struct coords *inps)
{
    want->y=(index-(index%inps->x))/inps->x;
    want->x=index%inps->x;
}
//get wanted index from y and x
int finind(int *wantindex, struct coords *curr, struct coords *pos)
{
    if (wantindex != NULL)
        *wantindex = curr->y*pos->x+curr->x;
    return curr->y*pos->x+curr->x;
}

//get adjacent cells of a given cell
//stored as up left right down
void getedges(int index, int *dirs, struct coords *xy)
{
    int i,j;
    int wanty, wantx;
    for (i =0; i<2; i++)
    {
        for (j=0; j<2; j++)
        {
            dirs[i+j*2] = (index-xy->x)+j*(xy->x+1)+i*(xy->x-1);
        }
    }
    
}

void createmaze(bool *cells, int *walls, struct coords *xy, int *amwalls)
{
    //initialization
    const int y = xy->y;
    int currentwall = rand()%(*amwalls);
    int i, j, count, amcells;
    struct coords check;
    count = amcells = 0;
    //adjacent edges are stored as up left right down
    int *adj = malloc(sizeof(int)*4);
    // value of cells which are part of the maze-path 
    // and values which are located on the opposite side of the screen are stored in donotplace
    int *donotplace = malloc(sizeof(int)*4);
    getedges(walls[currentwall], adj, xy);
    finyx(walls[currentwall],&check, xy);
    
    if (check.x+1 == xy->x)
    {
        donotplace[count]=adj[2];
        ++count;
    }
    if (check.y+1 == xy->y)
    {
        donotplace[count]=adj[0];
        ++count;
    }
    if (check.x-1 < 0)
    {
        donotplace[count]=adj[1];
        ++count;
    }
    if (check.y-1 < 0)
    {
        donotplace[count]=adj[3];
        ++count;
    }
    
    for (i=0; i<4; i++)
    {
        if (cells[adj[i]])
        {
            ++amcells;
            donotplace[count]=adj[i];
            ++count;
        }
    }

    while(count<4)
    {
        donotplace[count] = -1;
        ++count;
    }
    if (amcells == 1)
    {
        finyx(walls[currentwall], &check, xy);
        mvaddch(check.y, check.x, BLANK);
        cells[walls[currentwall]]=true;
        for (i=currentwall; i < (*amwalls); i++)
        {
            walls[i]=walls[i+1];
        }
        --(*amwalls);
        
        for (i=0; i<4; i++)
        {
            if (adj[i] != donotplace[0] && adj[i] != donotplace[1] && adj[i] != donotplace[2] && adj[i] != donotplace[3])
            {
                walls[*amwalls] = adj[i];
                ++*amwalls;
            }
        }

    }
    else
    {
        for (i=currentwall; i < (*amwalls); i++)
        {
            walls[i]=walls[i+1];
        }
        --(*amwalls);
    }
    adj = NULL;
    donotplace = NULL;
    free(adj);
    free(donotplace);
    return;
}

//assuming storage is already checked for null pointer and opened
//pointer to file, pointer to array of booleans, length of boolean array
void fwriteB(FILE * storage, bool *array, int len)
{
    int i;
    for (i=0; i<len; i++)
        (array[i]) ? fputc('1',storage) : fputc('0', storage);
    fputc('\n', storage);
}