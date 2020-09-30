#ifndef PART_2
#define PART_2

#include <semaphore.h>

//Data structures i used

typedef struct {

  int src;
  int dest;
  int id;

} Passenger;


typedef struct {

  int cap;
  int on_board;
  int src;
  int dest;
  Passenger* passengers;
  sem_t train_queue;

} Train;

typedef struct {

  int waiting;
  sem_t station_queue;

} Station;


/**
 * Do any initial setup work in this function.
 * numStations: Total number of stations. Will be >= 5
 * maxNumPeople: The maximum number of people in a train
 */
void initializeP2(int numTrains, int numStations);
/**
 * Print data in the format described in part 5
 */
void * goingFromToP2(void * user_data);

//helper functions

void enterStation(Station* station, Passenger* p);
void exitStation(Station* station, Passenger* p);
void boardTrain(Train* train, Passenger* p);
void departTrain(Train* train, Passenger* p);

int getTrainSrc(int index);
Train* getTrainDest(int index);




void * startP2();
#endif
