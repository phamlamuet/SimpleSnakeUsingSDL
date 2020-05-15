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
struct sNode
{
    bool bObstacle;
    bool bVisited;
    float fGlobalGoal;
    float fLocalGoal;
    vector<sNode*>vecNeighbours;
    sNode*parent;
    int x;
    int y;
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
void pathRender(SDL_Renderer *renderer,vector<sNode*>path)
{
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    for(int i=1;i<path.size();i++)
    {
        SDL_Rect pathRect;
        pathRect.x=path[i]->x;
        pathRect.y=path[i]->y;
        pathRect.h=scale;
        pathRect.w=scale;
        SDL_RenderFillRect(renderer,&pathRect);
    }
}
/*void visitedBlockRender(SDL_Renderer *renderer,sNode*nodes)
{
   SDL_SetRenderDrawColor(renderer,0,100,100,255);

   for(int i=0; i<nMapWidth; i++)
    {
        for(int j=0; j<nMapHeight; j++)
        {
            if(nodes[j*nMapWidth + i].bVisited ==true)
            {
                SDL_Rect visitedBlockRect;
                visitedBlockRect.x=nodes[j*nMapWidth+i].x;
                visitedBlockRect.y=nodes[j*nMapWidth+i].y;
                visitedBlockRect.h=scale;
                visitedBlockRect.w=scale;
                SDL_RenderFillRect(renderer,&visitedBlockRect);
            }
        }
    }
}   */
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

vector<sNode*> solveAstar(sNode*nodeStart,sNode*nodeEnd,sNode*nodes,Snake &snake)
{
    vector<sNode*>path;
    for(int x=0; x<nMapWidth; x++)
    {
        for(int y=0; y<nMapHeight; y++)
        {
            nodes[y*nMapWidth+x].fGlobalGoal=INFINITY;
            nodes[y*nMapWidth+x].fLocalGoal=INFINITY;
            nodes[y*nMapWidth + x].bVisited = false;
            nodes[y*nMapWidth + x].parent = nullptr;
            nodes[y*nMapWidth+x].bObstacle=false;
        }
    }
    for(int i=0; i<snake.tail.size(); i++)
    {
        nodes[snake.tail[i].x/24+snake.tail[i].y].bObstacle=1;
       // cout<<snake.tail[i].x/24<<","<<snake.tail[i].y/24;
       // cout<<"is obstacle"<<endl;
    }
   // nodes[snake.head.x/24+snake.head.y].bObstacle=1;
    auto distance=[](sNode*a,sNode*b)
    {
        //return max(fabs(a->x-b->x),fabs(a->y-b->y));
        return fabs((a->x-b->x))+fabs((a->y-b->y));
       // return sqrt((a->x-b->x)*(a->x-b->x)+(a->y-b->y)*(a->y-b->y));
    };
    auto heuristic=[distance](sNode*a,sNode*b)
    {
        return distance(a,b);
    };
    sNode *nodeCurrent=nodeStart;
    nodeStart->fLocalGoal=0.0f;
    nodeStart->fGlobalGoal=heuristic(nodeStart,nodeEnd);
    /** list of nodes to be tested ->> openlist */
    list<sNode*> listNotTestedNodes;
    listNotTestedNodes.push_back(nodeStart); /** startNode is the first to be tested*/
    while (!listNotTestedNodes.empty() && nodeCurrent != nodeEnd  )// Find absolutely shortest path // && nodeCurrent != nodeEnd)
    {
        // Sort Untested nodes by global goal, so lowest is first
        listNotTestedNodes.sort([](const sNode* lhs, const sNode* rhs)
        {
            return lhs->fGlobalGoal < rhs->fGlobalGoal;
        } );

        // Front of listNotTestedNodes is potentially the lowest distance node. Our
        // list may also contain nodes that have been visited, so ditch these...
        while(!listNotTestedNodes.empty() && listNotTestedNodes.front()->bVisited)
            listNotTestedNodes.pop_front();

        // ...or abort because there are no valid nodes left to test
        if (listNotTestedNodes.empty())
            break;

        nodeCurrent = listNotTestedNodes.front();
        nodeCurrent->bVisited = true; // We only explore a node once

        // Check each of this node's neighbours...
        for (auto nodeNeighbour : nodeCurrent->vecNeighbours)
        {
            // ... and only if the neighbour is not visited and is
            // not an obstacle, add it to NotTested List
            if (!nodeNeighbour->bVisited && nodeNeighbour->bObstacle == 0)
                listNotTestedNodes.push_back(nodeNeighbour);

            // Calculate the neighbours potential lowest parent distance
            float fPossiblyLowerGoal = nodeCurrent->fLocalGoal + distance(nodeCurrent, nodeNeighbour);

            // If choosing to path through this node is a lower distance than what
            // the neighbour currently has set, update the neighbour to use this node
            // as the path source, and set its distance scores as necessary
            if (fPossiblyLowerGoal < nodeNeighbour->fLocalGoal)
            {
                nodeNeighbour->parent = nodeCurrent;
                nodeNeighbour->fLocalGoal = fPossiblyLowerGoal;
                nodeNeighbour->fGlobalGoal = nodeNeighbour->fLocalGoal + heuristic(nodeNeighbour, nodeEnd);
            }
        }
    }
    if(nodeEnd!=nullptr)
    {
        sNode *p=nodeEnd;
        while(p->parent!=nullptr)
        {
            path.push_back(p);
            p=p->parent;
        }
    }
  /*  for(int i=path.size()-1;i>=0;i--)
    {
        cout<<path[i]->x/24<<","<<path[i]->y/24<<endl;
    }  */
    return path;
}
void getPath(vector<sNode*>path,Snake&snake)
{
    int pathSize=path.size();
    if(pathSize==0)
    {
        return;
    }
    else if(path[pathSize-1]->x==snake.head.x&&path[pathSize-1]->y==snake.head.y)
    {
          return;
    }
    else if(path[pathSize-1]->x==snake.head.x&&path[pathSize-1]->y<snake.head.y&&snake.status!=DOWN)
    {
        snake.status=UP;

    }
     else if(path[pathSize-1]->y==snake.head.y&&path[pathSize-1]->x<snake.head.x&&snake.status!=RIGHT)
     {
         snake.status=LEFT;

     }
     else if(path[pathSize-1]->y==snake.head.y&&path[pathSize-1]->x>snake.head.x&&snake.status!=LEFT)
     {
         snake.status=RIGHT;
     }
     else if(path[pathSize-1]->x==snake.head.x&&path[pathSize-1]->y>snake.head.y&&snake.status!=UP)
     {
         snake.status=DOWN;

     }
}
void getPath2(vector<sNode*>path,Snake&snake)
{
    int pathSize=path.size();
    if(pathSize==0)
    {
        return;
    }
    else if(path[pathSize-1]->x==snake.head.x&&path[pathSize-1]->y==snake.head.y)
    {
          return;
    }
    else if(path[pathSize-1]->y>snake.head.y)
    {
        snake.status=DOWN;
    }
    else if(path[pathSize-1]->y<snake.head.y)
    {
        snake.status=UP;
    }

    else if(path[pathSize-1]->x>snake.head.x)
    {
        snake.status=RIGHT;
    }
    else if(path[pathSize-1]->x<snake.head.x)
    {
        snake.status=LEFT;
    }
}

int main(int argc, char* argv[])
{
    cout<<"Just Do It";
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

    sNode *nodes=new sNode[nMapWidth*nMapHeight]; /** array of nodes */

    for(int x=0; x<nMapWidth; x++)
    {
        for(int y=0; y<nMapHeight; y++)
        {
            nodes[y * nMapWidth + x].x = x*scale;
            nodes[y * nMapWidth + x].y = y*scale;
            nodes[y * nMapWidth + x].bObstacle = false;
            nodes[y * nMapWidth + x].parent = nullptr;
            nodes[y * nMapWidth + x].bVisited = false;
        }
    }
    sNode*nodeStart=&nodes[3];  /**start node */
    sNode*nodeEnd=&nodes[8];   /**end node */


    /**make connection */
    for(int x=0; x<nMapWidth; x++)
    {
        for(int y=0; y<nMapHeight; y++)
        {
            if(y>0)
                nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y - 1) * nMapWidth + (x + 0)]);
            if(y<nMapHeight-1)
                nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 1) * nMapWidth + (x + 0)]);
            if (x>0)
                nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 0) * nMapWidth + (x - 1)]);
            if(x<nMapWidth-1)
                nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 0) * nMapWidth + (x + 1)]);
        }
    }
    nodeStart=&nodes[snake.head.x/24+snake.head.y];
    nodeEnd=&nodes[food.x/24+food.y];

    cout<<"test ok ok"<<endl;
    nodeStart=&nodes[snake.head.x/24+snake.head.y];
    nodeEnd=&nodes[food.x/24+food.y];
    vector<sNode*>path;
    path=solveAstar(nodeStart,nodeEnd,nodes,snake);

    while(true)
    {
        // cout<<nodeStart->x<<" "<<snake.head.x<<endl;
        //SDL_Delay(100);
        preHead.x=snake.head.x;
        preHead.y=snake.head.y;
        // cout<<snake.head.x/24<<" "<<snake.head.y/24<<" "<<snake.status<<endl;
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
        else
        {
            //path.clear();
            //std::vector<sNode*>().swap(path);
           // nodeStart=&nodes[snake.head.x/24+snake.head.y];
           // nodeEnd=&nodes[food.x/24+food.y];
            //path=solveAstar(nodeStart,nodeEnd,nodes,snake);

            getPath2(path,snake);
            //tracePath(path,snake);
            path.pop_back();

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

            snake.tailLength++;
            snake.tail.push_back(preHead);
            food= spawnFood(snake);
            path.clear();
            std::vector<sNode*>().swap(path);
            SDL_Delay(100);
            nodeStart=&nodes[snake.head.x/24+snake.head.y];
            nodeEnd=&nodes[food.x/24+food.y];
            path=solveAstar(nodeStart,nodeEnd,nodes,snake);
        //    cout<<"path size is"<<path.size()<<endl;
        //   cout<<"start node is "<<nodeStart->x/24<<","<<nodeStart->y/24<<endl;
        //   cout<<"node end is "<<nodeEnd->x/24<<","<<nodeEnd->y/24<<endl;
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

        pathRender(renderer,path);
        scoreRender(renderer,snake);
        //visitedBlockRender(renderer,nodes);
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
