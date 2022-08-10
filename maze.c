//Credit to wikipedia:
//https://en.wikipedia.org/wiki/Maze_generation_algorithm
//Randomized Prim's algorithm
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#define BLANK ' '
struct coords
{
    int y;
    int x;
};

void finyx(int, int *, int *, struct coords *);
int finind(int *, int, int, struct coords *);
int getrandind(int);
void getedges(int, int *, struct coords *);
void createmaze(bool *, int *, struct coords *, int *);

int main(int argc, char **argv)
{
    //get terminal size
    int i, j, pry, prx;

    //declare timers
    clock_t start, end;

    struct coords xy;
    //all possible positions on the terminal
    //seed randomizer, initiate stdscr, hide cursor, initiate colour pairs
    srand(time(0));
    initscr();
    getmaxyx(stdscr, xy.y, xy.x);
    const int x = getmaxx(stdscr);
    const int y = getmaxy(stdscr);
    curs_set(0);
    start_color();
    use_default_colors();
    init_color(COLOR_BLACK,0,0,0);
    init_pair(2, COLOR_BLACK, COLOR_BLACK);
    int *walls = malloc(sizeof(int)*xy.x*xy.y);
    bool *cells = calloc(sizeof(int),xy.x*xy.y);
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
    //bottom left corner
    cells[xy.x*(xy.y-1)]=true;
    int amwalls = 2;
    walls[0] = (xy.x*(xy.y-1))+1;
    walls[1] = (xy.x*(xy.y-1))-xy.x;

    finyx(xy.x*(xy.y-1), &pry, &prx, (void *) &xy);
    mvaddch(pry, prx, BLANK);
    for (i=0; i<2; i++)
    {
        finyx(walls[i], &pry, &prx, &xy);
    }

    struct timespec *slp = malloc(sizeof(struct timespec));
    slp=NULL;

    //start timer
    start = clock();
    while (amwalls > 1)
    {
        createmaze(cells, walls, (void *) &xy, &amwalls);
        nanosleep(slp, NULL);
    }
    refresh(); 
    //end timer
    end=clock();
    cells=NULL;
    walls=NULL;
    free(walls);
    free(slp);
    //measure time taken 
    double time_taken = (double)(end-start) / (double) (CLOCKS_PER_SEC);

    getch();
    endwin();
    free(cells);
    printf("maze was created in %lf seconds\n",time_taken);
    return 0;
}

//helper functions

//find wanted y and x from the index
void finyx(int index, int *wanty, int *wantx, struct coords *inps)
{
    *wanty=(index-(index%inps->x))/inps->x;
    *wantx=index%inps->x;
}
//get wanted index from y and x
int finind(int *wantindex, int curry, int currx, struct coords *pos)
{
    if (wantindex != NULL)
        *wantindex = curry*pos->x+currx;
    return curry*pos->x+currx;
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
    int checky, checkx, i, j, count, amcells;
    count = amcells = 0;
    
    //adjacent edges are stored as up left right down
    int *adj = malloc(sizeof(int)*4);
    // value of cells which are part of the maze-path 
    // and values which are located on the opposite side of the screen are stored in donotplace
    int *donotplace = malloc(sizeof(int)*4);
    getedges(walls[currentwall], adj, xy);
    finyx(walls[currentwall],&checky,&checkx, xy);
    if (checkx+1 == xy->x)
    {
        donotplace[count]=adj[2];
        ++count;
    }
    if (checkx-1 < 0)
    {
        donotplace[count]=adj[1];
        ++count;
    }
    if (checky+1 == xy->y)
    {
        donotplace[count]=adj[0];
        ++count;
    }
    if (checky-1 < 0)
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
        finyx(walls[currentwall], &checky, &checkx, xy);
        mvaddch(checky, checkx, BLANK);
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