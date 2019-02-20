#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


// struct pageTable { //Don't actually use this but it's a representation of the page table
//     int pageFrameNumbers[4]; //4 ints representing physical pages.
//     int protectionBits[4]; //Representing respective pages' protection bits
// };

void getTheInp();
int addPage();
void mapPage(int pID, unsigned int virtAdd, int valid);
void storeOrLoadData(int pID, unsigned int virtAdd, int value, int storeOrLoad); //Condensed into one function b/c all but 2 lines are the same. For storeOrLoad: 0 is store, 1 is load
int* decToBin(unsigned int virtAdd);
int binToDec(int * binary, int binSize);

//Page has the physical reference to memory, which is an index in the memory array
unsigned char memory[64]; //'e' when empty, 'p' when page exists but no data
static unsigned int PAGE_SIZE = 16;
int pageTableLoc[4];


int main() {
    int i;
    for(i = 0; i<64; i++) { //value of byte is 'e' when empty page
        memory[i] = 'e';
    }
    int j;
    for(j = 0; j<4; j++) {
        pageTableLoc[j] = -1;//initialize to all not existing
    }
    getTheInp();
}

void getTheInp() {
    int quit = 0;
    printf("\nInstruction?");

    char theInput[128];
    fgets(theInput, 128, stdin);
    int i = 0; // index to store things
    int procID;
    char instructType[10];
    int virtAddr, val;
    char *token = strtok (theInput, ","); // token is a comma
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
        token = strtok (NULL, ",");
        i++;
    }

    if (i!=4){
        printf("Error: args should be process_id,instruction_type,virtual_address, value\n");
        getTheInp();
    }
    if (procID<0||procID>3||virtAddr<0||virtAddr>63||val<0||val>255){
        printf("Error: ranges are [0,3] for process_id, [0,63] for virtual_address, [0,255] for value\n");
        getTheInp();
    }

    //check type of instruction
    if(!strcmp(instructType, "map")) {
        if (val!=0 && val!=1){//only 0 or 1 are valid for permission
            printf("Error: read write permission can only be 0 or 1\n");
            getTheInp();
        }
        mapPage(procID, virtAddr, val);
    }
    else if(!strcmp(instructType, "store")) {
        storeOrLoadData(procID, virtAddr, val, 0);
    }
    else if(!strcmp(instructType, "load")) {
        storeOrLoadData(procID, virtAddr, val, 1);
    }
    else {
        printf("Error: instructions can only be 'map', 'store', or 'load'\n");
    }
    getTheInp();//done with instructions so ask for new ones
}

int addPage() {
    int pageLocation = 0;
    while(memory[pageLocation] != 'e') { //Find first empty page
        pageLocation++;
    }
    int i;
    for(i = pageLocation; i < pageLocation + 16; i++) { //'p' is empty page
        memory[i] = 'p';
    }
    return pageLocation;
}

void mapPage(int pID, unsigned int virtAdd, int valid) {
    int *convertedAddr;
    convertedAddr = decToBin(virtAdd); //Get the virtual address in binary
    int pageNumberBinary[2];
    int offset[4];
    int z;
    for(z = 0; z < 6; z++) { //Get the two different portions of the virt addr to convert to decimal
        if(z<2) { pageNumberBinary[z] = convertedAddr[z]; }
        else { offset[z-2] = convertedAddr[z]; }
    }
    int pageNum = binToDec(pageNumberBinary, 2); //The page number relevant to the process (0-3 or)
    int byteAdd = binToDec(offset, 4); //The actual place within the page

    // printf("Have pID %d, virtAdd %d, valid %d\n", pID, virtAdd, valid);
    // printf("pageNum was just converted to %d and the byteOffset is %d\n", pageNum, byteAdd);

    //Now we have the two numbers we need to actually map the page
    if(pageTableLoc[pID] == -1) { //If the page table doesn't exist
        pageTableLoc[pID] = addPage();
        printf("Put page table for pID %d into physical frame %d\n", pID, pageTableLoc[pID]%(16-1));
    }
    else {
        printf("Already a process made\n");
        return;
    }

    int thePageTableIndex = pageTableLoc[pID]; //Get the index of the page table for this process

    if(pageNum > 3) { //TODO Change for evicting?
        printf("Can't fit page in\n"); //Max number of 16 byte pages with 64 byte space (3)
        return;
    }
    if(memory[thePageTableIndex + pageNum] != 'p') { //TODO Change for evicting?
        printf("Page already at index %d for pID %d\n", pageNum, pID); //Page already exists
        return;
    }
    //So now we've got the page table made for the process (assuming one didn't already exist) and now we are going to add a page

    int pageLoc = addPage(); //Make the actual page
    int physPageFrame = pageLoc%(16-1); //Gets the physical page frame #
    memory[thePageTableIndex+pageNum] = physPageFrame; //Puts in the page table the page frame # of which the page is at

    //Make sure valid bit is set
    memory[thePageTableIndex+pageNum+3] = valid;
    printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtAdd, pageNum, physPageFrame);
}

void storeOrLoadData(int pID, unsigned int virtAdd, int value, int storeOrLoad) {
    int thePageTableIndex = pageTableLoc[pID];

    int *convertedAddr;
    convertedAddr = decToBin(virtAdd); //Get the virtual address in binary
    int pageNumberBinary[2];
    int offset[4];
    int z;
    for(z = 0; z < 6; z++) { //Get the two different portions of the virt addr to convert to decimal
        if(z<2) { pageNumberBinary[z] = convertedAddr[z]; }
        else { offset[z-2] = convertedAddr[z]; }
    }
    int pageNum = binToDec(pageNumberBinary, 2); //The page number relevent to the process (0-3 or)
    int byteAdd = binToDec(offset, 4); //The actual place within the page

    if(memory[thePageTableIndex+pageNum+3] == 0) { //If the table has the value of read only
        printf("Error: Table set to READ ONLY\n");
        return;
    }

    int pageLocation = memory[thePageTableIndex+pageNum] * 16;
    int insertionLocation = pageLocation + byteAdd;
    
    if(!storeOrLoad) {//store
        memory[insertionLocation] = value;
        printf("Stored value %d at virtual address %d (physical address %d)\n", value, virtAdd, insertionLocation);
    }
    else {//load
        
        if(memory[insertionLocation] != 'p' && memory[insertionLocation] != 'e') {
            printf("The value %d at virtual address %d (physical address %d)\n", memory[insertionLocation], virtAdd, insertionLocation);
        }
        else{//nothing to load
            printf("No byte has been stored at virtual address %d\n", virtAdd);
        }
    }

}

int* decToBin(unsigned int virtAdd) {
    int* binaryNum = malloc(sizeof(int) * 6);
    int k = 5;
    while (virtAdd > 0) {
        binaryNum[k] = virtAdd % 2;
        virtAdd /= 2;
        k--;
    }
    return binaryNum;
}

int binToDec(int * binary, int binSize) {
    int decimal = 0;
    int index;
    for(index = 0; index<binSize; index++) {
        decimal += (binary[index] * (1 << binSize-index - 1)); //Gets the decimal val
    }
    return decimal;
}