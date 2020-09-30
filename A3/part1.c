#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "part1.h"
#include "main.h"

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */

const int MAX_NUM_FLOORS = 20;
int totalFloors, maxPeople,currentFloor,direction;
int * request;
pthread_cond_t * conditions;
pthread_cond_t capacityFull;
int currentReq;
pthread_mutex_t * peopleMutex;
pthread_mutex_t capacityLock;
pthread_mutex_t reqLock;
pthread_mutex_t waitLock;
int currentPeople;

int maxFloor,minFloor;
pthread_mutex_t maxFLock;
pthread_mutex_t minFLock;

void* moveElevator(){
	sleep(1);
	while(1){
		while(maxFloor>=0 || minFloor<totalFloors){
			sleep(1);
			if(request[currentFloor]==1){
				//printf("Broadcast for %d\n",currentFloor );
				if(pthread_cond_broadcast(&conditions[currentFloor])!=0){
					printf("\n Lift Broadcast Failed at %d \n",currentFloor);
					exit(1);
				}
				pthread_mutex_lock(&reqLock);
				request[currentFloor]=0;
				pthread_mutex_unlock(&reqLock);
			}
			
			if(maxFloor==currentFloor || currentFloor==totalFloors-1){
				direction=-1;
				pthread_mutex_lock(&maxFLock);
				maxFloor=-1;
				pthread_mutex_unlock(&maxFLock);
			}
			else if(minFloor==currentFloor || currentFloor==0){
				direction=1;
				pthread_mutex_lock(&minFLock);
				minFloor=totalFloors;
				pthread_mutex_unlock(&minFLock);
			}
			currentFloor+=direction;
		}
	}	
}


/**
 * Do any initial setup work in this function.
 * numFloors: Total number of floors elevator can go to | will be smaller or equal to MAX_NUM_FLOORS
 * maxNumPeople: The maximum capacity of the elevator
 * 
 * Two elevators should start, one from the ground floor (elevator 1) and
 * the other one from the top floor (elevator 2).
 *
 * Note that: Elevator 1 starts from ground floor and goes up
 * Elevator 2 starts from Top floor and goes down.
 */
void initializeP1(int numFloors, int maxNumPeople) {
	//printf("Starting Lift Number of Floors = %d and Max People = %d \n",numFloors,maxNumPeople);
	totalFloors=numFloors;
	currentReq=0;
	request = (int *)(malloc(sizeof(int)*totalFloors));
	maxFloor=-1;
	minFloor=totalFloors;
	maxPeople=maxNumPeople;
	currentPeople=0;
	currentFloor=0; //Lift starts at ground floor
	direction=1;	//initial direction will be up because starts at ground floor
	conditions = (pthread_cond_t*)(malloc(sizeof(pthread_cond_t)*totalFloors));
	peopleMutex = (pthread_mutex_t*)(malloc(sizeof(pthread_mutex_t)*totalFloors));
	if(pthread_mutex_init(&capacityLock,NULL)!=0 ){
		printf("\n capacity Mutex Init Failed\n");
		return;
	}
	if(pthread_mutex_init(&waitLock,NULL)!=0 ){
		printf("\n Wait Mutex Init Failed\n");
		return;
	}
	if(pthread_mutex_init(&maxFLock,NULL)!=0 ){
		printf("\n Max Floor Mutex Init Failed\n");
		return;
	}
	if(pthread_mutex_init(&minFLock,NULL)!=0 ){
		printf("\n Min Floor Mutex Init Failed\n");
		return;
	}
	if(pthread_mutex_init(&reqLock,NULL)!=0 ){
		printf("\n Request Mutex Init Failed\n");
		return;
	}
	if(pthread_cond_init(&capacityFull,NULL)!=0){
		printf("\n Cond (capacityFull) Init Failed\n");
		return;
	}
	int i;
	for(i =0;i<totalFloors;i++){
		request[i]=0;		
		if(pthread_cond_init(&conditions[i],NULL)!=0){
			printf("\n Cond Init Failed of %d \n",i);
			return;
		}
		if(pthread_mutex_init(&peopleMutex[i],NULL)!=0){
			printf("\n Mutex Init Failed of %d \n",i);
			return;
		}
	}
	return;
}

/**
 * Every passenger will call this function when 
 * he/she wants to take the elevator. (Already
 * called in main.c)
 * 
 * This function should print info "id from to" without quotes,
 * where:
 * 	id = id of the user
 * 	from = source floor (from where the passenger is taking the elevator)
 * 	to = destination floor (floor where the passenger is going)
 * 
 * info of a user x getting off the elevator before the user xx
 * should be printed before.
 * 
 * Suppose a user 1 from floor 1 wants to go to floor 4 and
 * a user 2 from floor 2 wants to go to floor 3 then the final print statements
 * will be 
 * 2 2 3
 * 1 1 4
 * 
 * Also, all print statements (info) of passengers in elevator 1 should be printed before
 * the print statements (info) of passengers in elevator 2.
 */
void* goingFromToP1(void *arg) {
	  struct argument *temp = (struct argument *) arg;
   		
    if(temp->to>totalFloors || temp->to<0 || temp->from <0 || temp->from >totalFloors){
    	printf("Wrong Floor Requested\n");
    	exit(1);
    }
    if(currentPeople==maxPeople){
    	if(pthread_cond_wait(&capacityFull,&waitLock)!=0){
    		printf("\n Cond Wait Failed of\n"); //Capacity Full Wait
    	}
    }

    pthread_mutex_lock(&reqLock);
    request[temp->from]=1;			//Button Pressed
    pthread_mutex_unlock(&reqLock);
    if(temp->from>maxFloor && temp->from>currentFloor){
    	pthread_mutex_lock(&maxFLock);
    	maxFloor=temp->from;
    	pthread_mutex_unlock(&maxFLock);
    	//printf("MaxFloor is now  %d \n" ,maxFloor);
    }
    else if(temp->from<minFloor && temp->from<currentFloor){
    	pthread_mutex_lock(&minFLock);
    	minFloor=temp->from;
    	pthread_mutex_unlock(&minFLock);
    	//printf("MinFloor is now  %d \n" ,minFloor);
    }

  	if (pthread_cond_wait(&conditions[temp->from],&peopleMutex[temp->id])!=0){
  		printf("\n Getting On Cond Wait Failed of %d \n",temp->id);			//Waiting to get "on" the lift
		exit(1);
  	}
  	pthread_mutex_lock(&capacityLock);
  	currentPeople++; //Count for people in lift
  	pthread_mutex_unlock(&capacityLock);
  //	printf("ID %d Entered Lift \n",temp->id);

  	pthread_mutex_lock(&reqLock);
  	request[temp->to]=1;			//Pressed Destnation Button
  	pthread_mutex_unlock(&reqLock);
  	if(temp->to>maxFloor && temp->to>currentFloor){
    	pthread_mutex_lock(&maxFLock);
    	maxFloor=temp->to;
    	pthread_mutex_unlock(&maxFLock);
    	//printf("MaxFloor is now  %d \n" ,maxFloor);
    }
    else if(temp->to<minFloor && temp->to<currentFloor){
    	pthread_mutex_lock(&minFLock);
    	minFloor=temp->to;
    	pthread_mutex_unlock(&minFLock);
    	//printf("MinFloor is now  %d \n" ,minFloor);
    }
  	if (pthread_cond_wait(&conditions[temp->to],&peopleMutex[temp->id])!=0){
  		printf("\n Getting Off Cond Wait Failed of %d \n",temp->id); //Traveling in lift waiting to get "off" it
		exit(1);
  	}

  	if(currentPeople==maxPeople){
    	if(pthread_cond_signal(&capacityFull)!=0){
    		printf("\n Lift Signal Failed at\n");		//if capacity was full tell the person lift has space now
			exit(1);
    	}
  	}

  	pthread_mutex_lock(&capacityLock);
  	currentPeople--; //cont for people out of lift
  	pthread_mutex_unlock(&capacityLock);

  	printf("%d %d %d\n", temp->id, temp->from, temp->to);

  
    //printf("ID %d Exited Lift \n",temp->id);
}
/*If you see the main file, you will get to 
know that this function is called after setting every
passenger.

So use this function for starting your elevators. In 
this way, you will be sure that all passengers are already
waiting for elevators.
*/
void startP1(){
	pthread_t mainThread;
	pthread_create(&mainThread,NULL,moveElevator,NULL);
}