#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#define RECT_W 64
#define RECT_H 64
#define ENEMY_OFFSET 320
#define BOARD_OFFSET_X 100
#define BOARD_OFFSET_Y 100
#define BOARD_SIZE_X 640
#define BOARD_SIZE_Y 640
#define NUM_TOWERS 5

/*
TODO:

    * Make an array[NUM_TOWERS] surfaces so I don't load the surface every time, and free them
    at the end
    * Make towers upgradable
    * Make the tower red when going off board right/bottom
    * Make an array[NUM_TOWERS] normal and red so you dont init the textures every time,
    but gotta find a workaround against the if(ptr) stuff if we're not freeing, probably a bool
    * Change 3rd tower background color
    * When dropping tower and you're over another tower or it blocks the path, make it red
    * Pathfinding algorithm for the tower matrix board
    * Split into multiple files
    * Put stuff into structs and get rid of global variables as much as possible
    * Import fonts to use
    * Show the price under the towers
    * Decrease the player money upon placing the tower
    * Main Screen
    * Save Game feature
    * Top Scores feature

    ... more
DONE:
    * When dropping tower, make sure it's within the board, otherwise make it red
    * Make the rectangle grip point in the middle when picking tower
    * If tower is placed, onmouseover show its range
    ... more
*/
typedef enum TD_TowerType
{
    TD_TOWER_NONE,
    TD_TOWER_1,
    TD_TOWER_2,
    TD_TOWER_3,
    TD_TOWER_4,
    TD_TOWER_5
}TD_TowerType;

typedef struct TD_Tower
{
    SDL_Texture *towerTexture;
    TD_TowerType towerType;
    SDL_Rect     towerRect;
    Uint16       towerCost;
    Uint16       towerDamagePerHit;
    Uint16       towerRadius;
}TD_Tower;

typedef struct TD_Enemy
{
    Uint16 health;
    SDL_Rect rect;
}TD_Enemy;

typedef struct TD_GameState
{
    TD_Tower towerMap[BOARD_SIZE_X/RECT_W][BOARD_SIZE_Y/RECT_H];
    Uint32 level;
    Uint32 lifes;
    Uint32 money;
    Uint32 score;
}TD_GameState;


SDL_bool running;
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *backgroundTexture;
SDL_Texture *towerTexture[NUM_TOWERS], *towerSelectedTexture[NUM_TOWERS];
SDL_Texture *towerTempTextureNormal, *towerTempTextureRed;
SDL_Texture *enemyTexture;
SDL_Texture *towerRadiusTexture;
SDL_Rect enemyDestR, towerDestR[NUM_TOWERS], towerTempDestR, mapDestR;
SDL_bool insideTowerTexture[NUM_TOWERS];

TTF_Font *font24;
int countX, countY;
int mouseX, mouseY;
int selectedTower;
int frames;
TD_GameState gameState;
TD_TowerType currentTower;

Uint16 towerCosts[NUM_TOWERS] = {50, 100, 250, 500, 1200};
Uint16 towerDamagePerHit[NUM_TOWERS] = {10, 20, 50, 100, 240};
Uint16 towerRadius[NUM_TOWERS] = {180, 250, 290, 320, 350};

void TD_DisplayText(const char* msg, int x, int y)
{
    SDL_Surface *tmpSurface;
    SDL_Texture *tmpTexture;
    SDL_Color color = {.r = 0, .g = 0, .b = 0, .a = 255};
    SDL_Rect rect;

    tmpSurface = TTF_RenderText_Solid(font24, msg, color);
    tmpTexture = SDL_CreateTextureFromSurface(renderer, tmpSurface);

    rect.x = x; 
    rect.y = y;
    rect.w = tmpSurface->w; 
    rect.h = tmpSurface->h;

    SDL_FreeSurface(tmpSurface);
    SDL_RenderCopy(renderer, tmpTexture, NULL, &rect);
    SDL_DestroyTexture(tmpTexture); 
}

SDL_Texture* TD_CreateTextureFromImage(SDL_Renderer* ren, const char *path)
{
    SDL_Surface *tmpSurface;
    SDL_Texture *texture;
    tmpSurface = IMG_Load(path);
    texture = SDL_CreateTextureFromSurface(ren, tmpSurface);
    SDL_FreeSurface(tmpSurface);
    return texture;
}

/* cleaning up everything that's left when the game is over */
void TD_Quit(void)
{
    // FILE *fp = fopen("matrix.txt", "w");
    SDL_DestroyWindow(window);

    for(int i = 0; i < BOARD_SIZE_X/RECT_W; i++)
    {
        for(int j = 0; j < BOARD_SIZE_Y/RECT_H; j++)
        {
            // fprintf(fp, "%d", gameState.towerMap[j][i].towerType);
            if(gameState.towerMap[i][j].towerTexture)
            {
                SDL_DestroyTexture(gameState.towerMap[i][j].towerTexture);
            }
        }
        // fprintf(fp, "\n");
    }

    for(int i = 0; i < NUM_TOWERS; i++)
    {
        SDL_DestroyTexture(towerTexture[i]);
        SDL_DestroyTexture(towerSelectedTexture[i]);
    }

    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(towerRadiusTexture);
    SDL_DestroyRenderer(renderer);
    
    TTF_CloseFont(font24);
    SDL_Quit();
    // fclose(fp);
    printf("Game Cleaned\n");
}


/* setting up stuff when the game starts */
void TD_Init(const char *title, int xpos, int ypos, int width, int height, SDL_bool fullscreen)
{
    Uint32 flags = 0;

    if(fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        running = SDL_FALSE;
        return;
    }

    selectedTower = -1;
    frames = 0;
    window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
    renderer = SDL_CreateRenderer(window,-1,0);
    running = SDL_TRUE;

    backgroundTexture = TD_CreateTextureFromImage(renderer, "assets/background.png");
    enemyTexture = TD_CreateTextureFromImage(renderer, "assets/enemy.png");
 

    for(int i = 0; i < NUM_TOWERS; i++)
    {
        char path[255];

        sprintf(path,"assets/tower%d.png", i+1);
        towerTexture[i] = TD_CreateTextureFromImage(renderer, path);

        sprintf(path,"assets/towerselected%d.png", i+1);
        towerSelectedTexture[i] = TD_CreateTextureFromImage(renderer, path);
    }

    towerTempTextureNormal = NULL;
    towerTempTextureRed = NULL;


    countX = countY = 0;

    enemyDestR.h = RECT_H;
    enemyDestR.w = RECT_W;
    enemyDestR.y = ENEMY_OFFSET + BOARD_OFFSET_Y;

    mapDestR.x = BOARD_OFFSET_X;
    mapDestR.y = BOARD_OFFSET_Y;
    mapDestR.w = BOARD_SIZE_X;
    mapDestR.h = BOARD_SIZE_Y;

    int xPos = BOARD_OFFSET_X + RECT_W * 4;
    int yPos = BOARD_OFFSET_Y + BOARD_SIZE_Y;
    for(int i = 0; i < NUM_TOWERS; i++)
    {
        towerDestR[i].x = xPos;
        towerDestR[i].y = yPos;
        towerDestR[i].w = RECT_W;
        towerDestR[i].h = RECT_H;
        xPos += RECT_W;
    }

    gameState.level = 1;
    gameState.lifes = 20;
    gameState.money = 10000;
    gameState.score = 0;

    xPos = BOARD_OFFSET_X;
    yPos = BOARD_OFFSET_Y;
    for(int i = 0; i < BOARD_SIZE_X/RECT_W; i++)
    {
        for(int j = 0; j < BOARD_SIZE_Y/RECT_H; j++)
        {
            gameState.towerMap[i][j].towerTexture = NULL;
            gameState.towerMap[i][j].towerType = TD_TOWER_NONE;
            gameState.towerMap[i][j].towerRect.w = RECT_W;
            gameState.towerMap[i][j].towerRect.h = RECT_H;
            gameState.towerMap[i][j].towerRect.x = xPos + i * RECT_W;
            gameState.towerMap[i][j].towerRect.y = yPos + j * RECT_H;
        }
    }

    towerTempDestR.w = RECT_W;
    towerTempDestR.h = RECT_H;

    currentTower = TD_TOWER_NONE;

    TTF_Init();
    font24 = TTF_OpenFont("Sans.ttf", 24);

    towerRadiusTexture = TD_CreateTextureFromImage(renderer, "assets/towerradius.png");
}


SDL_bool TD_InsideRect(int mouseX, int mouseY, SDL_Rect *rect)
{
    if(mouseX >= rect->x && mouseX <= (rect->x + rect->w))
        if(mouseY >= rect->y && mouseY <= (rect->y + rect->h))
            return SDL_TRUE;
    return SDL_FALSE;
}

void TD_HandleEvents(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
            {
                running = SDL_FALSE;
                break;
            } 
            case SDL_MOUSEMOTION:
            {
                SDL_GetMouseState(&mouseX, &mouseY);
                for(int i = 0; i < NUM_TOWERS; i++)
                {
                    if(insideTowerTexture[i])
                    {
                        if(!TD_InsideRect(mouseX, mouseY, &towerDestR[i]))
                        {
                            insideTowerTexture[i] = SDL_FALSE;
                            break;  
                        }
                    }
                }      
                for(int i = 0; i < NUM_TOWERS; i++)
                {
                    if(TD_InsideRect(mouseX, mouseY, &towerDestR[i]))
                    {
                        insideTowerTexture[i] = SDL_TRUE;
                        break;
                    }
                }
                // FIXME: don't iterate, use a vector to see if sth is selected
                for(int i = 0; i < BOARD_SIZE_X/RECT_W; i++)
                {
                    for(int j = 0; j < BOARD_SIZE_Y/RECT_H; j++)
                    {
                        if(gameState.towerMap[i][j].towerTexture == NULL)
                            continue;
                        if(selectedTower == -1 && TD_InsideRect(mouseX, mouseY, &gameState.towerMap[i][j].towerRect))
                        {
                            selectedTower = i * BOARD_SIZE_X/RECT_W + j;
                            break;
                        }
                    }
                }
                
                if(selectedTower != -1)
                {
                    int curr_i = selectedTower / (BOARD_SIZE_X/RECT_W);
                    int curr_j = selectedTower % (BOARD_SIZE_X/RECT_W);
                    if(!TD_InsideRect(mouseX, mouseY, &gameState.towerMap[curr_i][curr_j].towerRect))
                    {
                        selectedTower = -1;
                    }
                }
                if(towerTempTextureNormal)
                {
                    towerTempDestR.x = mouseX - towerTempDestR.w / 2;
                    towerTempDestR.y = mouseY - towerTempDestR.h / 2;
                }         
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                SDL_GetMouseState(&mouseX, &mouseY);

                if(event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED) 
                {
                    for(int i = 0; i < NUM_TOWERS; i++)
                    {
                        if(TD_InsideRect(mouseX, mouseY, &towerDestR[i]))
                        {
                            char path[255];

                            sprintf(path, "assets/tower%d.png", i+1);
                            towerTempTextureNormal = TD_CreateTextureFromImage(renderer, path);

                            sprintf(path, "assets/towerred%d.png", i+1);
                            towerTempTextureRed = TD_CreateTextureFromImage(renderer, path);

                            towerTempDestR.x = mouseX;
                            towerTempDestR.y = mouseY;
                            currentTower = i + 1;
                            break;
                        }
                    }
                }
                if(event.button.button == SDL_BUTTON_RIGHT && event.button.state == SDL_PRESSED)
                {
                    for(int i = 0; i < NUM_TOWERS; i++)
                    {
                        if(TD_InsideRect(mouseX, mouseY, &towerDestR[i]))
                        {
                            break;
                        }
                    }
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                SDL_GetMouseState(&mouseX, &mouseY);
                if(event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_RELEASED)
                {
                    if(towerTempTextureNormal && towerTempTextureRed)
                    {
                        SDL_DestroyTexture(towerTempTextureNormal);
                        SDL_DestroyTexture(towerTempTextureRed);
                        
                        towerTempTextureNormal = NULL;
                        towerTempTextureRed = NULL;
                        if(TD_InsideRect(mouseX, mouseY, &mapDestR))
                        {
                            // i could do the check for which rect it is smarter
                            for(int i = 0; i < BOARD_SIZE_X/RECT_W; i++)
                            {
                                for(int j = 0; j < BOARD_SIZE_Y/RECT_H; j++)
                                {
                                    if(TD_InsideRect(mouseX, mouseY, &gameState.towerMap[i][j].towerRect))
                                    {
                                        char path[255];
                                        sprintf(path, "assets/tower%d.png", currentTower);
                                        // FIXME: dont actually allow dragging a tower if it's not buyable
                                        if(gameState.towerMap[i][j].towerTexture == NULL && gameState.money >= towerCosts[currentTower-1])
                                        {
                                            gameState.towerMap[i][j].towerTexture = TD_CreateTextureFromImage(renderer, path);
                                            gameState.towerMap[i][j].towerCost = towerCosts[currentTower-1];
                                            gameState.towerMap[i][j].towerDamagePerHit = towerDamagePerHit[currentTower-1];
                                            gameState.towerMap[i][j].towerRadius = towerRadius[currentTower-1];
                                            gameState.towerMap[i][j].towerType = currentTower;
                                            gameState.money -= towerCosts[currentTower-1];
                                            // printf("Tower Dropped at %d %d\n", gameState.towerMap[i][j].towerRect.x, gameState.towerMap[i][j].towerRect.y);
                                        }
                                        else
                                        {
                                            // printf("money: %d costs = %d\n", gameState.money, costs[currentTower-1]);
                                            // printf("Tower Existent at %d %d\n", gameState.towerMap[i][j].towerRect.x, gameState.towerMap[i][j].towerRect.y);
                                        }
                                        break;
                                    }    
                                }
                            }
                                                
                        }
                        currentTower = TD_TOWER_NONE;
                        //printf("left button released\n");
                    }
                    if(event.button.button == SDL_BUTTON_RIGHT && event.button.state == SDL_RELEASED)
                    {
                        //printf("right button released\n");
                    }
                    break;
                }
            }
            default:
            {
                // printf("Unhandled event %d\n", event.type);
                break;
            }
        }
        
    }
    
}

void TD_DisplayShop(void)
{
    int xPos = BOARD_OFFSET_X + RECT_W * 4;
    int yPos = BOARD_OFFSET_Y + BOARD_SIZE_Y + RECT_H;
    char text[16];
    for(int i = 0; i < NUM_TOWERS; i++)
    {
        if(insideTowerTexture[i])
            SDL_RenderCopy(renderer, towerSelectedTexture[i], NULL, &towerDestR[i]);
        else
            SDL_RenderCopy(renderer, towerTexture[i], NULL, &towerDestR[i]);
    
        sprintf(text, "$%d", towerCosts[i]);
        TD_DisplayText(text, xPos, yPos);
        xPos += RECT_W;
    }
}

void TD_DisplayScore(void)
{
    int xPos = BOARD_OFFSET_X + RECT_W;
    int yPos = BOARD_OFFSET_Y / 2;
    char text[16];
    sprintf(text, "Score: %d", gameState.score);
    TD_DisplayText(text, xPos, yPos);
}

void TD_DisplayMoney(void)
{
    int xPos = BOARD_OFFSET_X + RECT_W * 5;
    int yPos = BOARD_OFFSET_Y / 2;
    char text[16];
    sprintf(text, "Money: $%d", gameState.money);
    TD_DisplayText(text, xPos, yPos);
}

void TD_DisplayLifes(void)
{
    int xPos = BOARD_OFFSET_X + RECT_W * 8;
    int yPos = BOARD_OFFSET_Y / 2;
    char text[16];
    sprintf(text, "Lifes: %d", gameState.lifes);
    TD_DisplayText(text, xPos, yPos);
}

void TD_DisplayMovingTower(void)
{
    if(towerTempTextureNormal)
    {
        if(TD_InsideRect(mouseX, mouseY, &mapDestR))
        {
            SDL_RenderCopy(renderer, towerTempTextureNormal, NULL, &towerTempDestR);
        }
        else
        {
            SDL_RenderCopy(renderer, towerTempTextureRed, NULL, &towerTempDestR);
        }
        
    }
}

void TD_DisplayTowers(void)
{
    for(int i = 0; i < BOARD_SIZE_X/RECT_W; i++)
    {
        for(int j = 0; j < BOARD_SIZE_Y/RECT_H; j++)
        {
            if(gameState.towerMap[i][j].towerTexture)
                SDL_RenderCopy(renderer, gameState.towerMap[i][j].towerTexture, NULL, &gameState.towerMap[i][j].towerRect);
        }
    }
    if(selectedTower != -1)
    {
        int curr_i = selectedTower / (BOARD_SIZE_X/RECT_W);
        int curr_j = selectedTower % (BOARD_SIZE_X/RECT_W);

        SDL_Rect rect;
        rect.w = gameState.towerMap[curr_i][curr_j].towerRadius;
        rect.h = gameState.towerMap[curr_i][curr_j].towerRadius;
        rect.x = gameState.towerMap[curr_i][curr_j].towerRect.x - rect.w/2 + RECT_W/2;
        rect.y = gameState.towerMap[curr_i][curr_j].towerRect.y - rect.h/2 + RECT_H/2;
        SDL_RenderCopy(renderer, towerRadiusTexture, NULL, &rect);
    }
}

void TD_Render(void)
{
    SDL_RenderClear(renderer);
    // this is where we would add stuff to render
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    SDL_RenderCopy(renderer, enemyTexture, NULL, &enemyDestR);
    TD_DisplayShop();
    TD_DisplayScore();
    TD_DisplayLifes();
    TD_DisplayMoney();
    TD_DisplayTowers();
    TD_DisplayMovingTower();
    SDL_RenderPresent(renderer);
}



void TD_Update(void)
{
    
    if(countX == BOARD_SIZE_X - RECT_W)
    {
        countX = 0;
        countY += RECT_H;
        enemyDestR.y += RECT_H;
    }
    if(countY > (BOARD_SIZE_Y - 6*RECT_H))
    {
        countY = 0;
        enemyDestR.y = ENEMY_OFFSET + BOARD_OFFSET_Y;
    }
    enemyDestR.x = countX + BOARD_OFFSET_Y;
    countX++;
    
}

void TD_MainLoop(void)
{
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    Uint32 frameStart;
    Uint32 frameTime;
    while(running)
    {
        frameStart = SDL_GetTicks();

        TD_HandleEvents();
        TD_Update();
        TD_Render();

        frameTime = SDL_GetTicks() - frameStart;

        if(frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
            if(frames % 60 == 0)
                printf("Delaying %d/%d\n", frameDelay - frameTime, frameDelay);
        }
        else
        {
            printf("it takes too much time %d\n", frameDelay - frameTime);
        }
        frames++;
    }
}

int main()
{
    TD_Init("Tower Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 840, 900, SDL_FALSE);
    TD_MainLoop();
    TD_Quit();
    return 0;
}