// Super Roomba.cpp : Defines the entry point for the console application.
//

#include "simulator.h"
#include <iostream>
#include <SDL2/SDL.h>
#undef main

using namespace std;

void draw_circle(SDL_Renderer *ren, Vec center, float radius) {
	const int POINTS = 32;
	SDL_Point pts[POINTS+1];
	for (int i = 0; i < POINTS; ++i) {
		pts[i].x = center.x + radius*cos(i*2.0f*PI / float(POINTS));
		pts[i].y = 800 - center.y + radius*sin(i*2.0f*PI / float(POINTS));
	}
	pts[POINTS] = pts[0];
	SDL_RenderDrawLines(ren, pts, POINTS+1);
}

void draw_room(SDL_Renderer *ren, Room r) {
	SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
	SDL_Point* pts = (SDL_Point*)malloc(sizeof(SDL_Point)*(r.corners.size()+1));
	if (pts == 0) {
		return;
	}
	int i = 0;
	for (Vec pt : r.corners) {
		pts[i] = { 100+int(pt.x * 400), 100+int(800-(pt.y * 400)) };// 1 pixel is 2.5 mm
		++i;
	}
	pts[i] = pts[0];
	SDL_RenderDrawLines(ren, pts, r.corners.size()+1);
	free(pts);
	for (Obstacle ob : r.obstacles) {
		draw_circle(ren, ob.loc * 400 + Vec{ 100,-100 }, ob.r * 400);
	}
}

void draw_robot(SDL_Renderer *ren, Roomba r) {
	SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
	draw_circle(ren, r.loc * 400 + Vec{100, -100}, r.r * 400);
	SDL_SetRenderDrawColor(ren, 128, 255, 128, 255);
	SDL_RenderDrawLine(ren, r.loc.x * 400+100, 100 + 800 - r.loc.y * 400,
		                    100+(r.loc.x+cos(r.theta)*r.r) * 400, 100 + 800 - (r.loc.y + sin(r.theta)*r.r) * 400);
}

int main()
{
	Room r;
	r.corners.push_back({ 0,0 });
	r.corners.push_back({ 2.0f,0 });
	r.corners.push_back({ 2.0f,2.0f });
	r.corners.push_back({ 0,2.0f });
	r.obstacles.push_back({ {1.0f,1.0f}, 0.03f });
	Roomba bot;
	bot.loc = { 0.2f,0.2f };
	bot.theta = 20.0f/180*PI;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		cout << "ERROR!" << endl;
		return 1;
	}
	
	SDL_Window *win = SDL_CreateWindow("Super Roomba", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000, 1000, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		cout << "ERROR!!" << endl;
		SDL_Quit();
		return 2;
	}

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr) {
		SDL_DestroyWindow(win);
		cout << "ERROR!!!" << endl;
		SDL_Quit();
		return 3;
	}
	bool running = true;
	while (running) {
		simulate(bot, r);
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
		SDL_RenderClear(ren);
		draw_room(ren,r);
		draw_robot(ren,bot);
		SDL_RenderPresent(ren);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_KEYDOWN) {
				running = false;
			}
			if (e.type == SDL_QUIT) {
				running = false;
			}
		}
		SDL_Delay(16);
	}
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
    return 0;
}

