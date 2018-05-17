/*  Hoi Kiu Haydn Pang
*   017463730
*   CECS 326
*   03/08/18
*   Assignment 2
*/

#include <sys/types.h>  //  Contains data types, including ssize_t which is used for msgrcv() and msgsnd()
#include <sys/ipc.h>    //  Interprocess communication access structure
#include <sys/msg.h>    //  Message queue structures
#include <string.h>     //  Defines data type string and functions to manipulate arrays of characters
#include <iostream>     //  Defines standard input/output stream objects
#include <unistd.h>     //  Standard symbolic constants and types
#include <sys/wait.h>   //  Declarations of waiting
#include <stdlib.h>     //  Standard library that defines 4 variable types, several macros, and various functions for general usage

using namespace std;

// declare my global message buffer
struct buf {
    long mtype; // required
    char greeting[50];  // message content
};

//  Receives a message from the message queue with a queue ID of qid and prints it out
void rcvmsg(int qid) {
    buf msg;
    int size = sizeof(msg) - sizeof(long);  //  size of the message buffer - size of a long variable
    //  Receives the message with mtype=114 from the message queue associated with qid
    msgrcv(qid, (struct msgbuf *)&msg, size, 114, 0);
    //  Outputs the message received
    cout << "Receiver, " << "PID " << getpid() << " receives message from Message Queue " << qid << endl;
    cout << "Message Received: " << msg.greeting << endl;
}

int main(int argc, const char* argv[]) {
    cout << "Receiver, " << "PID " << getpid() << ", begins execution" << endl;
    int msgq_id = atoi(argv[0]);    //  Gets the queue ID from the argument executed from master.cpp
    rcvmsg(msgq_id);
    return 0;
}