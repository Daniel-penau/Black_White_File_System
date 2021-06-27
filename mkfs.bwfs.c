#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/file.h> 
#include "superblock.h"
#include "dataStorage.h"


static void superblock(){
	FILE *fd = NULL;
	time_t seconds = time(NULL);
	fd = fopen("FileSysData/fusedata.0","w");
	fprintf(fd,"{creationTime:%ld,mounted:50,devId:20,freeStart:1,freeEnd:25,root:26,maxBlocks:10000}",seconds);
	fclose(fd);
}

static void rootblock(int link){
	FILE *fd = NULL;
	time_t seconds = time(NULL);
	fd = fopen("FileSysData/fusedata.26","w");
	fprintf(fd,"{size:%d,uid:1000,gid:1000,mode:16877,atime:%ld,ctime:%ld,mtime:%ld,linkcount:%d,filename_to_inode_dict:{d:.:26,d:..:26}}",size,seconds,seconds,seconds,link);
	fclose(fd);
}

static void freeBlocks(){
	FILE *fd = NULL;
	char *addedPath = "/fusedata.";
	int i =0, j = 1;
	char str[5];
	fd = fopen("FileSysData/fusedata.1","w");
	fprintf(fd,"{");
	fclose(fd);
	fd = fopen("FileSysData/fusedata.1","a");
	char *tmpPath=(char *) malloc(30);
	for(i = 27; i < MAX_BLOCKS ;i++){
		if(i % 400 == 0){
			j++;
			strcpy(tmpPath,"FileSysData");
			sprintf(str, "%d", j);
			strcat(tmpPath,addedPath);
			char *filename = strcat(tmpPath,str);
			fprintf(fd,"}");
			fclose(fd);			
			fd = fopen(filename,"w");
			fprintf(fd,"{");
			fclose(fd);
			fd = fopen(filename,"a");
		}
		fprintf(fd,"%d,",i);		
	}
	fprintf(fd,"}");
	fclose(fd);
}



static void initialize(){	
			superblock();
			freeBlocks();
			rootblock(2);
}






static void my_init(){
	system("mkdir -p FileData");
	char *buff = (char *) malloc(512);
	memset(buff,'0',512);
    
	FILE *fd = NULL;
	fd = fopen("FileData/data.0","r");
    printf("max \n");
	if(fd == NULL){
		int i=0,j=0;
		char *addedPath = "/data.";
		char str[5];
		char *tmpPath=(char *) malloc(30);
        printf("max \n");
		for(i = 0; i < MAX_BLOCKS ;i++){
			strcpy(tmpPath,"FileData");
			sprintf(str, "%d", i);
			strcat(tmpPath,addedPath);
			char *filename = strcat(tmpPath,str);
			fd = fopen(filename,"w");
			for(j = 0; j < 8;j++){ 
				fprintf(fd,"%s",buff);
			}
			fclose(fd);
		}
		initialize();
	}	
    
	getFreeBlocks();
    printf("aca3\n");
	free(buff);	
    	
}


static void  MakeFiles(const char *path, int block){
	FILE *fd = NULL;
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	char str[5];
	sprintf(str, "%d", block);
	char *tmpPath=(char *) malloc(30);
	strcpy(tmpPath, "FileData/data.");
	strcat(tmpPath, str);
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);	
	char *point = strstr (fileContent,"indirect:");
	int pos1 = point - fileContent + 9;
	char ch = fileContent[pos1];
	char *substr = (char *) malloc(10);	
	point = strstr (fileContent,"location:");
    pos1 = point - fileContent + 9;
	int pos2 = pos1;	
	while(fileContent[pos2++]!='}'){}
	strcpy(substr, "");
	strncat(substr, fileContent+pos1,pos2-pos1-1);	
	strcpy(tmpPath, "FileData/data.");
	strcat(tmpPath, substr);
	strcpy(fileContent,"");	
	fd = fopen(tmpPath,"r");
	fscanf(fd,"%[^\n]s", fileContent);
	fclose(fd);
	if(ch  == '0'){
		fd = fopen(path,"w");		
		fprintf(fd, "%s", fileContent);
		fclose(fd);
	}else{

		int numFiles = 0;
		pos2 =0;
		while(fileContent[pos2]!='}'){if(fileContent[pos2]==','){numFiles++;}pos2++;}		
		int listofFile[numFiles];	
		int tmp = 0;
		pos1 = 0;
		for(tmp = 0 ; tmp < 5 ; tmp++){
			str[tmp] = '\0';
		}tmp = 0;
		int i = 1 ;
		while(fileContent[i]!='}'){						
			if(fileContent[i]==','){
				i++;
				;	
				listofFile[pos1++] = atoi(str);

				for(tmp = 0 ; tmp < 5 ; tmp++){
					str[tmp] = '\0';
				}tmp=0;
				if(fileContent[i]=='}'){break;}
			}
			str [tmp++] = fileContent[i++];
		}
		for(i = 0 ; i < pos1 ; i++){				
			strcpy(tmpPath, "FileData/data.");
			sprintf(str, "%d", listofFile[i]);
			strcat(tmpPath, str);	
			fd = fopen(tmpPath,"r");
			fscanf(fd,"%[^\n]s", fileContent);
			fclose(fd);
			fd = fopen(path,"a");
			fprintf(fd, "%s", fileContent);
			fclose(fd);
		}
	}	
	free(fileContent);
	free(tmpPath);
	free(substr);
}

static void  LoadFS(const char *path, int block){	
	FILE *fd = NULL;
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	char str[5];
	sprintf(str, "%d", block);
	char *tmpPath=(char *) malloc(30);
	strcpy(tmpPath, "FileData/data.");
	strcat(tmpPath, str);
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);	
	int tmpComma = 0;
	char *point = strstr(fileContent,"filename_to_inode_dict:{");
	int pos1 = point - fileContent;
	while(fileContent[pos1]!='}' && tmpComma < 2){
		if(fileContent[pos1]==','){ 
			tmpComma++;
		} pos1++;
	}	
	int pos2 = point - fileContent;
	if(fileContent[pos1]=='}'){return;}
	pos1--;	
	char *command = (char *) malloc(512);
	char *paths = (char *) malloc(512);
	char *tmp = (char *) malloc(30);		
	strcpy(paths,path);
	printf("paths %s\n", paths);
	while(fileContent[pos1]!='}'){
		if(fileContent[pos1] == ','){			
			if(fileContent[pos1+1] == 'd'){
				pos2 = pos1+3;				
				while(fileContent[pos2++]!=':'){}
				strcpy(tmp,"");
				strncat(tmp,fileContent+pos1+3 , pos2-pos1-4);
				pos1 = pos2;
				while(1){					
					if(fileContent[pos2]==',' || fileContent[pos2]=='}'){break;}pos2++;
				}
				strncpy(str,fileContent+pos1 , pos2-pos1);
				block = atoi(str);				
				strcat(paths,"/");
				strcat(paths,tmp);				
				strcpy(command,"mkdir -p \"");
				strcat(command,paths);
				strcat(command,"\"");
				system(command);
				LoadFS(paths,block);				
				pos1 = pos2;								
			}else if(fileContent[pos1+1] == 'f'){				
				pos2 = pos1+3;
				while(fileContent[pos2++]!=':'){}
				strcpy(tmp,"");
				strncat(tmp,fileContent+pos1+3 , pos2-pos1-4);
				pos1 = pos2;				
				while(1){					
					if(fileContent[pos2]==',' || fileContent[pos2]=='}'){break;}pos2++;
				}				
				strncpy(str,fileContent+pos1 , pos2-pos1);
				block = atoi(str);	
				strcpy(command,path);
				strcat(command,"/");
				strcat(command,tmp);					
				MakeFiles(command, block);				
				pos1 = pos2;
			}
		}
	}
	free(fileContent);
	free(paths);
	free(tmp);
	free(command);

}


int main(int argc, char *argv[]){
	printf("aca\n");
    my_init();
    
	system("rm -rf /tmp/*");
	LoadFS("/tmp",26);				
	if(argc != 1){
		printf("Usage : ./mkfs [mount point] [password]\n");
		return -1;
	}
	system("rm -rf BWFS");
	system("mkdir -p BWFS");
	argv[argc++] = "BWFS";
	umask(0);
}