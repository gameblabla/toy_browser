#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#define Image_data SDL_Surface
#define Sound_data Mix_Chunk
#define Font_data TTF_Font

extern SDL_Surface* sdl_screen;
extern Font_data *internal_fonts[3];

#define Screen_buffer sdl_screen->pixels

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define Get_Image_Width(a) a->w
#define Get_Image_Height(a) a->h

extern void Poll_Controls();

extern void Set_title(const char* str, char* icon_img);

extern unsigned int Return_RGB_color(unsigned char r, unsigned char g, unsigned char b);

extern unsigned int keys_status[];

extern Image_data* Load_Image_game(const char* str);

extern void Display_image(Image_data* texture_tmp, int x, int y);

extern int Init_Video(int w, int h, uint32_t bitdepth);
extern void Flip_video(unsigned int background_color);
extern void Quit_video(void);

extern void Unload_Image(Image_data* img);

extern int Init_Audio();
extern void Load_Music(const char* directory);

extern void Play_Music(uint8_t loop);

extern void Play_Snd(Sound_data* chk);

extern Sound_data* Load_SFX(const char* directory);

extern void Unload_SFX(Sound_data* chk);
extern void Unload_music();

extern Font_data* Load_Font(const char* str, size_t size_font);
extern void Unload_Font(Font_data* font);

extern void Display_Font(Font_data* font, char* str, int x, int y, unsigned char r, unsigned g, unsigned b, unsigned char a, unsigned int types);

#endif
