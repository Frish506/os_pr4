#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


struct pageTable { //Don't actually use this but it's a representation of the page table
    int pageFrameNumbers[4]; //4 ints representing physical pages.
    int protectionBits[4]; //Representing respective pages' protection bits
};

void getTheInp();
int addPage();
void mapPage(int pID, unsigned int virtAdd, int valid);
void storeData(int pID, unsigned int virtAdd, int value);
void loadData(int pID, unsigned int virtAdd);
int* virtAddrConv(unsigned int virtAdd);
int binToDec(int * binary);

//Page has the physical reference to memory, which is an index in the memory array
unsigned char memory[64]; //There can be a max
static unsigned int PAGE_SIZE = 16;
unsigned int pageTableLoc[4];


int main() {
    int i;
    for(i = 0; i<320; i++) { //If there's nothing in the slot (ie empty page), the values of those bytes are -2
        memory[i] = -2;
    }
    for(i = 0; i<4; i++) {
        pageTableLoc[i] = -1;
    }
    getTheInp();
}

void getTheInp() {
    int quit = 0;
    printf("Instruction?\n");

    char* theInput;
    fgets(theInput, 128, stdin);
    int i = 0; // index to store things
    int procID;
    char* instructType[10];
    int virtAddr, val;
    char *token = strtok (theInput, " "); // token is a space

    while (token != NULL) { //before null terminator
        switch(i) {
            case 0:
                procID = atoi(token);
                break;
            case 1:
                strcpy(instructType, token);
                break;
            case 2:
                virtAddr = atoi(token);
                break;
            case 3:
                val = atoi(token);
                break;
        }
        token = strtok (NULL, " ");
        i++;
    }
    if(i>=4) {
        printf("TOO MANY ARGS FUCKER\n");
    }

    if(!strcmp(instructType, "map")) {

    }
    else if(!strcmp(instructType, "store")) {

    }
    else if(!strcmp(instructType, "load")) {

    }
    else {
        printf("We didn't recognize that instruction sweetheart. Try again\n");
    }
    if(quit) {
        getTheInp();
    }
}

int addPage() {
    int pageLocation = 0;
    while(memory[pageLocation] != -2) { //Find first empty page
        pageLocation++;
    }
    int i;
    for(i = pageLocation; i < pageLocation + 16; i++) { //empty page is filled with -1
        memory[i] = -1;
    }
    return pageLocation;
}

void mapPage(int pID, unsigned int virtAdd, int valid) {
    int convertedAddr[6] = virtAddrConv(virtAdd); //Get the virtual address in binary
    int pageNumberBinary[2];
    int offset[4];
    int z;
    for(z = 0; z < 6; z++) { //Get the two different portions of the virt addr to convert to decimal
        if(z<2) { pageNumberBinary[z] = convertedAddr[z]; }
        else { offset[z-2] = convertedAddr[z]; }
    }
    int pageNum = binToDec(pageNumberBinary); //The page number relevent to the process (0-3 or)
    int byteAdd = binToDec(offset); //The actual place within the page

    //Now we have the two numbers we need to actually map the page

    if(pageTableLoc[pID] == -2) { //If the page table doesn't exist
        pageTableLoc[pID] = addPage();
        printf("Put page table for pID %d into physical frame %d\n", pID, pageTableLoc[pID]%(16-1))
    }

    int thePageTableIndex = pageTableLoc[pID]; //Get the index of the page table for this process

    if(pageNum > 3) { //TODO Change for evicting?
        printf("Can't fit page in\n"); //Max number of 16 byte pages with 64 byte space (3)
        return;
    }
    if(memory[thePageTableIndex + pageNum] != -1) { //TODO Change for evicting?
        printf("Page already at index %d for pID %d\n", pageNum, pID); //Page already exists
        return;
    }
    //So now we've got the page table made for the process (assuming one didn't already exist) and now we are going to add a page

    int pageLoc = addPage(); //Make the actual page
    int physPageFrame = pageLoc%(16-1); //Gets the physical page frame #
    memory[thePageTableIndex+pageNum] = physPageFrame;
    printf("Mapped virtual address %d (page %d) into physical frame %d", virtAdd, pageNum, physPageFrame);
}

int* decToBin(unsigned int virtAdd) {
    int binaryNum[6];
    int k = 0;
    while (virtAdd > 0) {
        binaryNum[k] = virtAdd % 2;
        virtAdd /= 2;
        k++;
    }
    return binaryNum;
}

int binToDec(int * binary) {
    int decimal = 0;
    size_n binSize = sizeof(binary) / sizeof(int);
    int ind;
    for(ind = 0; i<binSize; i++) {
        decimal += (binary[ind] * (1 << binSize-ind));
    }
    return decimal;
}