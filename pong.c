//rqcoon, 07/06/21
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <math.h>


#define PI 3.14159265
//define pi for some funky sine wave shit for the opponent AI
#define FLOAT_TO_INT(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))
//jesus christ

bool inRange();
void output();					//only needed to declare these 2 without gcc complaining
int game_logic();				//so i will only declare these 2 because im a lazy fuck
int newEnemyCoord();

float hitdir[5] = {1.0,0.5,0.0,-0.5,-1};
int ball_vel[3];
int ball[3] = {9,9,1};
int pad_1, pad_2, t, q, loser;
float angle;

int main() {
	struct winsize ts;
	ioctl(0,TIOCGWINSZ,&ts);		//another day where i thank jod for ioctl
	int cols = ts.ws_col;			//and also for not making me french
	int rows = ts.ws_row - 1;		//trolled!

	while (q == 0) {
		t++;
		usleep(53000);
		system("stty cbreak -echo");
		game_logic(t,rows,cols);
		system("stty cooked echo");	//this shit just wont work if i dont use stty
	}					//this means windows users wont be able to play this
	if (q == 2) {
		printf("Player %d is the loser!\n",loser);
	}
	return 0;			
}

int getch(void)					//returns keyboard input
{
	struct termios oldattr, newattr;	//this is some cryptic shit
	int ch;					//i took from stackoverflow
	tcgetattr(0, &oldattr);			//termios is really so wacky shit i dont understand
	newattr=oldattr;			//:)
	newattr.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( 0, TCSANOW, &newattr);
	ch=getchar();
	tcsetattr(0, TCSANOW, &oldattr);
	return(ch);
}

bool kbhit() 					//return true or false if there is keyboard input
{
	int byteswaiting;
	ioctl(0, FIONREAD, &byteswaiting);	//dont question it, ioctl is just on some different shit
	return byteswaiting > 0;		//this is a bool so if the input buffer > 0 it returns true
}						//again, dont question it.

void output(int pad_1, int pad_2, int ball[], int rows, int cols) {
	//need faux-objects - arrays containing values for:
	//	[0] = Y Pos
	//	[1] = X Pos
	//	[2] = balls rotation, doesn't really matter in this functions
	//	the paddles are only the Y position, seeing as X is constant
	//this function will place everything together and then print it out
	
	int i,j;
	char ball_symbol = 'O'; 		//arrays are fucking weird, dont question it
	char pad_symbol = '#';
	
	/*
	 * don't need this here anymore
	struct winsize ts;
	ioctl(0,TIOCGWINSZ,&ts);		//another day where i thank jod for ioctl
	int cols = ts.ws_col;			//and also for not making me french
	int rows = ts.ws_row - 1;		//trolled!
	* not having to redefine this every frame may save on time as well as allowing me to pass it
	* into the game logic function
	*/

	char screen[rows][cols];
	//fill the screen with blank
	//ill use the same nested for() method to print this all out later
	for(i=0;i<rows;i++) {
		for(j=0;j<cols;j++) {
			screen[i][j] = ' ';
		}
	}

	//unreadable ass code
	int ballx = ball[0];
	int bally = ball[1];
	screen[ballx][bally] = ball_symbol;
	
	//to place the paddles I will have to do something stupid
	//this something may involve loops
	//seeing as the paddles are only passed in here with their x and y co ordinates
	//and not their size, seeing as that is constant
	//I can just use a loop to place the symbols correctly
	//screen[pad_1[0] .. pad_1[0] + height of paddle],[pad_1[1]] = pad_symbol;
	//this is the best i can explain this.

	for(int k=0;k<5;k++) {
		screen[pad_1+k][cols+4] = pad_symbol;
		screen[pad_2+k][cols-5] = pad_symbol;
	}
	
	//print the bitch
	for(i=0;i<rows;i++) {
		for(j=0;j<cols;j++) {
			printf("%c",screen[i][j]);
		}
		printf("\n");
	}	
}

int game_logic(int time, int rows, int cols) {
	//actual game logic, use inputs yada yada
	//ball physics and logic
	//	int for ball direction 1 and -1
	//	1 is moving to the right and -1 is moving to the left
	//		multiply by -1 to get the opposite value
	//	oh fuck i need a way of figuring out the area of the paddle
	//	shitfuck

	if (kbhit()) {
		int ch = getch();
		switch(ch) {
			case 'i':
				pad_1 -= 1;
				break;
			case 'k':
				pad_1 += 1;
				break;
			case 'q':
				q = 1;		//this flips the quit variable, causing main() to return 0
		}
	}

	if (pad_1 > rows - 5) { 		//make sure the paddle doesn't go
		pad_1 = rows - 6;		//out of bounds and cause a segfault
	} else if (pad_1 < 0) {
		pad_1 = 0;
	}

	//ball physics 
	//use velocity to then find our new x and y co-ordinates
	//need some way of pushing off the bat, e.g.
	//	#		#
	//	#		# O	this should push the ball upwards
	//	#		#
	//	# O		#
	//	# 		#
	//	this should push the ball downwards
	//
	//angle of the ball is calculated with a float (angle)
	//each side of the paddle can have "influence"/weight
	//so angle would be equal to angle*influence
	//for example
	// 1	# 0.5
	// 2	# 0.25
	// 3	# 0
	// 4	# -0.25
	// 5	# -0.5
	//	so if the ball hits section 5, angle = -angle*-0.5
	//	the negative is to invert the direction

	ball[0] += FLOAT_TO_INT(angle); //lol
	ball[1] += ball[2];
	
	if (ball[1] < 6 && inRange(ball[0]-1,pad_1,pad_1+5)) {
		ball[2] *= -1;
		int offset = pad_1+6 - ball[0];
		angle = hitdir[offset - 1];
		printf("%d %f\n",offset,hitdir[offset - 1]);
	} else if (ball[1] < 1) {
		loser = 1;
		q = 2;
	}
	
	pad_2 = newEnemyCoord(ball,t,pad_2);

	//idk what is going on here, the ball gets trapped behind the enemy paddle if it misses
	//why???
	//the enemy cant lose??????
	if (ball[1] > cols - 7 && inRange(ball[0],pad_2,pad_2+5)) {
		ball[2] *= -1;
		//angle *= -1;
	} else if (ball[1] > cols-1) {
		loser = 2;
		q = 2;
	}


	if (pad_2 > rows - 4) { 		//I should redo this as a function rather
		pad_2 = rows - 7;		//than copy pasting the same thing and replacing the variable
	} else if (pad_2 < 1) {			//too bad!
		pad_2 = 1;
	}

	if (ball[0] < 1) {
		angle *= -1;
	} else if (ball[0] > rows - 2) {
		angle *= -1;
	}

	output(pad_1,pad_2,ball,rows,cols);
	return 0;
}

bool inRange(int val, int low, int high) {
	return ((val-high)*(val-low) <= 0);
}

int newEnemyCoord(int ball[], int t, int current) {
	return ball[0] - 2;
}
