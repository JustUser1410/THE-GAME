#define vid 0x045e
#define pid 0x028e
#define one 0x01
#define endpOut 0x81
#define endpIn 0x01

#define ledOne 0x06
#define ledTwo 0x07
#define ledThree 0x08
#define ledFour 0x09
#define blink 0x01
#define off 0x00

#include <menu.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

//Initialize USB device
libusb_device_handle *h;
int error, transferred;
uint8_t dataIn[20];

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4

char *choices[] = {
                        "Play",
                        "Help",
                        "Exit",
                        (char *)NULL,
                  };

//Moethod to print text in the middle of the window
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
//Return value 1, 2 or 3 according to menu value
void Help(int *value);
void Play(int *value);
void Menu(int *value);

// Simple windows where user is prompted wether he wants to continue or exit
// 0 to continue and 3 to exit
void Winner(int *output);
void Looser(int *output);

int main()
{
  // Number which tells program what sould be it's next action:
  //      o 0 - Go to Menu
  //      o 1 - Play the game
  //      o 2 - Go to Help
  //      o 3 - Exit

  int action = 0;

  // Initialize curses
  initscr();                              // Initialize screen
  start_color();                          // Enable colours
  cbreak();                               // To get input without ENTER
  noecho();                               // Don't print input
  keypad(stdscr, TRUE);                   // Enable kayboard
  init_pair(1, COLOR_RED, COLOR_BLACK);   // Initialize colour pair for title bar
  curs_set(0);			                      // No cursor

  //Initialize USB device
  libusb_init(NULL);
    libusb_set_debug(NULL, 3);
	h = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if (h == NULL) {
		fprintf(stderr, "Failed to open device\n");
		return (1);
	}
	if (libusb_kernel_driver_active(h, 0) == 1){
		printf("Kernel driver found\n");
		if (libusb_detach_kernel_driver(h, 0) != 0){
			printf("Can't detach from the kernel\n");
			return -1;
		}
	}

  // Start the program
  while (action != 3) // Not "Exit"
  { // Keep checking what is the next action
    if(action == 0)
      Menu(&action);
    else if(action == 1)
      Play(&action);
    else if(action == 2)
      Help(&action);
  }
  // End
  return (0);
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}

void Menu(int *value)
{
    ITEM **my_items;
  	//int c;       // Not needed anymore
  	MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;
    int output = 3;

  	// Create Menu items
          n_choices = ARRAY_SIZE(choices);
          my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
          for(i = 0; i < n_choices; ++i)
                  my_items[i] = new_item(choices[i], "");

  	// Crate Menu
  	my_menu = new_menu((ITEM **)my_items);

  	// Create the window to be associated with the menu
          my_menu_win = newwin(15, 50, 4, 4);
          keypad(my_menu_win, TRUE);          // Enable keyboard on that window

  	// Set main window and sub window
          set_menu_win(my_menu, my_menu_win);
          set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));

  	// Set menu mark to the string " * "
          set_menu_mark(my_menu, " * ");

  	// Print a border around the main window and print a title
          box(my_menu_win, 0, 0);
  	print_in_middle(my_menu_win, 1, 0, 50, "THE GAME", COLOR_PAIR(1));
  	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
  	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 48);
  	mvwaddch(my_menu_win, 2, 39, ACS_RTEE);

  	/* Post the menu */
  	post_menu(my_menu);
  	wrefresh(my_menu_win);              // Refres the window

    bool done = false;
  	while(!done ) //&& (c = wgetch(my_menu_win)) != 'q' )
  	{ if ((error = libusb_interrupt_transfer(h, endpOut, dataIn, sizeof dataIn, &transferred, 0)) == 0)
  	        {	if ((dataIn[4])> 0)               // LT to go  Down
        				menu_driver(my_menu, REQ_DOWN_ITEM);
        			else if ((dataIn[3]>>0)&one)      // LB to go Up
        				menu_driver(my_menu, REQ_UP_ITEM);

              if ((dataIn[3]>>6)&one)      // X to ENTER
              {
          			if(item_index(current_item(my_menu)) == 0)
                { // First menu item was selected (Play)
                  output = 1;
                  done = true;
                }
                if(item_index(current_item(my_menu)) == 1)
                { // Second menu item was selected (Help))
                  output = 2;
                  done = true;
                }
                if(item_index(current_item(my_menu)) == 2)
                { // Third menu item was selected (Exit)
                  output = 3;
                  done = true;
                }
  		        }
      }
      usleep(250000);
      wrefresh(my_menu_win);              // Refres the window
  	}

  	// Unpost and free all the memory taken up
          unpost_menu(my_menu);
          free_menu(my_menu);
          for(i = 0; i < n_choices; ++i)
                  free_item(my_items[i]);
  	endwin();
    *value = output;
}

void Help(int *value)
{
  //int c;       // Not needed anymore
  int output = 0;
  // Create new window
  WINDOW *help_win;
  help_win = newwin(15, 50, 4, 4);
  keypad(help_win, TRUE);          // Enable keyboard input
  box(help_win, 0, 0);             // Put a box around the wimdow
  // Add title
  print_in_middle(help_win, 1, 0, 50, "HELP", COLOR_PAIR(1));
  mvwaddch(help_win, 2, 0, ACS_LTEE);
  mvwhline(help_win, 2, 1, ACS_HLINE, 48);
  mvwaddch(help_win, 2, 39, ACS_RTEE);
  // Add text
  mvwaddstr(help_win, 3, 2, "WE BOTH KNOW YOU DON'T NEED HELP! ");
  mvwaddstr(help_win, 4, 2, "JUST GO AND PLAY");
  mvwaddstr(help_win, 4, 2, "X = ENTER, everything else explained OTG");
  mvwaddstr(help_win, 7, 2, "Press A to go back");
  wrefresh(help_win);              // Refres the window
  // Wait for correct user input
  while(true)
  {
    if ((error = libusb_interrupt_transfer(h, endpOut, dataIn, sizeof dataIn, &transferred, 0)) == 0)
        if (((dataIn[3]>>4)&one) )   // A was pressed
            break;
  }
  usleep(250000);
  // Destroy the window
  endwin();
  //Write the output
  *value = output;
}

void Play(int *value)
{
  //int c;        //Not needed anymore
  int output = 3; // Set default output value to "Exit"
  // Create new window
  WINDOW *game_win;
  game_win = newwin(15,50,4,4);
  keypad(game_win, TRUE);           // Enable keyboard input
  box(game_win, 0, 0);              // Put a box around the wimdow
  // Add title
  print_in_middle(game_win, 1, 0, 50, "THE GAME", COLOR_PAIR(1));
  mvwaddch(game_win, 2, 0, ACS_LTEE);
  mvwhline(game_win, 2, 1, ACS_HLINE, 48);
  mvwaddch(game_win, 2, 39, ACS_RTEE);
  //sleep(500);
  //Add text
  mvwaddstr(game_win, 3, 2, "This is it, the faith of the whole world is in");
  //sleep(500);
  mvwaddstr(game_win, 4, 2, "your hands. You are the only one who has the");
  //sleep(500);
  mvwaddstr(game_win, 5, 2, "power to defeat evil forces and save the human");
  mvwaddstr(game_win, 6, 2, "kind.");
  //sleep(500);
  mvwaddstr(game_win, 6, 7, "... or DESTROY IT!!!");
  //sleep(500);
  mvwaddstr(game_win, 8, 2, "The power is in your hands. Take the controller");
  mvwaddstr(game_win, 9, 2, "and make your choise:");
  mvwaddstr(game_win, 10, 4, "press A");
  mvwaddstr(game_win, 10, 16, "press B");
  wrefresh(game_win);              // Refres the window

  bool done = false;
  bool win = false;
  // Wait for correct user input
  while(!done && !((dataIn[2]>>5)&one)) // You can press "Back" to surender
  {
      //To do:
      //Choose wining button at random
      //For now:
      if ((error = libusb_interrupt_transfer(h, endpOut, dataIn, sizeof dataIn, &transferred, 0)) == 0)
      {
          if (((dataIn[3]>>4)&one))    // A was pressed
          {
            win = true;
            break;
          }
          else if ((dataIn[3]>>5)&one) // B was pressed
          {
            win = false;
            break;
          }
          usleep(250000);
      }
  }
  // Destroy the window
  endwin();
  // Salute the winner or admit the looser
  if (win)
    Winner(&output);
  else
    Looser(&output);
  *value = output;
}

void Winner(int *output)
{
  //int c;       // Not needed anymore
  *output = 3;
  // Create new window
  WINDOW *win_win;
  win_win = newwin(15, 50, 4, 4);
  keypad(win_win, TRUE);          // Enable keyboard input
  box(win_win, 0, 0);             // Put a box around the wimdow
  // Add title
  print_in_middle(win_win, 1, 0, 50, "CONGRATULATIONS", COLOR_PAIR(1));
  mvwaddch(win_win, 2, 0, ACS_LTEE);
  mvwhline(win_win, 2, 1, ACS_HLINE, 48);
  mvwaddch(win_win, 2, 39, ACS_RTEE);
  //sleep(500);
  // Add text
  mvwaddstr(win_win, 3, 2, "How does it feel to be a hero?");
  //sleep(500);
  mvwaddstr(win_win, 4, 2, "It must feel grate! Keep it up like this!");
  //sleep(500);
  mvwaddstr(win_win, 6, 2, "Still feeling lucky?");
  mvwaddstr(win_win, 7, 2, "Press Y to go again");
  mvwaddstr(win_win, 7, 22, "Press X to quit");
  wrefresh(win_win);              // Refres the window
  // Wait for correct user input
  while(true)
  {
    if ((error = libusb_interrupt_transfer(h, endpOut, dataIn, sizeof dataIn, &transferred, 0)) == 0)
    {
      if ((dataIn[3]>>7)&one)       // Y was pressed
      {
        *output = 0;
         break;
      }
      else if ((dataIn[3]>>6)&one)  // X was pressed
        break;
    }
    usleep(250000);
  }
  // Destroy the window
  endwin();
}

void Looser(int *output)
{
  //int c;       // Not needed anymore
  *output = 3;
  // Create new window
  WINDOW *win;
  win = newwin(15, 50, 4, 4);
  keypad(win, TRUE);          // Enable keyboard input
  box(win, 0, 0);             // Put a box around the wimdow
  // Add title
  print_in_middle(win, 1, 0, 50, "WHAT HAVE YOU DONE!?", COLOR_PAIR(1));
  mvwaddch(win, 2, 0, ACS_LTEE);
  mvwhline(win, 2, 1, ACS_HLINE, 48);
  mvwaddch(win, 2, 39, ACS_RTEE);
  //sleep(500);
  // Add text
  mvwaddstr(win, 3, 2, "We all put our faith in you and now, that you ");
  //sleep(500);
  mvwaddstr(win, 4, 2, "have lost, there is no more hope...");
  mvwaddstr(win, 5, 2, "no more future...");
  //sleep(500);
  mvwaddstr(win, 7, 2, "Still think you can make this right?");
  mvwaddstr(win, 8, 2, "Press Y to go again");
  mvwaddstr(win, 8, 28, "Press X to quit");
  wrefresh(win);              // Refres the window
  // Wait for correct user input
  while(true)
  {
    if ((error = libusb_interrupt_transfer(h, endpOut, dataIn, sizeof dataIn, &transferred, 0)) == 0)
    {
      if ((dataIn[3]>>7)&one)       // Y was pressed
      {
        *output = 0;
         break;
      }
      else if ((dataIn[3]>>6)&one)  // X was pressed
        break;
    }
    usleep(250000);
  }
  // Destroy the window
  endwin();
}
