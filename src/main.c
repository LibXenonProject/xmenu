#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL/SDL.h>
#include <input/input.h>
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <newlib/dirent.h>
#include <diskio/ata.h>
#include <elf/elf.h>

#include "fctSdl.h"
#include "font.h"
#include "video.h"
#include "bitmap/bitmap.h"
#include "common.h"
#include "mount.h"

#define MAXDEPTH 16
#define MAXDIRNUM 1024
#define MAXPATH 0x108
#define BUFSIZE 65536
#define MAXDIRSHOW 8

static int handle = 0;

struct xdirent
{
	int	d_type;
	char	d_name[NAME_MAX + 1];
};

struct		xdirent	dlist[MAXDIRNUM];
int		dlist_num;
char		now_path[MAXPATH];
char		target[MAXPATH];
char		path_tmp[MAXPATH];
int		dlist_start;
int		dlist_curpos = 0;
int		cbuf_start[MAXDEPTH];
int		cbuf_curpos[MAXDEPTH];
int		now_depth;
char		buf[BUFSIZE];

int is_elf( int curpos )
{
	int i;

	for(i=0;i<MAXPATH;i++) if (dlist[curpos].d_name[i]==0) break;

	if ( i > 3 )
	{
		if (((dlist[curpos].d_name[i-4]) == '.') &&
			((dlist[curpos].d_name[i-3]) == 'e') &&
			((dlist[curpos].d_name[i-2]) == 'l') &&
			((dlist[curpos].d_name[i-1]) == 'f'))
		{
			return 1;
		}

		if (((dlist[curpos].d_name[i-4]) == '.') &&
			((dlist[curpos].d_name[i-3]) == 'E') &&
			((dlist[curpos].d_name[i-2]) == 'L') &&
			((dlist[curpos].d_name[i-1]) == 'F'))
		{
			return 1;
		}
		i--;

	}
	return 0;				
}

void Get_DirList(char *path) 
{
	DIR *rep;
	rep = opendir( path );
	dlist_num = 0;

	memset(&dlist, 0, sizeof(struct xdirent));

	struct dirent *file;

	while( ( ( file = readdir(rep) ) != 0 ) && ( dlist_num < MAXDIRNUM ) )
	{
		if ( file->d_name[0] == '.' )
			continue;

		strncpy( dlist[dlist_num].d_name, file->d_name, file->d_namlen );
		dlist[dlist_num].d_type = file->d_type;
		
		if ( file != 0 ) dlist_num++;
	}
	closedir(rep);

	if (dlist_start  >= dlist_num) { dlist_start  = dlist_num-1; }
	if (dlist_start  <  0)         { dlist_start  = 0;           }
	if (dlist_curpos >= dlist_num) { dlist_curpos = dlist_num-1; }
	if (dlist_curpos <  0)         { dlist_curpos = 0;           }
}


void drawFiler()
{
	SDL_Rect dest;
	dest.x = 0;
	dest.y = 0;
	dest.w = 1280;
	dest.h = 720;

	SDL_BlitSurface( bgSurf, NULL, SDLScreen, &dest );

	memset( path_tmp, 0, MAXPATH );
	strncpy( path_tmp, now_path, 30 );
	SDLprintf( SDLScreen, 90, 14, path_tmp );

	//Display directory listing

	int i = dlist_start;
	while ( i < ( dlist_start + MAXDIRSHOW ) )
	{
		int y = ( ( i - dlist_start ) + 2 ) * 64;

		if ( i < dlist_num ) 
		{
			if ( i == dlist_curpos ) 
			{
				dest.x = 100;
				dest.y = y;
				dest.w = 64;
				dest.h = 64;

				SDL_BlitSurface( cursor, NULL, SDLScreen, &dest );
			}

			dest.x = 190;
			dest.y = y;

			if ( dlist[i].d_type == 0x01 ) // Directory
			{
				SDL_BlitSurface( folderIcon, NULL, SDLScreen, &dest );
			}
			else
			{
				SDL_BlitSurface( fileIcon, NULL, SDLScreen, &dest );
			}

			memset( path_tmp, 0, MAXPATH );
			strncpy( path_tmp, dlist[i].d_name, 21 );
			SDLprintf(SDLScreen, 270, y, path_tmp );
		}
		i++;
	}

	SDLprintf( SDLScreen, 10, 650, progress );

	updateGFXPlane();
	drawBg(1280,720);
	updateScreen(1280,720);
}

void handleInput()
{
	struct controller_data_s pad;
	get_controller_data(&pad, 0);

	if( pad.up )
	{
		if (dlist_curpos > 0) 
		{
			dlist_curpos--;
			if (dlist_curpos < dlist_start)
				dlist_start = dlist_curpos;
		} 	
	}

	if( pad.down )
	{
		if ( dlist_curpos < ( dlist_num - 1 ) ) 
		{
			dlist_curpos++;
			if ( dlist_curpos >= ( dlist_start + MAXDIRSHOW ) )
				dlist_start++;
		}
	}

	if( pad.a )	
	{
		if ( dlist_num )
		{
			if ( dlist[dlist_curpos].d_type == 0x01 ) // Directory
			{
				if ( now_depth < MAXDEPTH )
				{
					strncat( now_path, "/", MAXPATH );
					strncat( now_path, dlist[dlist_curpos].d_name, MAXPATH );
					cbuf_start[now_depth] = dlist_start;
					cbuf_curpos[now_depth] = dlist_curpos;
					dlist_start  = 0;
					dlist_curpos = 0;
					now_depth++;
					Get_DirList( now_path );
				}
			}
			else
			{
				if ( is_elf(dlist_curpos) )
				{
					memset( target, 0, MAXPATH );
					sprintf( target, "%s/%s", now_path, dlist[dlist_curpos].d_name );
					printf("elf ok : %s\n", target );
					load = 1;
				}
				else
				{
					printf("elf nok : %s\n", target );
				}
			}
		}
	}
	if( pad.b )	
	{
		if (now_depth > 0)
		{
			int i;
			for( i = 0; i < MAXPATH; i++ )
			{
				if ( now_path[i] == 0 ) break;
			}

			i--;

			while( i > 4 )
			{
				if ( now_path[i - 1] == '/' )
				{
					now_path[i] = 0;
					now_path[i - 1] = 0;

					break;
				}
				i--;
			}
			now_depth--;
			dlist_start  = cbuf_start[now_depth];
			dlist_curpos = cbuf_curpos[now_depth];
			Get_DirList( now_path );
		}
	}
	if( pad.x )
	{
		handle = get_devices(handle, now_path);
		now_depth = 0;
		//dlist_start  = cbuf_start[now_depth];
		//dlist_curpos = cbuf_curpos[now_depth];
		Get_DirList( now_path );
	}
	if ( pad.logo )
	{
		return_to_xell = 1;
		load = 1;
		stop = 1;
	}
	mdelay(80);
}

void menu( void )
{
	handle = get_devices(handle, now_path);
	//strcpy( now_path, "sda1:" );
	usb_do_poll();
	Get_DirList( now_path );
	dlist_curpos = 0;

	while(!stop)
	{
		usb_do_poll();
		handleInput();
		drawFiler();
	}

	if ( SDLScreen != NULL )
		SDL_FreeSurface( SDLScreen );
	if ( font != NULL )
		SDL_FreeSurface( font );
	if ( bgSurf != NULL )
		SDL_FreeSurface( bgSurf );
	if ( fileIcon != NULL )
		SDL_FreeSurface( fileIcon );
	if ( folderIcon != NULL )
		SDL_FreeSurface( folderIcon );
	if ( cursor != NULL )
		SDL_FreeSurface( cursor );
	if ( GLOBALstring != NULL )
		SDL_FreeSurface( GLOBALstring );
}

int main(void)
{
	//init
	xenos_init(VIDEO_MODE_AUTO);
	console_init();

	xenon_thread_startup();

	usb_init();
	usb_do_poll();
	
	xenon_ata_init();
	xenon_atapi_init();
	
	mount_all_devices();
	findDevices();

	initSDLScreen();
	initScreen(1280,720);

//	console_close();

	stop = 0;
	load = 0;
	return_to_xell = 0;
	*progress = 0;

	while (xenon_run_thread_task(2, ((unsigned char*)memalign(256, 128*1024)) + 127*1024, (void*)menu));

//	network_init();

	while (!load)
	{
		mdelay(50);
//		network_poll();
	}
	
	if (return_to_xell)
		return 0;


	elf_runFromDisk( target );

	return 0;
}

