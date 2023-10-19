/*******************************************************************************
 * @file    digipot.cpp
 * @brief   Configures the values of the digital potentiometers
 *
 * @author 	Rohan Jha
 * @date 	  July 20, 2023
 ******************************************************************************/

#include "digipot.h"

pot potvar[NUM_POTS];

//Configuring digital potentiometers
 void initpots(){
  potvar[0].clk = 4;
  potvar[0].ud = 5;
  potvar[0].cs = 6;

  potvar[1].clk = 27;
  potvar[1].ud = 25;
  potvar[1].cs = 23;

  potvar[2].clk = 29;
  potvar[2].ud = 31;
  potvar[2].cs = 33;
  
  pinMode(potvar[0].clk, OUTPUT);
  pinMode(potvar[1].clk, OUTPUT);
  pinMode(potvar[2].clk, OUTPUT);

  pinMode(potvar[0].ud, OUTPUT);
  pinMode(potvar[1].ud, OUTPUT);
  pinMode(potvar[2].ud, OUTPUT);

  pinMode(potvar[0].cs, OUTPUT);
  pinMode(potvar[1].cs, OUTPUT);
  pinMode(potvar[2].cs, OUTPUT);

  digitalWrite(potvar[0].cs, HIGH);
  digitalWrite(potvar[1].cs, HIGH);
  digitalWrite(potvar[2].cs, HIGH);

  digitalWrite(potvar[0].clk, HIGH);
  digitalWrite(potvar[1].clk, HIGH);
  digitalWrite(potvar[2].clk, HIGH);
}
/*
Function : Set digipot value
Args:      Potentiometer number
Working : . Thet hree inputs are clock (CLK), CS and UP/DOWN (U/D).The negative-edge sensitive CLK input 
requires clean transitions to avoid clocking multiple pulses into the internal UP/DOWNcounter register.
When CS is taken active low the clock begins to incre-ment or decrement the internal UP/DOWN counter dependent 
upon the state of the U/D control pin. The UP/DOWN countervalue (D) starts at 40H at system power ON. 
Each new CLKpulse will increment the value of the internal counter by one LSB until the full scale value of 3FH is
 reached as long as theU/D pin is logic high. If the U/D pin is taken to logic low thecounter will count down stopping at code 00H (zero-scale).
*/
void DownPot(int Pot_Num){
  digitalWrite(potvar[Pot_Num].cs, LOW);
  digitalWrite(potvar[Pot_Num].ud, LOW);
  // digitalWrite(num.clk, HIGH);
  for(int i = 0; i < 128; i++){
    digitalWrite(potvar[Pot_Num].clk, LOW);
    delay(1);
    digitalWrite(potvar[Pot_Num].clk, HIGH);
    delay(1);
  }
  digitalWrite(potvar[Pot_Num].cs, HIGH);
}

void UpPot(int Pot_Num){
  digitalWrite(potvar[Pot_Num].cs, LOW);
  digitalWrite(potvar[Pot_Num].ud, HIGH);
  // digitalWrite(num.clk, HIGH);
  for(int i = 0; i < 128; i++){
    digitalWrite(potvar[Pot_Num].clk, LOW);
    delay(1);
    digitalWrite(potvar[Pot_Num].clk, HIGH);
    delay(1);
  }
  digitalWrite(potvar[Pot_Num].cs, HIGH);
}

//Level could be anywhere between 0-128 based on the value of resistance required upto 10k
void SetPotLevel(int Pot_Num,int level){
  DownPot(Pot_Num);
  digitalWrite(potvar[Pot_Num].cs, LOW);
  digitalWrite(potvar[Pot_Num].ud, HIGH);
  // digitalWrite(num.clk, HIGH);
  for(int i = 0; i < level; i++){
    digitalWrite(potvar[Pot_Num].clk, LOW);
    delay(1);
    digitalWrite(potvar[Pot_Num].clk, HIGH);
    delay(1);
  }
  digitalWrite(potvar[Pot_Num].cs, HIGH);
}
