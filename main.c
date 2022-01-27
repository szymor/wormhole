#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>

#define SCREEN_WIDTH		(320)
#define SCREEN_HEIGHT		(240)
#define SCREEN_BPP			(32)

#define TEXTURE_SIZE		(32)

SDL_Surface *screen = NULL;
int fps = 0;
bool fps_on = false;

Uint32 getPixel(SDL_Surface *s, int x, int y)
{
	int i = y * s->w + x;
	Uint32 *p = (Uint32*)s->pixels;
	return p[i];
}

void setPixel(SDL_Surface *s, int x, int y, Uint32 c)
{
	int i = y * s->w + x;
	Uint32 *p = (Uint32*)s->pixels;
	p[i] = c;
}

void fps_counter(double dt)
{
	static double total = 0;
	static int count = 0;
	total += dt;
	++count;
	if (total > 1.0)
	{
		fps = count;
		total -= 1.0;
		count = 0;
	}
}

void fps_draw(void)
{
	char string[8] = "";
	sprintf(string, "%d", fps);
	stringRGBA(screen, 0, 0, string, 255, 255, 255, 255);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);
	//SDL_EnableKeyRepeat(100, 50);
	SDL_ShowCursor(SDL_DISABLE);

	SDL_Surface *texture = IMG_Load("checker.png");
    texture = SDL_DisplayFormat(texture);
	SDL_Surface *shadow = IMG_Load("shadow.png");

	int distmap[SCREEN_HEIGHT][SCREEN_WIDTH];
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		int yy = y - SCREEN_HEIGHT / 2;
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			int xx = x - SCREEN_WIDTH / 2;
			distmap[y][x] = (int)(2 * TEXTURE_SIZE * (SCREEN_WIDTH / 2) / sqrt(xx * xx + yy * yy)) % TEXTURE_SIZE;
		}
	}
	int anglemap[SCREEN_HEIGHT][SCREEN_WIDTH];
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		int yy = y - SCREEN_HEIGHT / 2;
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			int xx = x - SCREEN_WIDTH / 2;
			anglemap[y][x] = (int)(8 * TEXTURE_SIZE * ((atan2(yy, xx) / M_PI) + 1)) % TEXTURE_SIZE;
		}
	}
	double rotate_speed = TEXTURE_SIZE;
	double move_speed = TEXTURE_SIZE;
	double angle_offset = 0;
	double dist_offset = 0;

	bool quit = false;
	Uint32 curr = SDL_GetTicks();
	Uint32 prev = curr;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
						case SDLK_LEFT:
						case SDLK_RIGHT:
							rotate_speed = 0;
							break;
						case SDLK_UP:
						case SDLK_DOWN:
							move_speed = 0;
							break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_LEFT:
							rotate_speed = TEXTURE_SIZE;
							break;
						case SDLK_RIGHT:
							rotate_speed = -TEXTURE_SIZE;
							break;
						case SDLK_UP:
							move_speed = TEXTURE_SIZE;
							break;
						case SDLK_DOWN:
							move_speed = -TEXTURE_SIZE;
							break;
						case SDLK_RETURN:
							fps_on = !fps_on;
							break;
						case SDLK_ESCAPE:
							quit = true;
							break;
					}
					break;
				case SDL_QUIT:
					quit = true;
					break;
			}
		}
		curr = SDL_GetTicks();
		Uint32 delta = curr - prev;
		prev = curr;
		double dt = delta / 1000.0;
		fps_counter(dt);

		angle_offset += rotate_speed * dt;
		if (angle_offset < 0)
			angle_offset += TEXTURE_SIZE;
		else if (angle_offset > TEXTURE_SIZE)
			angle_offset -= TEXTURE_SIZE;
		dist_offset += move_speed * dt;
		if (dist_offset < 0)
			dist_offset += TEXTURE_SIZE;
		else if (dist_offset > TEXTURE_SIZE)
			dist_offset -= TEXTURE_SIZE;
		int aoff = angle_offset;
		int doff = dist_offset;

		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_LockSurface(screen);
		for (int y = 0; y < SCREEN_HEIGHT; ++y)
			for (int x = 0; x < SCREEN_WIDTH; ++x)
			{
				int tx = (anglemap[y][x] + aoff) % TEXTURE_SIZE;
				int ty = (distmap[y][x] + doff) % TEXTURE_SIZE;
				Uint32 c = getPixel(texture, tx, ty);
				setPixel(screen, x, y, c);
			}
		SDL_UnlockSurface(screen);
		SDL_BlitSurface(shadow, NULL, screen, NULL);
		if (fps_on)
			fps_draw();
		SDL_Flip(screen);
	}
	SDL_Quit();
	return 0;
}