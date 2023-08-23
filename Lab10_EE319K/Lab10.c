// Lab10.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// previousSW2 Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay10ms(uint32_t count); // time delay in 0.1 seconds
uint32_t Convert(uint32_t input);
void SysTick_Init(unsigned long period);
void SysTick_Handler(void);
void Button_Init(void);
void moveShip(void);
void moveSprite(void);
void DrawSprite(void);
void fire(void);
void MoveFire(void);
void DrawFire(void);
void waveDone(void);
void checkcollision(void);
void pause(void);
//void shootsound(void);
void PlayShootSound (void);

//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins
int i =0;
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x22;      // activate port B,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF,3,2,1

	GPIO_PORTF_DIR_R &= ~0x11;	//PF4,0
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;		//unlock
	GPIO_PORTF_CR_R = 0x11;			//Turn PF4 on
	GPIO_PORTF_PUR_R = 0x11;			//pull-up register
	//nowSW2 negative logic switch
	GPIO_PORTF_DEN_R |= 0x11;			//enable PF0,4
		
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5  
}
//********************************************************************************
 void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void SysTick_Init(unsigned long period){
  //*** students write this ******
	NVIC_ST_CTRL_R = 0x0;
	NVIC_ST_RELOAD_R = period;  				// reload value (sound off when RELOAD = 0)
  NVIC_ST_CURRENT_R = 0x0;  	   // any write will reload counter and clear count
  //NVIC_SYS_PRI3_R =  (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; //set priority level
  NVIC_ST_CTRL_R = 0x07;			//enables SysTick Interrupt bit
}
struct Sprite
{
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t life;
	uint32_t score;
	const uint8_t *images;
	const uint8_t *blank;// something like this blank alien all black
	int32_t vx;
	int32_t vy;
	int32_t oldX;
	int32_t oldY;
	uint16_t Colour;
};
typedef struct Sprite Sprite_t;
Sprite_t aliens[5] = {
	{0,9,12,8,1,300, Alien30pointA,BlankAlien,2,1,0,0,SSD1306_WHITE},
	{20,9,12,8,1,200,Alien20pointB,BlankAlien,2,1,0,0,SSD1306_WHITE},
	{40,9,12,8,1,200,Alien20pointA,BlankAlien,2,1,0,0,SSD1306_WHITE},
	{60,9,12,8,1,100,Alien10pointB,BlankAlien,2,1,0,0,SSD1306_WHITE},
	{80,9,12,8,1,100,Alien10pointA,BlankAlien,2,1,0,0,SSD1306_WHITE}
};
struct bullet{
	int32_t x;
	int32_t y;
	int32_t vy;
	uint32_t life;
	int32_t oldY;
};
typedef struct bullet bullet_t;
bullet_t bullets[10];

struct ship{
	int32_t x;
	int32_t y;
	uint32_t life;
	int32_t oldX;
};
typedef struct ship ship_t;
ship_t battleship[1];

uint32_t data=0;
int endGame = 1;
int allDead = 1;
int wave = 0;
int misCount = 0;
int score = 0;
const uint8_t *img[6] = {Alien30pointA,Alien20pointB,Alien20pointA,Alien10pointB,Alien10pointA,AlienBossA};
int pauseGame = 0;
int flag=0;
void Intro (void){
	
	SSD1306_OutClear();
	  SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  SSD1306_OutBuffer();
	Delay100ms(20);
	SSD1306_OutClear();
	SSD1306_SetCursor(1,2);
	SSD1306_OutString("English or Spanish.Press SW2 for English");
	SSD1306_SetCursor(1,4);
	SSD1306_OutString("SW1 for Spanish");
	//Delay100ms(50)
	while((GPIO_PORTF_DATA_R&0x10)>0){
	if((GPIO_PORTF_DATA_R&0x01)== 0)
	{	flag = 1;
	break;}

	}
	if(flag == 1)
	{
		SSD1306_OutClear();
	SSD1306_SetCursor(1,2);
	SSD1306_OutString("SPACE INVADERS");
	SSD1306_SetCursor(1,3);
	SSD1306_OutString("Press Button To");
SSD1306_SetCursor(1,4);
	SSD1306_OutString("Begin. Press SW1");
	while((GPIO_PORTF_DATA_R&0x10)>0 ){};
	}
	else if (flag == 0)
	{
		SSD1306_OutClear();
		SSD1306_SetCursor(1,2);
	SSD1306_OutString("Invasores espaciales");
	SSD1306_SetCursor(1,3);
	SSD1306_OutString("Presione el boton");
SSD1306_SetCursor(1,4);
		SSD1306_OutString("para Empezar.");
		SSD1306_SetCursor(1,5);
		SSD1306_OutString("Presione SW2");
	while((GPIO_PORTF_DATA_R&0x01)>0 ){};
}

}

int main(void){uint32_t time=0;
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
	ADC_Init();
	Timer0_Init(&Sound_Init,7256);
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
  SSD1306_ClearBuffer();
	
		Intro();
SSD1306_OutClear();
SSD1306_ClearBuffer();
  EnableInterrupts();
  
 
	

/*	
 SSD1306_DrawBMP(47, 63, PlayerShip0, 0, SSD1306_WHITE); // player ship bottom
 //SSD1306_DrawBMP(53, 55, Bunker0, 0, SSD1306_WHITE);

  SSD1306_DrawBMP(0, 9, Alien10pointA, 0, SSD1306_BLACK);
  SSD1306_DrawBMP(20,9, Alien10pointB, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(40, 9, Alien20pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(60, 9, Alien20pointB, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(80, 9, Alien30pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(50, 19, AlienBossA, 0, SSD1306_WHITE);
  SSD1306_OutBuffer();
	*/
  Delay100ms(30);
	SysTick_Init(2666666);
	EnableInterrupts();
  while(endGame)
	{
		allDead = 1;
		//EnableInterrupts();
		while(allDead && endGame)
		{
			while(pauseGame){}
			moveShip();
			checkcollision();
			waveDone();
		}
		//DisableInterrupts();
		if(endGame)
		{
			wave++;
			if(wave % 2 == 0)
			{
				for(int i = 0; i < 5; i++)
				{
					aliens[i].x = 80 - (20*i);
					aliens[i].y = 14;
					aliens[i].width = 16;
					aliens[i].height = 10;
					aliens[i].life = 1;
					aliens[i].score = 200;
					aliens[i].images = img[i];
					aliens[i].vx = 2;
					aliens[i].vy = 1;
					aliens[i].oldX = 0;
					aliens[i].oldY = 0;
				}
			}
			else{
				for(int i = 0; i < 5; i++)
				{
					aliens[i].x = 0 + (20*i);
					aliens[i].y = 14;
					aliens[i].width = 16;
					aliens[i].height = 10;
					aliens[i].life = 1;
					aliens[i].score = 200;
					aliens[i].images = img[i];
					aliens[i].vx = -2;
					aliens[i].vy = 1;
					aliens[i].oldX = 0;
					aliens[i].oldY = 0;
				}
			}
		}
	}
	DisableInterrupts();
	if(flag == 1)
	{
  SSD1306_OutClear();  
  SSD1306_SetCursor(1, 1);
  SSD1306_OutString("GAME OVER");
  SSD1306_SetCursor(1, 2);
  SSD1306_OutString("Nice try,");
  SSD1306_SetCursor(1, 3);
  SSD1306_OutString("Earthling!");
  SSD1306_SetCursor(2, 5);
		SSD1306_OutString("Score: ");
		LCD_OutDec(score);
	}
	else{
		SSD1306_OutClear();  
  SSD1306_SetCursor(1, 1);
  SSD1306_OutString("Juego terminado");
  SSD1306_SetCursor(1, 2);
  SSD1306_OutString("Buen intento,");
  SSD1306_SetCursor(1, 3);
  SSD1306_OutString("Terricola");
  SSD1306_SetCursor(2, 5);
		SSD1306_OutString("Resultado: ");
		LCD_OutDec(score);
	}
  while(1){
    Delay100ms(10);
    SSD1306_SetCursor(19,0);
    SSD1306_OutUDec2(time);
    time++;
    PF1 ^= 0x02;
  }
}
void waveDone(void)
{
	allDead = 0;
	for(int i = 0; i < 5; i++)
	{
		allDead += aliens[i].life;
	}
}
void checkcollision(void){

	for(int i=0;i<10;i++)
	{
		for(int a=0;a<5;a++)
		{
			if(aliens[a].life && bullets[i].life)
			{
				if((aliens[a].x < bullets[i].x) && ((aliens[a].x+aliens[a].width) > bullets[i].x) && (aliens[a].y < bullets[i].y) && ((aliens[a].y+aliens[a].height) > bullets[i].y))
				{
					aliens[a].life=0;
					bullets[i].life=0;	
					score += aliens[a].score;
					//PlayShootSound();
				}
			}
		}
	}
}
uint16_t lastShip = 0;
void moveShip(void)
{	
	//lastShip = data;
	data = ADC_In()*112;
	data= data/4096;
	battleship[1].x = data;
	if(data > 110)
	{
		data = 110;
	}
	  SSD1306_ClearBuffer();
		//SSD1306_DrawBMP(lastShip,63,PlayerShip0,7,SSD1306_BLACK);
	SSD1306_DrawBMP(data,58,PlayerShip0,0,SSD1306_WHITE);
	

	 //SSD1306_OutBuffer();
}
void moveSprite(void)
{
	for(int i = 0; i < 5; i++)
	{
		if(aliens[i].life)
		{
			aliens[i].oldX = aliens[i].x;
			aliens[i].oldY = aliens[i].y;
			/*if((aliens[i].x + aliens[i].vx) > 127 || (aliens[i].x + aliens[i].vx) < 0)	//if hit edge
			{
				aliens[i].vx *= -1;
			}*/
			//aliens[i].x += aliens[i].vx;
			aliens[i].y += aliens[i].vy;
			if(aliens[i].y > 63)
			{
				endGame = 0;
			}
		}	
	}
}
void DrawSprite(void)
{
	for(int i = 0; i < 5; i++)
	{
		if(aliens[i].life)
		{
			//SSD1306_ClearBuffer();
		//	SSD1306_DrawBMP(aliens[i].oldX,aliens[i].oldY,aliens[i].blank,0,SSD1306_BLACK);	//EraseEnemy old one flashing
			SSD1306_DrawBMP(aliens[i].x,aliens[i].y,aliens[i].images,0,SSD1306_WHITE);	//print new pos
			
		}	
		else
		{
			SSD1306_DrawBMP(aliens[i].x,aliens[i].y,aliens[i].blank,0,SSD1306_WHITE);	
			//print new pos
		}
	}
}
void fire(void)
{
	bullets[misCount].life = 1;
	bullets[misCount].x = data ;
	bullets[misCount].y = 114;
	bullets[misCount].vy = -2;
	misCount = (misCount+1)%10;
}
void MoveFire(void)
{
	for(int i = 0; i < 10; i++)
	{
		if(bullets[i].life)
		{
			bullets[i].oldY = bullets[i].y;
			bullets[i].y += bullets[i].vy;
		}
	}
}
void DrawFire(void)
{
	for(int i = 0; i < 10; i++)
	{
		if(bullets[i].life)
		{
			SSD1306_DrawPixel(bullets[i].x,bullets[i].oldY,SSD1306_BLACK);
			SSD1306_DrawPixel(bullets[i].x,bullets[i].y,SSD1306_WHITE);
			//SSD1306_OutBuffer();
		}
		else
		{
			SSD1306_DrawPixel(bullets[i].x,bullets[i].y,SSD1306_BLACK);
			//SSD1306_OutBuffer();
		}
	}
}
uint32_t nowSW2 = 0;
uint32_t previousSW2 = 0;
uint32_t nowSW1 = 0;
uint32_t previousSW1 = 0;
void SwitchInput(void)
{
	nowSW2 = GPIO_PORTF_DATA_R & 0x1;
	if(nowSW2 == 0 && previousSW2 == 1 )
	{
		fire();
		shootsound(); 
	}
	previousSW2 = nowSW2;
	
	nowSW1 = GPIO_PORTF_DATA_R & 0x10;
	if(nowSW1 == 0 && previousSW1 > 0)
	{
		pause();
	}
	previousSW1 = nowSW1;
}
void pause(void)
{
	if(pauseGame == 1)
	{
		pauseGame = 0;
	}
	else
	{
		pauseGame = 1;
	}		//toggles pause
}
int sysCount = 0;
void SysTick_Handler(void)
{
	SwitchInput();
	if(pauseGame == 0)
	{
		if(sysCount == 6)
		{
			moveSprite();
		}
		DrawSprite();
		if(sysCount % 2 == 1)
		{
			MoveFire();
		}
		DrawFire();
		sysCount = (sysCount+1)%7;
	}
	SSD1306_OutBuffer();
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}

