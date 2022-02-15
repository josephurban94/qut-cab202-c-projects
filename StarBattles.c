#include <avr/io.h>
#include <math.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "cpu_speed.h"
#include "graphics.h"
#include "lcd.h"
#include "sprite.h" 
#include "usb_serial.h"

#define MAX_TIE_FIGHTERS (8)
#define MAX_MISSILES (5)

/*
* Global variables
*/
Sprite xwing;
Sprite missile[MAX_MISSILES];
Sprite tie_fighter[MAX_TIE_FIGHTERS];
Sprite death_star;
Sprite death_star_health;

const int LCDX = LCD_X - 1; 
const int LCDY = LCD_Y - 1; 
int screen_x_0 = 1, screen_y_0 = 11, screen_x_1 = LCD_X - 2, screen_y_1 = LCD_Y - 2;
int player_lives = 10, player_score = 0;
int total_seconds_int = 0, timer_seconds = 0, timer_minutes = 0;
int game_over_flag = 0;
int is_missile[MAX_MISSILES] = {0};
char player_heading = 'u';
char missile_heading[MAX_MISSILES] = {};
volatile unsigned long overflow_count = 0;
int dash_attack_countdown[MAX_TIE_FIGHTERS] = {0};
int is_tie_fighter[MAX_TIE_FIGHTERS] = {0};
volatile unsigned long timer0_overflow = 0;
volatile unsigned long timer3_overflow = 0;
int start_debugging_flag = 0;
char keyboard_buffer = 'x';
int tie_fighters_killed = 0;
int is_death_star = 0;
int death_star_health_int = 10;
int tie_fighter_respawn = 0;
int death_star_dash_attack_countdown = 0;

void init_hardware();
void init_timers();
void setup_screen();
void setup_io();
void send_debug_string(char* string);
void draw_title_screen();
void draw_countdown();
void draw_border();
void draw_hud();
void reset_values();
double get_system_time(unsigned int timer_count);
void process_time();
void draw_game_over_screen();
void sprite_move();
int random_int(int min, int max);
void process_collisions();
void init_missile(char direction, int missile_number);
void move_missile(int missile_number);
void tie_fighter_spawn(int tie_number);
void xwing_spawn();
void dash_attack(int tie_number);
double get_overall_system_time();
void draw_serial_greeting_screen();
void debug_reporting();
void tie_fighter_move(int tie_number);
void death_star_spawn();
void death_star_health_modify();
void death_star_dash_attack();
void death_star_move();

unsigned char xwing_image[8] = {
	0x18,
	0x18,
	0x99,
	0x99,
	0xBD,
	0xFF,
	0x7E,
	0x24,
};
unsigned char xwing_image_left[8] = {
	0x3C,
	0x06,
	0x0F,
	0xFE,
	0xFE,
	0x0F,
	0x06,
	0x3C,
};
unsigned char xwing_image_right[8] = {
	0x3C,
	0x60,
	0xF0,
	0x7F,
	0x7F,
	0xF0,
	0x60,
	0x3C,
};
unsigned char xwing_image_down[8] = {
	0x24,
	0x7E,
	0xFF,
	0xBD,
	0x99,
	0x99,
	0x18,
	0x18,
};
unsigned char missile_image[8] = {
	0xC0,
	0xC0,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
};
unsigned char tie1_image[8] = {
	0x82,
	0x82,
	0xBA,
	0xEE,
	0xBA,
	0x82,
	0x82,
	0x00,
};
unsigned char death_star_image[] = {
	0x0C,0x00,
	0x3F,0x00,
	0x47,0x80,
	0x57,0x80,
	0xC7,0xC0,
	0xFF,0xC0,
	0x7F,0x80,
	0x7F,0x80,
	0x3F,0x00,
	0x0C,0x00,
};
unsigned char death_star_health_10[] = {
	0xFF,0xF0,
	0xFF,0xF0,
	0xFF,0xF0,
};
unsigned char death_star_health_9[] = {
	0xFF,0xF0,
	0xFF,0xD0,
	0xFF,0xF0,
};
unsigned char death_star_health_8[] = {
	0xFF,0xF0,
	0xFF,0x90,
	0xFF,0xF0,
};
unsigned char death_star_health_7[] = {
	0xFF,0xF0,
	0xFF,0x10,
	0xFF,0xF0,
};
unsigned char death_star_health_6[] = {
	0xFF,0xF0,
	0xFE,0x10,
	0xFF,0xF0,
};
unsigned char death_star_health_5[] = {
	0xFF,0xF0,
	0xFC,0x10,
	0xFF,0xF0,
};
unsigned char death_star_health_4[] = {
	0xFF,0xF0,
	0xF8,0x10,
	0xFF,0xF0,
};
unsigned char death_star_health_3[] = {
	0xFF,0xF0,
	0xF0,0x10,
	0xFF,0xF0,
};
unsigned char death_star_health_2[] = {
	0xFF,0xF0,
	0xE0,0x10,
	0xFF,0xF0,
};
unsigned char death_star_health_1[] = {
	0xFF,0xF0,
	0xC0,0x10,
	0xFF,0xF0,
};
unsigned char death_star_health_0[] = {
	0xFF,0xF0,
	0x80,0x10,
	0xFF,0xF0,
};

int main() {
	init_hardware();
	init_timers();
	clear_screen();
	setup_io();
	death_star_health.x = 150;
	death_star_health.y = 150;
	death_star.x = 150;
	death_star.y = 150;
	draw_serial_greeting_screen();
	while(1) {
		draw_title_screen();
		draw_countdown();
		srand(TCNT0);
		init_sprite(&xwing,random_int(screen_x_0, screen_x_1 - 7),random_int(screen_y_0, screen_y_1 - 7),8,8, xwing_image);
		for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
			tie_fighter_spawn(i);
			is_tie_fighter[i] = 1;
		}
		overflow_count = 0;
		TCNT1 = 0;
		start_debugging_flag = 1;
			
		while (!game_over_flag) {
			
			clear_screen();
			draw_border();
			process_time();
			draw_hud();
			
			keyboard_buffer = usb_serial_getchar();
			if (((PINB>>1) & 0b1) || ((PINB>>7) & 0b1) || ((PIND>>0) & 0b1) || ((PIND>>1) & 0b1)
				|| (keyboard_buffer == 'w') || (keyboard_buffer == 'a') || (keyboard_buffer == 's') || (keyboard_buffer == 'd')) {
				sprite_move();
			}
			
			if (((PINF>>5) & 0b1) || (keyboard_buffer == 'c')) {
				keyboard_buffer = 'x';
				int i = 0;
				int break_flag = 0;
				while ((i < MAX_MISSILES) && (!break_flag)) {
					if (is_missile[i] == 0) {
						is_missile[i] = 1;
						init_missile(player_heading, i);
						break_flag = 1;
					}
					else {
						i++;
					}
				}
			}
			for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
				
				if (is_tie_fighter[i]) {
					if (dash_attack_countdown[i] < 1) {
						dash_attack_countdown[i] = random_int(3,5);
					}
					
					if ((total_seconds_int % 7) == dash_attack_countdown[i]) {
						dash_attack(i);
						dash_attack_countdown[i] = 0;
					}
				}
			}
			if (is_death_star) {
				if (death_star_dash_attack_countdown < 1) {
					death_star_dash_attack_countdown = random_int(3,5);
				}
				if ((total_seconds_int % 7) == death_star_dash_attack_countdown) {
					death_star_dash_attack();
					death_star_dash_attack_countdown = 0;
				}
			}
			for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
				if (is_tie_fighter[i]) {	
					if ((tie_fighter[i].x > screen_x_0) && (tie_fighter[i].y > screen_y_0) && (tie_fighter[i].x < screen_x_1 - 6) && (tie_fighter[i].y < screen_y_1 - 6)) {
						tie_fighter_move(i);
					}
				}
			}
			if (is_death_star) {
				death_star_move();
			}
			if (tie_fighter_respawn) {
				for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
					tie_fighters_killed = 0;
					tie_fighter_spawn(i);
					is_tie_fighter[i] = 1;
					tie_fighter_respawn = 0;
				}
			}
			if ((tie_fighters_killed >= MAX_TIE_FIGHTERS) && (!is_death_star)) {
				death_star_spawn();
			}
			draw_sprite(&death_star);
			draw_sprite(&death_star_health);
			draw_sprite(&xwing);
			for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
				draw_sprite(&(tie_fighter[i]));
			}
			for (int i = 0; i < MAX_MISSILES; i++) {
				if (is_missile[i]) {
					draw_sprite(&(missile[i]));
				}
			}
			for (int i = 0; i < MAX_MISSILES; i++) {
				if (is_missile[i]) {
					move_missile(i);
				}
			}
			process_collisions();
			show_screen();
			if (player_lives <= 0) {
				game_over_flag = 1;
			}
		}
		draw_game_over_screen();
		reset_values();
	}
	show_screen();
	return 0;
}

void init_hardware() {
	set_clock_speed(CPU_8MHz);
	DDRC |= 0b1<<7;
	PORTC |= 0b1<<7;
	lcd_init(LCD_DEFAULT_CONTRAST + 7);
    usb_init();
}
void send_debug_string(char* string) {
	double real_buffer = get_overall_system_time();
	char buffer1[20];
	sprintf(buffer1, "%07.3f", real_buffer);
    usb_serial_write("[DEBUG @ ", 9);
	usb_serial_write(buffer1, 7);
	usb_serial_write("] ", 2);
    unsigned char char_count = 0;
    while (*string != '\0') {
        usb_serial_putchar(*string);
        string++;
        char_count++;
    }
    usb_serial_putchar('\r');
    usb_serial_putchar('\n');
}
void init_timers() {
	TCCR0B &= ~(0b1<<3);
	TCCR0B |= 0b1<<0;
	TCCR0B &= ~(0b1<<1);
	TCCR0B |= 0b1<<2;
    TIMSK0 |= (1 << TOIE0);
    TIMSK1 |= (1 << TOIE1);
	TIMSK3 |= (1 << TOIE3);
	TCCR1B &= ~(1<<WGM02);
	TCCR1B |= 0b1<<0;
	TCCR1B &= ~(0b1<<1);
	TCCR1B |= 0b1<<2;
	TCCR3B &= ~(1<<WGM02);
	TCCR3B |= 0b1<<0;
	TCCR3B &= ~(0b1<<1);
	TCCR3B |= 0b1<<2;
    sei();
}
void setup_io() {
	DDRF &= ~(0b1<<5);
	DDRF &= ~(0b1<<6);	
	DDRB &= ~((1 << PB1) | (1 << PB7));
	DDRD &= ~((1 << PD0) | (1 << PD1));
}
void draw_title_screen() {
	int x0 = LCD_X/2;
	int while_flag = 1;
	char my_name[] = "Joseph Urban";
	char student_number[] = "n8882959";
	char game_title[] = "Death Star!";
	char continue_text_pt1[] = "Press a button ";
	char continue_text_pt2[] = "to continue...";
	while (while_flag) {
		clear_screen();
		draw_string((x0 - (strlen(game_title)/2 * 5)),1, game_title);
		draw_string((x0 - (strlen(my_name)/2 * 5)),11, my_name);
		draw_string((x0 - (strlen(student_number)/2 * 5)),21, student_number);
		draw_string((x0 - (strlen(continue_text_pt1)/2 * 5)),31, continue_text_pt1);
		draw_string((x0 - (strlen(continue_text_pt2)/2 * 5)),41, continue_text_pt2);
		show_screen();
		if(((PINF>>6) & 0b1) || ((PINF>>5) & 0b1)) {
			while_flag = 0;
			clear_screen();
			_delay_ms(80);
			while (((PINF>>6) & 0b1) || ((PINF>>5) & 0b1)) {}
			_delay_ms(80);
		}
	}
}
void draw_border() {
	draw_line(0,10,LCDX,10);
	draw_line(0,0,LCDX,0);
	draw_line(LCDX, 0, LCDX, LCDY);
	draw_line(LCDX, LCDY, 0, LCDY);
	draw_line(0, LCDY, 0, 0);	
}
void draw_countdown() {
	int countdown = 3;
	char buffer[5];
	while (countdown > 0) {
		clear_screen();
		sprintf(buffer, "%d", countdown);
		draw_string((LCD_X/2)-3,(LCD_Y/2)-3, buffer);
		show_screen();
		countdown--;
		_delay_ms(300);
		clear_screen();
	}
}
void draw_hud() {

	int x0 = LCD_X/2;
	char lives_text[10] = "L:";
	char score_text[10] = "S:";
	char time_text1[20] = "T:";
	char time_text2[20] = ":";
	char buff[20];
	char buff2[20];
	char buff3[20];
	char buff4[20];
	
	sprintf(buff, "%02d", player_lives);
	strcat(lives_text, buff);
	draw_string(2, 2, lives_text);
	
	sprintf(buff2, "%02d", player_score);
	strcat(score_text, buff2);
	draw_string(((x0 - (strlen(score_text)/2 * 5)-8)), 2, score_text);
	
	sprintf(buff3, "%02d", timer_minutes);
	sprintf(buff4, "%02d", timer_seconds);
	strcat(time_text1, buff3);
	strcat(time_text1, time_text2);
	strcat(time_text1, buff4);
	draw_string(47,2, time_text1);
	
}
double get_system_time(unsigned int timer_count) {
	
	double float_time = timer_count * 0.000128;
	
	return float_time;
}
double get_overall_system_time() {
	return (TCNT3 * 0.000128) + (8.388608 * timer3_overflow);
}
void process_time() {
	double total_seconds_double = 0.0;
	
	total_seconds_double = get_system_time(TCNT1) + (overflow_count*8.3);
	total_seconds_int = round(total_seconds_double);

	timer_seconds = total_seconds_int % 60;
	timer_minutes = total_seconds_int / 60;
	
}
void draw_game_over_screen() {
	start_debugging_flag = 0;
	
	int x0 = LCD_X/2;
	int while_flag = 1;
	char game_over_text[] = "Game Over!";
	char continue_text[] = "Continue..?";
	char continue_text_pt1[] = "Press a button ";
	char continue_text_pt2[] = "to play again...";
	
	while (while_flag) {
		clear_screen();
		draw_string((x0 - (strlen(game_over_text)/2 * 5)),1, game_over_text);
		draw_string((x0 - (strlen(continue_text)/2 * 5)),16, continue_text);
		draw_string((x0 - (strlen(continue_text_pt1)/2 * 5)),31, continue_text_pt1);
		draw_string((x0 - (strlen(continue_text_pt2)/2 * 5)),41, continue_text_pt2);
		show_screen();
		
		if(((PINF>>6) & 0b1) || ((PINF>>5) & 0b1)) {
			while_flag = 0;
			clear_screen();
			_delay_ms(80);
			while (((PINF>>6) & 0b1) || ((PINF>>5) & 0b1)) {}
			_delay_ms(80);
		}
	}
}
void sprite_move() {
	
	if ((((PINB>>1) & 0b1) || (keyboard_buffer == 'a')) && (xwing.x > screen_x_0)) {
		xwing.x = xwing.x - 2;
		init_sprite(&xwing, xwing.x, xwing.y, 8, 8, xwing_image_left);
		player_heading = 'l';
		keyboard_buffer = 'x';
	}
	else if ((((PIND>>1) & 0b1) || (keyboard_buffer == 'w')) && (xwing.y > screen_y_0)) {
		xwing.y = xwing.y - 2;
		init_sprite(&xwing, xwing.x, xwing.y, 8, 8, xwing_image);
		player_heading = 'u';
		keyboard_buffer = 'x';
	}
	else if ((((PIND>>0) & 0b1) ||(keyboard_buffer == 'd')) && (xwing.x < screen_x_1 - 7)) {
		xwing.x = xwing.x + 2;
		init_sprite(&xwing, xwing.x, xwing.y, 8, 8, xwing_image_right);
		player_heading = 'r';
		keyboard_buffer = 'x';
	}
	else if ((((PINB>>7) & 0b1) || (keyboard_buffer == 's')) && (xwing.y < screen_y_1 - 7)) {
		xwing.y = xwing.y + 2;
		init_sprite(&xwing, xwing.x, xwing.y, 8, 8, xwing_image_down);
		player_heading = 'd';
		keyboard_buffer = 'x';
	}
	if (xwing.x <= screen_x_0) {
		xwing.x = screen_x_0;
	} 
	if (xwing.x >= screen_x_1 - 7) {
		xwing.x = screen_x_1 - 7;
	} 
	if (xwing.y <= screen_y_0) {
		xwing.y = screen_y_0;
	} 
	if (xwing.y >= screen_y_1 - 7) {
		xwing.y = screen_y_1 - 7;
	}
}
int random_int(int min, int max) {
	return min + rand() % (max+1 - min);
}
void process_collisions() {
	for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
		
		if ((tie_fighter[i].x + 6 < xwing.x) || (tie_fighter[i].x > xwing.x + 7) || (tie_fighter[i].y + 6 < xwing.y) || (tie_fighter[i].y > xwing.y + 7)) {} //tie figher x player collision
		else {
			xwing_spawn();
			player_lives--;
			player_heading = 'u';
			send_debug_string("X-Wing destroyed by TIE Fighter!\n\n");
			for (int i = 0; i < MAX_MISSILES; i++) {
				missile_heading[i] = player_heading;
				is_missile[i] = 0;
				missile[i].is_visible = 0;
				missile[i].x = missile[i].x + 90;
				missile[i].y = missile[i].y + 60;
			}
		}
	}
	for (int i = 0; i < MAX_MISSILES; i++) {
		if ((missile[i].x > screen_x_1 - 1) || (missile[i].x < screen_x_0) || (missile[i].y > screen_y_1 - 1) || (missile[i].y < screen_y_0)) { //missile x boundary collision
			is_missile[i] = 0;
			missile[i].is_visible = 0;
		}
	}
	for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
		for (int j = 0; j < MAX_MISSILES; j++) {	
			
			if ((tie_fighter[i].x + 6 < missile[j].x) || (tie_fighter[i].x > missile[j].x + 1) || (tie_fighter[i].y + 6 < missile[j].y) || (tie_fighter[i].y > missile[j].y + 1)) {} //tie figher x missile collision
			
			else {
				is_tie_fighter[i] = 0;
				tie_fighters_killed++;
				tie_fighter[i].is_visible = 0;
				tie_fighter[i].x =  150;
				tie_fighter[i].y = 150;
				
				is_missile[j] = 0;
				missile[j].is_visible = 0;
				missile[j].x = missile[j].x + 90;
				missile[j].y = missile[j].y + 60;
				player_score++;
				send_debug_string("TIE Fighter destroyed!\n\n");
			}
		}
	}
	for (int i = 0; i < MAX_MISSILES; i++) {
		if ((death_star_health.x + 11 < missile[i].x) || (death_star_health.x > missile[i].x + 1) || (death_star_health.y + 13 < missile[i].y) || (death_star_health.y > missile[i].y + 1)) {} //death star x missile collision
		else {
			is_missile[i] = 0;
			missile[i].is_visible = 0;
			missile[i].x = missile[i].x + 90;
			missile[i].y = missile[i].y + 60;
			if (death_star_health_int <= 1) {
				player_score += 10;
				death_star_health_int = 10;
				death_star.is_visible = 0;
				death_star_health.is_visible = 0;
				death_star_health.x = 150;
				death_star_health.y = 150;
				is_death_star = 0;
				tie_fighter_respawn = 1;
				send_debug_string("Death Star destroyed by X-Wing! Yavin 4 is saved!\n\n");
			}
			if (death_star_health_int >= 2) {
				death_star_health_modify();
				death_star_health_int--;
			}
		}
	}

	if ((death_star_health.x + 11 < xwing.x) || (death_star_health.x > xwing.x + 7) || (death_star_health.y + 13 < xwing.y) || (death_star_health.y > xwing.y + 7)) {} //death star x player collision
	else {
		xwing_spawn();
		player_lives--;
		player_heading = 'u';
		send_debug_string("X-Wing destroyed by Death Star!\n\n");
		for (int i = 0; i < MAX_MISSILES; i++) {
			missile_heading[i] = player_heading;
			is_missile[i] = 0;
			missile[i].is_visible = 0;
			missile[i].x = missile[i].x + 90;
			missile[i].y = missile[i].y + 60;
		}
	}

	
}

void init_missile(char direction, int missile_number) {
	if (direction == 'u'){
		init_sprite(&(missile[missile_number]), xwing.x + 3, xwing.y, 2, 2, missile_image);	
	} 
	else if (direction == 'd') {
		init_sprite(&(missile[missile_number]), xwing.x + 3, xwing.y + 7, 2, 2, missile_image);
	} 
	else if (direction == 'l') {
		init_sprite(&(missile[missile_number]), xwing.x, xwing.y + 3, 2, 2, missile_image);
	} 
	else if (direction == 'r') {
		init_sprite(&(missile[missile_number]), xwing.x + 7, xwing.y + 3, 2, 2, missile_image);
	}
	is_missile[missile_number] = 1;
	missile_heading[missile_number] = direction;
}

void move_missile(int missile_number) {
	if (missile_heading[missile_number] == 'u') {
		missile[missile_number].y = missile[missile_number].y - 5;
	}
	else if (missile_heading[missile_number] == 'd') {
		missile[missile_number].y = missile[missile_number].y + 5;
	}
	else if (missile_heading[missile_number] == 'l') {
		missile[missile_number].x = missile[missile_number].x - 5;
	}
	else if (missile_heading[missile_number] == 'r') {
		missile[missile_number].x = missile[missile_number].x + 5;
	}
}

void tie_fighter_spawn(int tie_number) {
	int valid = 0;
	int x_buffer;
	int y_buffer;
	
	while (!valid) {
		x_buffer = random_int(screen_x_0, screen_x_1 - 6);
		y_buffer = random_int(screen_y_0, screen_y_1 - 6);
		
		if ((x_buffer + 6 < xwing.x) || (x_buffer > xwing.x + 7) || (y_buffer + 6 < xwing.y) || (y_buffer > xwing.y + 7)) {
			init_sprite(&(tie_fighter[tie_number]), x_buffer, y_buffer, 7, 7, tie1_image);
			valid = 1;
		}
		
	}
}

void xwing_spawn() {
	int valid = 0;
	int x_buffer;
	int y_buffer;
	int good_buffer = 0;
	
	if (is_death_star){ 
		while (!valid) {
			x_buffer = random_int(screen_x_0, screen_x_1 - 7);
			y_buffer = random_int(screen_y_0, screen_y_1 - 7);
			
			if ((x_buffer + 7 < death_star_health.x) || (x_buffer > death_star_health.x + 12) || (y_buffer + 7 < death_star_health.y) || (y_buffer > death_star_health.y + 13)) {
				valid = 1;
			}
		}
		init_sprite(&xwing, x_buffer, y_buffer, 8, 8, xwing_image);
	}
		
	else {
		while (!valid) {
			x_buffer = random_int(screen_x_0, screen_x_1 - 7);
			y_buffer = random_int(screen_y_0, screen_y_1 - 7);
			
			for (int i = 0; i < MAX_TIE_FIGHTERS; i++) {
				if ((x_buffer + 7 < tie_fighter[i].x) || (x_buffer > tie_fighter[i].x + 6) || (y_buffer + 7 < tie_fighter[i].y) || (y_buffer > tie_fighter[i].y + 6)) {
					good_buffer++;
				}
			}
			
			if (good_buffer >= (MAX_TIE_FIGHTERS - tie_fighters_killed)) {
				valid = 1;
			}
			else {
				good_buffer = 0;
			}
		}
		init_sprite(&xwing, x_buffer, y_buffer, 8, 8, xwing_image);
	}
}

void reset_values() {
	player_lives = 10;
	player_score = 0;
	game_over_flag = 0;
	overflow_count = 0;
	TCNT1 = 0;
	player_heading = 'u';
	is_death_star = 0;
	tie_fighters_killed = 0;
	death_star_health.is_visible = 0;
	death_star.is_visible = 0;
	death_star_health.x = 150;
	death_star_health.y = 150;
	death_star.x = death_star_health.x;
	death_star.y = death_star_health.y;
	death_star_health_int = 10;
	
	for (int i = 0; i < MAX_TIE_FIGHTERS; i++) { 
		is_tie_fighter[i] = 0;
	}
	for (int i = 0; i < MAX_MISSILES; i++) {
		missile_heading[i] = 'x';
	}
}

void draw_serial_greeting_screen() {
	
	int x0 = LCD_X/2;
	int y0 = LCD_Y/2;
	int while_flag = 1;
	char my_name[] = "Waiting for";
	char student_number[] = "PC connection";
	char connected_1[] = "Connected to";
	char connected_2[] = "PC via serial!";
	char continue_text_pt1[] = "Press a button ";
	char continue_text_pt2[] = "to continue...";

	clear_screen();
	draw_string((x0 - (strlen(my_name)/2 * 5)),y0 - 5, my_name);
	draw_string((x0 - (strlen(student_number)/2 * 5)),y0 + 5, student_number);
	show_screen();
	while(!usb_configured() || !usb_serial_get_control());
	
	clear_screen();
	draw_string((x0 - (strlen(connected_1)/2 * 5)), 1, connected_1);
	draw_string((x0 - (strlen(connected_2)/2 * 5)), 11, connected_2);
	draw_string((x0 - (strlen(continue_text_pt1)/2 * 5)),31, continue_text_pt1);
	draw_string((x0 - (strlen(continue_text_pt2)/2 * 5)),41, continue_text_pt2);
	
	show_screen();
	
	send_debug_string("Welcome to 'Death Star!', a Teensy adaptation");
	send_debug_string("of an iconic Star Wars dogfight");
	send_debug_string("Author: Joseph Urban, n8882959, 2017");
	send_debug_string("This debug menu will allow you to control the game via");
	send_debug_string("keyboard input, and see various information outputs\n\n");
	send_debug_string("Controls:");
	send_debug_string("WASD - Movement");
	send_debug_string("C - Fire Proton Torpedoes\n\n");
	while(while_flag) {
		if(((PINF>>6) & 0b1) || ((PINF>>5) & 0b1)) {
			while_flag = 0;
			clear_screen();
			_delay_ms(80);
			while (((PINF>>6) & 0b1) || ((PINF>>5) & 0b1)) {}
			_delay_ms(80);
		}
	}
}

void debug_reporting() {
	char buffer[50] = "X-Wing current X position: ";
	char buffer2[50];
	
	if (start_debugging_flag) {
		sprintf(buffer2, "%02.0f", round(xwing.x));
		strcat(buffer, buffer2);
		send_debug_string(buffer);
		
		strcpy(buffer,  "X-Wing current Y position: ");
		sprintf(buffer2, "%02.0f", round(xwing.y));
		strcat(buffer, buffer2);
		send_debug_string(buffer);
		
		if (player_heading == 'u') {
			send_debug_string("X-Wing current heading is up/North\n\n");
		}
		else if (player_heading == 'd') {
			send_debug_string("X-Wing current heading is down/South\n\n");
		}
		else if (player_heading == 'l') {
			send_debug_string("X-Wing current heading is left/West\n\n");
		}
		else if (player_heading == 'r') {
			send_debug_string("X-Wing current heading is right/East\n\n");
		}
	}
}

void dash_attack(int tie_number) {
	
	double dx = xwing.x - tie_fighter[tie_number].x;
	double dy = xwing.y - tie_fighter[tie_number].y;
	double dist = sqrt(dx * dx + dy * dy);
	
	dx = dx * 1.5 / dist;
	dy = dy * 1.5 / dist;
	
	tie_fighter[tie_number].dx = dx;
	tie_fighter[tie_number].dy = dy;
	tie_fighter[tie_number].x += dx;
	tie_fighter[tie_number].y += dy;
}

void death_star_dash_attack() {
	
	double dx = xwing.x - death_star_health.x;
	double dy = xwing.y - death_star_health.y;
	double dist = sqrt(dx * dx + dy * dy);
	
	dx = dx * 0.8 / dist;
	dy = dy * 0.8 / dist;
	
	death_star_health.dx = dx;
	death_star_health.dy = dy;
	death_star_health.x += dx;
	death_star_health.y += dy;
	death_star.x = death_star_health.x + 1;
	death_star.y = death_star_health.y + 4;
}

void tie_fighter_move(int tie_number) {
	
	tie_fighter[tie_number].x += tie_fighter[tie_number].dx;
	tie_fighter[tie_number].y += tie_fighter[tie_number].dy;
	
	if (tie_fighter[tie_number].x <= screen_x_0) {
		tie_fighter[tie_number].x = screen_x_0;
	} 
	if (tie_fighter[tie_number].x >= screen_x_1 - 6) {
		tie_fighter[tie_number].x = screen_x_1 - 6;
	} 
	if (tie_fighter[tie_number].y <= screen_y_0) {
		tie_fighter[tie_number].y = screen_y_0;
	} 
	if (tie_fighter[tie_number].y >= screen_y_1 - 6) {
		tie_fighter[tie_number].y = screen_y_1 - 6;
	}
}

void death_star_move() {
	
	death_star_health.x += death_star_health.dx;
	death_star_health.y += death_star_health.dy;
	
	if (death_star_health.x <= screen_x_0) {
		death_star_health.x = screen_x_0;
		death_star_health.dx = 0;
		death_star_health.dy = 0;
	} 
	if (death_star_health.x >= screen_x_1 - 11) {
		death_star_health.x = screen_x_1 - 11;
		death_star_health.dx = 0;
		death_star_health.dy = 0;
	} 
	if (death_star_health.y <= screen_y_0) {
		death_star_health.y = screen_y_0;
		death_star_health.dy = 0;
		death_star_health.dx = 0;
	} 
	if (death_star_health.y >= screen_y_1 - 13) {
		death_star_health.y = screen_y_1 - 13;
		death_star_health.dy = 0;
		death_star_health.dx = 0;
	}
	
	death_star.x = death_star_health.x + 1;
	death_star.y = death_star_health.y + 4;
}

void death_star_spawn() {
	
	int valid = 0;
	int x_buffer;
	int y_buffer;
	
	while (!valid) {
		
		x_buffer = random_int(screen_x_0, screen_x_1 - 12);
		y_buffer = random_int(screen_y_0, screen_y_1 - 14);
		
		if ((x_buffer + 11 < xwing.x) || (x_buffer > xwing.x + 7) || (y_buffer + 13 < xwing.y) || (y_buffer > xwing.y + 7)) {
			valid = 1;
		}
	}
	
	init_sprite(&death_star_health, x_buffer, y_buffer, 12, 3, death_star_health_10);
	init_sprite(&death_star, (death_star_health.x + 1), (death_star_health.y + 4), 10, 10, death_star_image);

	is_death_star = 1;
}

void death_star_health_modify() {
	
	if (death_star_health_int == 10) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_9);
	}
	if (death_star_health_int == 9) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_8);
	}
	if (death_star_health_int == 8) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_7);
	}
	if (death_star_health_int == 7) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_6);
	}
	if (death_star_health_int == 6) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_5);
	}
	if (death_star_health_int == 5) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_4);
	}
	if (death_star_health_int == 4) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_3);
	}
	if (death_star_health_int == 3) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_2);
	}
	if (death_star_health_int == 2) {
		init_sprite(&death_star_health, death_star_health.x, death_star_health.y, 12, 3, death_star_health_1);
	}
}

ISR(TIMER1_OVF_vect) {
    overflow_count++;
}

ISR(TIMER0_OVF_vect) {
    timer0_overflow++;
	if (timer0_overflow == 15) {
		timer0_overflow = 0;
		debug_reporting();
	} 
}

ISR(TIMER3_OVF_vect) {
    timer3_overflow++;
}