/**
Function Utiliser pour l'ecriture, creation de surface et l'initialisation de sdl


**/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL/SDL.h>
#include <input/input.h>
#include "video.h"
#include <xenos/xe.h>
#include "fctSdl.h"
#include "bitmap/bitmap.h"
#include "bitmap/arial.h"

/**
Initialisation de l'ecran
**/
void initSDLScreen()
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	SDLScreen=SDL_CreateRGBSurface(SDL_SWSURFACE, 1280, 720, 32, rmask, gmask, bmask, amask);
	
	SDL_FillRect(SDLScreen, 0, 0x88BB2233);

	//Texture frequement utilisé
	font=getSDLFont();

	bgSurf=getBitmapFromMemory(1280,720,background);
	fileIcon=getBitmapFromMemory(64,64,xenonFileData);
	folderIcon=getBitmapFromMemory(64,64,xenonFolderData);
	cursor=getBitmapFromMemory(64,64,xenonCursorData);

	GLOBALstring=SDL_CreateRGBSurface(SDL_SWSURFACE, 1280, 720, 32,rmask, gmask, bmask, amask);
}

//Copie SDLScreen dans gfxplane (à faire avant affichage)
void updateGFXPlane()
{
	unsigned long size=(SDLScreen->w)*(SDLScreen->h)*(4) + 1;
	memcpy(gfxplane->base,SDLScreen->pixels,size);
}

SDL_Surface * getBitmapFromMemory(int width,int height,void *pixeldata)
{
	SDL_Surface *surf=SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                   rmask, gmask, bmask, amask);
	
	unsigned long size=(surf->h)*(surf->pitch) + 1;

	SDL_LockSurface(surf);
	memcpy(surf->pixels,pixeldata,size);
	SDL_UnlockSurface(surf);

	return surf;
}

//Retourne une surface sdl correspondant a la fonte d'ecriture
SDL_Surface * getSDLFont()
{
	//utilise la nouvelle font (arial)

	int fontWidth=512;
	int fontHeight=512;
	extern unsigned char arial_0[];
	SDL_Surface *font=getBitmapFromMemory(fontWidth,fontHeight,arial_0);

	return font;
}

//Affiche du texte a l'ecran -> a continué --> fmt -> va_args
void  SDLprintf(SDL_Surface *surDest,int x,int y, char *str)
{
	DEBUG("\t\t\t SDLprintf()\n");
	//surface de la font
	//SDL_Surface *font=getSDLFont();
	//surface de la phrase a retourné
	SDL_Surface *string;
	

	int strLength=strlen(str);
	
	DEBUG("\t\t\t SDLprintf()->SDL_FillRect()\n");

	//string=SDL_CreateRGBSurface(SDL_SWSURFACE, 1280, 720, 32,rmask, gmask, bmask, amask);
	
	
	//GLOBALstring=(SDL_Surface*)malloc(sizeof(SDL_Surface));

	
//	SDL_FillRect(string, 0, 0xFF000000);
	DEBUG("\t\t\t SDLprintf()->SDL_Rect rect\n");
	SDL_Rect rect;//rect d'une lettre
	rect.x = 0;
	rect.y = 0;
	rect.w = 0;
	rect.h = 0;

	SDL_Rect dest;
	dest.x = x;
	dest.y = y;	
	dest.w = 0;
	dest.h = 0;


	int i=0;
	for(i=0;i<strLength;i++)
	{
		
		//arial_fnt
		DEBUG("\t\t\t SDLprintf()->fntPosData=&arial_fnt\n");
		struct fnt_s *fntPosData=&arial_fnt[(unsigned char)str[i]];

		rect.x=fntPosData->x;
		rect.y=fntPosData->y;
		rect.w=fntPosData->width;
		rect.h=fntPosData->height;
		
		dest.y=y+fntPosData->yoffset;
		dest.x+=fntPosData->xoffset;
		dest.h=rect.h;
		dest.w=rect.w;
			
		DEBUG("\t\t\t SDLprintf()->SDL_BlitSurface( font, &rect, GLOBALstring, &dest );\n");
		SDL_BlitSurface( font, &rect, surDest, &dest );

		dest.x+=fntPosData->xadvance;
		//printf("Lettre \"%c\" posX=%d posY=%d width=%d height=%d\n",str[i],rect.x,rect.y,rect.w,rect.h);
	}
	DEBUG("\t\t\t SDLprintf()->SDL_SetColorKey\n");
	//SDL_SetColorKey(string,SDL_SRCCOLORKEY,0);//Applique la transparence au noir
	
	SDL_FreeSurface(string);
	//return GLOBALstring;
}

