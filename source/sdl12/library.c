/*
Copyright 2021 Gameblabla

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include "library.h"

SDL_Surface* sdl_screen;

Font_data *internal_fonts[3];

#define FPS_VIDEO 60
const float real_FPS = 1000/FPS_VIDEO;

static Mix_Music* mus;

#ifndef SDL_TRIPLEBUF
#define SDL_TRIPLEBUF SDL_DOUBLEBUF
#endif

#define SIZE_LIST 15

const unsigned int keys_to_check[SIZE_LIST] =
{
	SDLK_UP,
	SDLK_DOWN,
	SDLK_LEFT,
	SDLK_RIGHT,
	SDLK_LCTRL,
	SDLK_LALT,
	SDLK_LSHIFT,
	SDLK_SPACE,
	SDLK_TAB,
	SDLK_BACKSPACE,
	SDLK_RETURN,
	SDLK_ESCAPE,
	SDLK_PAGEUP,
	SDLK_PAGEDOWN
};
unsigned int keys_status[SIZE_LIST];

void Set_title(const char* str, char* icon_img)
{
	SDL_WM_SetCaption(str, icon_img);
}

/* Images have to include transparency in their own files. */
Image_data* Load_Image_game(const char* str)
{
	SDL_Surface *tmp;
	tmp = IMG_Load(str);
	if (!tmp)
	{
		printf("Could not load image file, exit now\n");
		return NULL;	
	}
	return tmp;
}

void Display_image(Image_data* texture_tmp, int x, int y)
{
	SDL_Rect position;
	position.x = x;
	position.y = y;

	SDL_BlitSurface(texture_tmp, NULL, sdl_screen, &position);
}

Font_data* Load_Font(const char* str, size_t size_font)
{
	TTF_Font *font;
	font = TTF_OpenFont(str, size_font);
	if (!font)
	{
		return 0;
	}
	return font;
}

void Unload_Font(Font_data* font)
{
	TTF_CloseFont(font);
}

void Display_Font(Font_data* font, char* str, int x, int y, unsigned char r, unsigned g, unsigned b, unsigned char a, unsigned int types)
{
	SDL_Surface* text_surface;
	SDL_Rect position;

	SDL_Color color_sdl = {r, g, b, a};
	TTF_SetFontStyle(font, types);
	text_surface = TTF_RenderText_Blended(font, str, color_sdl);
	
	position.x = x;
	position.y = y;
	SDL_BlitSurface(text_surface, NULL, sdl_screen, &position);
}

int Init_Video(int w, int h, uint32_t bitdepth)
{
	SDL_Init( SDL_INIT_VIDEO );
	SDL_ShowCursor(0);
	sdl_screen = SDL_SetVideoMode(w, h, bitdepth, SDL_HWSURFACE | SDL_TRIPLEBUF);
	
	if (!sdl_screen)
	{
		printf("SDL_Init failed Window: %s\n", SDL_GetError());
		return 0;
	}
	
	if (TTF_Init() == -1)
	{
		printf("TTF_init failed: %s\n", TTF_GetError());
		return 0;
	}
	
	internal_fonts[0] = Load_Font("font.ttf", 12);
	
	memset(keys_status, 0, SIZE_LIST - 1);
	
	return 1;
}

unsigned int Return_RGB_color(unsigned char r, unsigned char g, unsigned char b)
{
	return SDL_MapRGB(sdl_screen->format, r, g, b);
}

void Flip_video(unsigned int background_color)
{
	Uint32 start;
	start = SDL_GetTicks();
	SDL_Flip(sdl_screen);
	if(real_FPS > SDL_GetTicks()-start) SDL_Delay(real_FPS-(SDL_GetTicks()-start));
	
	SDL_FillRect(sdl_screen, NULL, background_color);
}

void Unload_Image(Image_data* img)
{
	if (img)
	{
		SDL_FreeSurface(img);
		img = NULL;
	}
}

void Quit_video(void)
{
	SDL_FreeSurface(sdl_screen);
	SDL_Quit();	
	TTF_Quit();
}

int Init_Audio()
{
	Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,1024);
	Mix_AllocateChannels(32);
	return 1;
}

void Load_Music(const char* directory)
{
	if (mus)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(mus);	
	}
	mus = Mix_LoadMUS(directory);
}

void Play_Music(uint8_t loop)
{
	if (loop == 1) 	Mix_PlayMusic(mus, -1);
	else Mix_PlayMusic(mus, 0);
}

void Play_Snd(Sound_data* chk)
{
	Mix_PlayChannel(-1, chk, 0) ;
}

Sound_data* Load_SFX(const char* directory)
{
	return Mix_LoadWAV(directory);
}

void Unload_SFX(Sound_data* chk)
{
	Mix_FreeChunk(chk);
	chk = NULL;
}

void Unload_music()
{
	Mix_HaltMusic();
	Mix_FreeMusic(mus);	
}


void Poll_Controls()
{
	SDL_Event event;
	unsigned char i;
	Uint8 *keystate = SDL_GetKeyState(NULL);
	
	for(i=0;i<SIZE_LIST;i++)
	{
		if (keys_status[i] == 3) keys_status[i] = 0;
		
		if (keys_status[i] == 1 && keystate[keys_to_check[i]] == 1)
		{
			keys_status[i] = 2;
		}
	}
	
	while (SDL_PollEvent(&event)) 
	{
		for(i=0;i<SIZE_LIST;i++)
		{
			switch(event.type) 
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == keys_to_check[i] && keys_status[i] == 0)
					{
						keys_status[i] = 1;
					}
				break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == keys_to_check[i] && keys_status[i] > 0)
					{
						keys_status[i] = 3;
					}
				break;
				case SDL_QUIT:
					//quit = 1;
				break;
			}
		}
	}
}
