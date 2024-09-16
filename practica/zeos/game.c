#include <game.h>
#include <libc.h>
#include <list.h>

int frames;

int offset_actual;

int lives;

int random;
int total = 0;

int phantom1[] = {2,2,1,1,1,2,1,1,2,2};
int phantom2[] = {2,2,1,1,1,2,1,1,2,2};
int phantom3[] = {1,1,2,2,1,1,2,2,1,1};
int phantom4[] = {1,1,2,2,1,1,2,2,1,1};

struct list_head movements;

void init_game(struct t_game *game, int offset) {

    enum t_cell Matrix[ROWS][COLUMNS] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0},
        {0,2,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0},
        {0,2,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0},
        {0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0},
        {0,2,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0},
        {0,2,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0},
        {0,2,2,2,2,2,2,0,0,2,2,2,2,0,0,2,2,2,2,2,2,2,0,0,2,2,2,2,0,0,2,2,2,2,2,2,2,0,0,2,2,2,2,0,0,2,2,2,2,0},
        {0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0},
        {0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0},
        {0,0,0,0,0,0,2,0,0,2,2,2,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,2,2,2,0,0,2,2,2,2,0,0,2,0,0,0,0,0,2,0,0,0,0},
        {0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0},
        {0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0},
        {0,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,0,0,0,0},
        {0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0},
        {0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,2,2,2,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0},
        {0,2,2,2,0,0,2,2,2,2,0,0,2,2,2,2,2,2,2,0,0,2,0,0,2,2,2,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,0},
        {0,2,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,0,0,2,2,2,2,0,2,0,2,0,2,0,0,2,0,0,2,0,2,0,2,0,2,0,0,2,3,3,3,0},
        {0,2,2,2,2,2,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,0,0,0,0,2,2,2,2,2,0,0,2,2,2,2,0,2,2,2,0,2,2,2,2,3,3,3,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };

    // Initializes game matrix
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            game->Matrix[i][j] = Matrix[i][j];
            if(Matrix[i][j] == 2) ++total;
	}
    }
    frames = 0;
    offset_actual = offset;
    INIT_LIST_HEAD(&movements);
    // Initializes units 
    // Pacman
    game->pacman.x = 1; 
    game->pacman.y = 1; 
    game->pacman.dir = DT_LEFT;
    game->pacman.type = PACMAN_TYPE;
    // Phantoms
    const int X_START = 47;
    const int Y_START = 17;
    for(int i = 0; i < NPHANTOM/2; ++i){
        for (int j = 0; j < NPHANTOM/2; j++) {
            game->phantom[i*2+j].x = X_START+i;
            game->phantom[i*2+j].y = Y_START + j;
            game->phantom[i*2+j].type = PHANTOM_TYPE;
        }
    }
    game->points = 0;
    lives = 3;
    random = 0;

    clear_screen();
    print_map(game);
    print_units(game);

    char buff[10] = "VIDAS:";
    gotoxy(64,3);
    write(1, buff, strlen(buff));

    gotoxy(71,3);
    char v;
    itoa(lives, v);
    write(1,v, 1);

    char buff2[10] = "POINTS:";
    gotoxy(64,5);
    write(1, buff2, strlen(buff2));
    
    gotoxy(72,5);
    char buff3[5];
    itoa(game->points, buff3);
    write(1, buff3, strlen(buff3));
    
}

enum t_dir bfs(struct t_game *game, int start_x, int start_y, int target_x, int target_y){
    int visited[ROWS][COLUMNS] = {0};

    struct node nodes[MAX_QUEUE_SIZE];
    int queue[MAX_QUEUE_SIZE];
    int front = 0;
    int rear = 0;

    nodes[0] = (struct node) { .x = start_x, .y = start_y, .direction = DT_NULL };
    queue[rear++] = 0;
    visited[start_x][start_y] = 1;

    while(front != rear){
        int current_index = queue[front++];
        struct node *current_node = &nodes[current_index];
        if(current_node->x == target_x && current_node->y == target_y){
            return current_node->direction;
        }

        enum t_dir directions[] = {DT_UP, DT_DOWN, DT_LEFT, DT_RIGHT};

        for(int i = 0; i < 4; i++){
            int new_x = current_node->x;
            int new_y = current_node->y;
            enum t_dir new_direction = directions[i];

            switch(new_direction){
                case DT_UP:
                    new_x--;
                    break;
                case DT_DOWN:
                    new_x++;
                    break;
                case DT_LEFT:
                    new_y--;
                    break;
                case DT_RIGHT:
                    new_y++;
                    break;
            }
            if(new_x >= 0 && new_x < ROWS && new_y >= 0 && new_y < COLUMNS && game->Matrix[new_x][new_y] != CT_WALL && !visited[new_x][new_y]){
                #if DEBUG
                gotoxy(2, 2);
                write(1, "Moving to ", 11);
                char buff[12];
                itoa(new_x, buff);
                write(1, buff, strlen(buff));
                write(1, " ", 1);
                itoa(new_y, buff);
                write(1, buff, strlen(buff));
                write(1, " with direction ", 15);
                char buff2[12];
                itoa(new_direction, buff2);
                write(1, buff2, strlen(buff2));
                write(1, "\n", 1);

                gotoxy(new_y + 10, new_x + 2);
                set_color(DEBUG_FOREGROUND, DEBUG_BACKGROUND);
                write(1, DEBUG_CODE, strlen(DEBUG_CODE));
                #endif

                if(rear < MAX_QUEUE_SIZE){
                    nodes[rear] = (struct node) { .x = new_x, .y = new_y, .direction = (current_node->direction == DT_NULL) ? new_direction : current_node->direction };
                    queue[rear++] = rear - 1;
                    visited[new_x][new_y] = 1;
                }
            }
        }
    }
    return DT_NULL;
}

void print_frame(struct t_game *game){
    //print_piece_map_pacman(game);
    //print_piece_map_phantom(game);
    print_map(game);
    print_units(game);
    frames++;
    int temps = gettime() - offset_actual;
    int segundos = temps / (18 * 60);
    int fps = 0;
    if(segundos > 0) fps = frames / segundos;
    char c;
    itoa(fps, c);
    gotoxy(1, 1);
    write(1, c, strlen(c));
    if (fps < 100){
        set_color(SURROUND_FOREGROUND, SURROUND_BACKGROUND);
        gotoxy(3, 1);
        char *s = " ";
        write(1, s, 1);
    }
    set_color(PHANTOM4_FOREGROUND, PHANTOM4_BACKGROUND);
    gotoxy(72,5);
    char buff3[5];
    itoa(game->points, buff3);
    write(1, buff3, strlen(buff3));
}

void print_piece_map_pacman(struct t_game *game){
    int vertical = 0;
    if(game->pacman.dir == DT_DOWN || game->pacman.dir == DT_UP) vertical = 1;
    if(vertical){
        for(int i = 0; i < ROWS; ++i){
            gotoxy(10+game->pacman.y, 2+i);
            enum t_cell cell = game->Matrix[i][game->pacman.y];
            if (cell == CT_WALL){
                set_color(WALL_FOREGROUND, WALL_BACKGROUND);
                write(1, WALL_CODE, strlen(WALL_CODE));
            }
            else if (cell == CT_FOOD) {
                set_color(FOOD_FOREGROUND, FOOD_BACKGROUND);
                write(1, FOOD_CODE, strlen(FOOD_CODE));
            }
            else if (cell == CT_EMPTY) {
                set_color(EMPTY_FOREGROUND, EMPTY_BACKGROUND);
                write(1, EMPTY_CODE, strlen(EMPTY_CODE));
            }
        }
    } else {
        for(int i = 0; i < COLUMNS; ++i){
            gotoxy(10+i, 2+game->pacman.x);
            enum t_cell cell = game->Matrix[game->pacman.x][i];
            if (cell == CT_WALL){
                set_color(WALL_FOREGROUND, WALL_BACKGROUND);
                write(1, WALL_CODE, strlen(WALL_CODE));
            }
            else if (cell == CT_FOOD) {
                set_color(FOOD_FOREGROUND, FOOD_BACKGROUND);
                write(1, FOOD_CODE, strlen(FOOD_CODE));
            }
            else if (cell == CT_EMPTY) {
                set_color(EMPTY_FOREGROUND, EMPTY_BACKGROUND);
                write(1, EMPTY_CODE, strlen(EMPTY_CODE));
            }
        }
    }
    return;
}

void print_piece_map_phantom(struct t_game *game){
    for(int j = 0; j < NPHANTOM; ++j){
        int vertical = 0;
        if(game->phantom[j].dir == DT_DOWN || game->phantom[j].dir == DT_UP) vertical = 1;
        if(vertical){
            for(int i = 0; i < ROWS; ++i){
                gotoxy(10+game->phantom[j].x, 2+i);
                enum t_cell cell = game->Matrix[i][game->phantom[j].x];
                if (cell == CT_WALL){
                    set_color(WALL_FOREGROUND, WALL_BACKGROUND);
                    write(1, WALL_CODE, strlen(WALL_CODE));
                }
                else if (cell == CT_FOOD) {
                    set_color(FOOD_FOREGROUND, FOOD_BACKGROUND);
                    write(1, FOOD_CODE, strlen(FOOD_CODE));
                }
                else if (cell == CT_EMPTY) {
                    set_color(EMPTY_FOREGROUND, EMPTY_BACKGROUND);
                    write(1, EMPTY_CODE, strlen(EMPTY_CODE));
                }
            }
        } else {
            for(int i = 0; i < COLUMNS; ++i){
                gotoxy(10+i, game->phantom[j].y +2);
                enum t_cell cell = game->Matrix[game->phantom[j].y][i];
                if (cell == CT_WALL){
                    set_color(WALL_FOREGROUND, WALL_BACKGROUND);
                    write(1, WALL_CODE, strlen(WALL_CODE));
                }
                else if (cell == CT_FOOD) {
                    set_color(FOOD_FOREGROUND, FOOD_BACKGROUND);
                    write(1, FOOD_CODE, strlen(FOOD_CODE));
                }
                else if (cell == CT_EMPTY) {
                    set_color(EMPTY_FOREGROUND, EMPTY_BACKGROUND);
                    write(1, EMPTY_CODE, strlen(EMPTY_CODE));
                }
            }
        }
        return;
    }
}

void print_map(struct t_game *game) {
    for (int i = 0; i < ROWS; i++) {
        gotoxy(10, 2+i);
        for (int j = 0; j < COLUMNS; j++) {
            char *code;
            enum t_cell cell = game->Matrix[i][j];
            if (cell == CT_WALL){
                set_color(WALL_FOREGROUND, WALL_BACKGROUND);
                write(1, WALL_CODE, strlen(WALL_CODE));
            }
            else if (cell == CT_FOOD) {
                set_color(FOOD_FOREGROUND, FOOD_BACKGROUND);
                write(1, FOOD_CODE, strlen(FOOD_CODE));
            }
            else if (cell == CT_EMPTY) {
                set_color(EMPTY_FOREGROUND, EMPTY_BACKGROUND);
                write(1, EMPTY_CODE, strlen(EMPTY_CODE));
            }
        }
    }
}

void print_units(struct t_game *game){

    gotoxy(game->pacman.y + 10, game->pacman.x + 2);
    set_color(PACMAN_FOREGROUND, PACMAN_BACKGROUND);
    write(1, PACMAN_CODE, strlen(PACMAN_CODE));
    
    for(int i = 0; i < NPHANTOM; ++i){
        gotoxy(game->phantom[i].x + 10, game->phantom[i].y + 2);
        if(i == 0) set_color(PHANTOM1_FOREGROUND, PHANTOM1_BACKGROUND);
        if(i == 1) set_color(PHANTOM2_FOREGROUND, PHANTOM2_BACKGROUND);
        if(i == 2) set_color(PHANTOM3_FOREGROUND, PHANTOM3_BACKGROUND);
        if(i == 3) set_color(PHANTOM4_FOREGROUND, PHANTOM4_BACKGROUND);
        write(1, PHANTOM_CODE, strlen(PHANTOM_CODE));
    }

    
}

void clear_screen() {
    set_color(SURROUND_FOREGROUND, SURROUND_BACKGROUND);
    gotoxy(0, 0);
    char *s = " ";
    for (int i = 0; i < 25 * 80; i++) {
        write(1, s, 1);
    }
}

void read_keyboard(int* shared_mem_addr){
    while(1){
        if(*shared_mem_addr == 0){
            char b[1];
            read(b, 1);
            if(b[0] == 'w') *shared_mem_addr = DT_UP;
            else if(b[0] == 's') *shared_mem_addr = DT_DOWN;
            else if(b[0] == 'a') *shared_mem_addr = DT_LEFT;
            else if(b[0] == 'd') *shared_mem_addr = DT_RIGHT;
            b[0] = 0;
        } else {
            yield();
        }
    }
}

void move_phantoms(struct t_game *game){
    for(int i = 0; i < NPHANTOM; i++){
        int distance = 10000;
        int px = game->phantom[i].x;
        int py = game->phantom[i].y;
        int pmx = game->pacman.y;
        int pmy = game->pacman.x;
        if(px > pmx){
            if(py > pmy){
                distance = (px - pmx) + (py - pmy);
            } else {
                distance = (px - pmx) + (pmy - py);
            }
        } else {
            if(py > pmy){
                distance = (pmx - px) + (py - pmy);
            } else {
                distance = (pmx - px) + (pmy - py);
            }
        }
        distance = distance - 10;

        if(distance <= 17){
            int indice = random%10;
            //Phantom1 movement
            if ( i == 0 && phantom1[indice]%2 != 0){
                if ( random < 2) game->phantom[i].dir = DT_LEFT;
                else{
                    game->phantom[i].dir = bfs(game, game->phantom[i].y, game->phantom[i].x, game->pacman.x, game->pacman.y);
                    move(game, &game->phantom[i], game->phantom[i].x, game->phantom[i].y);
                }
            }
            else if ( i == 0 && phantom1[indice]%2 != 0 ) game->phantom[i].dir = DT_NULL;

            //Phantom2 movement
            else if ( i == 1 && phantom2[indice]%2 == 0){
                if ( random < 4) game->phantom[i].dir = DT_LEFT;
                else{
                    game->phantom[i].dir = bfs(game, game->phantom[i].y, game->phantom[i].x, game->pacman.x, game->pacman.y);
                    move(game, &game->phantom[i], game->phantom[i].x, game->phantom[i].y);
                }
            }
            else if ( i == 1 && phantom2[indice]%2 != 0 ) game->phantom[i].dir = DT_NULL;

            //Phanthom3 movement
            else if ( i == 2 && phantom3[indice]%2 == 0){
                if(random < 1) game->phantom[i].dir = DT_LEFT;
                else{
                    game->phantom[i].dir = bfs(game, game->phantom[i].y, game->phantom[i].x, game->pacman.x, game->pacman.y);
                    move(game, &game->phantom[i], game->phantom[i].x, game->phantom[i].y);
                }
            }
            else if ( i == 2 && phantom3[indice]%2 != 0 ) game->phantom[i].dir = DT_NULL;

            //Phantom4 movement
            else if ( i == 3 && phantom4[indice]%2 == 0){
                game->phantom[i].dir = bfs(game, game->phantom[i].y, game->phantom[i].x, game->pacman.x, game->pacman.y);
                move(game, &game->phantom[i], game->phantom[i].x, game->phantom[i].y);
            }
            else if ( i == 3 && phantom4[indice]%2 != 0 ) game->phantom[i].dir = DT_NULL;

            random++;
        }
    }
}

/*int cell_ocupado(struct t_game *game, int nx, int ny, int id_phantom){
    int result = 0;
    for ( int i = 0; i < NPHANTOM; ++i){
        if(id_phantom != i){
            if (game->phantom[i].x == nx && game->phantom[i].y == ny) result=1;
        }
    }
    return result;
}*/
//AQUI ESTA EL PROBLEMA DE QUE NO SE MUEVA BIEN
int is_possible_move(struct t_game *game, int x, int y, int dir){
    if(dir == DT_UP) x -= 1;
    else if(dir == DT_DOWN) x += 1;
    else if(dir == DT_LEFT) y -= 1;
    else if(dir == DT_RIGHT) y += 1;

    return (x >= 0 && x < COLUMNS && y >= 0 && y < ROWS && (game->Matrix[y][x] == CT_EMPTY || game->Matrix[y][x] == CT_FOOD));
}

int is_viable(struct list_head *curr_lhpos, struct t_mov *curr_pos, struct t_game *game){
    int current_x = game->pacman.x;
    int current_y = game->pacman.y;
    if(curr_pos->direction == DT_LEFT) {
        if(game->Matrix[current_x][current_y - 1] != CT_WALL){
            list_del(curr_lhpos);
            return 1;
        }
    }
    else if(curr_pos->direction == DT_UP){
        if(game->Matrix[current_x - 1][current_y] != CT_WALL){
            list_del(curr_lhpos);
            return 1;
        }
    } else if(curr_pos->direction == DT_DOWN){
        if(game->Matrix[current_x + 1][current_y] != CT_WALL){
            list_del(curr_lhpos);
            return 1;
        }
    } else if(curr_pos->direction == DT_RIGHT){
        if(game->Matrix[current_x][current_y + 1] != CT_WALL){
            list_del(curr_lhpos);
            return 1;
        }
    }
    return 0;
}

void move(struct t_game *game, struct t_unit *unidad, int current_x, int current_y){
    if(unidad->type == PACMAN_TYPE){
        if(unidad->dir == DT_LEFT){
            if(game->Matrix[current_x][current_y - 1] != CT_WALL){
                current_y--;
            }
        } else if(unidad->dir == DT_UP){
            if(game->Matrix[current_x - 1][current_y] != CT_WALL){
                current_x--;
            }
        } else if(unidad->dir == DT_DOWN){
            if(game->Matrix[current_x + 1][current_y] != CT_WALL){
                current_x++;
            }
        } else if(unidad->dir == DT_RIGHT){
            if(game->Matrix[current_x][current_y + 1] != CT_WALL){
                current_y++;
            }
        }
    } else {
        if(unidad->dir == DT_LEFT){
            current_x--;
        } else if(unidad->dir == DT_UP){
            current_y--;
        } else if(unidad->dir == DT_DOWN){
            current_y++;
        } else if(unidad->dir == DT_RIGHT){
            current_x++;
        }
    }
    unidad->x = current_x;
    unidad->y = current_y;
}

void start_game(struct t_game *game, int* shared_mem_addr){
    int curr_time = gettime();
    struct t_mov moviments;
    enum t_dir direction;
    while(lives > 0 && game->points < total*10){
        if(*shared_mem_addr != 0){
            direction = *shared_mem_addr;
            moviments = (struct t_mov) {.direction = direction};
            list_add_tail(&moviments.list, &movements);
            *shared_mem_addr = 0;
        }
        if(!list_empty(&movements)){
            struct list_head *curr_lhpos = list_first(&movements);
            struct t_mov *curr_pos = list_entry(curr_lhpos, struct t_mov, list);
            if(is_viable(curr_lhpos, curr_pos, game)) game->pacman.dir = curr_pos->direction;
            int current_x = game->pacman.x;
            int current_y = game->pacman.y;
            move(game, &game->pacman, current_x, current_y);


            if(game->Matrix[current_x][current_y] == CT_FOOD){
                game->points += 10;
                game->Matrix[current_x][current_y] = CT_EMPTY;
            }
        }
        move_phantoms(game);
        imprimir_coordenadas(game);
        for(int i = 0; i < NPHANTOM; ++i){
            if(game->pacman.y == game->phantom[i].x && game->pacman.x == game->phantom[i].y){
                lives--;
                restart(game);
                i = 5;
            }
        }
        
        print_frame(game);
        while(gettime() <= curr_time + DELAY){} // para que sea jugable
        curr_time = gettime();
    }
    clear_screen();
    end_game();
}


void imprimir_coordenadas(struct t_game *game){
    //Imprime COORDENADAS PACMAN
            gotoxy(0,2);
            char buff1[10];
            itoa(game->pacman.x, buff1);
            write(1,buff1, strlen(buff1));
            gotoxy(0,3);
            char buff2[10];
            itoa(game->pacman.y, buff2);
            write(1,buff2, strlen(buff2));

            //IMPRIME COORDENADAS PHANTOM4
            gotoxy(0,5);
            char buff3[10];
            itoa(game->phantom[0].x, buff3);
            write(1,buff3, strlen(buff3));
            gotoxy(0,6);
            char buff12[10];
            itoa(game->phantom[0].y, buff12);
            write(1,buff12, strlen(buff12));


            gotoxy(0,8);
            char buff4[10];
            itoa(game->phantom[1].x, buff4);
            write(1,buff4, strlen(buff4));
            gotoxy(0,9);
            char buff5[10];
            itoa(game->phantom[1].y, buff5);
            write(1,buff5, strlen(buff5));



            gotoxy(0,11);
            char buff6[10];
            itoa(game->phantom[2].x, buff6);
            write(1,buff6, strlen(buff6));
            gotoxy(0,12);
            char buff7[10];
            itoa(game->phantom[2].y, buff7);
            write(1,buff7, strlen(buff7));



            gotoxy(0,14);
            char buff8[10];
            itoa(game->phantom[3].x, buff8);
            write(1,buff8, strlen(buff8));
            gotoxy(0,15);
            char buff9[10];
            itoa(game->phantom[3].y, buff9);
            write(1,buff9, strlen(buff9));

            //Imprime las vidas
            gotoxy(0,17);
            char buff10[10];
            itoa(lives, buff10);
            write(1,buff10, strlen(buff10));
}

void end_game(){
    set_color(PACMAN_FOREGROUND, PACMAN_BACKGROUND);
    gotoxy(16, 12);
    if(lives > 0){
        char message1[60] = "CONGRATULATIONS, YOU'VE COMPLETED SOA :P SIUUU";
        write(1, message1, strlen(message1));
        return;

    }
    gotoxy(20,12);
    char message2[40] = "HAHAHA GAME OVER LOSER XD";
    write(1, message2, strlen(message2));
    return;
}

void restart(struct t_game *game){
    game->pacman.dir = DT_NULL;
    game->pacman.x = 1;
    game->pacman.y = 1;
    const int X_START = 47;
    const int Y_START = 17;
    for(int i = 0; i < NPHANTOM/2; ++i){
        for (int j = 0; j < NPHANTOM/2; j++) {
            game->phantom[i*2+j].x = X_START+i;
            game->phantom[i*2+j].y = Y_START + j;
            game->phantom[i*2+j].dir = DT_NULL;
        }
    }
    print_frame(game);
    gotoxy(71,3);
    char v;
    itoa(lives, v);
    write(1,v, 1 );  
}
