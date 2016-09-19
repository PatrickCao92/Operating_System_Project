/*
 * =====================================================================================
 *       Filename:  p6scao.c	(Mini File System)
 *        Version:  1.0
 *        Created:  2015/11/23
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

int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int dismount_fs(char *disk_name);
int fs_open(char *name);
int fs_close(int fildes);
int fs_create(char *name);
int fs_delete(char *name);
int fs_read(int fildes, void *buf, size_t nbyte);
int fs_write(int fildes, void *buf, size_t nbyte);
int fs_get_filesize(int fildes);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);

char metaBuf[160];
char toChar(int Int);
int toInt(char Char);
int toBlock(int fat);
int toFAT(int block);
int fileNum=0;

#define BLOCK_SIZE 16
int OFT[4][3];
char blockBuf[BLOCK_SIZE];
off_t offset;
off_t length;

char toChar(int Int){
	unsigned char Char;
	Char = (unsigned char)Int;
	return Char;
}
int toInt(char Char){
	int Int;
	Int = (int)Char;
	return Int;
}

int make_fs(char *disk_name){
	int mk,op;	
	mk=make_disk(disk_name);
	op=open_disk(disk_name);
	if(mk==-1||op==-1){
		return -1;	
	}else{
		blockBuf[0]=toChar(1);//OFT
		blockBuf[1]=toChar(2);//FAT START
		blockBuf[2]=toChar(5);//FAT END
		blockBuf[3]=toChar(6);//DIR START
		blockBuf[4]=toChar(9);//DIR END
		blockBuf[5]=toChar(32);//datablock START
		blockBuf[6]=toChar(63);//datablock END
		
		block_write(0,blockBuf);
		close_disk();
		return 0;
	}
}
int mount_fs(char *disk_name){
	int op;	
	op=open_disk(disk_name);
	if(op==-1){
		//printf("disk could not open");
		return -1;
	}	
	//load meta data
	int i,j;
	for(i=0;i<10;i++){
		char *string = (char *) malloc(16);
		block_read(i,string);
		for(j=0;j<16;j++){
			metaBuf[10*i+j]=string[j];
		}
	}
	//initialize the OFT
	int n;
	for(n=0;n<4;n++){
		OFT[n][0]=0;//whether exist
		OFT[n][1]=0;//fileDes
		OFT[n][2]=0;//offset
	}
	return 0;		
}
int dismount_fs(char *disk_name){
	if(block_write(0,metaBuf)==-1){
		printf("write error\n");
		return -1;
	}
	//write meta data back
	int i,j;
	for(i=0;i<10;i++){
		char *string = (char *) malloc(16);
		for(j=0;j<16;j++){
			string[j]=metaBuf[10*i+j];
		}
		block_write(i,string);
	}
	int close;	
	close=close_disk(disk_name);
	if(close==-1){
		return -1;
	}
	return 0;
}
int fs_open(char *name){
	int namelen=strlen(name);
	if (namelen>4){
		printf("Invalid name\n");
		return -1;
	}
	int i;	
	int m=0;
	for(i=0;i<4;i++){
		if(OFT[i][0]==1){
                   m++;
		}
	}
	if (m==4){
		printf("Over the open limit");
		return -1;
	}
	//whether the file exist
	char *string;
	string = (char *) malloc(4);
	int nameptr;
	for (nameptr=6*16+3;nameptr<16*10;nameptr=nameptr+8){
				
		for (offset=0;offset<4;offset++){	   
			string[offset]=metaBuf[nameptr+offset];
		}

		if(strcmp(string,name)==0){
			//printf("find file\n");
			//check whether is opened
			int k;
			for(k=0;k<4;k++){
				if(toInt(metaBuf[nameptr+4])==OFT[k][1]){
					printf("File already open\n");
					return -1;				
				}
			}
			//open
			int j;
			for(j=0;j<4;j++){
				if(OFT[j][0]==0){
					OFT[j][0]=1;//exist
					OFT[j][1]=toInt(metaBuf[nameptr+4]);//file des
					OFT[j][2]=0;//offset
					break;
				}
			}
			return j;
		}
	}
	//printf("can't find the file\n");
	return -1;
}

int fs_close(int fildes){
	if(OFT[fildes][0]==0){
		printf("this file is not open\n");
		return -1;
	}
	if(fildes>3||fildes<0){
		printf("invalid file descriptor\n");
		return -1;	
	}
	OFT[fildes][0]=0;
	OFT[fildes][1]=0;
	OFT[fildes][2]=0;
	return 0;
}
int fs_create(char *name){
	int nameLen=strlen(name);
	if(nameLen>4){
		return -1;	
	}
	if(fileNum==8){
		printf("Over the max file limit\n");
		return -1;
	}
	// check duplicate
	int nameptr;
	int offset;
	char *string;
	string = (char *) malloc(4);
	for (nameptr=6*16+3;nameptr<16*10;nameptr=nameptr+8){
				
		for (offset=0;offset<4;offset++){	   
			string[offset]=metaBuf[nameptr+offset];
		}

		if(strcmp(string,name)==0){
		 printf("This file is already existed\n");
		 return -1;
		}
	}
	// check FAT	
	int fatptr;
	for(fatptr=16*2;fatptr<16*6;fatptr=fatptr+2){
		if(toInt(metaBuf[fatptr])==0){
			metaBuf[fatptr]=toChar(1);
			break;		
		}	
	}

	if (fatptr>16*6){
		printf("Disk is full");
		return -1;	
	}
	//write dir
	for (nameptr=6*16+3;nameptr<16*10;nameptr=nameptr+8){
		  
		if(toInt(metaBuf[nameptr-3])==0){
			metaBuf[nameptr-3]=toChar(1);
		        metaBuf[nameptr-2]=toChar(0);
			metaBuf[nameptr-1]=toChar(0);

			for (offset=0;offset<nameLen;offset++){
			 	metaBuf[nameptr+offset]=name[offset];
			}
			metaBuf[nameptr+4]=toChar((fatptr-32)/2+32);
			break;
		}			
	}
	fileNum++;
}
int fs_delete(char *name){
	//check name
	char *string;
	string = (char *) malloc(4);
	int found=0;
	int nameptr;
	for (nameptr=6*16+3;nameptr<16*10;nameptr=nameptr+8){
		for (offset=0;offset<4;offset++){	   
			string[offset]=metaBuf[nameptr+offset];
		}
		if(strcmp(string,name)==0){
			found=1;			
			break;	
		}
	}
	if(found==0){
		printf("file dose not exist\n");
		return -1;	
	}
	//check if open
	int firstBlock=metaBuf[nameptr+4];
	int i;	
	for(i=0;i<4;i++){
		if(firstBlock==OFT[i][1]){
			printf("file can not be deleted, because it is open\n");
			return -1;		
		}
	}
	//delete FAT
	char *bufTemp;
	bufTemp = (char *) malloc(16);
	int next=firstBlock;

	while(next!=0){
		bufTemp="";
		block_write(next,bufTemp);
		metaBuf[toFAT(next)-1]=0;
		metaBuf[toFAT(next)]=0;
		next=metaBuf[toFAT(next)+1];
	}
	//delete Dir
	int j;
	for(j=nameptr-3;j<nameptr+5;j++){
		metaBuf[j]=0;
	}
	fileNum--;
}
int fs_read(int fildes, void *buf, size_t nbyte){
	if(fildes>3||fildes<0){
		return -1;	
	}
	int length=fs_get_filesize(fildes);
	if(OFT[fildes][2]==length){
		return 0;	
	}
	void *dummyBuf=buf;
	int next=OFT[fildes][1];
	while(next!=0){
		block_read(next,buf);
		next=metaBuf[toFAT(next)+1];
		buf+=16;
	}
	buf=dummyBuf;

	if(nbyte+OFT[fildes][2]>length){
		buf+=OFT[fildes][2];
		return length-OFT[fildes][2];
	}
	int i;
	for(i=OFT[fildes][2];i<OFT[fildes][2]+nbyte;i++){
		memcpy(buf,buf+OFT[fildes][2],nbyte);
		
	}
	//update offset
	if(OFT[fildes][2]+nbyte>length){
		OFT[fildes][2]=length;
	}else{
		OFT[fildes][2]+=nbyte;
	}
	return nbyte;
}

int fs_write(int fildes, void *buf, size_t nbyte){	
	int checkFAT;
	int diskAva=0;
	for(checkFAT=16*2;checkFAT<16*6;checkFAT=checkFAT+2){
		if(toInt(metaBuf[checkFAT])==0){
			diskAva=1;
			break;
		}
	}
	if(diskAva==0){
		return 0;
	}
	//write in	
	if(OFT[fildes][0]==0){
		printf("the file is not open\n");
		return -1;
	}

	if(OFT[fildes][2]==0){
		int blockNum=(nbyte-1)/16;
		if(nbyte==0){
			blockNum=0;
		}else{
			blockNum=(nbyte-1)/16;
		}
		if(blockNum<=0){
			block_write(OFT[fildes][1],buf);
		}else{
			int currentFAT=toFAT(OFT[fildes][1]);
			char *buff = (char *) malloc(nbyte);
			strcpy(buff,buf);
			block_write(toBlock(currentFAT),buff);
			buff=buff+16;
			
			int blk;
			for(blk=0;blk<blockNum;blk++){
				int fatptr;
				for(fatptr=16*2;fatptr<16*6;fatptr=fatptr+2){
					if(toInt(metaBuf[fatptr])==0){
						metaBuf[fatptr]=toChar(1);
						metaBuf[currentFAT+1]=toBlock(fatptr);
						currentFAT=fatptr;
						block_write(toBlock(fatptr),buff);
						buff=buff+16;
						break;
					}	
				}
			}
	
		}
	}else{
		int lastOffset=OFT[fildes][2]%16;
		int lastBlockNum=OFT[fildes][2]/16;
		if(lastOffset==0){
			lastOffset=16;
			lastBlockNum--;
		}
		
		int currFAT=toFAT(OFT[fildes][1]);
		int j;		
		for(j=0;j<lastBlockNum;j++){
			int fatptr=currFAT;
			currFAT=toFAT(metaBuf[currFAT+1]);		
		}
		int dummyCurrFAT=currFAT;
		if(metaBuf[dummyCurrFAT+1]!=0){
			dummyCurrFAT=toFAT(metaBuf[dummyCurrFAT+1]);
			metaBuf[dummyCurrFAT]=0;
		}
		while(metaBuf[dummyCurrFAT+1]!=0){
			metaBuf[dummyCurrFAT]=0;
			dummyCurrFAT=toFAT(metaBuf[dummyCurrFAT+1]);
		}

		char *buffer = (char *) malloc(16);
		char *buffercut = (char *) malloc(16);
		block_read(toBlock(currFAT),buffer);
		strncpy(buffercut,buffer,lastOffset);
		strcat(buffercut,buf);

		int followBlockNum=(strlen(buffercut)-1)/16;
		block_write(toBlock(currFAT),buffercut);
		buffercut=buffercut+16;

		int blk;
		for(blk=0;blk<followBlockNum;blk++){
			int fatptr;
			for(fatptr=16*2;fatptr<16*6;fatptr=fatptr+2){
				if(toInt(metaBuf[fatptr])==0){
					metaBuf[fatptr]=toChar(1);
					metaBuf[currFAT+1]=toBlock(fatptr);
					currFAT=fatptr;	
					block_write(toBlock(fatptr),buffercut);
					buffercut=buffercut+16;
					break;
				}	
			}
		}
		
	
	}
	//set offset
	OFT[fildes][2]+=nbyte;

	//update length
	int len1=(OFT[fildes][2])/256;
	int len2=(OFT[fildes][2])%256;
	int i;
	for(i=0;i<8;i++){
		if(OFT[fildes][1]==toInt(metaBuf[96+i*8+7])){
			metaBuf[96+i*8+1]=toChar(len1);
			metaBuf[96+i*8+2]=toChar(len2);
			break;	
		}
	}
	//check if memory overflow (>512)
	int blockOccupied=(OFT[fildes][2])/16;
	int finalOffset=(OFT[fildes][2])%16;
	
	if(blockOccupied=31&&finalOffset>0){
		return 0;
	}


	return 0;
		
}
int fs_get_filesize(int fildes){
	int length;
	int i;
	for(i=0;i<8;i++){
		if(OFT[fildes][1]==toInt(metaBuf[96+i*8+7])){
			length=256*toInt(metaBuf[96+i*8+1])+metaBuf[96+i*8+2];
		}
	}	
	return length;
}
int fs_lseek(int fildes, off_t offset){
	if(fildes>3||fildes<0){
		return -1;	
	}
	int length=fs_get_filesize(fildes);
	if(offset>length||offset<-length){
		return -1;
	}
	OFT[fildes][2]+=offset;
	return 0;
}
int fs_truncate(int fildes, off_t length){
	if(fildes>3||fildes<0){
		return -1;	
	}
	int len=fs_get_filesize(fildes);
	if(length>len){
		return -1;
	}
	
	char *buf;
	buf = (char *) malloc(16);
	char *bufTemp;
	bufTemp = (char *) malloc(16);
	int cutBlock=length/16;
	int cutByte=length%16;
	int count=0;
	int next=OFT[fildes][1];

	while(next!=0){
		int cut=-1;
		if(count==cutBlock){
			block_read(next,buf);
			strncpy(bufTemp,buf,cutByte);
			block_write(next,bufTemp);
			cut=next;
		}
		if(count>cutBlock){
			bufTemp="";
			block_write(next,bufTemp);
			metaBuf[toFAT(next)]=0;
		}
		block_read(next,buf);
		next=metaBuf[toFAT(next)+1];
		if(cut>0){
			metaBuf[toFAT(cut)+1]=0;
			cut=-1;
		}
		count++;
	}
	//change length
	int newLen1=length/256;
	int newLen2=length%256;
	int i;
	for(i=0;i<8;i++){
		if(OFT[fildes][1]==toInt(metaBuf[96+i*8+7])){
			metaBuf[96+i*8+1]=newLen1;
			metaBuf[96+i*8+2]=newLen2;
		}
	}	
	//change offset
	OFT[fildes][2]=0;

}
int toBlock(int fat){
	int block=(fat-32)/2+32;
	return block;	
}
int toFAT(int block){
	int fat=(block-32)*2+32;
	return fat;
}
