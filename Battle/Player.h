#ifndef _PLAYER_H
#define _PLAYER_H

#define FACE_LEFT 0
#define FACE_RIGHT 1

#define SPR_R		0
#define SPR_R_WALK1	1
#define SPR_R_WALK2	2
#define SPR_R_WALK3	3
#define SPR_R_BRAKE	4
#define SPR_R_JUMP	5
#define SPR_L		6
#define SPR_L_WALK1	7
#define SPR_L_WALK2	8
#define SPR_L_WALK3	9
#define SPR_L_BRAKE	10
#define SPR_L_JUMP	11

#define CYCLE_UP 0
#define CYCLE_DN 1

class Player {
public:
	Player(const char * sprite_file);
	~Player();
	SDL_Rect * position;
	SDL_Surface * sprites;
	SDL_Rect * clip[20];

	int momentumx, momentumy;
	int last_speedx, last_speedy;

	bool is_running;

	bool is_jumping, is_falling;
	int jump_start;

	int current_sprite;
	int cycle_direction;
	int distance_walked;

	bool keydn_l, keydn_r, keydn_u, keydn_run, keydn_shoot;
	int key_l, key_r, key_u, key_run, key_shoot;
	int joystick_idx;
	int js_btn_u, js_btn_run, js_btn_shoot, js_btn_start;

	bool is_hit;
	int hit_start;
	int hit_delay;
	int hit_flicker_frame;

	int shoot_start;
	int shoot_delay;

	int score;
	int hitpoints;

	static const int jump_height;

	void handle_input(SDL_Event * event);
	void show(SDL_Surface * screen);
	void set_sprite(int sprite);
	void cycle_sprite(int first, int last);
	void cycle_sprite_updown(int first, int last);

private:
	void load_images(const char * sprite_file);
	void free_images();
	void set_clips();
};

#endif
