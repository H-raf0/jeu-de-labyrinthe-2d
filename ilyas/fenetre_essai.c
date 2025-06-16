#include <SDL2/SDL.h>
#include <stdio.h>


void Init_SDL()
{
    /* Initialisation de la SDL  + gestion de l'échec possible */
    if(SDL_Init(SDL_INIT_VIDEO)!=0)
    {
        // l'initialisation de la SDL a échoué
        SDL_Log("Error : SDL initialisation - %s\n",SDL_GetError());
        exit(EXIT_FAILURE);
    }
}
SDL_Window * Create_Window (int posX, int posY, int length, int height, SDL_Renderer ** renderer)
{
    SDL_Window * window;
    window = SDL_CreateWindow("Fenetre",posX, posY, length, height,SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        SDL_Log("Error : SDL window creation - %s\n", 
                    SDL_GetError());                 // échec de la création de la fenêtre
        SDL_Quit();                              // On referme la SDL       
        exit(EXIT_FAILURE);
    }
    *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (*renderer == NULL) {
        SDL_Log("Error : SDL renderer creation - %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);

    return window;
}


void free_window(SDL_Window * window, SDL_Renderer *renderer)
{
    //s'il y a rien "pas de renderer"
    if (renderer == NULL)
    {
        SDL_Log("Error : SDL window creation - %s\n", 
                    SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);

    }
    //s'il y a rien "pas de window"
    if (window == NULL)
    {
        SDL_Log("Error : SDL window creation - %s\n", 
                    SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    //on fait un " destroy" de windows
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void CreatediagWindows(SDL_Window ** windowsArray, SDL_Renderer ** renderersArray, int a , int b, int N )
{
    for (int i=0;i<N;i++)
    {
        windowsArray[i] = Create_Window(a,b,100,100,&renderersArray[i]);
        printf("a= %d , b= %d \n", a , b);
        a+=80;
        b+=40;
        SDL_Delay(100);
    }

}
void Move_diag(SDL_Window ** windowsArray,int count)
{
    for (int i = 0; i < count; i++) {
        int x, y;
        SDL_GetWindowPosition(windowsArray[i], &x, &y);
        SDL_SetWindowPosition(windowsArray[i], x + 2, y + 2);
    }
    SDL_Delay(100);
}




int main()
{
    int M=15;
    int N=30;
    int count = 2;
    SDL_Window **windowsArray = (SDL_Window **) malloc(sizeof(SDL_Window *) * N);
    SDL_Renderer **renderersArray = (SDL_Renderer **)malloc(sizeof(SDL_Renderer *) * N);
    Init_SDL();
    
    for (int j = 0; j < N; j++) {
      CreatediagWindows(windowsArray,renderersArray,2,2,N);
    }
     for(int i=0;i<M;i++)
     {
         free_window(windowsArray[i],renderersArray[i]);
    }
     free(windowsArray);
     free(renderersArray);
    

     SDL_Quit();                                // quitter la SDL
    return 0;
}