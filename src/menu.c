#include <stdint.h>
#include <stdio.h>
#include "terminal.h"
#include "input.h"
#include "menu.h"

static int menu_display(Menu *menu) {
	int i;
	
	for(i = 0; i < menu->items; i++) {
		printf("\t");
		if(i == menu->selected) {
			terminal_set_bg(TERMINAL_COLOR_LIGHT_GRAY);
			terminal_set_fg(TERMINAL_COLOR_BLACK);
			
			printf("[%s]\n", menu->item[i].text);
			
			terminal_set_fg(TERMINAL_COLOR_LIGHT_GRAY);
			terminal_set_bg(TERMINAL_COLOR_BLACK);
		} else {
			printf(" %s \n", menu->item[i].text);
		}
	}
	
	if(menu->has_back) {
		printf("\t");
		if(i == menu->selected) {
			terminal_set_bg(TERMINAL_COLOR_LIGHT_GRAY);
			terminal_set_fg(TERMINAL_COLOR_BLACK);
			
			printf("[Go back]\n");
			
			terminal_set_fg(TERMINAL_COLOR_LIGHT_GRAY);
			terminal_set_bg(TERMINAL_COLOR_BLACK);
		} else {
			printf(" Go back \n");
		}
	}
	
	printf("\n");
	return 0;
}

void menu_execute(void *m) {
	Menu *menu = m;
	InputKeyboardEvent key_ev;
	int x, y;
	
	menu->header(menu->arg);
	terminal_get_pos(&x, &y);
	
	for(;;) {
		terminal_set_pos(x, y);
		menu_display(menu);
		
		//btn = input_poll();
		input_poll();
		key_ev = input_keyboard_event_pop();
		
		if(key_ev.type != INPUT_KEYBOARD_EVENT_TYPE_PRESS)
			continue;
		
		if(key_ev.keysym == INPUT_KEY_UP) {
			if(menu->selected > 0)
				menu->selected--;
			
			continue;
		}
		
		if(key_ev.keysym == INPUT_KEY_DOWN) {
			if(menu->selected < menu->items + menu->has_back - 1)
				menu->selected++;
			
			continue;
		}
		
		if(key_ev.keysym == INPUT_KEY_RETURN) {
			if(menu->selected == menu->items)
				return;
			
			if(menu->item[menu->selected].func)
				menu->item[menu->selected].func(menu->item[menu->selected].arg);
			
			menu->header(menu->arg);
			terminal_get_pos(&x, &y);
			
			continue;
		}
	}
}
