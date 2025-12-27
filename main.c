#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#define _POSIX_C_SOURCE 200809L

// ANSI colors
#define COLOR_LIVE "\033[97m"  // bright white
#define COLOR_DEAD "\033[90m"  // dim gray
#define RESET "\033[0m"

#define ROWS 24
#define COLS 80
#define LIVE 'L'
#define DEAD '.'
#define LIVE_CHAR "●" // displayed character for live cells (UTF-8 string)

//function declarations
void init_grid(char grid[ROWS][COLS]);
void input_cells(char grid[ROWS][COLS]);
void save_grid(const char *filename, char grid[ROWS][COLS]);
void load_grid(const char *filename, char grid[ROWS][COLS]);
void display_grid(char grid[ROWS][COLS], int generation);
int  count_neighbors(char grid[ROWS][COLS], int r, int c);
void update_grid(char grid[ROWS][COLS], char next[ROWS][COLS]);
void choose_grid(char grid[ROWS][COLS]);
int kbhit(void);


//main
int main(void)
{
    system("clear");
    char grid[ROWS][COLS], next[ROWS][COLS];
    int speed_ms;
    int generation = 0;
    char c;

    printf("Conway's Game of Life (80 x 24)\n\n\n");
    printf("--------------------------------\n");

    //choose grid
    choose_grid(grid);

    //ask for speed
    printf("Enter game speed in milliseconds (e.g. 200): ");
    scanf("%d", &speed_ms);
    system("clear");
    //clear input buffer
    while ((c = getchar()) != '\n' && c != EOF);

    printf("Controls: p = pause/resume, s = step, q = quit\n");

    int paused = 0;
    while (1) {
        display_grid(grid, generation);

        if (!paused) {
            update_grid(grid, next);
            memcpy(grid, next, sizeof(grid));
            generation++;
        }

        usleep(speed_ms * 1000);

        if (kbhit()) {
            c = getchar();
            if (c == 'p') paused = !paused;
            else if (c == 's') { paused = 1; update_grid(grid, next); memcpy(grid, next, sizeof(grid)); generation++; }
            else if (c == 'q') { save_grid("grid.txt", grid); break; }
        }
    }

    return 0;
}

//init grid
void init_grid(char grid[ROWS][COLS])
{
    for (int i=0;i<ROWS;i++)
        for (int j=0;j<COLS;j++)
            grid[i][j]=DEAD;
}

//ask users to set live cells
void input_cells(char grid[ROWS][COLS])
{
    int row,col;
    char choice='y';

    while(choice=='y'||choice=='Y'){
        system("clear");
        display_grid(grid,0);
        printf("Grid size: %d rows x %d columns\n",ROWS,COLS);
        printf("Enter row (1-%d) and column (1-%d): ",ROWS,COLS);
        scanf("%d %d",&row,&col);

        if(row>=1 && row<=ROWS && col>=1 && col<=COLS)
            grid[row-1][col-1]=LIVE;
        else{
            printf("Invalid position!\n");
            usleep(800000);
        }
        system("clear");
        display_grid(grid,0);
        printf("Add more live cells? (y/n): ");
        scanf(" %c",&choice);
    }
}

//save grid to file
void save_grid(const char *filename,char grid[ROWS][COLS]){
    FILE *fp=fopen(filename,"w");
    if(!fp){ perror("fopen"); exit(EXIT_FAILURE);}
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLS;j++)
            fputc(grid[i][j],fp);
        fputc('\n',fp);
    }
    fclose(fp);
}

//load grid from file
void load_grid(const char *filename,char grid[ROWS][COLS]){
    FILE *fp=fopen(filename,"r");
    if(!fp){ perror("fopen"); exit(EXIT_FAILURE);}
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLS;j++)
            grid[i][j]=fgetc(fp);
        fgetc(fp);
    }
    fclose(fp);
}

//print grid with borders and colors
void display_grid(char grid[ROWS][COLS], int generation){
    printf("\033[H"); // move cursor to top-left
    printf("\n\nGeneration: %d\n",generation);

    //top border
    putchar('+');
    for(int j=0;j<COLS;j++) putchar('-');
    putchar('+');
    putchar('\n');

    for(int i=0;i<ROWS;i++){
        putchar('|');
        for(int j=0;j<COLS;j++){
            if(grid[i][j]==LIVE)
                printf(COLOR_LIVE "%s" RESET, LIVE_CHAR);
            else
                printf(COLOR_DEAD "%c" RESET, DEAD);
        }
        putchar('|');
        putchar('\n');
    }

    //bottom border
    putchar('+');
    for(int j=0;j<COLS;j++) putchar('-');
    putchar('+');
    putchar('\n');
}

//count neighbours with wrap-around edges
int count_neighbors(char grid[ROWS][COLS],int r,int c){
    int count=0;
    for(int i=-1;i<=1;i++){
        for(int j=-1;j<=1;j++){
            if(i==0 && j==0) continue;
            int nr=(r+i+ROWS)%ROWS;
            int nc=(c+j+COLS)%COLS;
            if(grid[nr][nc]==LIVE) count++;
        }
    }
    return count;
}

//main conway logic
void update_grid(char grid[ROWS][COLS], char next[ROWS][COLS]){
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLS;j++){
            int n=count_neighbors(grid,i,j);
            if(grid[i][j]==LIVE)
                next[i][j]=(n==2||n==3)?LIVE:DEAD;
            else
                next[i][j]=(n==3)?LIVE:DEAD;
        }
    }
}

//choose grid source
void choose_grid(char grid[ROWS][COLS]){
    int option=0;
    char path[256];
    DIR *d;
    struct dirent *dir;

    printf("Choose grid:\n");
    printf("1. Use previous grid\n");
    printf("2. Make new grid\n");
    printf("3. Use pre-created grids (live cell positions only)\n");
    printf("4. Random grid\n");
    printf("Enter option (1-4): ");
    scanf("%d",&option);

    if(option==1){
        load_grid("grid.txt",grid);
    }else if(option==2){
        init_grid(grid);
        input_cells(grid);
        save_grid("grid.txt",grid);
    }else if(option==3){
        d=opendir("grids");
        if(!d){ printf("No grids folder found.\n"); exit(EXIT_FAILURE);}
        printf("\nAvailable pre-created grids:\n");
        int i=1;
        char files[100][256];
        while((dir=readdir(d))!=NULL){
            if(dir->d_type==DT_REG){
                printf("%d. %s\n", i, dir->d_name);
                strcpy(files[i-1], dir->d_name);
                i++;
            }
        }
        closedir(d);
        int choice;
        printf("Select a grid (1-%d): ", i-1);
        scanf("%d",&choice);
        if(choice<1 || choice>=i){ printf("Invalid choice.\n"); exit(EXIT_FAILURE);}
        snprintf(path, sizeof(path), "grids/%s", files[choice-1]);

        init_grid(grid); // clear grid

        // open file and read live cell positions
        FILE *fp = fopen(path, "r");
        if(!fp){ perror("fopen"); exit(EXIT_FAILURE);}
        int row, col;
        while(fscanf(fp,"%d %d",&row,&col)==2){
            if(row>=1 && row<=ROWS && col>=1 && col<=COLS)
                grid[row-1][col-1]=LIVE;
        }
        fclose(fp);
    }else if(option==4){
        init_grid(grid);
        for(int i=0;i<ROWS;i++)
            for(int j=0;j<COLS;j++)
                if(rand()%5==0) grid[i][j]=LIVE;
                save_grid("grid.txt",grid);
    }else{
        printf("Invalid option.\n"); exit(EXIT_FAILURE);
    }
}

//check if key is pressed (non-blocking)
int kbhit(void){
    struct termios oldt,newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO,&oldt);
    newt=oldt;
    newt.c_lflag&=~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&newt);
    oldf=fcntl(STDIN_FILENO,F_GETFL,0);
    fcntl(STDIN_FILENO,F_SETFL,oldf|O_NONBLOCK);
    ch=getchar();
    tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
    fcntl(STDIN_FILENO,F_SETFL,oldf);
    if(ch!=EOF){ ungetc(ch,stdin); return 1;}
    return 0;
}
