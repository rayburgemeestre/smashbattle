#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"

#include "Main.h"
#include "Battle.h"
#include "Player.h"

#include "CharacterSelect.h"

#define CHARACTERS_PER_LINE 5
#define CHARACTER_WIDTH 44
#define CHARACTER_SPACING 4

#define DIRECTION_LEFT 0
#define DIRECTION_RIGHT 1
#define DIRECTION_UP 2
#define DIRECTION_DOWN 3

CharacterSelect::CharacterSelect() {

}

void CharacterSelect::run() {
	SDL_Event event;
	ControlScheme controls1, controls2;

	load_fonts();
	load_sprites();

	ready1 = false;
	ready2 = false;

	ready = false;

	controls1 = Main::instance->controls1;
	controls2 = Main::instance->controls2;

	select1 = 0;
	if(Battle::CHARACTER_COUNT >= CHARACTERS_PER_LINE)
		select2 = CHARACTERS_PER_LINE - 1;
	else
		select2 = Battle::CHARACTER_COUNT;

	frame = 0;

	while (Main::running && !ready) {
		while(SDL_PollEvent(&event)) {
			Main::instance->handle_event(&event);
			if(event.type == SDL_KEYDOWN) {
				// Keyboard 1
				if(controls1.use_keyboard) {
					if(event.key.keysym.sym == controls1.kb_left && !ready1)
						select(&select1, DIRECTION_LEFT);
					else if(event.key.keysym.sym == controls1.kb_right && !ready1)
						select(&select1, DIRECTION_RIGHT);
					if(event.key.keysym.sym == controls1.kb_up && !ready1)
						select(&select1, DIRECTION_UP);
					else if(event.key.keysym.sym == controls1.kb_down && !ready1)
						select(&select1, DIRECTION_DOWN);
					else if(event.key.keysym.sym == controls1.kb_shoot || 
						event.key.keysym.sym == controls1.kb_run ||
						(controls1.kb_up != controls1.kb_jump &&
						event.key.keysym.sym == controls1.kb_jump)) {
							ready1 = !ready1;
							if(ready1) {
								flicker1 = true;
								flicker1_start = frame;
								flicker1_frame = 0;
							}
							Main::audio->play(SND_SELECT_CHARACTER);
					}
				}
				// Keyboard 2
				if(controls2.use_keyboard) {
					if(event.key.keysym.sym == controls2.kb_left && !ready2)
						select(&select2, DIRECTION_LEFT);
					else if(event.key.keysym.sym == controls2.kb_right && !ready2)
						select(&select2, DIRECTION_RIGHT);
					if(event.key.keysym.sym == controls2.kb_up && !ready1) 
						select(&select2, DIRECTION_UP);
					else if(event.key.keysym.sym == controls2.kb_down && !ready1)
						select(&select2, DIRECTION_DOWN);
					else if(event.key.keysym.sym == controls2.kb_shoot || 
						event.key.keysym.sym == controls2.kb_run ||
						(controls2.kb_up != controls2.kb_jump &&
						event.key.keysym.sym == controls2.kb_jump)) {
							ready2 = !ready2;
							if(ready2) {
								flicker2 = true;
								flicker2_start = frame;
								flicker2_frame = 0;
							}
							Main::audio->play(SND_SELECT_CHARACTER);
					}
				}
			}
			if(event.type == SDL_JOYBUTTONDOWN) {
				// Joystick 1 Buttons
				if(controls1.use_joystick && event.jbutton.which == controls1.joystick_idx) {
					if(event.jbutton.button == controls1.js_left && !ready1)
						select(&select1, DIRECTION_LEFT);
					if(event.jbutton.button == controls1.js_right && !ready1)
						select(&select1, DIRECTION_RIGHT);
					if(event.jbutton.button == controls1.js_jump ||
						event.jbutton.button == controls1.js_run ||
						event.jbutton.button == controls1.js_shoot) {
							ready1 = !ready1;
							if(ready1) {
								flicker1 = true;
								flicker1_start = frame;
								flicker1_frame = 0;
							}
							Main::audio->play(SND_SELECT_CHARACTER);
					}
				}
				// Joystick 2 Buttons
				if(controls2.use_joystick && event.jbutton.which == controls2.joystick_idx) {
					if(event.jbutton.button == controls2.js_left && !ready2)
						select(&select2, DIRECTION_LEFT);
					if(event.jbutton.button == controls2.js_right && !ready2)
						select(&select2, DIRECTION_RIGHT);
					if(event.jbutton.button == controls2.js_jump ||
						event.jbutton.button == controls2.js_run ||
						event.jbutton.button == controls2.js_shoot) {
							ready2 = !ready2;
							if(ready2) {
								flicker2 = true;
								flicker2_start = frame;
								flicker2_frame = 0;
							}
							Main::audio->play(SND_SELECT_CHARACTER);
					}
				}
			}
			if(event.type == SDL_JOYAXISMOTION) {
				// Joystick 1 Axis
				if(controls1.use_joystick && event.jbutton.which == controls1.joystick_idx) {
					if(event.jaxis.axis == 0) {
						if(event.jaxis.value < -6400 && !ready1)
							select(&select1, DIRECTION_LEFT);
						else if(event.jaxis.value > 6400 && !ready1)
							select(&select1, DIRECTION_RIGHT);
					} else {
						if(event.jaxis.value < -6400 && !ready1)
							select(&select1, DIRECTION_UP);
						else if(event.jaxis.value > 6400 && !ready1)
							select(&select1, DIRECTION_DOWN);
					}
				}
				// Joystick 2 Axis
				if(controls2.use_joystick && event.jbutton.which == controls2.joystick_idx) {
					if(event.jaxis.axis == 0) {
						if(event.jaxis.value < -6400 && !ready2)
							select(&select2, DIRECTION_LEFT);
						else if(event.jaxis.value > 6400 && !ready2)
							select(&select2, DIRECTION_RIGHT);
					} else {
						if(event.jaxis.value < -6400 && !ready1)
							select(&select2, DIRECTION_UP);
						else if(event.jaxis.value > 6400 && !ready1)
							select(&select2, DIRECTION_DOWN);
					}
				}
			}
		}

		if(ready1 && ready2 && !flicker1 && !flicker2)
			ready = true;

		name1 = Battle::characters[select1].name;
		name2 = Battle::characters[select2].name;

		file1 = Battle::characters[select1].filename;
		file2 = Battle::characters[select2].filename;

		draw();

		frame++;
		Main::instance->flip();
	}

	Main::audio->stop_music();

	free_sprites();
	free_fonts();
}

void CharacterSelect::select(int * select, int direction) {
	int * other;

	other = (select == &select1) ? &select2 : &select1;

	switch(direction) {
		case DIRECTION_LEFT:
			if(*select % CHARACTERS_PER_LINE == 0)
				*select += CHARACTERS_PER_LINE;
			*select -= 1;
			break;
		case DIRECTION_RIGHT:
			if(*select % CHARACTERS_PER_LINE == CHARACTERS_PER_LINE - 1)
				*select -= CHARACTERS_PER_LINE;
			*select += 1;
			break;
		case DIRECTION_UP:
			*select -= CHARACTERS_PER_LINE;
			break;
		case DIRECTION_DOWN:
			*select += CHARACTERS_PER_LINE;
			break;
	}

	while(*select < Battle::CHARACTER_COUNT) {
		*select += Battle::CHARACTER_COUNT;
	}
	while(*select >= Battle::CHARACTER_COUNT) {
		*select -= Battle::CHARACTER_COUNT;
	}

	if(*select == *other) {
		this->select(select, direction);
	}
}

void CharacterSelect::draw() {
	SDL_Surface * screen;
	SDL_Surface * surface;
	SDL_Rect rect, rect_b;
	SDL_Rect * clip;
	Uint32 color;

	screen = Main::instance->screen;

	SDL_FillRect(screen, NULL, 0);

	rect_b.x = (screen->clip_rect.w - ((clip_avatar->w + (CHARACTER_SPACING * 2)) * CHARACTERS_PER_LINE)) / 2;
	rect_b.y = 20;
	rect_b.w = clip_avatar->w + (CHARACTER_SPACING * 2);
	rect_b.h = clip_avatar->h + (CHARACTER_SPACING * 2);

	for(int idx = 0; idx < Battle::CHARACTER_COUNT; idx++) {
		if(idx > 0 && idx % CHARACTERS_PER_LINE == 0) {
			rect_b.x = (screen->clip_rect.w - ((clip_avatar->w + (CHARACTER_SPACING * 2)) * CHARACTERS_PER_LINE)) / 2;
			rect_b.y += rect_b.h;
		}

		rect.x = rect_b.x + CHARACTER_SPACING;
		rect.y = rect_b.y + CHARACTER_SPACING;

		clip = clip_avatar;

		color = 0;

		if(select1 == idx) {
			color = 0xff0000;
			if(ready1 && flicker1) {
				if(flicker1_frame > 0x20)
					flicker1 = false;
				if(flicker1_frame & 0x4)
					color = 0xffffff;
				flicker1_frame++;
			}
			if(ready1) clip = clip_avatar_selected;
		}
		if(select2 == idx) {
			color = 0x0000ff;
			if(ready2 && flicker2) {
				if(flicker2_frame > 0x20)
					flicker2 = false;
				if(flicker2_frame & 0x4)
					color = 0xffffff;
				flicker2_frame++;
			}
			if(ready2) clip = clip_avatar_selected;
		}
		SDL_FillRect(screen, &rect_b, color);

		SDL_BlitSurface(character_sprites->at(idx), clip, screen, &rect);

		rect_b.x += clip_avatar->w + (CHARACTER_SPACING * 2);
	}

	// Player 1
	rect.x = 40;
	rect.y = 40;
	SDL_BlitSurface(character_sprites->at(select1), clip_left, screen, &rect);

	surface = TTF_RenderText_Solid(font26, name1, fontColor);
	rect.x = 50 + PLAYER_W;
	rect.y = 45;
	SDL_BlitSurface(surface, NULL, screen, &rect);
	SDL_FreeSurface(surface);

	if(ready1) {
		surface = TTF_RenderText_Solid(font26, "READY", fontColor);
		rect.x = 50 + PLAYER_W;
		rect.y = 65;
		SDL_BlitSurface(surface, NULL, screen, &rect);
		SDL_FreeSurface(surface);
	}

	// Player 2
	rect.x = screen->clip_rect.w - PLAYER_W - 40;
	rect.y = 40;
	SDL_BlitSurface(character_sprites->at(select2), clip_right, screen, &rect);

	surface = TTF_RenderText_Solid(font26, name2, fontColor);
	rect.x = screen->clip_rect.w - PLAYER_W - surface->clip_rect.w - 50;
	rect.y = 45;
	SDL_BlitSurface(surface, NULL, screen, &rect);
	SDL_FreeSurface(surface);

	if(ready2) {
		surface = TTF_RenderText_Solid(font26, "READY", fontColor);
		rect.x = screen->clip_rect.w - PLAYER_W - surface->clip_rect.w - 50;
		rect.y = 65;
		SDL_BlitSurface(surface, NULL, screen, &rect);
		SDL_FreeSurface(surface);
	}
}

void CharacterSelect::load_fonts() {
	font26 = TTF_OpenFont("fonts/slick.ttf", 26);
	font13 = TTF_OpenFont("fonts/slick.ttf", 13);
	fontColor.r = 255;
	fontColor.g = 255;
	fontColor.b = 255;
}

void CharacterSelect::free_fonts() {
	TTF_CloseFont(font26);
	TTF_CloseFont(font13);
}

void CharacterSelect::load_sprites() {
	SDL_Surface * loaded;
	SDL_Surface * sprites;
	Uint32 colorkey;

	character_sprites = new std::vector<SDL_Surface*>(0);

	for(int idx = 0; idx < Battle::CHARACTER_COUNT; idx++) {
		loaded = SDL_LoadBMP(Battle::characters[idx].filename);
		sprites = SDL_DisplayFormat(loaded);
		SDL_FreeSurface(loaded);
		colorkey = SDL_MapRGB(sprites->format, 0, 255, 255);
		SDL_SetColorKey(sprites, SDL_SRCCOLORKEY, colorkey);
		
		character_sprites->push_back(sprites);
	}

	clip_avatar = new SDL_Rect();
	clip_avatar->x = 220;
	clip_avatar->y = 0;
	clip_avatar->w = 44;
	clip_avatar->h = 44;

	clip_avatar_selected = new SDL_Rect();
	clip_avatar_selected->x = 220;
	clip_avatar_selected->y = 44;
	clip_avatar_selected->w = 44;
	clip_avatar_selected->h = 44;
	
	clip_left = new SDL_Rect();
	clip_left->x = 0;
	clip_left->y = 0;
	clip_left->w = PLAYER_W;
	clip_left->h = PLAYER_H;
	
	clip_right = new SDL_Rect();
	clip_right->x = 0;
	clip_right->y = PLAYER_H;
	clip_right->w = PLAYER_W;
	clip_right->h = PLAYER_H;
}

void CharacterSelect::free_sprites() {
	for(unsigned int idx = 0; idx < character_sprites->size(); idx++) {
		SDL_FreeSurface(character_sprites->at(idx));
		character_sprites->erase(character_sprites->begin() + idx);
	}
	delete character_sprites;

	delete clip_avatar;
	delete clip_avatar_selected;
	delete clip_left;
	delete clip_right;
}