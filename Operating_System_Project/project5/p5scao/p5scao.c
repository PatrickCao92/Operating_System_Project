/*
 * =====================================================================================
 *       Filename:  virtual memory manager.c
 *        Version:  1.0
 *        Created:  2015/11/09
 *         Author:  Shuaiqi Cao 
 * =====================================================================================
 */
#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAMENUM 8
#define PAGENUM 16
#define TRANSNUM 4
#define	PAGESIZE 256
#define READIN_BUF_SIZE 64

//set a struct for the current holden by the virtual memory manager
struct currentAddr	
{
	int vAddr; //virtual address
	int pAddr; //physical address
	int pageNum;	
	int offset;	
	int value;
};
struct PageTable
{
	int frameNum;
	int accessTime;
};
struct TLB
{
	int frameNum;
     	int pageNum;
     	int accessTime;
};
struct FrameTable
{
	int page;
};

struct PageTable pagetable[PAGENUM];
struct TLB tlb[TRANSNUM];
struct FrameTable frametable[FRAMENUM];
struct currentAddr addr={0,0,0,0,0};

int phyMem[FRAMENUM][PAGESIZE];
int pageFault=0;
int frameToken=0;
int requestNum=0;
int addressArray[1000];
int timer= 0;
int TLBhit=0;
int TLBfault=0;

void backingStoreLoad();
void pageTableSeek();
void TLBSeek(); 
void addressExtract();
void getValue();
void initTLB();
void iniPageTable();
void iniFrametable();
void printResult();

void main(int argc, char *argv[]){
    //Read in addresses
	FILE *stream;
	char address[READIN_BUF_SIZE];
	stream = fopen("addresses.txt","r");
	int index=0;
	while(1){
		fgets(address,READIN_BUF_SIZE,stream);      
		if(feof(stream)){ 
			break;
	      	}
		addressArray[index] = atoi(address);
		index++;	      	
   	}
   	fclose(stream);
	requestNum=index;

    //Initialize the TLB, pagetable and frametable
	initTLB();
	iniPageTable();
	iniFrametable();

    //use loop to page the addressses one by one
        for(index=0; index< requestNum; index++){
		timer++;
		addr.vAddr=addressArray[index];
		addressExtract();
		TLBSeek(); 
	}
    //print out results
	printResult();
}

void addressExtract(){
    //get the page number by shifting
    	addr.pageNum = (addr.vAddr & 0x0000FF00) >> (8);
    	addr.offset = addr.vAddr & 0x000000FF;
}

//set all parameters' values to -1
void initTLB()
{	
	int i;
	for(i=0; i<TRANSNUM;i++){
		tlb[i].frameNum = -1;
		tlb[i].accessTime= -1;
		tlb[i].pageNum = -1;
	}
}
void iniPageTable()
{
	int i;
	for(i=0; i<PAGENUM;i++){
		pagetable[i].frameNum = -1;
		pagetable[i].accessTime= -1;
	}
}

void iniFrametable()
{
	int i;
	for(i=0; i<FRAMENUM;i++){
		frametable[i].page = -1;
	}

}
//seek the papge number in TLB, if not find, call pagetable to seek
void TLBSeek()
{
	bool TLBmiss=true;
	int min =timer;
	int i;
	for(i=0; i<TRANSNUM;i++){
		if (addr.pageNum==tlb[i].pageNum){
			TLBmiss=false;		 	
			TLBhit++;
			tlb[i].accessTime=timer;
			pagetable[addr.pageNum].accessTime=timer;
			addr.pAddr=tlb[i].frameNum*PAGESIZE+addr.offset;
			break;
		}
	}

	if(TLBmiss){
		TLBfault++;
		pageTableSeek();
		int lru_p;
		int j;
		for(j=0; j<TRANSNUM;j++){
			if (tlb[j].accessTime<min){
				min=tlb[j].accessTime;
				lru_p=j;	
			}
		}
		tlb[lru_p].pageNum=addr.pageNum;
		tlb[lru_p].frameNum=pagetable[addr.pageNum].frameNum;	
		tlb[lru_p].accessTime=timer;
	}
		
}

//seek in page table
void pageTableSeek()
{
    //if frame is not full and dosen't find page, assign a new frame to the page
	if (frameToken< FRAMENUM && pagetable[addr.pageNum].frameNum==-1){
		pageFault++;
		pagetable[addr.pageNum].frameNum=frameToken;
		frameToken++;
		pagetable[addr.pageNum].accessTime=timer;
		addr.pAddr=pagetable[addr.pageNum].frameNum*PAGESIZE+addr.offset;
		backingStoreLoad();
		frametable[pagetable[addr.pageNum].frameNum].page=addr.pageNum;	
	}
    //if frame is full and doesn't find page, use LRU to do the replacement
    else if (frameToken == FRAMENUM && pagetable[addr.pageNum].frameNum==-1){
		pageFault++;		
		int min=timer;
		int lru_p,index;
		for(index=0; index<PAGENUM; index++){
			if(pagetable[index].frameNum!=-1){
			if(pagetable[index].accessTime<min){
				min=pagetable[index].accessTime;
				lru_p=index;
				}
			    }			
			}
			pagetable[addr.pageNum].frameNum=pagetable[lru_p].frameNum;
			pagetable[lru_p].frameNum=-1;
			
			pagetable[addr.pageNum].accessTime=timer;
			addr.pAddr=pagetable[addr.pageNum].frameNum*PAGESIZE+addr.offset;
			backingStoreLoad();		
			frametable[pagetable[addr.pageNum].frameNum].page=addr.pageNum;	
	}
    //if the page is find
    else if (pagetable[addr.pageNum].frameNum > -1){
		pagetable[addr.pageNum].accessTime=timer;
		addr.pAddr=pagetable[addr.pageNum].frameNum*PAGESIZE+addr.offset;
	}
}

//load data from backing store
void backingStoreLoad()
{
	char buff[PAGESIZE];
	FILE *stream;
	stream=fopen("BACKING_STORE.bin","rb");	
	if(stream == NULL){
        	printf("Error: cannot open BACKING_STORE.bin");
    	}
    	else if(fseek(stream, addr.pageNum * PAGESIZE, SEEK_SET) != 0){
        	printf("Error: fseek failed");
    	}
	else if(fread(buff, 1, PAGESIZE, stream) != PAGESIZE){	
	        printf("Error: fread failed");
 	}
    	fclose(stream);
	
	int numOfFrame=pagetable[addr.pageNum].frameNum;
	int i;    	
	for(i = 0; i < PAGESIZE; i++){
    	    phyMem[numOfFrame][i] = buff[i];
    	}
}
// get value from backing store according to address
void getValue()
{
	addr.value = phyMem[pagetable[addr.pageNum].frameNum][addr.offset];
}

void printResult()
{
	printf("(1)\n");
	int i;
 	for(i=0; i< PAGENUM; i++){
		if(pagetable[i].frameNum==-1){
			printf("Page %d: Not in Memory\n",i);
		}else{
			printf("Page %d: Frame %i\n",i,pagetable[i].frameNum);
		}
	}
	printf("(2)\n");
	for(i=0; i< FRAMENUM; i++){
		if(frametable[i].page==-1){
			printf("Frame %d: Empty\n",i);
		}else{
		printf("Frame %d: Page%d\n",i,frametable[i].page);
		}	
	}
	printf("(3)\n");	
	printf("%d page faults out of %d reference\n",pageFault,requestNum);
	printf("(4)\n");
	printf("%d TLB hits out of %d references\n",TLBhit,requestNum);	
	printf("%d TLB faults out of %d references\n",TLBfault,requestNum);	

}