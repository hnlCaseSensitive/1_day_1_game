typedef struct bullet {
    Rectangle rec;
    bool active;
    int velx, vely;
}TIRO;

typedef struct walls {
    Rectangle rec;
    bool active;
}BLOCOS;

typedef struct player {
    bool alive, movement, energetico;
    Rectangle rec;
    char last_key_pressed;
    int vel, score, lives, timer;
    TIRO gun;
}PLAYER;

typedef struct enemy {
    bool alive, needs_direction;
    int lives, vel;
    char direction;
    Rectangle rec;
    TIRO gun;
}ENEMY;

typedef struct power_up {
    Rectangle rec;
    bool active;
    int timer;
}CELULA;
