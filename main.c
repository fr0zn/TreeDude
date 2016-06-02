#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define NUM_TRUNKS 7
#define PROB_LEAF 15

#define PADDING_INT_TOP_BOT 1

#define PLAYER_W 18
#define PLAYER_H 6
#define TREE_W 16
#define TREE_H 6
#define LEAF_W 12
#define LEAF_H TREE_H
#define LEFT 0
#define RIGHT 1
#define SCREEN_W 51
#define SCREEN_H NUM_TRUNKS*TREE_H

#define PLAYER_Y SCREEN_H-TREE_H+PADDING_INT_TOP_BOT


void initCurses() {

	initscr();
	//raw();
	cbreak();
	noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
}



static char player_stop_left[PLAYER_H][PLAYER_W]={

	"      ###      ",
	"[   ]#####     ",
	"  | _|- -|_    ",
	"  |/  \\_/  \\   ",
	"  ||_______|   ",
	"   |___|___|   "

};

static char player_stop_right[PLAYER_H][PLAYER_W]={

	"       ###      ",
	"      #####[   ]",
	"     _|- -|_ |  ",
	"    /  \\_/  \\|  ",
	"    |_______||  ",
	"    |___|___||  "

};

static char player_cutting_left[PLAYER_H][PLAYER_W]={

	"      ###      ",
	"     #####     ",
	"    _|o o|_    ",
	"   /  \\_/  \\   ",
	"   |___   ----===",
	"   |___|___|   "

};

static char player_cutting_right[PLAYER_H][PLAYER_W]={

	"       ###      ",
	"      #####     ",
	"     _|o o|_    ",
	"    /  \\_/  \\   ",
	"==----   ___|   ",
	"    |___|___|   "

};

static char tree_base[TREE_H][TREE_W]={

	"|             |",
	"|             }",
	"{             |",
	"|             }",
	"|             |",
	"{             }"

};

static char right_leaf[LEAF_H][LEAF_W]={

    "     oOo",
    "     / oO",
    "   /--00oO",
    "--<   OOo ",
    "   \\oOO"
};

static char left_leaf[LEAF_H][LEAF_W]={

    "   o  o    ",
    " o0Oo--\\   ",
    "O0Oo  --\\  ",
    "oOOOo  /->-",
    "  o0O-/    "

};

typedef struct Tree_Piece{

    char piece[TREE_H][TREE_W];
    int leaf_side;
    struct Tree_Piece *next_piece;

}tree_piece;

typedef struct Tree {
    tree_piece *first_piece;
    tree_piece *last_piece;
    int size;
}tree;

void rand_leaf(tree_piece *piece_t,int first){

    if(!first){
    int perc = rand() % 100;
        if(perc < PROB_LEAF){
            piece_t->leaf_side=RIGHT;
        }else if(perc > 100-PROB_LEAF) {
            piece_t->leaf_side=LEFT;
        }
    }
}

void rand_trunk(tree_piece *piece_t){

    int x;
    int y;
    for(y=0; y<TREE_H;y++){
        for(x=1; x<TREE_W-2; x++){
            char randomletter = "   \\   /   |   }   {   "[rand () % 22];
            piece_t->piece[y][x] = randomletter;
        }
    }
}

tree_piece *gen_piece(int first){

    tree_piece *p;
    p = (tree_piece*)malloc(sizeof(tree_piece)) ;
    p->leaf_side = -1;
    p->next_piece = NULL;
    memcpy(p->piece,tree_base,TREE_H*TREE_W);

    rand_trunk(p);
    rand_leaf(p,first);

    return p;
}

void initTree(tree *tree_trunk){

	int i;
    tree_piece *p;
    p = gen_piece(1);
    tree_trunk->first_piece = p;
    tree_piece *tmp = tree_trunk->first_piece;

    for(i = 1; i < NUM_TRUNKS; i++){
        p = gen_piece(0);
        tmp->next_piece = p;
        tmp = tmp->next_piece;
        tree_trunk->last_piece = tmp;
    }
}

void show_trunk(WINDOW *win,int n, tree_piece *first_piece){

    int i;
    tree_piece *p;
    p = first_piece;
    for(i = 0; i < n; i++ ){
        p = p->next_piece;
    }
    for(i = 0; i < TREE_H; i++){
        mvwprintw(win,SCREEN_H-TREE_H-(TREE_H*n)+i+PADDING_INT_TOP_BOT,PLAYER_W,p->piece[i]);
    }
    if(p->leaf_side == LEFT){
        for(i = 0; i < TREE_H; i++){
            mvwprintw(win,SCREEN_H-TREE_H-(TREE_H*n)+i+PADDING_INT_TOP_BOT,PLAYER_W-LEAF_W+1,left_leaf[i]);
        }
    }else if(p->leaf_side == RIGHT){
        for(i = 0; i < TREE_H; i++){
            mvwprintw(win,SCREEN_H-TREE_H-(TREE_H*n)+i+PADDING_INT_TOP_BOT,PLAYER_W+TREE_W-1,right_leaf[i]);
        }
    }
}

void show_tree(WINDOW *win,tree *tree_trunk){
	int i;
	for (i = 0; i < NUM_TRUNKS ; i++) {
        show_trunk(win,i,tree_trunk->first_piece);
	}

}
int check_colision(tree *tree_trunk,int side){

    int leaf_side = tree_trunk->first_piece->next_piece->leaf_side;

    if(leaf_side != -1){
        if(side == leaf_side){
            return 1;
        }else{
            return 0;
        }
    }
    return 0;
}

void cut_tree(tree *tree_trunk){
    tree_piece *p;
    tree_piece *tmp;
    p = tree_trunk->first_piece;
    tree_trunk->first_piece = p->next_piece;
    free(p);
    tmp = gen_piece(0);
    tree_trunk->last_piece->next_piece = tmp;
    tree_trunk->last_piece = tmp;
}


void show_player(WINDOW *win,int side,int cutting){

    int base = PLAYER_Y;

    if(cutting){
        base -= 1;
    }
	int i;
	for (i = 0; i < PLAYER_H; i++) {
		if(side){
            if(cutting){
                mvwprintw(win,i+base,PLAYER_W+TREE_W-1,player_cutting_right[i]);
            }else{
                mvwprintw(win,i+base,PLAYER_W+TREE_W-1,player_stop_right[i]);
            }
		}else{
            if(cutting){
			    mvwprintw(win,i+base,2,player_cutting_left[i]);
            }else{
			    mvwprintw(win,i+base,2,player_stop_left[i]);
            }

		}
	}
}

void show_border(WINDOW *win){
	box(win, 0 , 0);
}

/* Draw game grid */
void drawGame(WINDOW *win,tree *tree_trunk,int side,int cutting) {
	/*clear();*/
	wclear(win);
	show_border(win);
	show_tree(win,tree_trunk);
	show_player(win,side,cutting);
	wrefresh(win);
}


void show_menu(WINDOW *win){
	/*clear();*/
	/*wclear(win);*/
    int i;
    for(i = 0; i < PLAYER_H; i++ ){
        mvwprintw(win,SCREEN_H/2-PLAYER_H+i,SCREEN_W/2-PLAYER_W/2,player_stop_right[i]);
    }
    mvwprintw(win,SCREEN_H/2+5,SCREEN_W/2-PLAYER_W/2,"Press Enter to play");
	show_border(win);
	/*refresh();*/
    wrefresh(win);
}

int main(int argc, char *argv[])
{
	char player[PLAYER_H][PLAYER_W];
	tree tree_trunk;
    int gameover = 0;
    int menu = 1;
    int cutting = 0;
	int side=RIGHT;
	int inputChar = 0;
	int x = 0, y = 0;
	int max_y = 0, max_x = 0;
    int seed = time(NULL);
    srand(seed);
    initCurses();
	initTree(&tree_trunk);
	// Global var `stdscr` is created by the call to `initscr()`
	WINDOW * win = newwin(SCREEN_H+2*PADDING_INT_TOP_BOT, SCREEN_W, 0, 0);
	while(inputChar != 'q' && !gameover) {
        usleep(50000);
        if(menu){
            show_menu(win);
            inputChar = getch();
            switch (inputChar){
                case 10: //ENTER
                    menu = 0;
                    break;
            }
        }else{
            getmaxyx(stdscr, max_y, max_x);
            inputChar = getch();
            switch (inputChar){
                case KEY_LEFT:
                case 'h':
                    side = LEFT;
                    if(check_colision(&tree_trunk,side)) gameover = 1;
                    cut_tree(&tree_trunk);
                    cutting = 1;
                    break;
                case KEY_RIGHT:
                case 'l':
                    side = RIGHT;
                    if(check_colision(&tree_trunk,side)) gameover = 1;
                    cut_tree(&tree_trunk);
                    cutting = 1;
                    break;
            }
            drawGame(win,&tree_trunk,side,cutting);
            cutting = 0;
	    }
    }

	endwin();
	return 0;
}
