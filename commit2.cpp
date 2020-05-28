
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include<cstdlib>
#include<ctime>
#include<algorithm>
#include<list>
using namespace std;

const int scale=24;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 577;
const int nMapWidth=24;
const int nMapHeight=24;

enum Status
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    STOP
};

const string WINDOW_TITLE = "Snake game";
void initSDL(SDL_Window* &window, SDL_Renderer* &renderer);
void logSDLError(std::ostream& os, //SOME PREBAKED FUNCTION
                 const std::string &msg, bool fatal = false);
void quitSDL(SDL_Window* window, SDL_Renderer* renderer);
void waitUntilKeyPressed();

struct ToaDo
{
    int x;
    int y;
};
struct sNode:ToaDo
{
  bool bObstacle;
  bool bVisited;
  float fGlobal;
  float fLocalGoal;
  vector<sNode*>vecNeighbours;
  sNode*parent;
};
struct Snake
{
    ToaDo head;
    vector<ToaDo> tail;
    int tailLength=0;
    Status status;
};
void snakeRender(SDL_Renderer* renderer,SDL_Rect snakeRec,Snake &snake )
{
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    snakeRec.w=scale;
    snakeRec.h=scale;

    for(int i=0; i<snake.tailLength; i++)
    {
        snakeRec.x=snake.tail[i].x;
        snakeRec.y=snake.tail[i].y;
        SDL_RenderFillRect(renderer,&snakeRec);
    }

    snakeRec.x=snake.head.x;
    snakeRec.y=snake.head.y;
    SDL_RenderFillRect(renderer,&snakeRec);

}
void foodRender(SDL_Renderer* rendererFood,SDL_Rect foodRect,ToaDo &food)
{
    SDL_SetRenderDrawColor(rendererFood,255,0,0,255);
    foodRect.x=food.x;
    foodRect.y=food.y;
    foodRect.w=scale;
    foodRect.h=scale;
    SDL_RenderFillRect(rendererFood,&foodRect);
}
void scoreRender(SDL_Renderer*renderer,Snake snake)
{
    SDL_Color Black= {0,0,0};
    TTF_Font* font = TTF_OpenFont((char*)"arial.ttf", 10);
    if(font==NULL)
    {
        cout<<"Font loading error"<<endl;
        return;
    }
    SDL_Surface* score = TTF_RenderText_Solid(font, (string("Score: ") + to_string(snake.tailLength * 10)).c_str(), Black);
    SDL_Texture* scoreMessage = SDL_CreateTextureFromSurface(renderer, score);
    SDL_Rect scoreRect;
    scoreRect.w = 150;
    scoreRect.h = 50;
    scoreRect.x = 616;
    scoreRect.y = 10;
    SDL_RenderCopy(renderer, scoreMessage, NULL, &scoreRect);

    TTF_CloseFont(font);
}
bool isFoodOnSnake(ToaDo &food,Snake &snake)
{
    bool valid=false;
    for(int i=0; i<snake.tailLength; i++)
    {
        if(snake.tail[i].x==food.x&&snake.tail[i].y==food.y)
        {
            valid=true;
            break;
        }
    }
    if(snake.head.x==food.x&&snake.head.y==food.y)
    {
        valid=true;
    }
    return valid;
}

ToaDo spawnFood(Snake &snake)
{
    srand(time(0));
    ToaDo foodTemp;
    foodTemp.x=scale*(rand()%scale);
    foodTemp.y=scale*(rand()%scale);
    while(isFoodOnSnake(foodTemp,snake))
    {

        foodTemp.x=scale*(rand()%scale);
        foodTemp.y=scale*(rand()%scale);
    }

    return foodTemp;
}

void screenRender(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer,255,255,0,255);
    SDL_RenderDrawLine(renderer, 0, 0, 0, scale*scale);
    SDL_RenderDrawLine(renderer, 0, scale*scale, scale*scale, scale*scale);
    SDL_RenderDrawLine(renderer, scale*scale, scale*scale, scale*scale, 0);
    SDL_RenderDrawLine(renderer, scale*scale, 0, 0, 0);


    for(int i=1; i<scale; i++)
    {
        SDL_RenderDrawLine(renderer, 0,i*scale, scale*scale, i*scale);
    }
    for(int i=1; i<scale; i++)
    {
        SDL_RenderDrawLine(renderer, i*scale,0, i*scale, scale*scale);
    }

}
bool gameOver(Snake snake)
{
    if(snake.head.x<0||snake.head.y<0||snake.head.x>scale*(scale-1)||snake.head.y>scale*(scale-1))
    {
        return true;
    }
    for(int i=0; i<snake.tailLength; i++)
    {
        if(snake.head.x==snake.tail[i].x&&snake.head.y==snake.tail[i].y)
        {
            return true;
        }
    }
    return false;
}

Snake snakeInit(Snake &snake,ToaDo &preHead)
{
    snake.tailLength=0;
    snake.head.x=0;
    snake.head.y=0;
    preHead.x=0;
    preHead.y=0;
    snake.status=STOP;

    return snake;
}

int main(int argc, char* argv[])
{
    cout<<"justDoIt";
    srand(time(0));
    TTF_Init();
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Event event;
    initSDL(window, renderer);

    SDL_SetRenderDrawColor(renderer,255,0,0,255);

    Snake snake;
    ToaDo preHead;
    snakeInit(snake,preHead);

    SDL_Rect snakeRect;
    snakeRect.x=0;
    snakeRect.y=0;
    snakeRect.h=0;
    snakeRect.w=0;

    ToaDo food;
    food=spawnFood(snake);
    SDL_Rect foodRect;
    while(true)
    {
        SDL_Delay(70);
        preHead.x=snake.head.x;
        preHead.y=snake.head.y;
        if(SDL_PollEvent(&event))
        {
            if(event.type==SDL_QUIT)
                exit(0);
            if(event.type==SDL_KEYDOWN)
            {

                if (snake.status!=DOWN && event.key.keysym.scancode == SDL_SCANCODE_UP)
                {
                    snake.status=UP;

                }
                else if (snake.status != RIGHT && event.key.keysym.scancode == SDL_SCANCODE_LEFT)
                {
                    snake.status=LEFT;

                }
                else if (snake.status !=UP && event.key.keysym.scancode == SDL_SCANCODE_DOWN)
                {
                    snake.status=DOWN;

                }
                else if (snake.status!=LEFT && event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                {
                    snake.status=RIGHT;

                }
            }
        }
        Snake snakeTemp;
        snakeTemp.head=preHead;
        snakeTemp.tailLength=snake.tailLength;
        for(int i=0; i<snake.tailLength; i++)
        {
            snakeTemp.tail.push_back(snake.tail[i]);
        }

        if(snake.status==UP)
        {
            snake.head.y-=scale;
        }
        else if(snake.status==LEFT)
        {
            snake.head.x-=scale;
        }
        else if(snake.status==RIGHT)
        {
            snake.head.x+=scale;
        }
        else if(snake.status==DOWN)
        {
            snake.head.y+=scale;
        }

        if(food.x==snake.head.x&&food.y==snake.head.y)
        {
            food= spawnFood(snake);
            snake.tailLength++;
        }

        if(snake.tail.size()!=snake.tailLength)
        {
            snake.tail.push_back(preHead);
        }
        for(int i=1; i<snake.tailLength; i++)
        {

            snake.tail[i-1].x=snake.tail[i].x;
            snake.tail[i-1].y=snake.tail[i].y;

        }
        if(snake.tailLength>0)
        {
            snake.tail[snake.tailLength-1].x=preHead.x;
            snake.tail[snake.tailLength-1].y=preHead.y;
        }

        if(gameOver(snake))
        {
            snakeRender(renderer,snakeRect,snakeTemp);
            break;
        }
        else
        {
            snakeRender(renderer,snakeRect,snake);
        }
        foodRender(renderer,foodRect,food);
        screenRender(renderer);
        scoreRender(renderer,snake);
        SDL_RenderPresent(renderer);
        SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255);
        SDL_RenderClear(renderer);
    }
    waitUntilKeyPressed();
    quitSDL(window, renderer);
    return 0;
}
void logSDLError(std::ostream& os,
                 const std::string &msg, bool fatal)
{
    os << msg << " Error: " << SDL_GetError() << std::endl;
    if (fatal)
    {
        SDL_Quit();
        exit(1);
    }
}
void initSDL(SDL_Window* &window, SDL_Renderer* &renderer)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        logSDLError(std::cout, "SDL_Init", true);

    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    //window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED,
    //    SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (window == nullptr)
        logSDLError(std::cout, "CreateWindow", true);


    //Khi thông thường chạy với môi trường bình thường ở nhà
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    //Khi chạy ở máy thực hành WinXP ở trường (máy ảo)
    //renderer = SDL_CreateSoftwareRenderer(SDL_GetWindowSurface(window));
    if (renderer == nullptr)
        logSDLError(std::cout, "CreateRenderer", true);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}
void quitSDL(SDL_Window* window, SDL_Renderer* renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void waitUntilKeyPressed()
{
    SDL_Event e;
    while (true)
    {
        if ( SDL_WaitEvent(&e) != 0 &&
                (e.type == SDL_KEYDOWN || e.type == SDL_QUIT) )
            return;
        SDL_Delay(100);
    }
}

