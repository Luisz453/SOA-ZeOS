#ifndef GAME_HH
#define GAME_HH

#include <list.h>

#define ROWS 20
#define COLUMNS 50
#define NPHANTOM 4
#define POINTS_TO_WIN 2000
#define DELAY 20
#define MAX_QUEUE_SIZE ROWS * COLUMNS
#define DEBUG 0

#define WALL_BACKGROUND 2
#define WALL_FOREGROUND 0
#define FOOD_BACKGROUND 4
#define FOOD_FOREGROUND 0
#define EMPTY_BACKGROUND 4
#define EMPTY_FOREGROUND 0
#define PACMAN_BACKGROUND 5
#define PACMAN_FOREGROUND 0
#define PHANTOM1_BACKGROUND 0
#define PHANTOM1_FOREGROUND 10 //
#define PHANTOM2_BACKGROUND 0
#define PHANTOM2_FOREGROUND 11 // al reves del rojo 
#define PHANTOM3_BACKGROUND 0
#define PHANTOM3_FOREGROUND 12 // rojo --> bfs
#define PHANTOM4_BACKGROUND 0
#define PHANTOM4_FOREGROUND 13 //
#define SURROUND_BACKGROUND 0
#define SURROUND_FOREGROUND 0
#define DEBUG_BACKGROUND 14
#define DEBUG_FOREGROUND 0


//#define VERTICAL_WALL_CODE "|"
//#define HORIZONTAL_WALL_CODE "-"
#define WALL_CODE "#"
#define FOOD_CODE "*"
#define EMPTY_CODE " "
#define PACMAN_CODE "C"
#define PHANTOM_CODE "A"
#define DEBUG_CODE "D"

//enum t_cell {CT_WALL_VERTICAL, CT_WALL_HORIZONTAL, CT_FOOD, CT_EMPTY};
enum t_cell {CT_WALL, CT_NULL, CT_FOOD, CT_EMPTY};
enum t_dir {DT_NULL, DT_UP, DT_DOWN, DT_LEFT, DT_RIGHT};
enum t_utype {PACMAN_TYPE, PHANTOM_TYPE};

struct node {
    int x, y;
    enum t_dir direction;
};

struct t_mov {
    enum t_dir direction;
    struct list_head list;
};

struct t_unit {
    int x,y;
    enum t_dir dir;
    enum t_utype type;
};

struct t_game {
    int lives;
    int points;
    enum t_cell Matrix[ROWS][COLUMNS];
    struct t_unit pacman;
    struct t_unit phantom[NPHANTOM];
};

void init_game(struct t_game *game, int offset);

void print_map(struct t_game *game);

void print_units(struct t_game *game);

void clear_screen();

void end_game();

void imprimir_coordenadas(struct t_game *game);

void read_keyboard(int* shared_mem_addr);

void start_game(struct t_game *game, int* shared_mem_addr);

int is_viable(struct list_head *curr_lhpos, struct t_mov *curr_pos, struct t_game *game);

void print_piece_map(struct t_game *game);

void move_phantoms(struct t_game *game);

enum t_dir bfs(struct t_game *game, int start_x, int start_y, int target_x, int target_y);

#endif