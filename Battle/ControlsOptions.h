#ifndef _CONTROLSOPTIONS_H
#define _CONTROLSOPTIONS_H
#include "OptionsScreen.h"

class ControlsOptions : public OptionsScreen {
public:
	ControlsOptions(GameInput * input);

	void run();
	void item_selected();
private:
	GameInput * input;
	GameInput * new_input;

	void redefine_keyboard();
	int poll_keyboard(const char * question);
	void redefine_joystick();
	void poll_joystick(int action, const char * question);
	void show_notification(const char * text);
};

class JoystickSelect : public OptionsScreen {
public:
	JoystickSelect(int index);

	void item_selected();

	int index;
private:
	int cancel_idx;
};

#endif
