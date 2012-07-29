void initSDLScreen();
void updateGFXPlane();
SDL_Surface * getSDLFont();
SDL_Surface * getSDLCursor();
void  SDLprintf(SDL_Surface *surDest,int x,int y, char *str);
SDL_Surface * getBitmapFromMemory(int width,int height,void *pixeldata);

//Ecran SDL
SDL_Surface *SDLScreen;
SDL_Surface *font;
SDL_Surface *bgSurf;
SDL_Surface *fileIcon;
SDL_Surface *folderIcon;
SDL_Surface *cursor;
SDL_Surface *GLOBALstring;
//Mask uiliser pour l'image
Uint32 rmask, gmask, bmask, amask;

