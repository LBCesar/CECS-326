/*	Hoi Kiu Haydn Pang
*	071463730
*	CECS 326
*	04/11/18
*	Assignment 4
*	shmp1.cpp 
*/

#include "registration.h"	//	includes the registration.h header file which declares the structure named CLASS
#include <sys/types.h>		//	Contains data types
#include <sys/ipc.h>		//  Interprocess communication access structure
#include <semaphore.h>		//	Defines the POSIX semaphore facility
#include <sys/shm.h>		//	Defines the XSI shared memory facility
#include <sys/wait.h> 		//  Declarations for waiting, including constants used with wait() function call
#include <unistd.h>			//  Standard symbolic constants and types
#include <stdlib.h> 		//  Standard library that defines 4 variable types, several macros, and various functions for general usage
#include <iostream>			//  Defines standard input/output stream objects
#include <stdio.h>			//	Used for standard input/output and defines several macro names used as positive integral constant expressions
#include <memory.h>			//	Defines dynamic memory allocation
#include <fcntl.h>			//	File control options, used for creating semaphore
#include <pthread.h>		//	Used for threads

//  Represents the namespace to be used, which in this case is std, the C++ Standard Library
//  For example, if the compiler sees string, it'll assume you are referring to std::string
using namespace std;

//	Declares a new CLASS structure
//	Initializes class_number = 1001
//	Initializes date = 120186
//	Initializes title = "Operating Systems"
//	Initializes seats_left = 15
CLASS myclass = { "1001", "120186", "Operating Systems", 15 };

#define NCHILD	3	//	indicate the number of child processes to create, which in this case is 3
#define SNAME "shmpSem"	//	name to be used for named semaphore

//	Declares the functions used in the main function before they are defined later in the program
int	shm_init( void * );								
void	wait_and_wrap_up( int [], void *, int , sem_t *);	
void	rpterror( char *, char * );					

main(int argc, char *argv[])
{
	int 	child[NCHILD];	//	Stores the process IDs of the child processes
	int 	i; 				//	Process number
	int		shmid;			//	Shared memory segment ID
	sem_t	*sem;			//	Semaphore
	void *shm_ptr;			//	Shared memory segment pointer
	char	ascshmid[10];	//	Shared memory segment ID as a character array
	char 	pname[14];		//	Process name

	strcpy (pname, argv[0]);			//	concatanates the first argument passed into the program to pname
	shmid = shm_init(shm_ptr);			//	initializes the shared memory segment and assigns its identifier to shmid
	sprintf (ascshmid, "%d", shmid);	//	stores shmid into the char array ascshmid 


	//	Initializes the semaphore and assigns to sem
	//	First parameter indicates the name to be assigned to the semaphore
	//	Second parameter indicates the flag to be used. 
	//	O_CREAT indicates the semaphore is created if it doesn't exist
	//	If both O_CREAT and O_EXCL is used, an error is returned if the semaphore
	//	with the given name already exists
	//	If the second parameter was O_CREAT, two more parameters are needed
	//	Third parameter indicates the permissions for the semaphore
	//	0644 indicates owner has read-write permissions, whereas everyone else has read only
	//	Fourth parameter indicates the initial value of the semaphore
	//	Essentially can be used to indicate the number of processes that can concurrently run
	sem = sem_open(SNAME, O_CREAT, 0644, 1);
	if(sem == SEM_FAILED) 
	{
		perror("Semaphore Initialization");
		exit(1);
	}

	//	Create NCHILD number of child processes 
	for (i = 0; i < NCHILD; i++) {
		//	Attempts to create a child process
		child[i] = fork();
		switch (child[i]) {
		//	Indicates the creation of the child process failed as a result of fork() returning -1
		case -1:
			//	Prints out "fork failure pname" followed by the error message (without the quotations)
			rpterror ("fork failure", pname);
			//	Exits the process and indicates the error is most likely due to generic error
			exit(1);
		//	Indicates the creation of the child process succeeded as a result of fork() returning 0
		//	Stores the process in the format "shmc%d" (without quotations) into pname
		//	%d represents the process number + 1 
		case 0:
			sprintf (pname, "shmc%d", i+1);
			//	Replaces the program code with that of shmc1 
			//  and passes the process name, process ID, and semaphore name as the arguments
			//	Runs the process until it terminates
			execl("shmc1", pname, ascshmid, SNAME, (char *)0);
			//	Prints out the error message "execl failed" (without the quotations)
			perror ("execl failed");
			//	Exits the process, indicating the error is most likely due to misuse of built-in functions
			exit (2);
		}
	}
	/*	Waits for all the child processes in child to terminate
		before detaching the shared memory segment from shm_ptr
	  	and destroys the shared memory segment and removes the semaphore from the system */
	wait_and_wrap_up (child, shm_ptr, shmid, sem);
	return 0;
}

//	Initializes the shared memory segment and assigns it to the passed pointer
//	Returns the shared memory segment ID
int shm_init(void *shm_ptr)
{
	int	shmid;	//	Shared memory segment ID

	/*	Gets a shared memory segment with a size equal to the CLASS structure
		First parameter represents the key used to generate shared memory segment associated with it
		Second parameter represents the size of the segment
		Third parameter is a flag that is used to set the permissions for the created segment
		0600 indicates only the owner has read-write permissions
		The IPC key generated by ftok() is based on the ID value 'u' and the current pathname
		The flag indicates read and write permissions for the process */
	shmid = shmget(ftok(".",'u'), sizeof(CLASS), 0600 | IPC_CREAT);
	/*	Indicates attempt at acquiring a shared memory segment has failed
		Prints out error message "shmget failed" (without quotations) */
	if (shmid == -1) {
		perror ("shmget failed");
		exit(3);
	}
	//	Attaches the created shared memory segment to the pointer shm_ptr
	shm_ptr = shmat(shmid, (void * ) 0, 0);
	/*	Indicates attempt at attaching shared memory segment to the pointer has failed
		Prints out error message "shmat failed" (without quotations) */
	if (shm_ptr == (void *) -1) {
		perror ("shmat failed");
		exit(4);
	}
	/*	Copies the data of the created CLASS structure myclass into shm_ptr
		The size of the data copied is the size of the CLASS structure in bytes */
	memcpy (shm_ptr, (void *) &myclass, sizeof(CLASS) );
	return (shmid);
}

//	Waits for all child processes to terminate before 
//	detaching the shared memory segment from the pointer *shm_ptr and 
//	removing the shared memory segment associated with the identifier shmid
//	Also unlinks and removes the named semaphore associated with *sem
void wait_and_wrap_up(int child[], void *shm_ptr, int shmid, sem_t *sem)
{
	int wait_rtn = NCHILD;	//	Flag to indicate when a child is returned
	int w = NCHILD;			//	Index of child process 
	int ch_active = NCHILD;	//	Number of child processes active

	//	Runs until there are no child processes active (indicated by ch_active = 0)
	while (ch_active > 0) {
		//	Waits for the child process to terminate, indicated by wait(0)
		wait_rtn = wait( (int *)0 );
		for (w = 0; w < NCHILD; w++)
			//	Decrements the number of child processes active when a child process terminates
			if (child[w] == wait_rtn) {
				ch_active--;
				//	Exits the for loop and and waits for another child process to terminate
				break;
			}
	}
	//	Outputs "Parent removing shm" (without the quotations) and ends the line
	cout << "Parent removing shm" << endl;
	//	Detaches the shared memory segment associated with the pointer
	shmdt (shm_ptr);
	//	Removes the shared memory segment identifier specified with shmid 
	//	and destroys the shared memory segment and data structure associated with it. 
	shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);

	//	Unlinks and closes the named semaphore
	sem_unlink(SNAME);
	sem_close(sem);
	//	Exits the process
	exit (0);
}

//	Prints out a custom message that is the concatanation of the passed parameters separated by a space
//	followed by the error message to the output stream
void rpterror(char *string, char *pname)
{
	char errline[50];	//	Declares the char array representing the custom error message

	//	Stores the passed parameters in the char array errline with the following format: "%s %s"
	//	%s represents a string
	sprintf (errline, "%s %s", string, pname);
	//	Prints out the custom message stored in errline 
	//	followed by the error message itself in the standard error output stream
	perror (errline);
}
