#include <iostream>
#include "Bokobolin.h"
#include "Enemy.h"

//ClassName::FunctionName : Parent Constructor (This is constructor which is fx)
// ":" means passing the value to the parent class constructor
Bokobolin::Bokobolin() : Enemy({.name = "Bokobolin", .health =80, .maxHealth = 80, .attackPower =10, .isAlive=true,.movspeed = 2}){
}

void Bokobolin::TakeAction(){
   std::cout << "Bokobolin attack you";

}

