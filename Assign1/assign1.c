/*  Hoi Kiu Haydn Pang
*   017463730
*   CECS 326
*   02/08/18
*   Assignment 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MBT_SIZE 1024

int pCTR = 0; // Process Counter to generate PID

// Structure for a size indicator and a memory block table
typedef struct MBT
{
    int freeBlocks; // indicates the number of free blocks available
    bool blocks[MBT_SIZE];  // represents the memory block table (default size of 1024 blocks)
} MBT;

MBT mbt = {.freeBlocks = MBT_SIZE - 32}; // First 32 blocks are reserved for OS

// Structure for a page table
typedef struct pageTable
{
    int start;      // starting index in the memory block table 
    //int ptSize;     // Size of Page Table
    int blocks[];   // page table of length size
} pageTable;

// Stores pointer to the next PCB if inside ready queue
// Stores pointer to page table for the respective PID
typedef struct PCB
{
    int PID;            // unique process ID to indicate a specific process
    int ptSize;         // Size of Page Table
    pageTable *pt;      // Pointer to the Page Table for the specific process
    struct PCB *next;   // Pointer to the next PCB in ready queue if applicable (Null by default)

} PCB;

// Ready Queue that stores PCBs in a linked list
typedef struct rdyQueue
{
    struct PCB *front, *back;
} rdyQueue;

rdyQueue *rdyQ; // Declared Ready Queue for global use

// Returns a new (empty) ready queue
rdyQueue *createReadyQueue()
{
    rdyQueue *rdyQ = malloc(sizeof(rdyQueue));
    rdyQ->front = rdyQ->back = NULL;
    return rdyQ; 
}

void addProcessQueue(PCB *pcb) 
{
    // If the ready queue is empty, the PCB is both the front and back of the queue
    if(rdyQ->back == NULL) 
    {
        rdyQ->front = rdyQ->back = pcb;
        return;
    }

    //Adds PCB to end of queue and changes the back of the queue
    rdyQ->back->next = pcb;
    rdyQ->back = pcb;
}

// Frees the Memory Block Table
void freeMBT(int startBlock, int pgTabSize) 
{
    for(int i = startBlock; i < pgTabSize + startBlock; i++) 
    {
        mbt.blocks[i] = true;
    }
    mbt.freeBlocks = mbt.freeBlocks + pgTabSize;
}

// Removes PCB with the passed PID value from the ready queue
bool removeProcessQueue(int pid) 
{
    PCB* temp = rdyQ->front, *prev; // prev is used for traversal purposes
    // If the PCB to be deleted is located at the front of the queue
    if(temp != NULL && temp->PID == pid) 
    {
        rdyQ->front = temp->next;               // Changes head of the queue
        freeMBT(temp->pt->start, temp->ptSize); // Frees the Memory Block Table associated with the Process
        free(temp->pt);                         // Frees the Page Table associated with Process
        free(temp);                             // Frees the memory used for the PCB
        return true;
    }

    // Traverses ready queue to find the PCB associated with the passed PID
    while(temp != NULL && temp->PID != pid) 
    {
        prev = temp;
        temp = temp->next;
    }
    // If the PCB was not found
    if (temp == NULL) return false;

    // Unlinks PCB from the ready queue
    prev->next = temp->next;
    freeMBT(temp->pt->start, temp->ptSize); // Frees the Memory Block Table associated with the Process
    free(temp->pt);                         // Frees the Page Table associated with Process
    free(temp);                             // Frees the memory used for the PCB
    return true;
}

// Outputs the entire ready queue, printing only the PID associated with each PCB in the queue
void printReadyQueue()
{
    PCB* temp = rdyQ->front;
    printf("Current Ready Queue\n");
    printf("[");
    while(temp != NULL) {
        printf(" %d ", temp->PID);

        temp = temp->next;
    }
    printf("]\n");
}

// Outputs the MBT
void printMBT()
{
    int maxPerRow = 10; // needed to be able to display the entire MBT
    printf("\nMemory Block Table\n");
    for(int i = 0; i < MBT_SIZE; i = i + 1) {
        printf("[%d: %s]", i, mbt.blocks[i] ? "free" : "not free"); // replaces true with free and false with not free
        if(i % maxPerRow == 0 && i != 0)
        {
            printf("\n");
        }
    }
    printf("\nFree Blocks: %d\n", mbt.freeBlocks);
}

// Outputs the passed Page Table
void printPageTable(pageTable *pgTable, int pgTabSize)
{
    int maxPerRow = 20; // used to easier display
    printf("\nPage Table\n");
    for(int i = 0; i < pgTabSize; i++)
    {
        printf("[%d: %d]", i, pgTable->blocks[i]);
        if(i % maxPerRow == 0 && i != 0)
        {
            printf("\n");
        }
    }
}

// Represents the menu with the available options
// Returns an int representing the chosen option
int menu(int choice)
{
    printf("*************************************************************\n");
    printf("(1) Initiate a process\n");
    printf("(2) Print all processes in ready queue\n");
    printf("(3) Terminate process with a specific PID\n");
    printf("(4) Exit\n");
    printf("*************************************************************\n");
    printf("Please enter an option (1-4): ");
    scanf("%d", &choice);
    
    return choice;
}

//  Returns the starting index of the block in the Memory Block Table that can be allocated
int freeBlockStart(int allocSize) 
{
    int startBlock = 32; // Due to first 32 blocks being reserved for OS
    int endBlock = 0; // temporary until first possible starting block is discovered
    // Finds the starting block to allocate
    while(startBlock != (MBT_SIZE - allocSize) + 1) {
        if(mbt.blocks[startBlock] == false) {
            startBlock++;
        }
        else {
            endBlock = startBlock;  // Indicates first free starting block found
            // Checks to ensure the needed consecutive blocks are free
            while(mbt.blocks[endBlock] != false) {
                // Starting block found
                if(endBlock - startBlock == allocSize - 1) {
                    return startBlock;
                }
                else {
                    endBlock++;
                }
            }
        }
    }
    // Executes if no free blocks are found (shouldn't execute)
    return 0;
}

// Returns the Page Table associated with the process
// startBlock indicates the starting block of this process
// allocSize indicates the size of the page table
pageTable* createPageTable(int startBlock, int pgTabSize) 
{
    pageTable* pgTable = malloc(sizeof(pageTable) + pgTabSize * sizeof(int));
    pgTable->start = startBlock;
    for(int i = 0; i < pgTabSize; i++) {
        pgTable->blocks[i] = startBlock + i;
    }
    return pgTable;
}

// Returns a PCB with a pointer to the Page Table associated with the process
// pgTableSize indicates the size of the Page Table
// *pgTable indicates a pointer to the Page Table associated with this process
PCB* createPCB(int pgTableSize, pageTable *pgTable) 
{
    PCB *pcb = malloc(sizeof(PCB));
    pcb->PID = pCTR;
    pcb->ptSize = pgTableSize;
    pcb->pt = pgTable;
    pcb->next = NULL;
    pCTR++;
    return pcb;
}

// Option 1 Initiate a Process
// Allocate dynamic memory for PCB
// Generate random number of blocks to allocate (10-250)
void initProcess()
{
    srand(time(NULL));                                              // uses time as the seed for random number generator
    int alloc = (10 + rand()) / (RAND_MAX / ((250 - 10 + 1) + 1));  // random number of blocks to allocate (10-250)
    printf("Number of blocks to allocate: %d\n", alloc);
    if(alloc <= mbt.freeBlocks) {
        // Attempts to creates a Page Table to represent the blocks allocated in the Memory Block Table
        int startBlock = freeBlockStart(alloc);
        if(startBlock == 0) {
            printf("Not enough consecutive blocks available\n");
        }
        else {
            for(int i = startBlock; i < startBlock + alloc; i++) {
                mbt.blocks[i] = false;
            }
            // Creates Page Table and dynamically allocates memory for it
            pageTable* pgTable = createPageTable(startBlock, alloc);
            
            PCB* pcb = createPCB(alloc, pgTable);   // Creates PCB and dynamically allocates memory for it
            addProcessQueue(pcb);                   // adds PCB into ready queue
            mbt.freeBlocks = mbt.freeBlocks - alloc;
            printPageTable(pgTable, alloc);
            printMBT();
        }
    } else {
        printf("Out of memory to be allocated\n");
    }
}

// Option 2 Prints all processes (just the PIDs) in the ready queue
void currProcesses()
{
    PCB* temp = rdyQ->front;
    printf("Current Ready Queue\n");
    printf("[");
    while(temp != NULL) {
        printf(" %d ", temp->PID);

        temp = temp->next;
    }
    printf("]\n");
}

// Option 3 Terminate
// Terminates the process for the passed PID
// Prints out error message if process is not found
void terminatePID(int pid) 
{
    bool termPID = removeProcessQueue(pid);
    if (!termPID) 
    {
        // Executes if the entire ready queue is traversed and process associated with passed PID is not found
        printf("Process not found or already terminated(PID): %d\n", pid);
    } 
}

// Option 4 Exit
bool exitProg() 
{
    currProcesses();    // output PIDs in ready queue
    printMBT();         // Outputs Memory Block Table
    
    // If ready queue is not empty, prompt user to confirm exit.
    if(rdyQ->front != NULL)
    {
        char c;
        printf("Confirm exit? (Y or N): ");
        scanf(" %c", &c);
        // Delete all dynamically allocated memory and end program
        // Goes through each PCB in the readyQueue and terminates them
        if(c == 'Y')
        {
            // Terminates all the processes in the ready queue
            for(int i = pCTR - 1; i > 0; i--)
            {
                terminatePID(i);
            }
            printf("Program Terminated\n");
            return true;
        }
        return false;
    }
    return true;
}

int main()
{
    int choice; // stores menu option
    bool exitP; // stores whether or not to exit the program
    // Reserves first 32 blocks of the Memory Block Table for the OS
    for(int i = 32; i < MBT_SIZE; i = i + 1) {
        mbt.blocks[i] = true;
    }
    rdyQ = createReadyQueue(); // Creates an empty ready queue
    do 
    {
        choice = menu(choice);
        if(choice == 1) {
            initProcess();
        } else if(choice == 2) {
            currProcesses();
        } else if(choice == 3) {
            int pid;
            printf("Please enter the PID you wish to terminate: ");
            scanf("%d", &pid);
            terminatePID(pid);
        } else if(choice == 4) {
            exitP = exitProg();
        }  else {
            printf("Invalid input.\n");
        }
    }
    while(!exitP);

    return 0;
}