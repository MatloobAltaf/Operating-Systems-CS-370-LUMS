#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "part3.h"
#include "main.h"


/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */

const int TOTAL_ROAD_LANES = 16;

int total_cars = 0;

RoadLane* roadlanes = NULL;

sem_t north;
sem_t south;
sem_t east;
sem_t west;
sem_t mutex;
sem_t mutex_n;
sem_t mutex_s;
sem_t mutex_w;
sem_t mutex_e;

int n = 0;
int s = 0;
int e = 0;
int w = 0;

int nw = 0;
int sw = 0;
int ew = 0;
int ww = 0;

int n_straight = 0;



void initializeP3() {

  roadlanes = (RoadLane*) malloc(sizeof(RoadLane)*TOTAL_ROAD_LANES);

  sem_init(&north,0,0);
  sem_init(&south,0,0);
  sem_init(&east,0,0);
  sem_init(&west,0,0);
  sem_init(&mutex,0,1);
  sem_init(&mutex_n,0,0);
  sem_init(&mutex_e,0,0);
  sem_init(&mutex_w,0,0);
  sem_init(&mutex_s,0,0);
}

/**
 * If there is a car going from SOUTH to NORTH, from lane LEFT,
 * print
 * SOUTH NORTH LEFT
 * Also, if two cars can simulateneously travel in the two lanes,
 * first print all the cars in the LEFT lane, followed by all the
 * cars in the right lane
 */
 void * goingFromToP3(void *argu){

 sem_wait(&mutex);
 total_cars += 1;
 sem_post(&mutex);

 //printf("Car came in.\n");

 struct argumentP3* parsed_data = (struct argumentP3*) argu;

 Car c;

 c.from = parsed_data->from;
 c.to   = parsed_data->to;
 c.lane = parsed_data->lane;

 char * res = NULL;

 if(c.from == SOUTH && c.to == NORTH && c.lane == LEFT)
 {
   res = "SOUTH NORTH LEFT\n";
   sem_wait(&mutex);
   n += 1;
   n_straight += 1;
   sem_post(&mutex);
   sem_wait(&north);
   sem_wait(&mutex);
   n -= 1;
   n_straight -= 1;
   if(n_straight == 0 && nw > 0)
   {
     sem_post(&mutex_n);
   }
   sem_post(&mutex);

 }
 else if(c.from == SOUTH && c.to == EAST && c.lane == RIGHT)
 {
   res = "SOUTH EAST RIGHT\n";
   sem_wait(&mutex);
   n += 1;
   sem_post(&mutex);
   sem_wait(&north);
   sem_wait(&mutex);
   n -= 1;
   sem_post(&mutex);
 }
 else if(c.from == SOUTH && c.to == WEST && c.lane == LEFT)
 {

   res = "SOUTH WEST LEFT\n";
   //we need to look if someone is ahead of him or not who will go straight
   if(n_straight > 0)
   {
     sem_wait(&mutex);
     nw += 1;
     sem_post(&mutex);
     sem_wait(&mutex_n);
     sem_wait(&mutex);
     nw -= 1;
     sem_post(&mutex);
   }

 }
 else if(c.from == NORTH && c.to == SOUTH && c.lane == LEFT)
 {
   res = "NORTH SOUTH LEFT\n";
   sem_wait(&mutex);
   s += 1;
   sem_post(&mutex);
   sem_wait(&south);
   sem_wait(&mutex);
   s -= 1;
   sem_post(&mutex);
 }
 else if(c.from == NORTH && c.to == EAST && c.lane == LEFT)
 {
   res = "NORTH EAST LEFT\n";
   //we need to look if someone is ahead of him or not who will go straight

 }
 else if(c.from == NORTH && c.to == WEST && c.lane == RIGHT)
 {
   res = "NORTH WEST RIGHT\n";
   sem_wait(&mutex);
   s += 1;
   sem_post(&mutex);
   sem_wait(&south);
   sem_wait(&mutex);
   s -= 1;
   sem_post(&mutex);
 }
 else if(c.from == EAST && c.to == WEST && c.lane == LEFT)
 {
   res = "EAST WEST LEFT\n";
   sem_wait(&mutex);
   w += 1;
   sem_post(&mutex);
   sem_wait(&west);
   sem_wait(&mutex);
   w -= 1;
   sem_post(&mutex);
 }
 else if(c.from == EAST && c.to == NORTH && c.lane == RIGHT)
 {
   res = "EAST NORTH RIGHT\n";
   sem_wait(&mutex);
   w += 1;
   sem_post(&mutex);
   sem_wait(&west);
   sem_wait(&mutex);
   w -= 1;
   sem_post(&mutex);
 }
 else if(c.from == EAST && c.to == SOUTH && c.lane == LEFT)
 {
   res = "WEST EAST LEFT\n";
   //we need to look if someone is ahead of him or not who will go straight
 }
 else if(c.from == WEST && c.to == EAST && c.lane == LEFT)
 {
   res = "WEST EAST LEFT\n";
   sem_wait(&mutex);
   e += 1;
   sem_post(&mutex);
   sem_wait(&east);
   sem_wait(&mutex);
   e -= 1;
   sem_post(&mutex);
 }
 else if(c.from == WEST && c.to == NORTH && c.lane == LEFT)
 {
   res = "WEST NORTH LEFT\n";
   // we need to look if someone is ahead of him ro not who will go straight
 }
 else if(c.from == WEST && c.to == SOUTH && c.lane == RIGHT)
 {
   res = "WEST SOUTH RIGHT\n";
   sem_wait(&mutex);
   e += 1;
   sem_post(&mutex);
   sem_wait(&east);
   sem_wait(&mutex);
   e -= 1;
   sem_post(&mutex);
 }

 sem_wait(&mutex);
 total_cars -= 1;
 sem_post(&mutex);

 printf(res);

 }

 void startP3(){

   //sleep(1);

   while(1)
   {
     sem_wait(&mutex);
     int val = total_cars;
     sem_post(&mutex);
     if(val == 0)
     {
       break;
     }

     if(n > 0)
     {
       sem_post(&north);
     }

     if(e > 0)
     {
       sem_post(&east);
     }

     if(w > 0)
     {
       sem_post(&west);
     }

     if(s > 0)
     {
       sem_post(&south);
     }

   }

 }
