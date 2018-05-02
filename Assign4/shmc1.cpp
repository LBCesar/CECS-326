/*	Hoi Kiu Haydn Pang
*	071463730
*	CECS 326
*	05/03/18
*	Assignment 4
*	shmc1.cpp 
*/

#include "registration.h" 		//	includes the registration.h header file which declares the structure CLASS
#include <sys/types.h>			//	Contains data types
#include <sys/ipc.h>	  		//  Interprocess communication access structure
#include <semaphore.h>			//	Defines the POSIX semaphore facility
#include <sys/shm.h>  			//	Defines the XSI shared memory facility
#include <sys/wait.h> 			//  Declarations for waiting, including constants used with wait() function call
#include <unistd.h>   			//  Standard symbolic constants and types
#include <stdlib.h>   			//  Defines 4 variable types, several macros, and various functions for general functions
#include <iostream>   			//  Defines standard input/output stream objects
#include <stdio.h>				//	Defines 3 variables types, macros, and functions used for standard input/output
#include <memory.h>   			//	Defines dynamic memory allocation
#include <pthread.h>			//	Used for threads 

//  Represents the namespace to be used, which in this case is std, the C++ Standard Library
//  For example, if the compiler sees string, it'll assume you are referring to std::string
using namespace std;

CLASS 	*class_ptr;									//	Declares pointer for a CLASS structure
void 	*memptr;									//	Declares pointer for shared memory segment
char	*pname;										//	Declares pointer for process name
int shmid; 											//	Declares variable for shared memory id (shmid)
int ret;											//	Shared memory segment detachment return value
sem_t *sem;											//	Named Semaphore
char	*semName;									//	Name of Semaphore
void rpterror(char *), srand(), perror(), sleep();	//	Declares functions used in the program
void sell_seats();									//	Declares the sell_seats() function

main(int argc, char* argv[])
{
	//	Checks to ensure there is at least 2 arguments passed
	//	If there are less than 2 arguments, it prints out a proper usage message and exits the process
	if (argc < 3) {
		fprintf (stderr, "Usage:, %s shmid semaphoreName\n", argv[0]);
		//	The passed value of 1 in the exit() function call indicates that it is a generic error
		exit(1);
	}

	pname = argv[0];					//	Initializes pname to the first argument, representing the name of the process
	sscanf (argv[1], "%d", &shmid);		//	Stores the second argument as a decimal (integer) into the variable shmid
	semName = argv[2];
	//	Attaches the shared memory segment associated with the shared memory identifier indicated by the variable shmid 
	//	to the first available address as selected by the system as the second parameter is a null pointer
	//	The shared memory segment's start address is assigned to memptr on completion of the shmat function call
	memptr = shmat (shmid, (void *)0, 0);	
	//	If memptr = -1, it indicates the shared memory segment attachment failed.
	//	The program outputs the custom message "shmat failed" followed by the default error message and exits the process
	if (memptr == (char *)-1 ) {
		rpterror ("shmat failed");
		//	The passed value of 2 in the exit() function call indicates it is most likely a misuse of builtin functions
		exit(2);
	}

	//	Assigns the shared memory segment pointer memptr to the declared CLASS structure pointer class_ptr
	class_ptr = (struct CLASS *)memptr;

	//	Opens the previously created semaphore with the name indicated by SNAME
	if ((sem = sem_open(semName, 0)) == SEM_FAILED)
	{
		perror("Failed to get semaphore");
		exit(1);
	}
	//	Sells all available seats for the class
	sell_seats();
	//	Detaches the shared memory segment associated with the pointer memptr
	//	and initializes ret to 0 on successful completion
	//	Otherwise, ret is initialized to -1 indicating the shared memory segment was not detached
	ret = shmdt(memptr);	
	exit(0); 				//	Exits the program indicating successful completion
}

//	Function to sell all seats in a class, indicated when seats_left <= 0 and all_out = 1
void sell_seats() 
{
	int all_out = 0;	//	Indicates if there are seats remaining (0 available, 1 unavailable)

	//	Initializes a pseudo-random number generator using the process id of this process as the seed
	srand ( (unsigned) getpid() );
	
	//	First checks to ensure there are seats remaining, exits the loop if there is none
	while ( !all_out) {   /* loop to sell all seats */
		//	Decrements (locks ) the passed semaphore, value goes from 1 -> 0
		//	Semaphore blocks when the value is zero
		sem_wait(sem);
		//	Checks to ensure there are seats left before starting to sell seats
		if (class_ptr->seats_left > 0) {	
			//	Sleeps for a random number of seconds between 1 and 5 seconds
			sleep ( (unsigned)rand()%5 + 1);
			//	Removes a seat from the class
			class_ptr->seats_left--;
			//	Sleeps for a random number of seconds between 1 and 5 seconds
			sleep ( (unsigned)rand()%5 + 1);
			//	Outputs the process name followed by the number of seats left
			//	Format is the following: "pname" SOLD SEAT -- "seats_left" left (without the quotations)
			cout << pname << " SOLD SEAT -- " 
			     << class_ptr->seats_left << " left" << endl;
		}
		else {
			//	Increments all_out variable to indicate there are no seats left
			all_out++;
			//	Outputs as the following: "pname" sees no seats left (without the quotations)
			cout << pname << " sees no seats left" << endl;
		}
		//	Increments (unlocks) the passed semaphore, value goes from 0 -> 1
		//	Since semaphore value > 0, another process blocked from previous sem_wait() call
		//	Is woken up to lock the semaphore
		sem_post(sem);
		//	Sleeps for a random number of seconds between 1 and 10 seconds
		sleep ( (unsigned)rand()%10 + 1);
	}
}

//	Prints out a custom message that is the concatanation of the passed parameters separated by a space
//	followed by the error message to the output stream
void rpterror(char* string)
{
	char errline[50]; 	//	Declares the char array representing the custom error message

	//	Stores the passed parameters in the char array errline with the following format: "%s %s"
	//	%s represents a string
	sprintf (errline, "%s %s", string, pname);
	//	Prints out the custom message stored in errline 
	//	followed by the error message itself in the standard error output stream
	perror (errline);
}
