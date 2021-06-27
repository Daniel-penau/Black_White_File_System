
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dataStorage.h"
#include "superblock.h"



void getFreeBlocks(){
	FILE *fd = NULL;
	char *addedPath = "/fusedata.";
	char str[5];
	char num[5];
	int i =0 , j = 0 , pos = 0 , number = 0;
	char *fileContent = (char *) malloc(BLOCK_SIZE);
	char *tmpPath=(char *) malloc(30);
	for(i = 1; i < 26 ;i++){
		strcpy(tmpPath,"FileSysData");
		sprintf(str, "%d", i);
		strcat(tmpPath,addedPath);
		char *filename = strcat(tmpPath,str);
		fd = fopen(filename,"r");
		fscanf(fd, "%s", fileContent);		
		j = 1;
		while(fileContent[j] != '}'){
			pos = 0;
			while(fileContent[j] != ','){	
				num[pos++] = fileContent[j++];						
			}
			sscanf(num, "%d", &number);
			freeblocks[number] = 1;	
			size ++;
			sprintf(num, "%d", 0);
			j++;			
		}		
		fclose(fd);
	}		
	free(fileContent);
	free(tmpPath);	
}

void setFreeBlocks(int block){
	freeblocks[block] = 0;
	int freeBlock = (block/400)+1;
	char str[5];	
	char *tmpPath=(char *) malloc(30);
	sprintf(str, "%d", freeBlock);
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	FILE *fd = NULL;
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%s", fileContent);
	fclose(fd);
	sprintf(str, "%d", block);
	char *point = strstr (fileContent,str);
	int pos1 = point - fileContent;
	int pos2 = pos1;
	while(fileContent[pos2++]!=','){}
	char* substr  = (char *) malloc(BLOCK_SIZE);
	strcpy(substr,"");
	strncat(substr, fileContent, pos1);
	strcat(substr, fileContent+pos2);
	fd = fopen(tmpPath,"w");
	fprintf(fd, "%s", substr);	
	fclose(fd);
	free(substr);
	free(fileContent);
	free(tmpPath);
}

void addFreeBlocks(int block){
	if(block == 0){return;}
	freeblocks[block] = 0;
	int freeBlock = (block/400)+1;
	char str[5];	
	char *tmpPath=(char *) malloc(30);
	sprintf(str, "%d", freeBlock);
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	FILE *fd = NULL;
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%s", fileContent);
	fclose(fd);
	sprintf(str, "%d", block);
	char *point = strstr (fileContent,"}");
	int pos1 = point - fileContent;
	char* substr  = (char *) malloc(BLOCK_SIZE);
	strcpy(substr,"");
	strncpy(substr, fileContent, pos1);
	strcat(substr,str);
	strcat(substr,",");
	strcat(substr,"}");	
	fd = fopen(tmpPath,"w");
	fprintf(fd, "%s", substr);
	fclose(fd);
	free(substr);
	free(fileContent);
	free(tmpPath);
}


int getOneBlock(){	
	int i = 0;
	for(i = 0; i < MAX_BLOCKS ;i++){
		if(freeblocks[i] == 1){
			char str[5];	
			char *tmpPath=(char *) malloc(30);
			sprintf(str, "%d", i);
			strcpy(tmpPath,"FileSysData/fusedata.");
			strcat(tmpPath,str);
			FILE *fd = NULL;	
			fd = fopen(tmpPath,"w");
			fprintf(fd, "%s", "");
			fclose(fd);
			free(tmpPath);
			return i;
		}	
	}
	return -1;
}

int getThatBlock(const char *path){
	int count = 0 , i = 0 , pos = 0;
	while(path[i]!='\0'){
		if(path[i]=='/'){
			count++;
		}i++;
	}	
	char folders[count][20];
	i = 1;
	char ch[20];
	int tmp = 0;
	while(path[i]!='\0'){
		if(path[i]=='/'){
			i++;				
			strcpy(folders[pos++],ch);
			for(tmp = 0 ; tmp < 20 ; tmp++){
				ch[tmp] = '\0';
			}
			tmp = 0;	
		}
		ch[tmp++]=path[i++];
	}
	FILE *fd = NULL;
	char *fileContent = (char *) malloc(BLOCK_SIZE);
	char *addedPath = "fusedata.";
	char str[5];
	char *tmpPath=(char *) malloc(30);
	int fileNum = 26;
	for(i = 1 ; i < pos ; i++){		
		strcpy(tmpPath,"FileSysData/");		
		sprintf(str, "%d", fileNum);
		strcat(tmpPath,addedPath);		
		char *filename = strcat(tmpPath,str);
		fd = fopen(filename,"r");
		fscanf(fd, "%[^\n]s", fileContent);	
		fclose(fd);					
		strcpy(filename,"");		
		char *folderPoint = strstr(fileContent,folders[i]);
		int pos1 = folderPoint - fileContent + strlen(folders[i]) +1;
		int pos2 = pos1;
		while(1){
			if(fileContent[pos2]=='}' || fileContent[pos2]==','){
				break;
			}pos2++;
		}
		char* substr  = (char *) malloc(10);
		strncpy(substr, fileContent+pos1, pos2-pos1);	
		fileNum = atoi(substr);	
	}
	free(fileContent);
	free(tmpPath);	
	return fileNum;		
}


