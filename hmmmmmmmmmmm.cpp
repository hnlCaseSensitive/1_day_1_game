//TRABALHO FINAL HENRIQUE LAVARDA
#include <stdio.h>
#include "raylib.h"

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600

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

//check de colisao entre retangulos, e a parede
Rectangle colisao_parede(Rectangle rec1, BLOCOS parede[], int direcao, int vel){
    for(int i = 0; i < 240; i++){
        if(CheckCollisionRecs(rec1, parede[i].rec)){
            switch(direcao){
                case 1:
                    rec1.x += vel;
                    break;
                case 2:
                    rec1.y += vel;
                    break;
                case 3:
                    rec1.x -= vel;
                    break;
                case 4:
                    rec1.y -= vel;
                    break;
                default:
                    break;
            }
        }
    }
    return rec1;
}

//check de colisao para os tiros
int colisao_parede_tiro(Rectangle rec1, BLOCOS parede[]){
    for(int i = 0; i < 240; i++){
        if(CheckCollisionRecs(rec1, parede[i].rec)){
            return i;
        }
    }
    return -1;
}

//funcao para spawnar em posicao aleatoria sem ter colisoes
Rectangle spawn_aleatorio(Rectangle rec, BLOCOS parede[]){
    do{
        rec.x = GetRandomValue(0, (SCREEN_WIDTH - rec.width));
        rec.y = GetRandomValue(0, (SCREEN_HEIGHT - rec.height));
    }while(colisao_parede_tiro(rec, parede) > 0);

    return rec;
}

//funcao para atirar
void atira(TIRO *gun, Rectangle atirador, char direcao){
    if(!gun->active){
        gun->active = true;
        gun->rec.x = atirador.x + (atirador.width / 2);
        gun->rec.y = atirador.y + (atirador.height / 2);
        switch(direcao){
            case 'A':
                gun->velx = -4;
                gun->vely = 0;
                break;
            case 'W':
                gun->velx = 0;
                gun->vely = -4;
                break;
            case 'D':
                gun->velx = 4;
                gun->vely = 0;
                break;
            case 'S':
                gun->velx = 0;
                gun->vely = 4;
                break;
            default:
                break;
            }
        }
}

//cria direcao aleatoria para os inimigos
char random_direction(void){
    char direc = 's';
    switch(GetRandomValue(1,4)){
        case 1:
            direc = 'A';
            break;
        case 2:
            direc = 'W';
            break;
        case 3:
            direc = 'D';
            break;
        case 4:
            direc = 'S';
            break;
        default:
            break;
    }
    return direc;
}

//achar o index das texturas conforme a direcao
int index_finder(char a){
    int direc = 0;
    switch(a){
        case 'A':
            direc = 0;
            break;
        case 'W':
            direc = 1;
            break;
        case 'D':
            direc = 2;
            break;
        case 'S':
            direc = 3;
            break;
        default:
            break;
    }
    return direc;
}

//salva a score em um arquivo txt
void salva_score(int score){
    FILE *pont;
    pont = fopen("pontuacao.txt", "a");

    if(pont != NULL){
        if(fprintf(pont, "score: %d", score) < 0)printf("erro na escrita");
        if(fprintf(pont, "\n") < 0)printf("erro na escrita");
    }else printf("erro na escrita");
    fclose(pont);
}

int jogo(void){

    int mapa[12][20] = {0};

    Color tr;
    tr.r = 255;
    tr.g = 255;
    tr.b = 255;
    tr.a = 120;

    SetTargetFPS(60);

    //bool para manter o jogo rodando e aumentando o lvl
    bool game_loop = true;

    int lvl = 1, kill_count = 0;

    PLAYER player1;
    BLOCOS parede[240];
    CELULA energia;
    ENEMY inimigos[10];

    //loading das imagens e transformando elas em texturas2d para poderem ser desenhadas na tela
    Image vida_imagem = LoadImage("./imgs/shield.png");
    Image jogador_image[4] = {LoadImage("./imgs/tank_azul_a.png"), LoadImage("./imgs/tank_azul_w.png"), LoadImage("./imgs/tank_azul_d.png"), LoadImage("./imgs/tank_azul_s.png")};
    Image inimigo_image[4] = {LoadImage("./imgs/tank_vermelho_a.png"), LoadImage("./imgs/tank_vermelho_w.png"), LoadImage("./imgs/tank_vermelho_d.png"), LoadImage("./imgs/tank_vermelho_s.png")};
    Image parede_image = LoadImage("./imgs/tijolo.png");
    Image energia_image = LoadImage("./imgs/buff.png");
    //resize para as texturas terem o  mesmo tamanho dos objetos
    for(int i = 0; i < 4; i++){
            ImageResize(&jogador_image[i], 50, 50);
            ImageResize(&inimigo_image[i], 50, 50);
    }
    ImageResize(&parede_image, 50, 50);
    ImageResize(&energia_image, 20, 20);
    ImageResize(&vida_imagem, 15, 15);
    Texture2D jogador_textura[4] = {LoadTextureFromImage(jogador_image[0]), LoadTextureFromImage(jogador_image[1]), LoadTextureFromImage(jogador_image[2]), LoadTextureFromImage(jogador_image[3])};
    Texture2D inimigo_textura[4] = {LoadTextureFromImage(inimigo_image[0]), LoadTextureFromImage(inimigo_image[1]), LoadTextureFromImage(inimigo_image[2]), LoadTextureFromImage(inimigo_image[3])};
    Texture2D parede_textura = LoadTextureFromImage(parede_image);
    Texture2D energia_textura = LoadTextureFromImage(energia_image);
    Texture2D vida_textura = LoadTextureFromImage(vida_imagem);
    //unload das imagems para não ficar na memoria
    UnloadImage(parede_image);
    UnloadImage(energia_image);
    UnloadImage(vida_imagem);
    for(int i = 0; i < 4; i++){
            UnloadImage(jogador_image[i]);
            UnloadImage(inimigo_image[i]);
    }

    player1.score = 0;

    while(game_loop){

        if(lvl > 10)lvl = 10;

        kill_count = 0;

        //colocando a matrix do mapa de volta ao estado em branco
        for(int i = 0; i < 12; i++){
            for(int j = 0; j < 20; j++){
                mapa[i][j] = 0;
            }
        }
        //ler um arquivo de texto aleatorio e carregar o mapa apartir dele
    //--------
        char nomes[4][40] = {"./maps/mapa1.txt","./maps/mapa2.txt","./maps/mapa3.txt","./maps/mapa4.txt"};
        FILE *fp;
        fp = fopen(nomes[GetRandomValue(0, 3)], "r");
        char buffer;
        int pos_x = 0, pos_y = 0;
        if(fp != NULL){
            while(!feof(fp)){
                buffer = fgetc(fp);
                if(buffer < 0){
                    if(!feof(fp))printf("erro na leitura");
                }else{
                    if(buffer == '\n'){
                        pos_y++;
                        pos_x = 0;
                    }else if(buffer == '#'){
                        mapa[pos_y][pos_x] = 1;
                    }
                }
                pos_x++;
            }
        }else printf("erro na abertura do arquivo.");
        fclose(fp);
    //----------

        //declaração das variaveis do jogador, dentro do loop para reiniciar dps do fim do lvl
        player1.rec.height = 50;
        player1.rec.width = 50;
        player1.alive = true;
        player1.lives = 3;
        player1.rec.x = SCREEN_WIDTH / 2;
        player1.rec.y = SCREEN_HEIGHT / 2;
        player1.vel = 4;
        player1.gun.rec.width = 10;
        player1.gun.rec.height = 10;
        player1.energetico = false;

        //declaração das variaveis dos blocos de acordo com o mapa que foi carregado
        for(int i = 0; i < 12; i++){
            for(int j = 0; j < 20; j++){
                parede[(i*20)+j].rec.width = 50;
                parede[(i*20)+j].rec.height = 50;
                if(mapa[i][j] != 0){
                    parede[(i*20)+j].active = true;
                    parede[(i*20)+j].rec.x = (j * 50);
                    parede[(i*20)+j].rec.y = (i* 50);
                }else{
                    parede[(i*20)+j].active = false;
                    parede[(i*20)+j].rec.x = -100;
                    parede[(i*20)+j].rec.y = -100;
                }
            }
        }

        //declaração da cedula de energia
        energia.active = false;
        energia.rec.height = 20;
        energia.rec.width = 20;
        energia.rec.x = -150;
        energia.rec.y = -150;

        //declaração dos inimigos ativos: eles vão spawnando de acordo com o lvl e param no 10
        for(int i = 0; i < lvl; i++){
            inimigos[i].alive = true;
            inimigos[i].rec.height = 40;
            inimigos[i].rec.width = 40;
            inimigos[i].rec = spawn_aleatorio(inimigos[i].rec, parede);
            inimigos[i].lives = lvl;
            inimigos[i].vel = 2;
            inimigos[i].direction = random_direction();
            inimigos[i].gun.rec.height = 10;
            inimigos[i].gun.rec.width = 10;
        }
        //removendo os inimigos que não fazem parte do lvl para fora da tela
        for(int i = lvl; i < 10; i++){
            inimigos[i].alive = false;
            inimigos[i].rec.height = 40;
            inimigos[i].rec.width = 40;
            inimigos[i].rec.x = -200;
            inimigos[i].rec.y = -200;
            inimigos[i].lives = lvl;
            inimigos[i].vel = 0;
        }

        while(!WindowShouldClose()){

            //caso de morte do player
            if(player1.lives <= 0){
                    game_loop = false;
                    //funcao para salvar o score em um arquivo texto
                    salva_score(player1.score);
                    break;
            }

            //check para ver se o player matou todos os inimigos
            if(kill_count == lvl)break;

            //valor aleatorio para ver se uma celula de energia é spawnada
            if(GetRandomValue(0,240) == 5){
                energia.active = true;
                energia.timer = 600;
                //funcao para obter uma posição aleatoria que não colide com nenhum bloco
                energia.rec = spawn_aleatorio(energia.rec, parede);
            }

            //logica da energia diminuindo o timer para desaparecer depois de 10 segundos
            if(energia.active){
                energia.timer--;
                //remove a celula da tela e a torna desativada
                if(energia.timer == 0){
                    energia.rec.x = -150;
                    energia.rec.y = -150;
                    energia.active = false;
                }
                //caso o player colida com a celula ele ganha os buffs
                if(CheckCollisionRecs(player1.rec, energia.rec)){
                    energia.active = false;
                    player1.vel = 8;
                    player1.timer = 600;
                    player1.energetico = true;
                }
            }
            //botao para fechar o jogo
            if(IsKeyPressed(KEY_ESCAPE)){
                    game_loop = false;
                    break;
            }

            //timer para remover o buff da celula de energia do player
            if(player1.energetico){
                player1.timer--;
                if(player1.timer == 0){
                    player1.energetico = false;
                    player1.vel /= 2;
                }
            }

            //checka para ver se o jogador tirou o dedo do ultimo botao pressionado para parar de mover o tanke
            if(IsKeyReleased(player1.last_key_pressed))player1.movement = false;

            //checka qual botao foi o ultimo pressionado para determinar direção e adiciona o movemento do player como verdadeiro
            if(IsKeyPressed(KEY_A)){
                player1.last_key_pressed = 'A';
                player1.movement = true;
            }
            if(IsKeyPressed(KEY_W)){
                player1.last_key_pressed = 'W';
                player1.movement = true;
            }
            if(IsKeyPressed(KEY_D)){
                player1.last_key_pressed = 'D';
                player1.movement = true;
            }
            if(IsKeyPressed(KEY_S)){
                player1.last_key_pressed = 'S';
                player1.movement = true;
            }

            //move o tank do player e faz os check de colisões contra parades e para nao sair da tela
            if(player1.movement){
                switch(player1.last_key_pressed){
                    case 'A':
                        player1.rec.x -= player1.vel;
                        player1.rec = colisao_parede(player1.rec, parede, 1, player1.vel);
                        if(player1.rec.x < 0)player1.rec.x += player1.vel;
                        break;
                    case 'W':
                        player1.rec.y -= player1.vel;
                        player1.rec = colisao_parede(player1.rec, parede, 2, player1.vel);
                        if(player1.rec.y < 0)player1.rec.y += player1.vel;
                        break;
                    case 'D':
                        player1.rec.x += player1.vel;
                        player1.rec = colisao_parede(player1.rec, parede, 3, player1.vel);
                        if(player1.rec.x + 50 > SCREEN_WIDTH)player1.rec.x -= player1.vel;
                        break;
                    case 'S':
                        player1.rec.y += player1.vel;
                        player1.rec = colisao_parede(player1.rec, parede, 4, player1.vel);
                        if(player1.rec.y + 50 > SCREEN_HEIGHT)player1.rec.y -= player1.vel;
                        break;
                    default:
                        break;
                }

            }

            //mecanismo para atirar
            if(IsKeyPressed(KEY_SPACE)){
                atira(&player1.gun, player1.rec, player1.last_key_pressed);
            }

            //funcao que calcula a posição e faz check de colisao do tiro do player
            if(player1.gun.active){
                player1.gun.rec.x += player1.gun.velx;
                player1.gun.rec.y += player1.gun.vely;
                int colisao = colisao_parede_tiro(player1.gun.rec, parede);
                if(colisao >= 0){
                    parede[colisao].active =  false;
                    parede[colisao].rec.x =  -100;
                    parede[colisao].rec.y =  -100;
                    player1.gun.active = false;
                    player1.gun.rec.x = -50;
                    player1.gun.rec.y = -50;
                }
                if(player1.gun.rec.x < 0 || player1.gun.rec.y < 0 || player1.gun.rec.x + player1.gun.rec.width > SCREEN_WIDTH || player1.gun.rec.y + player1.gun.rec.height > SCREEN_HEIGHT){
                        player1.gun.active = false;
                        player1.gun.rec.x = -50;
                        player1.gun.rec.y = -50;
                }
            }

            //ajusta a posicao do inimigo, faz check de colisão contra parede e bordas da tela,
            //funcao random_direction chama uma direção aleatoria
            for(int i = 0; i < lvl; i++){
                if(inimigos[i].alive){
                    switch(inimigos[i].direction){
                        case 'A':
                            inimigos[i].rec.x -= inimigos[i].vel;
                            for(int j = 0; j < 240; j++){
                                if(CheckCollisionRecs(inimigos[i].rec, parede[j].rec)){
                                    inimigos[i].rec.x += inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                                }
                            }
                            if(inimigos[i].rec.x < 0){
                                    inimigos[i].rec.x += inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                            }
                            break;
                        case 'W':
                            inimigos[i].rec.y -= inimigos[i].vel;
                            for(int j = 0; j < 240; j++){
                                if(CheckCollisionRecs(inimigos[i].rec, parede[j].rec)){
                                    inimigos[i].rec.y += inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                                }
                            }
                            if(inimigos[i].rec.y < 0){
                                    inimigos[i].rec.y += inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                            }
                            break;
                        case 'D':
                            inimigos[i].rec.x += inimigos[i].vel;
                            for(int j = 0; j < 240; j++){
                                if(CheckCollisionRecs(inimigos[i].rec, parede[j].rec)){
                                    inimigos[i].rec.x -= inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                                }
                            }
                            if(inimigos[i].rec.x + inimigos[i].rec.width > SCREEN_WIDTH){
                                    inimigos[i].rec.x -= inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                            }
                            break;
                        case 'S':
                            inimigos[i].rec.y += inimigos[i].vel;
                            for(int j = 0; j < 240; j++){
                                if(CheckCollisionRecs(inimigos[i].rec, parede[j].rec)){
                                    inimigos[i].rec.y -= inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                                }
                            }
                            if(inimigos[i].rec.y + inimigos[i].rec.height > SCREEN_HEIGHT){
                                    inimigos[i].rec.y -= inimigos[i].vel;
                                    inimigos[i].direction = random_direction();
                            }
                            break;
                        default:
                            break;
                    }

                    //modo de perseguição para caso o player esteja em uma linha reta com o inimigo o tal seguilo e disparar
                    if(inimigos[i].rec.x < player1.rec.x + 25 && inimigos[i].rec.x > player1.rec.x - 25){
                        if(inimigos[i].rec.y > player1.rec.y){
                            inimigos[i].direction = 'W';
                            atira(&inimigos[i].gun, inimigos[i].rec, inimigos[i].direction);
                        }else {
                            inimigos[i].direction = 'S';
                            atira(&inimigos[i].gun, inimigos[i].rec, inimigos[i].direction);
                        }
                    }
                    if(inimigos[i].rec.y < player1.rec.y + 25 && inimigos[i].rec.y > player1.rec.y - 25){
                        if(inimigos[i].rec.x > player1.rec.x){
                            inimigos[i].direction = 'A';
                            atira(&inimigos[i].gun, inimigos[i].rec, inimigos[i].direction);
                        }else {
                            inimigos[i].direction = 'D';
                            atira(&inimigos[i].gun, inimigos[i].rec, inimigos[i].direction);
                        }
                    }

                    //check de colisao contra os tiros do player
                    if(CheckCollisionRecs(inimigos[i].rec, player1.gun.rec)){
                        inimigos[i].alive = false;
                        inimigos[i].rec.x = -200;
                        inimigos[i].rec.y = -200;
                        player1.score += 100;
                        player1.gun.active = false;
                        player1.gun.rec.x = -100;
                        player1.gun.rec.y = -100;
                        kill_count++;
                    }

                    //check de colisao contra o tank do player
                    if(CheckCollisionRecs(inimigos[i].rec, player1.rec)){
                        inimigos[i].alive = false;
                        inimigos[i].rec.x = -200;
                        inimigos[i].rec.y = -200;
                        player1.lives--;
                        player1.score += 100;
                        kill_count++;
                    }
                }

                //calcula posicao, check de colisoes e contra bordas dos tiros dos inimigos
                if(inimigos[i].gun.active){
                    inimigos[i].gun.rec.x += inimigos[i].gun.velx;
                    inimigos[i].gun.rec.y += inimigos[i].gun.vely;
                    int colisao = colisao_parede_tiro(inimigos[i].gun.rec, parede);
                    if(colisao >= 0){
                        parede[colisao].active =  false;
                        parede[colisao].rec.x =  -100;
                        parede[colisao].rec.y =  -100;
                        inimigos[i].gun.active = false;
                        inimigos[i].gun.rec.x = -50;
                        inimigos[i].gun.rec.y = -50;
                    }
                    if(inimigos[i].gun.rec.x < 0 || inimigos[i].gun.rec.y < 0 || inimigos[i].gun.rec.x + inimigos[i].gun.rec.width > SCREEN_WIDTH || inimigos[i].gun.rec.y + inimigos[i].gun.rec.height > SCREEN_HEIGHT){
                            inimigos[i].gun.active = false;
                            inimigos[i].gun.rec.x = -50;
                            inimigos[i].gun.rec.y = -50;
                    }
                    if(CheckCollisionRecs(inimigos[i].gun.rec, player1.rec)){
                        player1.lives--;
                        inimigos[i].gun.active = false;
                        inimigos[i].gun.rec.x = -50;
                        inimigos[i].gun.rec.y = -50;
                    }
                }
            }

            //inicio da renderização
            BeginDrawing();
                for(int i = 0; i < 10; i++){
                        //DrawRectangleRec(inimigos[i].rec, YELLOW);
                        DrawTexture(inimigo_textura[index_finder(inimigos[i].direction)], inimigos[i].rec.x, inimigos[i].rec.y , WHITE );
                        DrawRectangleRec(inimigos[i].gun.rec, WHITE);
                }
                if(energia.active)DrawTexture(energia_textura, energia.rec.x, energia.rec.y, WHITE); //DrawRectangleRec(energia.rec, PINK);
                //DrawRectangleRec(player1.rec, RED);
                DrawTexture(jogador_textura[index_finder(player1.last_key_pressed)], player1.rec.x, player1.rec.y , WHITE );
                if(player1.gun.active)DrawCircle(player1.gun.rec.x + (player1.gun.rec.width / 2), player1.gun.rec.y + (player1.gun.rec.height / 2), player1.gun.rec.height / 2, GREEN);
                for(int i = 0; i < 240; i++)DrawTexture(parede_textura, parede[i].rec.x, parede[i].rec.y, WHITE); //DrawRectangleRec(parede[i].rec, BLUE);
                for(int i = 0; i < player1.lives; i++)DrawTexture(vida_textura, player1.rec.x + 55 + (i*15), player1.rec.y, WHITE); //DrawRectangle(player1.rec.x + 55 + (i*6), player1.rec.y, 5, 5, ORANGE);
                ClearBackground(BLACK);
            EndDrawing();
        }
        //aumenta o nivel e reinicia o jogo
        lvl++;
    }

    //descarrega as texturas e fecha o programa
    for(int i = 0; i < 4; i++){
            UnloadTexture(jogador_textura[i]);
            UnloadTexture(inimigo_textura[i]);
    }
    UnloadTexture(parede_textura);
    UnloadTexture(energia_textura);
    UnloadTexture(vida_textura);

    return 0;
}


int main(void){


    int menuIndex = 1;
    char options[5][50] = {{"jogo de tank sla"}, {"Start"}, {"HighScores"}, {"Settings"} ,{"End"}};
    int fz = 40;
    int selected = 0;
    Color cor1 = GRAY;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "titleScreen");

    Image thingy = LoadImage("./imgs/image_to_load.png");
    ImageResize(&thingy, 1000, 700);
    Texture2D texture = LoadTextureFromImage(thingy);
    UnloadImage(thingy);

    SetTargetFPS(60);


    while(!WindowShouldClose()){

        if(IsKeyPressed(KEY_W)){
            if(menuIndex > 1){
                menuIndex--;
            }else{
                menuIndex = 4;
            }
        }
        if(IsKeyPressed(KEY_S)){
            if(menuIndex < 4){
                menuIndex++;
            }else{
                menuIndex = 1;
            }
        }

        if(IsKeyPressed(KEY_ENTER)){
            switch(menuIndex){
                case 1:
                    selected = 1;
                    jogo();
                    break;
                case 2:
                    selected = 2;
                    break;
                case 3:
                    selected = 3;
                    break;
                case 4:
                    selected = 4;
                    break;
            }
        }

        if(selected == 4)break;

        BeginDrawing();
            ClearBackground(WHITE);

            DrawTexture(texture, SCREEN_WIDTH/2 - texture.width/2, SCREEN_HEIGHT/2 - texture.height/2 - 40, WHITE);

            DrawText(options[0], SCREEN_WIDTH/2 - MeasureText(options[0], 70)/2, 100, 70, PINK);

            for(int i = 1; i < 5; i++){
                    cor1 = GRAY;
                    if(i == menuIndex)cor1 = YELLOW;
                DrawText(options[i], SCREEN_WIDTH/2 - MeasureText(options[i], fz)/2, 300 + (i * 50), fz, cor1);

            }

            if(selected == 2){
                DrawText("abre o arquivo pontuacao.txt e olha", SCREEN_WIDTH/2 - MeasureText("abre o arquivo pontuacao.txt e olha", fz)/2, 200, fz, RED);
            }

        EndDrawing();
    }

    UnloadTexture(texture);

    CloseWindow();

    return 0;
}
