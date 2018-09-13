// Includes
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include "oi.h"

// Constants
#define START_SONG 1
#define SHORT_ERROR  2
#define LONG_ERROR  3

#define diameter_bot 335
#define positive_y 1
#define negative_x 2
#define negative_y 3
#define positive_x 4

#define square_width 500
#define wall_width 38
#define maze_width 1538
#define maze_length 1038

#define start_x 1
#define start_y 0
#define end_x 5
#define end_y 0

#define start_direction 1
#define array_size_x 7
#define array_size_y 7

// Global variables
volatile uint16_t timer_cnt = 0;
volatile uint8_t timer_on = 0;
volatile uint8_t sensors_flag = 0;
volatile uint8_t sensors_index = 0;
volatile uint8_t sensors_in[Sen6Size];
volatile uint8_t sensors[Sen6Size];
volatile int16_t distance = 0;
volatile int16_t angle = 0;

//n=maze width/square width   # of array=2n+1
int maze[array_size_x][array_size_y];
int x, y;


// Functions
void byteTx(uint8_t value);
void delayMs(uint16_t time_ms);
void delayAndUpdateSensors(unsigned int time_ms);
void initialize(void);
void powerOnRobot(void);
void baud(uint8_t baud_code);
void defineSongs(void);

//My Functions
void drive_straight (int velocity);
void rotate_spot (int velocity); //CWW-Positive velocity, CW-Negative Velocity
void rotate_spot_90_degrees(int direction); //1=CWW.....-1=CW
void rotate_spot_180_degrees(void); 
void allign_right(void);
void allign_left(void);
void set_outside_walls (void);
void first_move (int direction);
void moved (int direction);
void backtrack (int direction);
void collide (int direction);
void collision_back_up (void);
int empty_right(int direction);
int empty_forward(int direction);
int empty_left(int direction);
int fix_direction(int direction);
void back_track(int direction);
void error(int error_number);


int main (void) {

	//My variables
	int direction=start_direction;
	int check_right=0, check_forward=0, check_left=0;
	int try_right=0, try_left=0;
	int initial_distance;
	int maze_solved=0;

	// Set up Create and module
	initialize();
	LEDBothOff;
	powerOnRobot();
	byteTx(CmdStart);
	baud(Baud28800);
	defineSongs();
	byteTx(CmdControl);
	byteTx(CmdFull);
	
	// Stop just as a precaution
	drive_straight(0);
	
	//set maze array to all zero
	int k, l;
  
	for(k=0;k<11;k++) {
		for(l=0;l<9;l++) {
			maze[k][l]=0;
		}
	}


	//Set all outside walls to 1-->blocked off  
	set_outside_walls();
	
	//Set start passage to 10 and end passage to 20
	//maze[start_x][start_y]=10;
	maze[end_x][end_y]=20;
	
	initial_distance=((diameter_bot/2)+(square_width/2)+(wall_width/2));

	while(1) {

		//Initial position of the iRobot
		x=start_x;
		y=start_y;
		check_right=5;
		check_left=5;
		check_forward=5;
		distance=0;
		try_right=0;
		try_left=0;
		
		direction=start_direction;
		
		//get into maze
		delayAndUpdateSensors(50);
		while (distance<initial_distance) {
			drive_straight(100);
			delayAndUpdateSensors(50);
		}
		drive_straight(0);
		delayMs(150);
			
			
		//Update position and array
		first_move(direction);
		
	
		for(;;) {
			
			//Check whether the right space has been checked yet
			check_right=empty_right(direction);
			if(check_right==5) {
				error(1);
				return 1;
			
			}else if (check_right==1) {
				//the position has been checked and it's blocked off,
				// or it's the beginning/end
				
				//right side is blocked, so check the forward square
				check_forward=empty_forward(direction);
				
				if (check_forward==5) {
					error(1);
					return 1;
					
				}else if (check_forward==1) {
					//the position has been checked and it's blocked off,
					// or it's the beginning/end
	
					//forward is blocked, so check the left side
				
					//Check whether the left space has been checked yet
					check_left=empty_left(direction);
						
					if (check_left==5) {
						error(1);
						return 1;
					
					}else if (check_left==1) {
						//the position has been checked and it's blocked off,
						// or it's the beginning/end
						
						//All three directions have been checked and they are all blocked,
						//turn around and return to the last square
						rotate_spot_180_degrees();
						
						//update direction
						direction++;
						direction=fix_direction(direction);
						direction++;
						direction=fix_direction(direction);
							
							
					}else if ((check_left==2) || (check_left==0) || (check_left==20)) {	
						//has not been checked yet, or it's open
						//Rotate left to check
						rotate_spot_90_degrees(1);
							
						//update direction
						direction++;
						direction=fix_direction(direction);
						
						/*Move Forward to Check*/
						
							
						
					}//endif	
						
				
				}else if ((check_forward==2) || (check_forward==0)|| (check_forward==20)) {
					//has not been checked yet, or it's open
					
					/*Move Forward to Check*/
					
				}//endif	
			
			}else if ((check_right==2) || (check_right==0)|| (check_right==20)) {
				//has not been checked yet, or it's open
				//Rotate right to check
				rotate_spot_90_degrees(-1);
				//update direction
				direction--;
				direction=fix_direction(direction);
				
				/*Move Forward to Check*/	
				
			}//endif
	
			//if at the end of the maze, go out of maze and stop, play a song and restart at the beginning to solve again
			
			if ((check_right==20) || (check_forward==20) || (check_left==20)) {
				distance=0;
				
				//exit maze
				delayAndUpdateSensors(50);
				while (distance<initial_distance) {
					drive_straight(100);
					delayAndUpdateSensors(50);
				}
				drive_straight(0);
				
				//play a song to celebrate ! 
				byteTx(CmdPlay);
				byteTx(START_SONG);
				delayAndUpdateSensors(13000);
				
				maze_solved=1;
		
				//break and restart program
				break;
			
			
			}//endif


			//The situations below are ones which are ready to be tested by moving
			//forward.  The others will simply skip this if statement and
			//return to the top of the for loop and start over
			if ((check_right==2) || (check_right==0) || (check_left ==2) || 
				(check_left==0) || (check_forward==2) || (check_forward==0)) {
		
			
				//reset distance count and counters
				distance=0;
				try_left=0;
				try_right=0;
		
				//Will move, will break out of loop when it reaches the middle of the following square,
				//or collides with the wall and backs up to the middle again
				while (1) {
				
					drive_straight(100);
					delayAndUpdateSensors(50);
					
					
					//Robot moves into the middle of the next square
					if (distance>=square_width ) {
					
						drive_straight(0);
	
						//Update the array.  Will get 2 if it had been 0 (first time in the square)
						//				     will get 1 if it had been 2 ( back-tracking)
						if(!maze_solved){
							back_track(direction);
						}
						
						moved(direction);
						
						//break out of while, and continue at top of for
						break;
						
					}//endif
		
				
					//Bump Sensor activated
					if (sensors[SenBumpDrop] & BumpEither){
						drive_straight(0);
						
						//Both Left and Right were hit (Head-on collision)
						if((sensors[SenBumpDrop] & BumpLeft) && (sensors[SenBumpDrop] & BumpRight)){
						
							//updates the array, showing it's blocked now
							collide(direction);
							
							//Back up to middle of the square
							collision_back_up();
							delayMs(500);
							
							//Rotate left to get back to the direction of before and try over again
							rotate_spot_90_degrees(1);
							direction++;
							direction=fix_direction(direction);
							
							//Break out of while, starts over at the for loop
							break;
							
							
						}else if(sensors[SenBumpDrop] & BumpLeft){
							// as a precaution
							if ( try_left==0 ) {
								allign_right();
								delayMs(200);
								try_left=1;
							}else if (try_left==1) {
								allign_left();
								delayMs(200);
								allign_left();
								delayMs(200);
								try_left=2;
							}else if (try_left==2){
								allign_left();
								delayMs(200);
							}else{
							}//endif
								
						}else if(sensors[SenBumpDrop] & BumpRight){
							// as a precaution
							if (try_right==0 ) {
								allign_left();
								try_right=1;
							}else if (try_right==1) {
								allign_right();
								allign_right();
								try_right=2;
							}else if (try_right==2){
								allign_right();
							}else{
							}//endif
						}else{
						}//endif
					
					}//endif
				}//endwhile
			}//endif	
		}//endfor
	}//endwhile
	return 0;
}//endmain



SIGNAL(SIG_USART_RECV){
// Serial receive interrupt to store sensor values
  uint8_t temp;

  temp = UDR0;

  if(sensors_flag)
  {
    sensors_in[sensors_index++] = temp;
    if(sensors_index >= Sen6Size)
      sensors_flag = 0;
  }
}


SIGNAL(SIG_OUTPUT_COMPARE1A) {
// Timer 1 interrupt to time delays in ms
  if(timer_cnt)
    timer_cnt--;
  else
    timer_on = 0;
}

void byteTx(uint8_t value) {
// Transmit a byte over the serial port
  while(!(UCSR0A & _BV(UDRE0))) ;
  UDR0 = value;
}



void delayMs(uint16_t time_ms) {
// Delay for the specified time in ms without updating sensor values
  timer_on = 1;
  timer_cnt = time_ms;
  while(timer_on) ;
}




void delayAndUpdateSensors(uint16_t time_ms) {
// Delay for the specified time in ms and update sensor values
  uint8_t temp;

  timer_on = 1;
  timer_cnt = time_ms;
  while(timer_on)
  {
    if(!sensors_flag)
    {
      for(temp = 0; temp < Sen6Size; temp++)
        sensors[temp] = sensors_in[temp];

      // Update running totals of distance and angle
      distance += (int)((sensors[SenDist1] << 8) | sensors[SenDist0]);
      angle += (int)((sensors[SenAng1] << 8) | sensors[SenAng0]);

      byteTx(CmdSensors);
      byteTx(6);
      sensors_index = 0;
      sensors_flag = 1;
    }
  }
}




void initialize(void) {
// Initialize the Mind Control's ATmega168 microcontroller
  cli();

  // Set I/O pins
  DDRB = 0x10;
  PORTB = 0xCF;
  DDRC = 0x00;
  PORTC = 0xFF;
  DDRD = 0xE6;
  PORTD = 0x7D;

  // Set up timer 1 to generate an interrupt every 1 ms
  TCCR1A = 0x00;
  TCCR1B = (_BV(WGM12) | _BV(CS12));
  OCR1A = 71;
  TIMSK1 = _BV(OCIE1A);

  // Set up the serial port with rx interrupt
  UBRR0 = 19;
  UCSR0B = (_BV(RXCIE0) | _BV(TXEN0) | _BV(RXEN0));
  UCSR0C = (_BV(UCSZ00) | _BV(UCSZ01));

  // Turn on interrupts
  sei();
}




void powerOnRobot(void) {
  // If Create's power is off, turn it on
  if(!RobotIsOn)
  {
      while(!RobotIsOn)
      {
          RobotPwrToggleLow;
          delayMs(500);  // Delay in this state
          RobotPwrToggleHigh;  // Low to high transition to toggle power
          delayMs(100);  // Delay in this state
          RobotPwrToggleLow;
      }
      delayMs(3500);  // Delay for startup
  }
}





void baud(uint8_t baud_code) {
// Switch the baud rate on both Create and module
  if(baud_code <= 11)
  {
    byteTx(CmdBaud);
    UCSR0A |= _BV(TXC0);
    byteTx(baud_code);
    // Wait until transmit is complete
    while(!(UCSR0A & _BV(TXC0))) ;

    cli();

    // Switch the baud rate register
    if(baud_code == Baud115200)
      UBRR0 = Ubrr115200;
    else if(baud_code == Baud57600)
      UBRR0 = Ubrr57600;
    else if(baud_code == Baud38400)
      UBRR0 = Ubrr38400;
    else if(baud_code == Baud28800)
      UBRR0 = Ubrr28800;
    else if(baud_code == Baud19200)
      UBRR0 = Ubrr19200;
    else if(baud_code == Baud14400)
      UBRR0 = Ubrr14400;
    else if(baud_code == Baud9600)
      UBRR0 = Ubrr9600;
    else if(baud_code == Baud4800)
      UBRR0 = Ubrr4800;
    else if(baud_code == Baud2400)
      UBRR0 = Ubrr2400;
    else if(baud_code == Baud1200)
      UBRR0 = Ubrr1200;
    else if(baud_code == Baud600)
      UBRR0 = Ubrr600;
    else if(baud_code == Baud300)
      UBRR0 = Ubrr300;

    sei();

    delayMs(100);
  }
}
void defineSongs(void) {
// Define songs to be played later

  // Start song
  byteTx(CmdSong);
  byteTx(START_SONG);
  byteTx(6);
  byteTx(69);
  byteTx(18);
  byteTx(72);
  byteTx(12);
  byteTx(74);
  byteTx(12);
  byteTx(72);
  byteTx(12);
  byteTx(69);
  byteTx(12);
  byteTx(77);
  byteTx(24);
  
  // Error short
  byteTx(CmdSong);
  byteTx(SHORT_ERROR);
  byteTx(1);
  byteTx(70);
  byteTx(16);
  
   // Error short
  byteTx(CmdSong);
  byteTx(LONG_ERROR);
  byteTx(1);
  byteTx(68);
  byteTx(40);
  
}
 
void drive_straight( int velocity) {
 int vr_input1, vr_input2;
 int vl_input1, vl_input2;
 
 if(velocity>=0) {
 
   vr_input1 = velocity / 0x100;
   vr_input2 = velocity - (vr_input1*0x100);
 
   vl_input1 = velocity / 0x100;
   vl_input2 = velocity - (vl_input1*0x100);
   
 }else if(velocity<0){
   velocity*=(-1);
   vr_input1 = velocity / 0x100;
   vr_input1 = 0xFF-vr_input1;
   vr_input2 = velocity - (vr_input1*0x100);
   vr_input2 = 0xFF-vr_input2;
 
   vl_input1 = velocity / 0x100;
   vl_input1 = 0xFF-vl_input1;
   vl_input2 = velocity - (vl_input1*0x100);
   vl_input2 = 0xFF-vl_input2;
  }else{//for completeness
  }
   

 byteTx(145);
 byteTx(vr_input1);
 byteTx(vr_input2);
 byteTx(vl_input1);
 byteTx(vl_input2);
}
void rotate_spot (int velocity) {
 int vr_input1, vr_input2;
 int vl_input1, vl_input2;

  if (velocity>=0){
  //rotating CWW--right wheel(+), left wheel(-)
  
   vr_input1 = velocity / 0x100;
   vr_input2 = velocity - (vr_input1*0x100);
  
   vl_input1 = velocity / 0x100;
   vl_input1 = 0xFF-vl_input1;
   vl_input2 = velocity - (vl_input1*0x100);
   vl_input2 = 0xFF-vl_input2;
   
  
  }else if (velocity<0) {
   //rotating CW--right wheel(-), left wheel(+)
   
   velocity *= (-1);
   
   vl_input1 = velocity / 0x100;
   vl_input2 = velocity - (vl_input1*0x100);
   
 
   vr_input1 = velocity / 0x100;
   vr_input1 = 0xFF-vr_input1;
   vr_input2 = velocity - (vr_input1*0x100);
   vr_input2 = 0xFF-vr_input2;

  }else{//for completeness
  }
  
 byteTx(145);
 byteTx(vr_input1);
 byteTx(vr_input2);
 byteTx(vl_input1);
 byteTx(vl_input2);


}
void rotate_spot_90_degrees(int direction) {

   int16_t i;
   
   delayAndUpdateSensors(500);
  

	if(direction==1){
		for (i=0;i<1250;i++) {
			rotate_spot(100);
			}
		rotate_spot(0);
	}else if(direction==-1){
		for (i=0;i<1250;i++) {
			rotate_spot(-100);
			}
		rotate_spot(0);
	
	}else{//for completeness
	}
	
	delayAndUpdateSensors(500);


}
void rotate_spot_180_degrees(void) {
	int16_t i;
   
   delayAndUpdateSensors(500);
  
	for (i=0;i<2450;i++) {
		rotate_spot(100);
	}
	
	rotate_spot(0);

	delayAndUpdateSensors(500);


}
void allign_right(void) {
   int16_t i;
   
   delayAndUpdateSensors(500);
   
    for (i=0;i<150;i++) {
		rotate_spot(100);
		}
    rotate_spot(0);
	delayMs(400);
}




void allign_left(void) {
   int16_t i;
   
   delayAndUpdateSensors(500);
   
    for (i=0;i<150;i++) {
		rotate_spot(-100);
		}
    rotate_spot(0);
	delayMs(400);
}
void set_outside_walls (void) {

int x, y;


for(x=0; x<array_size_x; x++) {
	maze[x][0]=1;
}

for(y=0; y<array_size_y; y++) {
	maze[0][y]=1;
}

for(x=0; x<array_size_x; x++) {
	maze[x][array_size_y-1]=1;
}

for (y=0; y<array_size_y; y++) {
	maze[array_size_x-1][y]=1;
}

}




void moved (int direction) {
//robot moved without hitting the wall, update the position and the array
//2 means the array position is empty

if (direction == positive_y) {
	//no wall present
	y++;
	//maze[x][y]=2;
	//moved into empty space
	y++;
	maze[x][y]=2;
	
}else if (direction == negative_y) {
	//no wall present
	y--;
	//maze[x][y]=2;
	//moved into empty space
	y--;
	maze[x][y]=2;
	
}else if (direction == positive_x) {
	//no wall present
	x++;
	//maze[x][y]=2;
	//moved into empty space
	x++;
	maze[x][y]=2;

}else if (direction == negative_x) {
	//no wall present
	x--;
	//maze[x][y]=2;
	//moved into empty space
	x--;
	
	maze[x][y]=2;

}else{
//for completeness
}//endif

}

void backtrack(int direction) {

if (direction == positive_y) {
	//space is dead end
	maze[x][y]=1;
	y++;
	//wall is dead end
	maze[x][y]=1;
	y++;
	
}else if (direction == negative_y) {
	//space is dead end
	maze[x][y]=1;
	y--;
	//wall is dead end
	maze[x][y]=1;
	y--;
	
}else if (direction == positive_x) {
	//space is dead end
	maze[x][y]=1;
	x++;
	//wall is dead end
	maze[x][y]=1;
	x++;

}else if (direction == negative_x) {
	//space is dead end
	maze[x][y]=1;
	x--;
	//wall is dead end
	maze[x][y]=1;
	x--;

}else{
//for completeness
}//endif

}


void first_move (int direction){
//initial move, only moved past the wall

if (direction == positive_y) {
	//moved into empty space
	y++;
	maze[x][y]=2;
	
}else if (direction == negative_y) {
	//moved into empty space
	y--;
	maze[x][y]=2;
	
}else if (direction == positive_x) {
	//moved into empty space
	x++;
	maze[x][y]=2;

}else if (direction == negative_x) {
	//moved into empty space
	x--;
	maze[x][y]=2;

}else{
//for completeness
}//endif

}

void collide (int direction) {
//robot hit the wall
//1 means the array is blocked off

if (direction == positive_y){
	//position blocked
	maze[x][y+1]=1;
	


}else if (direction == negative_y) {
	//position blocked
	maze[x][y-1]=1;
	
}else if (direction == positive_x) {
	//postive blocked
	maze[x+1][y]=1;
	
}else if (direction == negative_x) {
	//position blocked
	maze[x-1][y]=1;

}else{
//for completeness
}//endif


}
void collision_back_up (void){

int distance;

distance= ((square_width/2) - (wall_width/2 + diameter_bot/2));

int input1, input2;

	input1 = distance / 0x100;
	input1 = 0xFF-input1;
	input2 = distance - (input1*0x100);
	input2 = 0xFF-input2;


drive_straight(-75);
	
  //travel proper distance to get back into the center of the square
  byteTx(156);
  byteTx(input1);
  byteTx(input2);

 
  drive_straight(0);
  delayMs(500);
  
 


}
int empty_right(int direction) {
//returns 2 if right is open
//returns 1 if right is closed
//returns 0 if right has not been checked

if (direction == positive_y) {

	return maze[x+1][y];
	
}else if (direction == negative_y ) {
	return maze[x-1][y];

}else if (direction == positive_x) {
	return maze[x][y-1];

}else if (direction == negative_x) {
	return maze[x][y+1];

}else{
	//for completeness
	return 5;
}//end if	
	

}

int empty_forward(int direction){


//returns 2 if right is open
//returns 1 if right is closed
//returns 0 if right has not been checked



if (direction == positive_y) {
	return maze[x][y+1];

}else if (direction == negative_y ) {
	return maze[x][y-1];

}else if (direction == positive_x) {
	return maze[x+1][y];

}else if (direction == negative_x) {
	return maze[x-1][y];

}else{
	//for completeness
	return 5;
}//end if	



}

int empty_left(int direction){


//returns 2 if right is open
//returns 1 if right is closed
//returns 0 if right has not been checked

if (direction == positive_y) {
	return maze[x-1][y];

	
}else if (direction == negative_y ) {
	return maze[x+1][y];

	
}else if (direction == positive_x) {
	return maze[x][y+1];


}else if (direction == negative_x) {
	return maze[x][y-1];


}else{
	//for completeness
	return 5;
}//end if	


}



int fix_direction( int direction){

if (direction==0){
	direction=4;

}else if (direction==5){
	direction=1;
	
}else if((direction<5)||(direction>0)){
//do nothing		
}else{

}
return direction;

}


void back_track(int direction) {

if (direction == positive_y && maze[x][y+2]==2 ){
	//block off dead ends
	maze[x][y+1]=1;

	
}else if (direction == negative_y && maze[x][y-2]==2 ){
	//block off dead ends
	maze[x][y-1]=1;
	
}else if (direction == positive_x && maze[x+2][y]==2 ){
	//block off dead ends
	maze[x+1][y]=1;

}else if (direction == negative_x && maze[x-2][y]==2 ){
	//block off dead ends
	maze[x-1][y]=1;

}else{
//dont change value, stay a 2
}//endif







}


void error(int error_number) {

	while(error_number>=1) {
		byteTx(CmdPlay);
		byteTx(SHORT_ERROR);
		delayAndUpdateSensors(700);
		error_number--;
		
		if (error_number>=1) {
			byteTx(CmdPlay);
			byteTx(LONG_ERROR);
			delayAndUpdateSensors(1400);
			error_number--;
		}
	}	
}





