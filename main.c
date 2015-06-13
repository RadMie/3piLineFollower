//=========================================================================================
//				3pi - LINE FOLLOWER MAIN
//=========================================================================================
#include <pololu/3pi.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "maze-solve.h"

void maze_solve();

void eeprom_write_byte1(uint16_t addr, uint8_t data)
{
  while(EECR & (1<<EEPE)) /*wait until previous write any*/
  ;
EEAR = addr;
EEDR = data;
EECR |= (1<<EEMPE);
EECR |= (1<<EEPE);
}

uint8_t eeprom_read_byte1(uint16_t addr)
{
  while(EECR & (1<<EEPE))/*wait until previous write any*/
  ;
EEAR = addr;
EECR |= (1<<EERE);
  return EEDR;
}

uint16_t addr_pid = 0;
uint16_t addr_no_pid = 1;

const char welcome_line1[] PROGMEM = " Pololu";
const char welcome_line2[] PROGMEM = "3\xf7 Robot";

const char demo_name_line1[] PROGMEM = " Main";
const char demo_name_line2[] PROGMEM = "Program";

const char go[] PROGMEM = "L16 cdegreg4";

const char instructions_line1[] PROGMEM = "Use B to";
const char instructions_line2[] PROGMEM = "select.";
const char instructions_line3[] PROGMEM = "Press B";
const char instructions_line4[] PROGMEM = "-try it!";

const char thank_you_line1[] PROGMEM = " Thank ";
const char thank_you_line2[] PROGMEM = "  you!";

const char main_menu_intro_line1[] PROGMEM = "  Main";
const char main_menu_intro_line2[] PROGMEM = "  Menu";

const char menu_bat_test[] PROGMEM = "Battery";
const char menu_led_test[] PROGMEM = "LEDs";
const char menu_lcd_test[] PROGMEM = "LCD";
const char menu_ir_test[] PROGMEM = "Sensors";
const char menu_motor_test[] PROGMEM = "Motors";
const char menu_music_test[] PROGMEM = "Music";
const char menu_pot_test[] PROGMEM = "Trimpot";
const char menu_time_test[] PROGMEM = "Timer";

const char menu_spinning[] PROGMEM = "Spinning";
const char menu_pid[] PROGMEM = "PID";
const char menu_no_pid[] PROGMEM = "NO PID";
const char menu_edit_pid[] PROGMEM = "Edit PID";
const char menu_edit_no_pid[] PROGMEM = "EditNoPI";
const char menu_maze[] PROGMEM = "Maze";

const char menu_line2[] PROGMEM = "\x7f" "A \xa5" "B C\x7e";
const char back_line2[] PROGMEM = "\7B";

void bat_test();
void led_test();
void lcd_test();
void ir_test();
void motor_test();
void music_test();
void time_test();
void pot_test();
void Spinning_Line_Follow( void );
void PID_Line_Follow();
void No_PID_Line_Follow();
void edit_pid();
void edit_no_pid();
void maze();

typedef void (*function)();
const function main_menu_functions[] = { bat_test, led_test, pot_test, ir_test, motor_test, music_test, time_test, Spinning_Line_Follow, PID_Line_Follow, No_PID_Line_Follow, edit_pid, edit_no_pid, maze };
const char *main_menu_options[] = { menu_bat_test, menu_led_test, menu_pot_test, menu_ir_test, menu_motor_test, menu_music_test, menu_time_test, menu_spinning, menu_pid, menu_no_pid, menu_edit_pid, menu_edit_no_pid, menu_maze };
const char main_menu_length = sizeof(main_menu_options)/sizeof(main_menu_options[0]);

const char welcome[] PROGMEM = ">g32>>c32";
const char thank_you_music[] PROGMEM = ">>c32>g32";
const char beep_button_a[] PROGMEM = "!c32";
const char beep_button_b[] PROGMEM = "!e32";
const char beep_button_c[] PROGMEM = "!g32";
const char timer_tick[] PROGMEM = "!v8>>c32";

const char levels[] PROGMEM = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};

const char note[] PROGMEM = {
	0b00100,
	0b00110,
	0b00101,
	0b00101,
	0b00100,
	0b11100,
	0b11100,
	0b00000,
};

const char back_arrow[] PROGMEM = {
	0b00000,
	0b00010,
	0b00001,
	0b00101,
	0b01001,
	0b11110,
	0b01000,
	0b00100,
};

void load_custom_characters()
{
	lcd_load_custom_character(levels+0,0); // no offset, e.g. one bar
	lcd_load_custom_character(levels+1,1); // two bars
	lcd_load_custom_character(levels+2,2); // etc...
	lcd_load_custom_character(levels+3,3);
	lcd_load_custom_character(levels+4,4);
	lcd_load_custom_character(levels+5,5);
	lcd_load_custom_character(levels+6,6);
	lcd_load_custom_character(back_arrow,7);
	clear();
}

const char bar_graph_characters[10] = {' ',0,0,1,2,3,3,4,5,255};

//========================================================================================

char wait_for_button_and_beep()
{
	char button = wait_for_button_press(ANY_BUTTON);
	
	if(button & BUTTON_A)
		play_from_program_space(beep_button_a);
	else if(button & BUTTON_B)
		play_from_program_space(beep_button_b);
	else
		play_from_program_space(beep_button_c);

	wait_for_button_release(button);
	return button;
}
//================================================================================
void display_readings(const unsigned int *calibrated_values)
{
	unsigned char i;

	for(i=0;i<5;i++) {
		// Initialize the array of characters that we will use for the
		// graph.  Using the space, an extra copy of the one-bar
		// character, and character 255 (a full black box), we get 10
		// characters in the array.
		const char display_characters[10] = {' ',0,0,1,2,3,4,5,6,255};

		// The variable c will have values from 0 to 9, since
		// calibrated values are in the range of 0 to 1000, and
		// 1000/101 is 9 with integer math.
		char c = display_characters[calibrated_values[i]/101];

		// Display the bar graph character.
		print_character(c);
	}
}

//====================================================================================
void print_two_lines_delay_1s(const char *line1, const char *line2)
{
	// Play welcome music and display a message
	clear();
	print_from_program_space(line1);
	lcd_goto_xy(0,1);

	print_from_program_space(line2);
	delay_ms(1000);
}
//========================================================================================
uint8_t speed_pid = 60;
uint8_t speed_no_pid = 60;
//========================================================================================
void initialize()
{

	speed_pid = eeprom_read_byte1((uint16_t)addr_pid);
	speed_no_pid = eeprom_read_byte1((uint16_t)addr_no_pid);

	// This must be called at the beginning of 3pi code, to set up the
	// sensors.  We use a value of 2000 for the timeout, which
	// corresponds to 2000*0.4 us = 0.8 ms on our 20 MHz processor.
	pololu_3pi_init(2000);
	load_custom_characters(); // load the custom characters
	
	play_from_program_space(welcome);
	//print_two_lines_delay_1s(welcome_line1,welcome_line2);
	//print_two_lines_delay_1s(demo_name_line1,demo_name_line2);
	//print_two_lines_delay_1s(instructions_line1,instructions_line2);

	clear();
	print_from_program_space(welcome_line2);
	lcd_goto_xy(0,1);
	print_from_program_space(instructions_line3);

	while(!(wait_for_button_and_beep() & BUTTON_B));

	play_from_program_space(thank_you_music);

	//print_two_lines_delay_1s(thank_you_line1,thank_you_line2);
}
//==========================================================================================
#define   FORWARD_OFFSET   0xA0            // Offset (0..255) of forward from the the front line
#define   MAX_SPEED      255               // Maximum speed the wheels will go
#define   MIN_SPEED      200               // Minimum speed the wheels will go

void Spinning_Line_Follow( void )
{
	unsigned int counter; // used as a simple timer
	unsigned int sensors[5]; // an array to hold sensor values
	
	// This must be called at the beginning of 3pi code, to set up the
	// sensors.  We use a value of 2000 for the timeout, which
	// corresponds to 2000*0.4 us = 0.8 ms on our 20 MHz processor.
	pololu_3pi_init(2000);
	load_custom_characters(); // load the custom characters
	
	// Play welcome music and display a message
	//print_from_program_space(welcome_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(welcome_line2);
	play_from_program_space(welcome);
	//delay_ms(1000);

	//clear();
	//print_from_program_space(demo_name_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(demo_name_line2);
	//delay_ms(1000);

	// Display battery voltage and wait for button press
	while(!button_is_pressed(BUTTON_B))
	{
		int bat = read_battery_millivolts();

		clear();
		print_long(bat);
		print("mV");
		lcd_goto_xy(0,1);
		print("Press B");

		delay_ms(100);
	}

	// Always wait for the button to be released so that 3pi doesn't
	// start moving until your hand is away from it.
	wait_for_button_release(BUTTON_B);
	delay_ms(1000);

	// Auto-calibration: turn right and left while calibrating the
	// sensors.
	for(counter=0;counter<80;counter++)
	{
		if(counter < 20 || counter >= 60)
			set_motors(40,-40);
		else
			set_motors(-40,40);

		// This function records a set of sensor readings and keeps
		// track of the minimum and maximum values encountered.  The
		// IR_EMITTERS_ON argument means that the IR LEDs will be
		// turned on during the reading, which is usually what you
		// want.
		calibrate_line_sensors(IR_EMITTERS_ON);

		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay_ms(20);
	}
	set_motors(0,0);

	// Display calibrated values as a bar graph.
	while(!button_is_pressed(BUTTON_B))
	{
		// Read the sensor values and get the position measurement.
		unsigned int position = read_line(sensors,IR_EMITTERS_ON);

		// Display the position measurement, which will go from 0
		// (when the leftmost sensor is over the line) to 4000 (when
		// the rightmost sensor is over the line) on the 3pi, along
		// with a bar graph of the sensor readings.  This allows you
		// to make sure the robot is ready to go.
		clear();
		print_long(position);
		lcd_goto_xy(0,1);
		display_readings(sensors);

		delay_ms(100);
	}
	wait_for_button_release(BUTTON_B);

	clear();

	print("Go!");		

	// Play music and wait for it to finish before we start driving.
	play_from_program_space(go);
	while(is_playing());




   unsigned short phase_start = get_ms();   // Start time of this rotation
   unsigned short last_phase_len = 100;   // Duration of the last rotation
   char last_line_side = 0;            // which side was the line on?
   char line_count = 0;               // Is this the front or back line?
   //char led_duration = 0;               // How much longer should the LED be on

   while ( 1 ) {
      unsigned short cur_time = get_ms();   // Grab the current time in ms
      unsigned int sensors[5];         // Is the line left or right?
      char line_side = (read_line(sensors,IR_EMITTERS_ON) < 2000);   
      left_led( 0 );                  // Turn off the "FRONT" LED
      if (line_side & !last_line_side) {   // If it just changed,
         if ( ++line_count & 1 ) {      // and if this is the front line
            left_led( 1 );            // Turn on "FRONT" LED
            last_phase_len = cur_time - phase_start;// save the last rotation duration
            phase_start = cur_time;      // and start counting this rotation
         }
      }
      last_line_side = line_side;         // Remember where the line was

      unsigned short cur_phase = cur_time - phase_start;   // How far are we into the curent rotation?
      cur_phase <<= 8;               // Multipy by 256
      cur_phase /= last_phase_len;      // based on the last rotation duration
      cur_phase += FORWARD_OFFSET;      // offset by which direction is "FORWARD"
      short left = cur_phase & 0xFF;      // Wrap back to 0 .. 255
      if ( left >= 128 ) {            // Convert to 0 .. 127 .. 0
         left = 256 - left;
      }
      left = (((left * (MAX_SPEED - MIN_SPEED))>>7) + MIN_SPEED);   // Scale the wheel speed to be MIN at 0, MAX at 127
      short right = MAX_SPEED + MIN_SPEED - left;   // the right is 180 degress out of phase from the left
      set_motors(left, -right);         // and the right goes backwards
   }
}
//============================================================================================
void bat_test()
{
	int bat = read_battery_millivolts();

	print_long(bat);
	print("mV");

	delay_ms(100);
}

char wait_for_250_ms_or_button_b()
{
	int i;
	for(i=0;i<25;i++)
	{
		delay_ms(10);
		if(button_is_pressed(BUTTON_B))
			return 1;
	}
	return 0;
}

// Blinks the LEDs
void led_test()
{
	play("c32");
	print("Red  ");

	red_led(1);
	if(wait_for_250_ms_or_button_b())
		return;
	red_led(0);
	if(wait_for_250_ms_or_button_b())
		return;

	play(">c32");
	lcd_goto_xy(0,0);
	print("Green");

	green_led(1);
	if(wait_for_250_ms_or_button_b())
		return;
	green_led(0);
	if(wait_for_250_ms_or_button_b())
		return;
}

void ir_test()
{
	unsigned int sensors[5]; // an array to hold sensor values

	if(button_is_pressed(BUTTON_C))
		read_line_sensors(sensors, IR_EMITTERS_OFF);
	else
		read_line_sensors(sensors,IR_EMITTERS_ON);

	unsigned char i;

	for(i=0;i<5;i++) {
		// Initialize the array of characters that we will use for the
		// graph.  Using the space, an extra copy of the one-bar
		// character, and character 255 (a full black box), we get 10
		// characters in the array.

		// The variable c will have values from 0 to 9, since
		// values are in the range of 0 to 2000, and 2000/201 is 9
		// with integer math.
		char c = bar_graph_characters[sensors[i]/201];

		// Display the bar graph characters.
		print_character(c);

	}

	// Display an indicator of whether IR is on or off
	if(button_is_pressed(BUTTON_C))
		print("IR-");
	else
		print("  C");

	delay_ms(100);
}

int m1_speed = 0;
int m2_speed = 0;

void motor_test()
{
	static char m1_back = 0, m2_back = 0;
	char m1_char, m2_char;

	if(button_is_pressed(BUTTON_A))
	{
		if(m1_speed == 0)
		{
			delay_ms(200);

			// If the button is pressed quickly when the motor is off,
			// reverse direction.
			if(!button_is_pressed(BUTTON_A))
				m1_back = !m1_back;
		}
		
		m1_speed += 10;
	}
	else
		m1_speed -= 20;

	if(button_is_pressed(BUTTON_C))
	{
		if(m2_speed == 0)
		{
			delay_ms(200);

			// If the button is pressed quickly when the motor is off,
			// reverse direction.
			if(!button_is_pressed(BUTTON_C))
				m2_back = !m2_back;
		}

		m2_speed += 10;
	}
	else
		m2_speed -= 20;

	if(m1_speed < 0)
		m1_speed = 0;

	if(m1_speed > 255)
		m1_speed = 255;

	if(m2_speed < 0)
		m2_speed = 0;

	if(m2_speed > 255)
		m2_speed = 255;

	// 255/26 = 9, so this gives values in the range of 0 to 9
	m1_char = bar_graph_characters[m1_speed / 26];
	m2_char = bar_graph_characters[m2_speed / 26];
	print_character(m1_char);
	print_character(m1_back ? 'a' : 'A');
	print_character(m1_char);
	lcd_goto_xy(5,0);
	print_character(m2_char);
	print_character(m2_back ? 'c' : 'C');
	print_character(m2_char);

	set_motors(m1_speed * (m1_back ? -1 : 1), m2_speed * (m2_back ? -1 : 1));
	delay_ms(50);
}

const char fugue[] PROGMEM = 
  "! T120O5L16agafaea dac+adaea fa<aa<bac#a dac#adaea f"
  "O6dcd<b-d<ad<g d<f+d<gd<ad<b- d<dd<ed<f+d<g d<f+d<gd<ad"
  "L8MS<b-d<b-d MLe-<ge-<g MSc<ac<a MLd<fd<f O5MSb-gb-g"
  "ML>c#e>c#e MS afaf ML gc#gc# MS fdfd ML e<b-e<b-"
  "O6L16ragafaea dac#adaea fa<aa<bac#a dac#adaea faeadaca"
  "<b-acadg<b-g egdgcg<b-g <ag<b-gcf<af dfcf<b-f<af"
  "<gf<af<b-e<ge c#e<b-e<ae<ge <fe<ge<ad<fd"
  "O5e>ee>ef>df>d b->c#b->c#a>df>d e>ee>ef>df>d"
  "e>d>c#>db>d>c#b >c#agaegfe fO6dc#dfdc#<b c#4";

const char bonnie[] PROGMEM =
  "o5 v12 g+32g8.g+32g-32 v14 g4a8 v16 b8..>c32b8a8 v14 g8..f#32g8e32f32e16"
  "v12 d8..d+32d8f#8 g4. v14 g+32g-32g8.bo6d8 v16 e8.e+32e-32 e4 v12 d2"
  "r16 v16 e8..e+32e8d8 o5b.o6d8 v14 d+32c16.o5b8b+32a16.g8 v13 e e+32d16.f#8"
  "g v14 g-32b16.o6d8 v16 e+32e8..d8o5b8 v14 a8..a+32a-32a8.. g2g+32g-32g2";

const char fugue_title[] PROGMEM = "       \7 Fugue in D Minor - by J.S. Bach \7       ";
const char bonnie_title[] PROGMEM = "The Bonnie Banks o' Loch Lomond";

void music_test()
{
	static char fugue_title_pos = 0;
	static long last_shift = 0;
	char c,i;

	if(get_ms() - last_shift > 250)
	{
		for(i=0;i<8;i++)
		{
			c = pgm_read_byte(bonnie_title + fugue_title_pos + i);
			print_character(c);
		}
		last_shift = get_ms();

		fugue_title_pos ++;
		if(fugue_title_pos + 8 >= sizeof(bonnie_title))
			fugue_title_pos = 0;
	}

	if(!is_playing())
	{
		play_from_program_space(bonnie);
	}

	delay_ms(100);
}

void pot_test()
{
	long start = get_ms();
	char elapsed_ms;
	int value;

	set_analog_mode(MODE_10_BIT);
	print_long(read_trimpot());
	print("   "); // to clear the display

	while((elapsed_ms = get_ms() - start) < 100)
	{
		value = read_trimpot();
		play_frequency(value, 200, 15);
		
		if(value < elapsed_ms*10)
		{
			red_led(0);
			green_led(1);
		}
		else
		{
			red_led(1);
			green_led(0);
		}
	}
}

void time_test()
{
	static long elapsed_time = 0;
	static long last_read = 0;
	static long is_ticking = 0;
	static char a_is_pressed = 0;
	static char c_is_pressed = 0;
	static char last_seconds = 0;

	long current_time = get_ms();
	if(is_ticking)
		elapsed_time += current_time - last_read;

	last_read = current_time;

	if(button_is_pressed(BUTTON_A) && !a_is_pressed)
	{
		// reset
		a_is_pressed = 1;
		is_ticking = 0;
		elapsed_time = 0;
		if(!is_playing()) // only play once
			play_from_program_space(beep_button_a);
	}

	// find the end of the button press without stopping
	if(!button_is_pressed(BUTTON_A))
		a_is_pressed = 0;

	if(button_is_pressed(BUTTON_C) && !c_is_pressed)
	{
		// start/stop
		c_is_pressed = 1;
		is_ticking = !is_ticking;
		play_from_program_space(beep_button_c);
	}

	// find the end of the button press without stopping
	if(!button_is_pressed(BUTTON_C))
		c_is_pressed = 0;

	print_long((elapsed_time/1000/60/10)%10); // tens of minutes
	print_long((elapsed_time/1000/60)%10); // minutes
	print_character(':');
	print_long((elapsed_time/1000)%60/10); // tens of seconds
	char seconds = ((elapsed_time/1000)%60)%10;
	print_long(seconds); // seconds
	print_character('.');
	print_long((elapsed_time/100)%10); // tenths of seconds
	print_long((elapsed_time/10)%10); // hundredths of seconds

	// beep every second
	if(seconds != last_seconds && elapsed_time != 0 && !is_playing())
		play_from_program_space(timer_tick);
	last_seconds = seconds;
}




//=================================================================================
void menu_select()
{
	static int menu_index = 0;

	//print_two_lines_delay_1s(main_menu_intro_line1,main_menu_intro_line2);

	while(1)
	{
		clear();
		lcd_goto_xy(0,1);
		print_from_program_space(menu_line2);
		lcd_goto_xy(0,0);
		print_from_program_space(main_menu_options[menu_index]);
		lcd_show_cursor(CURSOR_BLINKING);
		// the cursor will be blinking at the end of the option name
	
		// wait for all buttons to be released, then a press
		while(button_is_pressed(ANY_BUTTON));
		char button = wait_for_button_press(ANY_BUTTON);

		if(button & BUTTON_A)
		{
			play_from_program_space(beep_button_a);
			menu_index --;
		}
		else if(button & BUTTON_B)
		{
			lcd_hide_cursor();
			clear();

			play_from_program_space(beep_button_b);
			wait_for_button_release(button);

			while(!button_is_pressed(BUTTON_B))
			{
				lcd_goto_xy(0,1);
				print_from_program_space(back_line2);
				lcd_goto_xy(0,0);
				main_menu_functions[menu_index]();
			}
			if(menu_index == 10) {

				eeprom_write_byte1((uint16_t)addr_pid, speed_pid);
			}

			if(menu_index == 11) {

				eeprom_write_byte1((uint16_t)addr_no_pid, speed_no_pid);
			}

			set_motors(0,0);
			stop_playing();
			m1_speed = 0;
			m2_speed = 0;
			red_led(0);
			green_led(0);
			play_from_program_space(beep_button_b);

			return;
		}
		else if(button & BUTTON_C)
		{
			play_from_program_space(beep_button_c);
			menu_index ++;
		}

		if(menu_index < 0)
			menu_index = main_menu_length-1;
		if(menu_index >= main_menu_length)
			menu_index = 0;
	}
}
//=================================================================================
void PID_Line_Follow(){
	unsigned int last_proportional=0;
	long integral=0;

	unsigned int counter; // used as a simple timer
	unsigned int sensors[5]; // an array to hold sensor values

	// This must be called at the beginning of 3pi code, to set up the
	// sensors.  We use a value of 2000 for the timeout, which
	// corresponds to 2000*0.4 us = 0.8 ms on our 20 MHz processor.
	pololu_3pi_init(2000);
	load_custom_characters(); // load the custom characters
	
	// Play welcome music and display a message
	//print_from_program_space(welcome_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(welcome_line2);
	//play_from_program_space(welcome);
	//delay_ms(1000);

	//clear();
	//print_from_program_space(demo_name_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(demo_name_line2);
	//delay_ms(1000);

	// Display battery voltage and wait for button press
	while(!button_is_pressed(BUTTON_B))
	{
		int bat = read_battery_millivolts();

		clear();
		print_long(bat);
		print("mV");
		lcd_goto_xy(0,1);
		print("Press B");

		delay_ms(100);
	}

	// Always wait for the button to be released so that 3pi doesn't
	// start moving until your hand is away from it.
	wait_for_button_release(BUTTON_B);
	delay_ms(1000);

	// Auto-calibration: turn right and left while calibrating the
	// sensors.
	for(counter=0;counter<80;counter++)
	{
		if(counter < 20 || counter >= 60)
			set_motors(40,-40);
		else
			set_motors(-40,40);

		// This function records a set of sensor readings and keeps
		// track of the minimum and maximum values encountered.  The
		// IR_EMITTERS_ON argument means that the IR LEDs will be
		// turned on during the reading, which is usually what you
		// want.
		calibrate_line_sensors(IR_EMITTERS_ON);

		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay_ms(20);
	}
	set_motors(0,0);

	// Display calibrated values as a bar graph.
	while(!button_is_pressed(BUTTON_B))
	{
		// Read the sensor values and get the position measurement.
		unsigned int position = read_line(sensors,IR_EMITTERS_ON);

		// Display the position measurement, which will go from 0
		// (when the leftmost sensor is over the line) to 4000 (when
		// the rightmost sensor is over the line) on the 3pi, along
		// with a bar graph of the sensor readings.  This allows you
		// to make sure the robot is ready to go.
		clear();
		print_long(position);
		lcd_goto_xy(0,1);
		display_readings(sensors);

		delay_ms(100);
	}
	wait_for_button_release(BUTTON_B);

	clear();

	print("Go!");		

	// Play music and wait for it to finish before we start driving.
	play_from_program_space(go);
	while(is_playing());

	while(1)
	{
		
		unsigned int position = read_line(sensors,IR_EMITTERS_ON);
		int proportional = ((int)position) - 2000;
		int derivative = proportional - last_proportional;
		integral += proportional;
		last_proportional = proportional;
		int power_difference = proportional/20 + integral/10000 + derivative*3/2;

		//const int max = 60;
		if(power_difference > speed_pid)
			power_difference = speed_pid;
		if(power_difference < -speed_pid)
			power_difference = -speed_pid;

		if(power_difference < 0)
			set_motors(speed_pid+power_difference, speed_pid);
		else
			set_motors(speed_pid, speed_pid-power_difference);
	}

}
void No_PID_Line_Follow(){

	unsigned int counter; // used as a simple timer
	unsigned int sensors[5]; // an array to hold sensor values

	// This must be called at the beginning of 3pi code, to set up the
	// sensors.  We use a value of 2000 for the timeout, which
	// corresponds to 2000*0.4 us = 0.8 ms on our 20 MHz processor.
	pololu_3pi_init(2000);
	load_custom_characters(); // load the custom characters
	
	// Play welcome music and display a message
	//print_from_program_space(welcome_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(welcome_line2);
	//play_from_program_space(welcome);
	//delay_ms(1000);

	//clear();
	//print_from_program_space(demo_name_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(demo_name_line2);
	//delay_ms(1000);

	// Display battery voltage and wait for button press
	while(!button_is_pressed(BUTTON_B))
	{
		int bat = read_battery_millivolts();

		clear();
		print_long(bat);
		print("mV");
		lcd_goto_xy(0,1);
		print("Press B");

		delay_ms(100);
	}

	// Always wait for the button to be released so that 3pi doesn't
	// start moving until your hand is away from it.
	wait_for_button_release(BUTTON_B);
	delay_ms(1000);

	// Auto-calibration: turn right and left while calibrating the
	// sensors.
	for(counter=0;counter<80;counter++)
	{
		if(counter < 20 || counter >= 60)
			set_motors(40,-40);
		else
			set_motors(-40,40);

		// This function records a set of sensor readings and keeps
		// track of the minimum and maximum values encountered.  The
		// IR_EMITTERS_ON argument means that the IR LEDs will be
		// turned on during the reading, which is usually what you
		// want.
		calibrate_line_sensors(IR_EMITTERS_ON);

		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay_ms(20);
	}
	set_motors(0,0);

	// Display calibrated values as a bar graph.
	while(!button_is_pressed(BUTTON_B))
	{
		// Read the sensor values and get the position measurement.
		unsigned int position = read_line(sensors,IR_EMITTERS_ON);

		// Display the position measurement, which will go from 0
		// (when the leftmost sensor is over the line) to 4000 (when
		// the rightmost sensor is over the line) on the 3pi, along
		// with a bar graph of the sensor readings.  This allows you
		// to make sure the robot is ready to go.
		clear();
		print_long(position);
		lcd_goto_xy(0,1);
		display_readings(sensors);

		delay_ms(100);
	}
	wait_for_button_release(BUTTON_B);

	clear();

	print("Go!");		

	// Play music and wait for it to finish before we start driving.
	play_from_program_space(go);
	while(is_playing());

while(1)
	{
		
		unsigned int position = read_line(sensors,IR_EMITTERS_ON);

		if(position < 1000)
		{
		
			set_motors(0,speed_no_pid);

			left_led(1);
			right_led(0);
		}
		else if(position < 3000)
		{

			set_motors(speed_no_pid,speed_no_pid);
			left_led(1);
			right_led(1);
		}
		else
		{

			set_motors(speed_no_pid,0);
			left_led(0);
			right_led(1);
		}
	}



}

//int speed_pid = 60;

void edit_pid() {

	lcd_goto_xy(0,0);
	print_long(speed_pid);

	if(button_is_pressed(BUTTON_A)) {

		delay_ms(200);

		speed_pid -= 5;	

	}
	else 
	if(button_is_pressed(BUTTON_C)) {

		delay_ms(200);
		speed_pid += 5;


	}

	if(speed_pid < 100) {

		lcd_goto_xy(2,0);
		print(" ");
	}
	if(speed_pid < 0) {

		speed_pid = 255;
	}
	if(speed_pid > 255) {

		speed_pid = 0;
	}
	

}

//int speed_no_pid = 60;

void edit_no_pid() {

lcd_goto_xy(0,0);

	print_long(speed_no_pid);

	if(button_is_pressed(BUTTON_A)) {

		delay_ms(200);

		speed_no_pid -= 5;	

	}
	else 
	if(button_is_pressed(BUTTON_C)) {

		delay_ms(200);
		speed_no_pid += 5;


	}

	if(speed_no_pid < 100) {

		lcd_goto_xy(2,0);
		print(" ");
	}
	if(speed_no_pid < 0) {

		speed_no_pid = 255;
	}
	if(speed_no_pid > 255) {

		speed_no_pid = 0;
	}
}

void maze() {

	unsigned int counter; // used as a simple timer
	unsigned int sensors[5]; // an array to hold sensor values

	// This must be called at the beginning of 3pi code, to set up the
	// sensors.  We use a value of 2000 for the timeout, which
	// corresponds to 2000*0.4 us = 0.8 ms on our 20 MHz processor.
	pololu_3pi_init(2000);
	load_custom_characters(); // load the custom characters
	
	// Play welcome music and display a message
	//print_from_program_space(welcome_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(welcome_line2);
	//play_from_program_space(welcome);
	//delay_ms(1000);

	//clear();
	//print_from_program_space(demo_name_line1);
	//lcd_goto_xy(0,1);
	//print_from_program_space(demo_name_line2);
	//delay_ms(1000);

	// Display battery voltage and wait for button press
	while(!button_is_pressed(BUTTON_B))
	{
		int bat = read_battery_millivolts();

		clear();
		print_long(bat);
		print("mV");
		lcd_goto_xy(0,1);
		print("Press B");

		delay_ms(100);
	}

	// Always wait for the button to be released so that 3pi doesn't
	// start moving until your hand is away from it.
	wait_for_button_release(BUTTON_B);
	delay_ms(1000);

	// Auto-calibration: turn right and left while calibrating the
	// sensors.
	for(counter=0;counter<80;counter++)
	{
		if(counter < 20 || counter >= 60)
			set_motors(40,-40);
		else
			set_motors(-40,40);

		// This function records a set of sensor readings and keeps
		// track of the minimum and maximum values encountered.  The
		// IR_EMITTERS_ON argument means that the IR LEDs will be
		// turned on during the reading, which is usually what you
		// want.
		calibrate_line_sensors(IR_EMITTERS_ON);

		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay_ms(20);
	}
	set_motors(0,0);

	// Display calibrated values as a bar graph.
	while(!button_is_pressed(BUTTON_B))
	{
		// Read the sensor values and get the position measurement.
		unsigned int position = read_line(sensors,IR_EMITTERS_ON);

		// Display the position measurement, which will go from 0
		// (when the leftmost sensor is over the line) to 4000 (when
		// the rightmost sensor is over the line) on the 3pi, along
		// with a bar graph of the sensor readings.  This allows you
		// to make sure the robot is ready to go.
		clear();
		print_long(position);
		lcd_goto_xy(0,1);
		display_readings(sensors);

		delay_ms(100);
	}
	wait_for_button_release(BUTTON_B);

	clear();

	print("Go!");		

	// Play music and wait for it to finish before we start driving.
	play_from_program_space(go);
	while(is_playing());

	maze_solve();

}
//=================================================================================
int main()
{

	initialize();

	while(1)
	{
		menu_select();
	}
}
