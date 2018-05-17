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
#include <string>       //  C++ library for strings

using namespace std;

// declare my global message buffer
struct buf {
    long mtype; // required
    char greeting[50];  // message content
};

//  Gets user input and stores it into msg.greeting then sends it to the message queue associated with a queue ID of qid
void sendmsg(int qid) {
    buf msg;                                //  message buffer
    int size = sizeof(msg) - sizeof(long);  //  size of the message buffer - size of a long variable
    msg.mtype = 114;                        //  positive integer for the receiver process to grab messages with this type
    string message;                         //  stores user input before being stored in the message buffer
    
    //  Pauses the sender process for 2 seconds to ensure the receiver process is ready as well before continuing
    sleep(2);
    cout << "Please enter your message (Press Enter when done):" << endl;
    //  Stores user input into the message buffer character array greeting
    getline(cin, message);
    strcpy(msg.greeting, message.c_str());
    //  Outputs the message sent along with the sender process ID
    cout << "Sender, " << "PID " << getpid() << " sends message to Message Queue " << qid << endl;
    cout << "Message Sent: " << msg.greeting << endl;  
    //  Sends the message to the message queue associated with a queue ID of qid
    msgsnd(qid, (struct msgbuf *)&msg, size, 0);
}

int main(int argc, const char* argv[]) {
    cout << "Sender, " << "PID " << getpid() << ", begins execution" << endl;
    int msgq_id = atoi(argv[0]);    //  Gets the queue ID from the argument executed from master.cpp
    sendmsg(msgq_id);                  
    return 0;
}