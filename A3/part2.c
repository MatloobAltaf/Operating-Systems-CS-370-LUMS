#include <stdio.h>
#include <stdlib.h>
#include "part2.h"
#include "main.h"

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */

const int INTER_ARRIVAL_TIME = 5;
const int NUM_TRAINS = 5;

Train*   trains   = NULL;
Station* stations = NULL;

int total_stations = 0;
int boarding_train = 0;
int total_passengers = 0;

sem_t mutex;
sem_t boarding;
sem_t departing;

/**
 * Do any initial setup work in this function.
 * numStations: Total number of stations. Will be >= 5. Assume that initially
 * the first train is at station 1, the second at 2 and so on.
 * maxNumPeople: The maximum number of people in a train
 */
void initializeP2(int numStations, int maxNumPeople) {

  if(numStations <= NUM_TRAINS)
  {
    return;
  }

  sem_init(&mutex,0,1);

  total_stations = numStations;

  int i = 0;

  stations = (Station*) malloc(sizeof(Station) * total_stations);
  trains   = (Train*) malloc(sizeof(Train) * NUM_TRAINS);

  for(i; i<NUM_TRAINS; i++)
  {
    trains[i].cap         = maxNumPeople;
    sem_init(&trains[i].train_queue,0,maxNumPeople);
    trains[i].passengers  = (Passenger*) malloc(sizeof(Passenger) * maxNumPeople);
    trains[i].on_board    = 0;
    trains[i].src         = i;
    trains[i].dest        = (i + 1) % NUM_TRAINS;
  }

  i = 0;

  for(i; i<total_stations; i++)
  {
    stations[i].waiting = 0;
    sem_init(&stations[i].station_queue,0,0);
  }
}

/**
 * Print in the following format:
 * If a user borads on train 0, from station 0 to station 1, and another boards
 * train 2 from station 2 to station 4, then the output will be
 * 0 0 1
 * 2 2 4
 */
 void * goingFromToP2(void * user_data) {

   sem_wait(&mutex);
   total_passengers += 1;
   sem_post(&mutex);

   struct argument* parsed_data = (struct argument*) user_data;

   Passenger p;

   p.src  = parsed_data->from;
   p.dest = parsed_data->to;
   p.id   = parsed_data->id;


   enterStation(&stations[p.src],&p);
   sem_wait(&stations[p.src].station_queue);
   boardTrain(&trains[boarding_train],&p);
   exitStation(&stations[p.src],&p);
   printf("%d %d %d\n",p.id,p.src,p.dest);
   sem_wait(&trains[p.id].train_queue);
   departTrain(&trains[p.id],&p);
 }

 void enterStation(Station* station, Passenger* p)
 {
   sem_wait(&mutex);
   station->waiting += 1;
   sem_post(&mutex);
 }

 void exitStation(Station* station, Passenger* p)
 {
   sem_wait(&mutex);
   station->waiting -= 1;
   sem_post(&mutex);
 }

 void boardTrain(Train* train, Passenger* p)
 {
   sem_wait(&mutex);
   train->passengers[train->on_board] = *p;
   train->on_board += 1;
   sem_post(&mutex);
   sem_post(&boarding);
 }

 void departTrain(Train* train, Passenger* p)
 {
   sem_wait(&mutex);
   for(int i = 0; i<train->on_board; i++)
   {
     if(train->passengers[i].src == p->src && train->passengers[i].dest == p->dest)
     {
       train->on_board -= 1;
       for(int j = i; j<train->on_board; j++)
       {
         train->passengers[j] = train->passengers[j + 1];
       }
       break;
     }
   }
   total_passengers -= 1;
   sem_post(&mutex);
   sem_post(&departing);
 }

 int getTrainSrc(int index)
 {
   sem_wait(&mutex);
   for(int i = 0; i<NUM_TRAINS; i++)
   {
     if(trains[i].src == index)
     {
       sem_post(&mutex);
       return index;
     }
   }
   sem_post(&mutex);
   return -1;
 }

 Train* getTrainDest(int index)
 {
   sem_wait(&mutex);
   for(int i = 0; i<NUM_TRAINS; i++)
   {
     if(trains[i].dest == index)
     {
       return &trains[i];
     }
   }
   sem_post(&mutex);
 }

 void * startP2(){

   int done = 0;

   while(1)
   {

     if(total_passengers == 0)
     {
       break;
     }

     //depart from train if arrived
     for(int i = 0; i<NUM_TRAINS; i++)
     {
       if(trains[i].on_board == 0)
       {
         continue;
       }

       for(int j = 0; j<trains[i].on_board; j++)
       {
         if(trains[i].passengers[j].dest == trains[i].src)
         {
           sem_post(&trains[i].train_queue);
           sem_wait(&departing);
         }
       }
     }

     //board train
     for(int i = 0; i<total_stations; i++)
     {

       //if station has no passengers do nothing
       sem_wait(&mutex);
       int waiting_passengers = stations[i].waiting;
       sem_post(&mutex);
       if(waiting_passengers == 0)
       {
         continue;
       }

       //otherwise get the train on station i
       int index = getTrainSrc(i);

       //if no train is coming to that station then go ahead
       if(index == -1)
       {
         continue;
       }

       //if train full do nothing
       if(trains[index].on_board == trains[index].cap)
       {
         continue;
       }

       //otherwise fill the train
       int capacity = trains[index].cap - trains[index].on_board;
       int min = capacity < waiting_passengers ? capacity : waiting_passengers;
       sem_wait(&mutex);
       boarding_train = index;
       sem_post(&mutex);
       for(int j = 0; j<min; j++)
       {
         sem_post(&stations[i].station_queue);
         sem_wait(&boarding);
       }

     }

     //arrival time of trains

     for(int i = 0; i<INTER_ARRIVAL_TIME; i++)
     {

     }

     //trains arrived at destinations
     sem_wait(&mutex);
     for(int i = 0; i<NUM_TRAINS; i++)
     {
       trains[i].src  = (trains[i].src + 1) % NUM_TRAINS;
       trains[i].dest = (trains[i].dest + 1) % NUM_TRAINS;
     }
     sem_post(&mutex);

   }

 }
