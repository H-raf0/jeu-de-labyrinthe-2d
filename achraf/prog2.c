#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


void end_sdl(char ok,                                               // fin normale : ok = 0 ; anormale ok = 1
	char const* msg,                                       // message à afficher
	SDL_Window* window,                                    // fenêtre à fermer
	SDL_Renderer* renderer) {                              // renderer à fermer
	
	char msg_formated[255];                                            
	int l;                                                     
										
	if (!ok) {                                                        // Affichage de ce qui ne va pas
	
		strncpy(msg_formated, msg, 250);                                         
		l = strlen(msg_formated);                                            
		strcpy(msg_formated + l, " : %s\n");                                     
											
		SDL_Log(msg_formated, SDL_GetError());                                   
	}                                                          
										
	if (renderer != NULL) {                                           // Destruction si nécessaire du renderer

		SDL_DestroyRenderer(renderer);                                  // Attention : on suppose que les NULL sont maintenus !!
		renderer = NULL;
	}
	if (window != NULL)   {                                           // Destruction si nécessaire de la fenêtre

		SDL_DestroyWindow(window);                                      // Attention : on suppose que les NULL sont maintenus !!
		window= NULL;
	}
										
	SDL_Quit();                                                    
										
	if (!ok) {                                       // On quitte si cela ne va pas  

		exit(EXIT_FAILURE);                                                  
	}                                                          
}                                                        

void drawCercle(SDL_Renderer* renderer, int x, int y, int diametre){

	for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 4000) {  

		SDL_RenderDrawPoint(renderer,                  
					x + diametre * cos(angle),                     // coordonnée en x
					y + diametre * sin(angle));                    //            en y   
		
	}
	
}

void drawLinkedCercles(SDL_Renderer* renderer, int x, int y, int diametre){

	static int angle = 0;
	int nbCercle = 16;

	SDL_SetRenderDrawColor(renderer,
				255,               // quantité de Rouge      
				255,               //          de vert 
				255,               //          de bleu
				255);                                    // opacité = opaque

	for(int i=0; i<nbCercle; i++){

		drawCercle(renderer, x+diametre*2*cos(angle+(M_PI/nbCercle)*2*i), y + diametre*2*sin(angle+(M_PI/nbCercle)*2*i), diametre);
	}
	angle += 6; //vitesse de rotation

}


int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	SDL_DisplayMode screen;

	/*********************************************************************************************************************/  
	/*                         Initialisation de la SDL  + gestion de l'échec possible                                   */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) end_sdl(0, "ERROR SDL INIT", window, renderer);

	SDL_GetCurrentDisplayMode(0, &screen);
	printf("Résolution écran\n\tw : %d\n\th : %d\n",
		screen.w, screen.h);

	/* Création de la fenêtre */
	window = SDL_CreateWindow("Premier dessin",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED, screen.w * 1,
				screen.h * 1,
				SDL_WINDOW_OPENGL);
	if (window == NULL) end_sdl(0, "ERROR WINDOW CREATION", window, renderer);

	/* Création du renderer */
	renderer = SDL_CreateRenderer(window, -1,
					SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) end_sdl(0, "ERROR RENDERER CREATION", window, renderer);

	/*********************************************************************************************************************/
	/*                                     On dessine dans le renderer                                                   */
	/*********************************************************************************************************************/
	/*             Cette partie pourrait avantageusement être remplacée par la boucle évènementielle                     */ 

	// appel de la fonction qui crée l'image 
	double t, x, y;
	int N = 300;

	
	for(int i = 0; i <= N; ++i){

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		t = 2 * M_PI * i / N;
        x = (float)screen.w /2 + 500 * pow(cos(t), 3);
        y = (float)screen.h /2 + 500 * pow(sin(t), 3);

		drawLinkedCercles(renderer, x, y,40);
		
		SDL_RenderPresent(renderer);
		SDL_Delay(5);
	}												

	/* on referme proprement la SDL */
	end_sdl(1, "Normal ending", window, renderer);
	return EXIT_SUCCESS;
}