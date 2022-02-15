#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

#define DELAY (10)
#define PLAYER_WIDTH (7)
#define PLAYER_HEIGHT (3)
#define LARGE_DIAMOND_WIDTH (5)
#define LARGE_DIAMOND_HEIGHT (5)
#define LASER_WIDTH (1)
#define LASER_HEIGHT (1)
#define MAX_DIAMONDS (10)
#define MAX_LASERS (100)

int playerscore = 0;
int time_played = 0;
int playerlives = 5;
int destroyed_diamonds = 0;

bool game_over = false;
bool quit = false;
bool update_screen = true;


bool is_lasers[MAX_LASERS];
bool to_destroy[MAX_LASERS];
char time_text[] = "Time elapsed: 0:00";


char * large_diamond_image =
/**/	"  C  "
/**/	" CCC "
/**/	"CCCCC"
/**/	" CCC "
/**/	"  C  ";

char * laser_image = 
/**/	"!";

char * player_image = 
/**/	"   |   "
/**/ 	"   A   "
/**/ 	"|_/_\\_|";

char * game_over_image =
/**/	"################################"
/**/	"#                              #"
/**/	"#          Game Over!          #"
/**/	"#                              #"
/**/	"#         Play Again?          #"
/**/	"#             Y/N              #"
/**/	"#                              #"
/**/	"################################";


char * help_image =
/**/	"################################"
/**/	"#      CAB202 Assignment 1     #"
/**/	"#      The Diamonds of Doom    #"
/**/	"#          Joseph Urban        #"
/**/	"#            n8882959          #"
/**/	"################################"
/**/	"#           Controls           #"
/**/	"#  Q       :   Quit            #"
/**/	"#  H       :   Help            #"
/**/	"#  A/D     :   Move Left/Right #"
/**/	"#  1/2/3   :   Fire            #"
/**/	"################################"
/**/	"#      Press any key to play   #"
/**/	"################################";

sprite_id player;
sprite_id large_diamond;
sprite_id med_diamond;
sprite_id small_diamond;
sprite_id laser;

sprite_id multi_diamonds [ MAX_DIAMONDS ];
sprite_id multi_lasers [ MAX_LASERS ];

timer_id game_timer;

void setup_game(void) {
	
	playerscore = 0;
	time_played = 0;
	playerlives = 5;
	destroyed_diamonds = 0;

	game_over = false;
	quit = false;
	update_screen = true;
	
	time_text[14] = 0 + '0';
	time_text[16] = 0 + '0';
	time_text[17] = 0 + '0';

	int w = screen_width();
	int pw = PLAYER_WIDTH, ph = PLAYER_HEIGHT;
	int x = (w - pw) / 2, y = screen_height() -4;
	int dx;
	int dy = LARGE_DIAMOND_HEIGHT;
	int angle = (rand() % (315-225)) + 225;
	
	for (int i = 0; i < MAX_LASERS; i++) {
		is_lasers[i] = false;
		to_destroy[i] = false;
}

	
	game_timer = create_timer( 1000 );
	
	player = sprite_create(x, y, pw, ph, player_image);
	sprite_draw(player);
	show_screen();
	
	for (int i = 0; i < MAX_DIAMONDS; i++) {
		dx = 1 + rand() % (w - LARGE_DIAMOND_WIDTH - 2);
		angle = (rand() % (315-225)) + 225;
		multi_diamonds[i] = sprite_create(dx, dy, LARGE_DIAMOND_HEIGHT, LARGE_DIAMOND_WIDTH, large_diamond_image);
		sprite_turn_to(multi_diamonds[i], 0.1, 0);
		sprite_turn(multi_diamonds[i], angle);
		timer_pause(DELAY);
	}
	
	show_screen();
	
}

void draw_border(void){
	
	draw_line(0, 3, screen_width() -1, 3, '#');
	draw_line(0, 2, screen_width() -1, 2, '#');
	draw_line(0, 0, screen_width() -1, 0, '#');
	draw_line(screen_width() -1, 0, screen_width() -1, screen_height() -1, '#');
	draw_line(screen_width() -1, screen_height() -1, 0, screen_height() -1, '#');
	draw_line(0, screen_height() -1, 0, 0, '#');
	
	
}

void help_window(void) {
	int w = screen_width(), h = screen_height();
	while ( get_char() >= 0 ) {}

	clear_screen();
	int message_width = 32;
	sprite_id help = sprite_create((w - message_width) / 2, (h - 2) / 2, message_width, 14, help_image);
	sprite_draw(help);
	show_screen();
	wait_char();
	return;
}

void process_score(void) {
	
	if (playerscore < 10) {
		char score_text[] = "Score:  ";
		char score_char = playerscore + '0';
		score_text[7] = score_char;
		draw_formatted((screen_width()/2 - 3), 1, score_text);
	} else {
		for (int i = 10; i < 99; i = i + 10) {
			if ((playerscore >= i) && (playerscore < i + 10)) {
				char score_text[] = "Score:   ";
				int score_int_ones = playerscore % i;
				char score_char_ones = score_int_ones + '0';
				int score_int_tens = playerscore/10;
				char score_char_tens = score_int_tens + '0';
				score_text[7] = score_char_tens;
				score_text[8] = score_char_ones;
				draw_formatted((screen_width()/2 - 3), 1, score_text);
			}
		}
	}
}

void process_timer(void) {
	
	if (timer_expired(game_timer)) {
		time_played++;
		
		int mins = time_played / 60;
		int seconds_pair = time_played % 60;
		int seconds_ones = seconds_pair % 10;
		int seconds_tens = seconds_pair / 10;
		
		char mins_char = mins + '0';
		char seconds_ones_char = seconds_ones + '0';
		char seconds_tens_char = seconds_tens + '0';
		
		time_text[14] = mins_char;
		time_text[16] = seconds_tens_char;
		time_text[17] = seconds_ones_char;
	}
	draw_formatted(screen_width() - 20, 1, time_text);
}

void is_collision(void) {
	int px = round(sprite_x(player));
	int py = round(sprite_y(player));
	int w = screen_width(), h = screen_height();
	int pw = PLAYER_WIDTH, ph = PLAYER_HEIGHT;
	int x = (w - pw) / 2, y = screen_height() -4;

	for (int i = 0; i < MAX_DIAMONDS; i++) {
		int dx = round(sprite_x(multi_diamonds[i]));
		int dy = round(sprite_y(multi_diamonds[i]));
		
		if ((dx + 5 < px) || (dx > px + 7) || (dy + 5 < py)) {
		} else {
			sprite_destroy(player);
			player = sprite_create(x, y, pw, ph, player_image);
			sprite_draw(player);
			playerlives--;
			destroyed_diamonds = 0;
			
			if (playerlives < 1) {
				while ( get_char() >= 0 ) {}

				
				int key = get_char();
				
				clear_screen();
				int message_width = 32;
				sprite_id msg = sprite_create((w - message_width) / 2, (h - 2) / 2, message_width, 8, game_over_image);
				sprite_draw(msg);
				show_screen();
				while ((key != 'n') || (key != 'y')) {
					int key = get_char();
					if (key == 'y') {
						game_over = true;
						clear_screen();
						return;
					}
					if (key == 'n'){
						game_over = true;
						quit = true;
						return;
					}
				}
			}
			
			for(int i = 0; i < MAX_DIAMONDS; i++) {
				timer_pause(DELAY);
				sprite_destroy(multi_diamonds[i]);
			
				int dx = 1 + rand() % (w - LARGE_DIAMOND_WIDTH - 2);
				int dy = LARGE_DIAMOND_HEIGHT;
				multi_diamonds[i] = sprite_create(dx, dy, LARGE_DIAMOND_WIDTH, LARGE_DIAMOND_HEIGHT, large_diamond_image);
				sprite_draw(multi_diamonds[i]);
				
				int angle = (rand() % (315-225)) + 225;
				sprite_turn_to(multi_diamonds[i], 0.1, 0);
				sprite_turn(multi_diamonds[i], angle);
				sprite_step(multi_diamonds[i]);
				
			}
			
			for (int j = 0; j < MAX_LASERS; j++) {
				if (is_lasers[j]) {
					sprite_hide(multi_lasers[j]);
					sprite_move_to(multi_lasers[j], 1000, 5);
					is_lasers[j] = false;
				}
			}
		}
	}

	for (int i = 0; i < MAX_LASERS; i++) {
		if (is_lasers[i]) {
			int lx = round(sprite_x(multi_lasers[i]));
			int ly = round(sprite_y(multi_lasers[i]));
			for(int k = 0; k < MAX_DIAMONDS; k++) {
				int dx = round(sprite_x(multi_diamonds[k]));
				int dy = round(sprite_y(multi_diamonds[k]));
				
				if ((dx + 5 < lx) || (dx > lx) || (dy + 5 < ly) || (dy > ly)){
				} else {
					sprite_move(multi_diamonds[k], 1000,1000);
					sprite_turn_to(multi_diamonds[k], 0, 0);
					sprite_hide(multi_lasers[i]);
					sprite_move_to(multi_lasers[i], 1000, 5);
					playerscore++;
					destroyed_diamonds++;
				
					if (destroyed_diamonds >= MAX_DIAMONDS) {
						destroyed_diamonds = 0;
						for(int j = 0; j < MAX_DIAMONDS; j++) {
							timer_pause(DELAY);
							sprite_destroy(multi_diamonds[j]);
			
							int w = screen_width();
							int dx = 1 + rand() % (w - LARGE_DIAMOND_WIDTH - 2);
							int dy = LARGE_DIAMOND_HEIGHT;
							multi_diamonds[j] = sprite_create(dx, dy, LARGE_DIAMOND_WIDTH, LARGE_DIAMOND_HEIGHT, large_diamond_image);
							sprite_draw(multi_diamonds[j]);
				
							int angle = (rand() % (315-225)) + 225;
							sprite_turn_to(multi_diamonds[j], 0.1, 0);
							sprite_turn(multi_diamonds[j], angle);
							sprite_step(multi_diamonds[j]);			
						}
						
						for (int j = 0; j < MAX_LASERS; j++) {
							if (is_lasers[j]) {
								sprite_hide(multi_lasers[j]);
								sprite_move_to(multi_lasers[j], 1000, 5);
								is_lasers[j] = false;
							}
						}
					}
				}
			}
			if (ly <=3) {
				sprite_hide(multi_lasers[i]);
				is_lasers[i] = false;
			}
		}
	}
}


void process(void) {
	
	int w = screen_width(), h = screen_height();
	int key = get_char();
	
	if ( key == 'q' ) {
		while ( get_char() >= 0 ) {}

		
		clear_screen();
		int message_width = 32;
		sprite_id msg = sprite_create((w - message_width) / 2, (h - 2) / 2, message_width, 8, game_over_image);
		sprite_draw(msg);
		show_screen();
		while ((key != 'n') || (key != 'y')) {
			key = get_char();
			if (key == 'y') {
				game_over = true;
				clear_screen();
				return;
			}
			if (key == 'n'){
				game_over = true;
				quit = true;
				return;
			}
		}
	}
	
		if ( key == 'h' ) {
			while ( get_char() >= 0 ) {}
			help_window();
	}

	int px = round(sprite_x(player));
	if ( key == 'a' && px > 1 ) sprite_move(player, -1, 0);

	if ( key == 'd' && px < w - sprite_width(player) - 1 ) sprite_move(player, +1, 0);
	

	if (key == '2'){
		for (int i = 0; i < MAX_LASERS; i++) {
			if (!is_lasers[i]) {
				multi_lasers[i] = sprite_create(px + 3, h - 5, LASER_WIDTH, LASER_HEIGHT, laser_image);
				is_lasers[i] = true;
				sprite_turn_to(multi_lasers[i], 0.5, 0);
				sprite_turn(multi_lasers[i], +90);
				break;
			}
		}
	}
	if (key == '1') {
		for (int i = 0; i < MAX_LASERS; i++) {
			if (!is_lasers[i]) {
				is_lasers[i] = true;
				multi_lasers[i] = sprite_create(px, h - 3, LASER_WIDTH, LASER_HEIGHT, laser_image);
				sprite_turn_to(multi_lasers[i], 0.5, 0);
				sprite_turn(multi_lasers[i], +90);
				break;
			}
		}
	}
	if (key == '3') {
		for (int i = 0; i < MAX_LASERS; i++) {
			if (!is_lasers[i]) {
				is_lasers[i] = true;
				multi_lasers[i] = sprite_create(px + 6, h - 3, LASER_WIDTH, LASER_HEIGHT, laser_image);
				sprite_turn_to(multi_lasers[i], 0.5, 0);
				sprite_turn(multi_lasers[i], +90);
				break;
			}
		}
	}
	
	if ( key == 'z' || key < 0 ) {
		
		for (int i = 0; i < MAX_DIAMONDS; i++) {
			if ( key == 'z' || key < 0 ) {
			sprite_step(multi_diamonds[i]);

			int zx = round(sprite_x(multi_diamonds[i]));
			int zy = round(sprite_y(multi_diamonds[i]));

			double zdx = sprite_dx(multi_diamonds[i]);
			double zdy = sprite_dy(multi_diamonds[i]);

			if ( zx <= 0 ) {
				zdx = fabs(zdx);
			}
			else if ( zx >= w - LARGE_DIAMOND_WIDTH ) {
				zdx = -fabs(zdx);
			}

			if ( zy <= 3 ) {
				zdy = fabs(zdy);
			}
			else if ( zy >= h - LARGE_DIAMOND_HEIGHT ) {
				zdy = -fabs(zdy);
			}

			if ( zdx != sprite_dx(multi_diamonds[i]) || zdy != sprite_dy(multi_diamonds[i]) ) {
				sprite_back(multi_diamonds[i]);
				sprite_turn_to(multi_diamonds[i], zdx, zdy);
			}
		}
	}
	}

	clear_screen();
	draw_border();
	sprite_draw(player);

	char lives_text[] = "Lives:  ";
	char lives_char = playerlives + '0';
	lives_text[7] = lives_char;
	draw_formatted(2,1, lives_text);
	
	process_score();
	process_timer();
	
	for (int i = 0; i < MAX_DIAMONDS; i++) {
		sprite_draw(multi_diamonds[i]);
	}
	
	is_collision();

	for (int i = 0; i < MAX_LASERS; i++) {
		if (is_lasers[i]) {
			sprite_draw(multi_lasers[i]);
			sprite_step(multi_lasers[i]);
		}
	}
}


int main( void ) {
	
	while (!quit) {
	
		setup_screen( );
		help_window();

		draw_border();
		setup_game();
	
		show_screen();
	
		while ( !game_over ) {
			process();

			if ( update_screen ) {
				show_screen();
			}

			timer_pause(DELAY);
		}
	}
	cleanup_screen( );
	return 0;
}