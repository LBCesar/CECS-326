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

int main()
{   
    //  Creates a message queue and assigns the queue ID to qid
    int qid = msgget(IPC_PRIVATE, IPC_EXCL|IPC_CREAT|0600);
    //  Converting int into a string, then into a character constant to allow it to be passed as an argument
    auto qidstr = to_string(qid);
    char const *qidchar = qidstr.c_str();
    
    cout << "Message Queue " << qidchar << " created" << endl;

    //  Creates the Receiver process (for receiving messages to the message queue)
    pid_t cpid = fork();
    if(cpid < 0) {
        cout << "Fork failed." << endl;
        return 1;
    }
    else if(cpid == 0) {
        //  Receiving message program execution, passing the message queue ID as an argument
        execlp("./receiver", qidchar, NULL);
        exit(0);
    }

    //  Creates the Sender process (for sending messages from the message queue)
    cpid = fork();
    if(cpid < 0) {
        cout << "Fork failed." << endl;
        return 1;
    }
    else if(cpid == 0) {
        //  Sending message program execution, passing the message queue ID as an argument
        execlp("./sender", qidchar, NULL);
        exit(0);
    }

    //  Waits for the child processes to finish executing
    while(wait(NULL) != -1);

    //  Terminates the message queue along with the data structure associated with it
    msgctl(qid, IPC_RMID, NULL); 

    cout << "Message Queue " << qid << " terminated" << endl;
    exit(0);    //  Terminates the parent process
    
    return 0;
}