#include "SDL/SDL.h"

#include "Main.h"
#include "Player.h"

#include <vector>

#include "LevelSelect.h"

#ifndef WIN32
#define sprintf_s snprintf
#endif

#define LEVELS_PER_LINE	4
#define LEVEL_WIDTH		44
#define LEVEL_HEIGHT	34
#define LEVEL_SPACING	4

#define DIRECTION_NONE	0
#define DIRECTION_LEFT	1
#define DIRECTION_RIGHT	2
#define DIRECTION_UP	4
#define DIRECTION_DOWN	8

#define MENU_TOP_OFFSET 180
#define TILES_COLS		10
#define TILES_ROWS		6

LevelSelect::LevelSelect(int players) {
	this->players = players;
}

void LevelSelect::run() {
	SDL_Event event;

	load_sprites();

	for(int i = 0; i < players; i++) {
		cursor_direction[i] = DIRECTION_NONE;
		cursor_direction_start[i] = 0;
		cursor_first[i] = true;
		cursor_enter[i] = false;
	}
	controls[0] = Main::instance->controls1;
	controls[1] = Main::instance->controls2;
	controls[2] = Main::instance->controls3;
	controls[3] = Main::instance->controls4;

	ready = false;
	ready_level = false;
	cancel = false;

	random = false;
	random_start = 0;
	random_before = 0;

	srand(SDL_GetTicks());
	
	level = 0;
	select(DIRECTION_NONE);

	frame = 0;

	while (Main::running && !ready) {
		while(SDL_PollEvent(&event)) {
			Main::instance->handle_event(&event);
			handle_input(&event);
		}

		process_cursors();

		process_random();

		frame++;

		draw();

		if(ready_level && flicker_frame > 0x20) {
			ready = true;
		}

		Main::instance->flip();
	}

	if(!ready)
		cancel = true;

	free_sprites();
}

void LevelSelect::handle_input(SDL_Event * event) {
	int old_direction[4];

	for(int i = 0; i < players; i++) {
		old_direction[i] = cursor_direction[i];

		// Keyboard
		if(event->type == SDL_KEYDOWN) {
			// Escape key always returns to main menu
			if(event->key.keysym.sym == SDLK_ESCAPE) {
				ready = true;
				cancel = true;
			}

			if(controls[i].use_keyboard) {
				if(event->key.keysym.sym == controls[i].kb_left)
					cursor_direction[i] |= DIRECTION_LEFT;
				if(event->key.keysym.sym == controls[i].kb_right)
					cursor_direction[i] |= DIRECTION_RIGHT;
				if(event->key.keysym.sym == controls[i].kb_up)
					cursor_direction[i] |= DIRECTION_UP;
				if(event->key.keysym.sym == controls[i].kb_down)
					cursor_direction[i] |= DIRECTION_DOWN;
				else if(event->key.keysym.sym == controls[i].kb_shoot || 
					event->key.keysym.sym == controls[i].kb_run ||
					(controls[i].kb_up != controls[i].kb_jump &&
					event->key.keysym.sym == controls[i].kb_jump)) {
						cursor_enter[i] = true;
				}
			}
		}
		if(event->type == SDL_KEYUP) {
			if(controls[i].use_keyboard) {
				if(event->key.keysym.sym == controls[i].kb_left && cursor_direction[i] & DIRECTION_LEFT)
					cursor_direction[i] ^= DIRECTION_LEFT;
				if(event->key.keysym.sym == controls[i].kb_right && cursor_direction[i] & DIRECTION_RIGHT)
					cursor_direction[i] ^= DIRECTION_RIGHT;
				if(event->key.keysym.sym == controls[i].kb_up && cursor_direction[i] & DIRECTION_UP)
					cursor_direction[i] ^= DIRECTION_UP;
				if(event->key.keysym.sym == controls[i].kb_down && cursor_direction[i] & DIRECTION_DOWN)
					cursor_direction[i] ^= DIRECTION_DOWN;
				else if(event->key.keysym.sym == controls[i].kb_shoot || 
					event->key.keysym.sym == controls[i].kb_run ||
					(controls[i].kb_up != controls[i].kb_jump &&
					event->key.keysym.sym == controls[i].kb_jump)) {
						cursor_enter[i] = false;
				}
			}
		}
		// Joystick Buttons
		if(event->type == SDL_JOYBUTTONDOWN) {
			if(controls[i].use_joystick && event->jbutton.which == controls[i].joystick_idx) {
				if(event->jbutton.button == controls[i].js_left)
					cursor_direction[i] |= DIRECTION_LEFT;
				if(event->jbutton.button == controls[i].js_right)
					cursor_direction[i] |= DIRECTION_RIGHT;
				if(event->jbutton.button == controls[i].js_jump ||
					event->jbutton.button == controls[i].js_run ||
					event->jbutton.button == controls[i].js_shoot) {
						cursor_enter[i] = true;
				}
			}
		}
		if(event->type == SDL_JOYBUTTONUP) {
			if(controls[i].use_joystick && event->jbutton.which == controls[i].joystick_idx) {
				if(event->jbutton.button == controls[i].js_left && cursor_direction[i] & DIRECTION_LEFT)
					cursor_direction[i] ^= DIRECTION_LEFT;
				if(event->jbutton.button == controls[i].js_right && cursor_direction[i] & DIRECTION_RIGHT)
					cursor_direction[i] ^= DIRECTION_RIGHT;
			}
		}
		// Joystick Axis
		if(event->type == SDL_JOYAXISMOTION) {
			if(controls[i].use_joystick && event->jbutton.which == controls[i].joystick_idx) {
				if(event->jaxis.axis == 0) {
					if(event->jaxis.value < -Main::JOYSTICK_AXIS_THRESHOLD)
						cursor_direction[i] |= DIRECTION_LEFT;
					else if(event->jaxis.value > Main::JOYSTICK_AXIS_THRESHOLD)
						cursor_direction[i] |= DIRECTION_RIGHT;
					else {
						if(cursor_direction[i] & DIRECTION_LEFT)
							cursor_direction[i] ^= DIRECTION_LEFT;
						if(cursor_direction[i] & DIRECTION_RIGHT)
							cursor_direction[i] ^= DIRECTION_RIGHT;
					}
				} 
				if(event->jaxis.axis == 1) {
					if(event->jaxis.value < -Main::JOYSTICK_AXIS_THRESHOLD)
						cursor_direction[i] |= DIRECTION_UP;
					else if(event->jaxis.value > Main::JOYSTICK_AXIS_THRESHOLD)
						cursor_direction[i] |= DIRECTION_DOWN;
					else {
						if(cursor_direction[i] & DIRECTION_UP)
							cursor_direction[i] ^= DIRECTION_UP;
						if(cursor_direction[i] & DIRECTION_DOWN)
							cursor_direction[i] ^= DIRECTION_DOWN;
					}
				}
			}
		}

		if(cursor_direction[i] != old_direction[i]) {
			cursor_first[i] = true;
		}
	}
}

void LevelSelect::process_cursors() {
	int delay;

	for(int i = 0; i < players; i++) {
		// Enter
		if(cursor_enter[i] && !ready_level) {
			cursor_enter[i] = false;
			Main::audio->play(SND_SELECT_CHARACTER);
			if(!ready_level) {
				ready_level = true;

				random = false;

				flicker = true;
				flicker_frame = 0;
			}
		}

		// Direction
		if(cursor_direction[i] != DIRECTION_NONE) {
			if(cursor_first[i])
				delay = 0;
			else
				delay = Main::CONTROLS_REPEAT_SPEED;
			if(frame - cursor_direction_start[i] > delay) {
				cursor_direction_start[i] = frame;
				cursor_first[i] = false;
				if(!ready_level) {
					select(cursor_direction[i]);
					Main::audio->play(SND_SELECT);
				}
			}
		}
	}
}

void LevelSelect::process_random() {
	if(random && (frame - random_start == 6)) {
		int last;
		bool is_last;

		last = level;

		do {
			is_last = false;
			level = rand() % Level::LEVEL_COUNT;

			if(level == last) is_last = true;
		} while(is_last);

		random_start = frame;
	}
}

void LevelSelect::select(int direction) {
	if(!random) {
		if(direction & DIRECTION_LEFT) {
			if(level % LEVELS_PER_LINE == 0)
				level += LEVELS_PER_LINE;
			level--;
		}
		if(direction & DIRECTION_RIGHT) {
			if(level % LEVELS_PER_LINE == LEVELS_PER_LINE - 1)
				level -= LEVELS_PER_LINE;
			level++;
		}
	}
	if(direction & DIRECTION_UP) {
		if(random) {
			if(random_before >= Level::LEVEL_COUNT - LEVELS_PER_LINE) {
				level = random_before;
			} else {
				level = random_before - LEVELS_PER_LINE + Level::LEVEL_COUNT;
			}
			random = false;
		} else if(level < LEVELS_PER_LINE) {
			random = true;
			random_before = level;
			random_start = frame;
		} else {
			level -= LEVELS_PER_LINE;
		}
	}
	if(direction & DIRECTION_DOWN) {
		if(random) {
			if(random_before < LEVELS_PER_LINE) {
				level = random_before;
			} else {
				level = random_before + LEVELS_PER_LINE - Level::LEVEL_COUNT;
			}
			random = false;
		} else if(level >= Level::LEVEL_COUNT - LEVELS_PER_LINE) {
			random = true;
			random_before = level;
			random_start = frame;
		} else {
			level += LEVELS_PER_LINE;
		}
	}

	if(level < 0) level += Level::LEVEL_COUNT;
	if(level >= Level::LEVEL_COUNT) level -= Level::LEVEL_COUNT;
}

void LevelSelect::draw() {
	SDL_Surface * screen;
	SDL_Surface * surface;
	SDL_Rect rect, rect_b, rect_s;
	Uint32 color;
	int width;

	screen = Main::instance->screen;

	SDL_BlitSurface(backgrounds->at(level), NULL, screen, NULL);

	// TILES
	rect.x = (WINDOW_WIDTH - (TILES_COLS * TILE_W)) / 2;
	rect.y = MENU_TOP_OFFSET - 40 - TILE_H;
	rect.w = TILES_COLS * TILE_W;
	rect.h = TILES_ROWS * TILE_H;

	SDL_FillRect(screen, &rect, 0);

	rect_s.x = 0;
	rect_s.y = 0;
	rect_s.w = TILE_W;
	rect_s.h = TILE_H;

	rect.x = (WINDOW_WIDTH - (TILES_COLS * TILE_W)) / 2;
	rect.y = MENU_TOP_OFFSET - 40 - TILE_H;
	for(int i = 0; i < TILES_COLS; i++) {
		SDL_BlitSurface(Main::graphics->tiles, &rect_s, screen, &rect);
		rect.x += TILE_W;
	}

	for(int i = 1; i < TILES_ROWS; i++) {
		rect.x = (WINDOW_WIDTH - (TILES_COLS * TILE_W)) / 2;
		rect.y = (MENU_TOP_OFFSET - 40 - TILE_H) + (TILE_H * i);
		SDL_BlitSurface(Main::graphics->tiles, &rect_s, screen, &rect);
		
		rect.x = rect.x + ((TILES_COLS - 1) * TILE_W);
		SDL_BlitSurface(Main::graphics->tiles, &rect_s, screen, &rect);
	}

	rect.x = (WINDOW_WIDTH - (TILES_COLS * TILE_W)) / 2;
	rect.y = (MENU_TOP_OFFSET - 40 - TILE_H) + (TILES_ROWS * TILE_H);
	for(int i = 0; i < TILES_COLS; i++) {
		SDL_BlitSurface(Main::graphics->tiles, &rect_s, screen, &rect);
		rect.x += TILE_W;
	}

	// STAGES
	width = (LEVEL_WIDTH + (LEVEL_SPACING * 2)) * LEVELS_PER_LINE;
	
	// Stage random
	if(random) {
		rect.x = (WINDOW_WIDTH - width) / 2;
		rect.y = MENU_TOP_OFFSET - 32;
		rect.w = width;
		rect.h = 32;

		color = 0x0088ff;
		SDL_FillRect(screen, &rect, color);

		rect.x += LEVEL_SPACING;
		rect.y += LEVEL_SPACING;
		rect.w -= (LEVEL_SPACING * 2);
		rect.h -= (LEVEL_SPACING * 2);

		SDL_FillRect(screen, &rect, 0);
	}

	surface = Main::graphics->text_random;
	rect.x = (screen->w - surface->w) / 2;
	rect.y = MENU_TOP_OFFSET - 24;
	SDL_BlitSurface(surface, NULL, screen, &rect);

	// Stage thumbnails
	rect_b.x = (screen->w - width) / 2;
	rect_b.y = MENU_TOP_OFFSET;
	rect_b.w = LEVEL_WIDTH + (LEVEL_SPACING * 2);
	rect_b.h = LEVEL_HEIGHT + (LEVEL_SPACING * 2);

	for(int idx = 0; idx < Level::LEVEL_COUNT; idx++) {
		if(idx > 0 && idx % LEVELS_PER_LINE == 0) {
			rect_b.x = (screen->w - ((LEVEL_WIDTH + (LEVEL_SPACING * 2)) * LEVELS_PER_LINE)) / 2;
			rect_b.y += rect_b.h;
		}

		rect.x = rect_b.x + LEVEL_SPACING;
		rect.y = rect_b.y + LEVEL_SPACING;

		color = 0;

		if(level == idx) {
			color = 0x0088ff;
			
			if(ready_level && flicker) {
				if(flicker_frame > 0x20)
					flicker = false;
				if(flicker_frame & 0x4)
					color = 0xffffff;
				flicker_frame++;
			}
		}
		SDL_FillRect(screen, &rect_b, color);

		SDL_BlitSurface(thumbs->at(idx), NULL, screen, &rect);

		rect_b.x += LEVEL_WIDTH + (LEVEL_SPACING * 2);
	}
	
	// Stage name
	surface = Main::text->render_text_medium(Level::LEVELS[level].name);
	rect.x = (screen->w - surface->w) / 2;
	rect.y = MENU_TOP_OFFSET + ((LEVEL_HEIGHT + (LEVEL_SPACING * 2)) * (Level::LEVEL_COUNT / LEVELS_PER_LINE)) + 10;
	SDL_BlitSurface(surface, NULL, screen, &rect);
	SDL_FreeSurface(surface);
}

void LevelSelect::load_sprites() {
	SDL_Surface * surface;

	thumbs = new std::vector<SDL_Surface*>(0);
	backgrounds = new std::vector<SDL_Surface*>(0);

	for(int idx = 0; idx < Level::LEVEL_COUNT; idx++) {
		surface = Level::get_thumbnail(Level::LEVELS[idx].filename);
		thumbs->push_back(surface);

		surface = Level::get_preview(Level::LEVELS[idx].filename);
		backgrounds->push_back(surface);
	}
}

void LevelSelect::free_sprites() {
	for(unsigned int i = 0; i < thumbs->size(); i++) {
		SDL_FreeSurface(thumbs->at(i));
	}
	delete thumbs;

	for(unsigned int i = 0; i < backgrounds->size(); i++) {
		SDL_FreeSurface(backgrounds->at(i));
	}
	delete backgrounds;
}
