// Super Roomba.cpp : Defines the entry point for the console application.
//

#include "simulator.h"
#include <iostream>
#include <chrono>
#include <random>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#undef main

using namespace std;

#define SCALE_FACTOR 200

void draw_circle(SDL_Renderer *ren, Vec center, float radius, Uint32 color) {
	/*const int POINTS = 32;
	SDL_Point pts[POINTS+1];
	for (int i = 0; i < POINTS; ++i) {
		pts[i].x = center.x + radius*cos(i*2.0f*PI / float(POINTS));
		pts[i].y = 800 - center.y + radius*sin(i*2.0f*PI / float(POINTS));
	}
	pts[POINTS] = pts[0];
	SDL_RenderDrawLines(ren, pts, POINTS+1);*/

	uint32_t real_color = ((color & 0xFF) << 24) | ((color & 0xFF00) << 8) | ((color >> 8) & 0xFF00) | ((color >> 24) & 0xFF);
	aacircleColor(ren, (int)center.x, int(800-center.y), int(radius), real_color);
}

void draw_room(SDL_Renderer *ren, Room r) {
	SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
	SDL_Point* pts = (SDL_Point*)malloc(sizeof(SDL_Point)*(r.corners.size()+1));
	if (pts == 0) {
		return;
	}
	int i = 0;
	for (Vec pt : r.corners) {
		pts[i] = { 100+int(pt.x * SCALE_FACTOR), 100+int(800-(pt.y * SCALE_FACTOR)) };// 1 pixel is 2.5 mm
		++i;
	}
	pts[i] = pts[0];
	SDL_RenderDrawLines(ren, pts, r.corners.size()+1);
	free(pts);
	for (Obstacle ob : r.obstacles) {
		draw_circle(ren, ob.loc * SCALE_FACTOR + Vec{ 100,-100 }, ob.r * SCALE_FACTOR,0xFFFFFFFF);
	}
}

void draw_robot(SDL_Renderer *ren, Roomba r) {
	draw_circle(ren, r.loc * SCALE_FACTOR + Vec{100, -100}, r.r * SCALE_FACTOR,0x00FF00FF);
	aalineRGBA(ren, r.loc.x * SCALE_FACTOR+100, 100 + 800 - r.loc.y * SCALE_FACTOR,
		                    100+(r.loc.x+cos(r.theta)*r.r) * SCALE_FACTOR, 100 + 800 - (r.loc.y + sin(r.theta)*r.r) * SCALE_FACTOR,128,255,128,255);
}

void draw_on_map(SDL_Renderer* ren, Roomba r) {
	vector<Sint16> x, y;
	x.resize(4);
	y.resize(4);
	x[0] = int(-0.05f * SCALE_FACTOR);
	y[0] = int( 0.09f * SCALE_FACTOR);
	x[1] = 0;
	y[1] = int( 0.09f * SCALE_FACTOR);
	x[2] = 0;
	y[2] = int(-0.09f * SCALE_FACTOR);
	x[3] = int(-0.05f * SCALE_FACTOR);
	y[3] = int(-0.09f * SCALE_FACTOR);
	for (int i = 0; i < 4; ++i) {
		Vec res = rotate({ (float)x[i],(float)y[i] }, r.theta);
		x[i] = int(res.x)+int(r.loc.x*SCALE_FACTOR)+100;
		y[i] = 900-(int(res.y) + int(r.loc.y * SCALE_FACTOR));
	}
	//SDL_SetRenderDrawColor(ren, 128, 128, 255, 10);

	filledPolygonRGBA(ren, x.data(), y.data(), 4, 128, 128, 255, 10);
}

void add_sub_room(Room &r, int start, int end, decltype(default_random_engine()) &eng) {
	float max_width = dist(r.corners[start],r.corners[end]);
	Vec par = (r.corners[end] - r.corners[start]) / max_width;
	Vec perp = rotate(par,-PI/2);
	float width = uniform_real_distribution<float>(0.18f, max_width*0.5f)(eng);
	float location = uniform_real_distribution<float>(0.0f, max_width-width)(eng);
	float depth = uniform_real_distribution<float>(0.18f, width)(eng);
	Vec p1 = r.corners[start] + par*location;
	r.corners.insert(r.corners.begin() + start + 1, p1);
	Vec p2 = p1 + perp*depth;
	r.corners.insert(r.corners.begin() + start + 2, p2);
	Vec p3 = p2 + par*width;
	r.corners.insert(r.corners.begin() + start + 3, p3);
	Vec p4 = p3 - perp*depth;
	r.corners.insert(r.corners.begin() + start + 4, p4);
}

void generate_space(Room &r, const Vec max_size, decltype(default_random_engine()) &eng) {
	auto prob = uniform_real_distribution<float>(0.0f, 1.0f);
	Vec space_size = { uniform_real_distribution<float>(2.0f,max_size.x)(eng), uniform_real_distribution<float>(2.0f,max_size.y)(eng) };
	for (int i = 0; i < 3; ++i) {// Four walls, three corners, first corner is always pre-existing
		
		Vec direction;
		switch (i) {
		case 0:
			direction = { space_size.x,0 };
			break;
		case 1:
			direction = { 0,space_size.y };
			break;
		case 2:
			direction = { -space_size.x,0 };
		}
		r.corners.push_back(r.corners.back() + direction);
	}
	// Add sub-rooms
	for (int i = 0; i < r.corners.size(); ++i) {
		if (prob(eng) < 0.4) {
			add_sub_room(r, i, (i + 1) % r.corners.size(),eng);
			i += 4;
		}
	}
}

void generate_walls(Room &r, decltype(default_random_engine()) &eng) {
	Vec max_size = { 3.0f,3.0f };
	r.corners.push_back({ 0,0 });
	generate_space(r, max_size, eng);
}

void generate_chair(Room &r, Vec loc, float theta, float size, float rad) {
	vector<Vec> pts;
	pts.resize(4);
	pts[0] = { 0,0 };
	pts[1] = { size,0 };
	pts[2] = { size, size };
	pts[3] = { 0,size };
	for (auto pt : pts) {
		Vec res = rotate(pt, theta);
		res = res + loc;
		r.obstacles.push_back({ res,rad });
	}
}

void generate_obstacles(Room &r, decltype(default_random_engine()) &eng) {
	generate_chair(r, { 1.3f, 1.3f }, 0.1f, 0.4f, 0.03f);
}

Room generate_room(decltype(default_random_engine()) &eng) {
	Room r;
	generate_walls(r,eng);
	generate_obstacles(r,eng);
	return r;
}

int main()
{
	auto eng = default_random_engine();
	Room r = generate_room(eng);
	/*r.corners.push_back({ 0,0 });
	r.corners.push_back({ 2.0f,0 });

	r.corners.push_back({ 2.0f,0.75f });
	r.corners.push_back({ 2.5f,0.75f });
	r.corners.push_back({ 2.5f,1.5f });
	r.corners.push_back({ 2.0f,1.5f });

	r.corners.push_back({ 2.0f,2.0f });

	r.corners.push_back({ 1.75f,2.0f });
	r.corners.push_back({ 1.75f,4.0f });
	r.corners.push_back({ 1.5f,4.25f });
	r.corners.push_back({ 0.5f,4.25f });
	r.corners.push_back({ 0.25f,4.0f });
	r.corners.push_back({ 0.25f,2.0f });

	r.corners.push_back({ 0,2.0f });
	r.obstacles.push_back({ { 1.3f,1.3f }, 0.03f });
	r.obstacles.push_back({ { 1.3f,1.7f }, 0.03f });
	r.obstacles.push_back({ { 1.7f,1.7f }, 0.03f });
	r.obstacles.push_back({ { 1.7f,1.3f }, 0.03f });*/
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

	SDL_Surface *map = SDL_CreateRGBSurface(0, 1000, 1000, 32, 0, 0, 0, 0);

	SDL_Renderer *map_render = SDL_CreateSoftwareRenderer(map);

	SDL_SetRenderDrawColor(map_render, 0, 0, 0, 0);
	SDL_RenderClear(map_render);

	SDL_Texture *map_text = SDL_CreateTextureFromSurface(ren, map);

	bool running = true;
	/*auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < 3600*100; ++i) {// Run for one hour; 3600 seconds * 100 ticks per second
		simulate(bot, r);
		draw_on_map(map_render, bot);
	}
	auto end = chrono::high_resolution_clock::now();
	chrono::duration<double, milli> time = end - start;
	cout << "Iterations per second: " << 3600*100*1000/time.count() << endl;*/
	while (running) {
		/*for (int i = 0; i < 20; ++i) {
			simulate(bot, r);
			draw_on_map(map_render, bot);
		}*/
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
		SDL_RenderClear(ren);
		SDL_DestroyTexture(map_text);
		map_text = SDL_CreateTextureFromSurface(ren, map);
		SDL_RenderCopy(ren, map_text, NULL, NULL);
		draw_room(ren,r);
		draw_robot(ren,bot);
		//draw_circle(ren, { 0,0 }, 50, 0xFFFFFFFF);
		SDL_RenderPresent(ren);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) {
					running = false;
				}
				else {
					r = generate_room(eng);
				}
			}
			if (e.type == SDL_QUIT) {
				running = false;
			}
		}
		SDL_Delay(16);
	}
	SDL_FreeSurface(map);
	SDL_DestroyTexture(map_text);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
    return 0;
}

