/*******************************************************************************
 * @file    digipot.h
 * @brief   Contains funtions and structures for digital potentiometers
 *
 * @author 	Rohan Jha
 * @date 	  July 20, 2023
 ******************************************************************************/
#include <Arduino.h>
#define NUM_POTS    3

struct pot{
  int clk;
  int ud;
  int cs;
};

void initpots();

void DownPot(int Pot_Num);

void UpPot(int Pot_Num);

void SetPotLevel(int Pot_Num,int level);