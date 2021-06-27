
#define FUSE_USE_VERSION 26
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <fuse.h>
#include <ulockmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include <sys/file.h> 

#include "superblock.h"

static int freeblocks[MAX_BLOCKS] = { 0 };
static int size = 0;
static int write_buf_flag = 0;
static char *buffer;



static void makedir(const char *path){	
	/*char *p = strstr(path,"/.");	
	int p1 =p-path;
	if(p1>=0){printf("returned\n");return;}	*/
	int block = getOneBlock();
	setFreeBlocks(block);
	int rootblocknum = getThatBlock(path);
	if(block == -1){
		printf("file system filled up");
		return;
	}
	char str[5];
	sprintf(str, "%d", block);
	FILE *fd = NULL;
	time_t seconds = time(NULL);
	char *tmpPath=(char *) malloc(30);
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);	
	fd = fopen(tmpPath,"w");	
	fprintf(fd,"{size:0,uid:1000,gid:1000,mode:16877,atime:%ld,ctime:%ld,mtime:%ld,linkcount:2,filename_to_inode_dict:{d:.:%d,d:..:%d}}",seconds,seconds,seconds,block,rootblocknum);
	fclose(fd);	
	sprintf(str, "%d", rootblocknum);		
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);
	char *point = strstr (fileContent,"size:");
	int pos1 = point - fileContent + 5;
	int pos2 = pos1;
	while(fileContent[pos2++]!=','){}
	char* substr  = (char *) malloc(BLOCK_SIZE);	
	strncpy(substr, fileContent+pos1, pos2-pos1-1);	
	int sizeblock = atoi(substr);	
	point = strstr (fileContent,"linkcount:");
	pos1 = point - fileContent + 10;
	pos2 = pos1;
	while(fileContent[pos2++]!=','){}
	strncpy(substr, fileContent+pos1, pos2-pos1-1);	
	int links = atoi(substr);
	point = strstr (fileContent,"filename_to_inode_dict:{");
	pos1 = point - fileContent + 24;
	pos2 = pos1;
	while(fileContent[pos2++]!='}'){}
	strcpy(substr,"");
	strncat(substr, fileContent+pos1, pos2-pos1-1);
	char* lastPath  = (char *) malloc(20);
	int tmp =0;
	char ch[20];
	int i = 0;
	while(path[i]!='\0'){
		if(path[i]=='/'){
			i++;
			for(tmp = 0 ; tmp < 20 ; tmp++){
				ch[tmp] = '\0';
			}		
			tmp = 0;			
		}
		ch[tmp++]=path[i++];
	}
	strcpy(lastPath,ch);
	strcat(substr,",d:");
	strcat(substr,lastPath);
	strcat(substr,":");
	sprintf(str, "%d", block);
	strcat(substr,str);	
	fd = fopen(tmpPath,"w");	
	fprintf(fd, "{size:%d,uid:1000,gid:1000,mode:16877,atime:%ld,ctime:%ld,mtime:%ld,linkcount:%d,filename_to_inode_dict:{%s}}",(sizeblock+1),seconds,seconds,seconds,(links+1),substr);
	fclose(fd);
	free(fileContent);
	free(substr);	
	free(lastPath);
	free(tmpPath);
}

static void removeFile(int block){	
	FILE *fd = NULL;
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	char str[5];
	sprintf(str, "%d", block);
	char *tmpPath=(char *) malloc(30);
	strcpy(tmpPath, "FileSysData/fusedata.");
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
	if(ch  == '0'){
		addFreeBlocks(block);
		int tmp = atoi(substr);
		addFreeBlocks(tmp);
	}else{
		addFreeBlocks(block);
		strcpy(tmpPath, "FileSysData/fusedata.");
		strcat(tmpPath, substr);
		strcpy(fileContent,"");	
		fd = fopen(tmpPath,"r");
		fscanf(fd,"%[^\n]s", fileContent);
		fclose(fd);
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
			addFreeBlocks(listofFile[i]);	
		}
	}	
	free(fileContent);
	free(tmpPath);
	free(substr);

}

static void removeDirectories(int block){	
	addFreeBlocks(block);
	FILE *fd = NULL;
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	char str[5];
	sprintf(str, "%d", block);
	char *tmpPath=(char *) malloc(30);
	strcpy(tmpPath, "FileSysData/fusedata.");
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
	while(fileContent[pos1]!='}'){
		if(fileContent[pos1] == ','){
			if(fileContent[pos1+1] == 'd'){
				pos2 = pos1+3;				
				while(fileContent[pos2++]!=':'){}				
				pos1 = pos2;
				while(1){					
					if(fileContent[pos2]==',' || fileContent[pos2]=='}'){break;}pos2++;
				}
				strncpy(str,fileContent+pos1 , pos2-pos1);
				block = atoi(str);				
				removeDirectories(block);				
				pos1 = pos2;								
			}else if(fileContent[pos1+1] == 'f'){
				pos2 = pos1+3;
				while(fileContent[pos2++]!=':'){}
				pos1 = pos2;				
				while(1){					
					if(fileContent[pos2]==',' || fileContent[pos2]=='}'){break;}pos2++;
				}
				strncpy(str,fileContent+pos1 , pos2-pos1);
				block = atoi(str);			
				removeFile(block);				
				pos1 = pos2;
			}
		}
	}
	free(fileContent);	
}

static void removeDir(const char *path, int fileDir){	
	int rootblocknum = getThatBlock(path);
	if(rootblocknum == -1){
		printf("not present");
		return -1;
	}	
	time_t seconds = time(NULL);
	char str[5];
	char *tmpPath=(char *) malloc(30);
	sprintf(str, "%d", rootblocknum);		
	strcat(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	FILE *fd = NULL;
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);
	char *point = strstr (fileContent,"size:");
	int pos1 = point - fileContent + 5;
	int pos2 = pos1;
	while(fileContent[pos2++]!=','){}
	char* substr  = (char *) malloc(BLOCK_SIZE);	
	strncpy(substr, fileContent+pos1, pos2-pos1-1);	
	int sizeblock = atoi(substr);	
	point = strstr (fileContent,"linkcount:");
	pos1 = point - fileContent + 10;
	pos2 = pos1;
	while(fileContent[pos2++]!=','){}
	strncpy(substr, fileContent+pos1, pos2-pos1-1);	
	int links = atoi(substr);
	char* lastPath  = (char *) malloc(20);
	int tmp =2;
	char ch[20];
	if(fileDir == 0){
		ch[0] = 'd';
	}else{
		ch[0] = 'f';
	}
	ch[1] = ':';
	int i = 0;
	while(path[i]!='\0'){
		if(path[i]=='/'){
			i++;
			for(tmp = 2 ; tmp < 20 ; tmp++){
				ch[tmp] = '\0';
			}
			tmp = 2;			
		}
		ch[tmp++]=path[i++];
	}	
	strcpy(lastPath,ch);	
	point = strstr (fileContent,lastPath);	
	int posBlock = point - fileContent + strlen(lastPath)+1;
	for(tmp = 0 ; tmp < 20 ; tmp++){
		ch[tmp] = '\0';
	}tmp=0;
	while(1){
		if(fileContent[posBlock]=='}'|| fileContent[posBlock]==','){break;}
		ch[tmp++] = fileContent[posBlock++];
	}		
	int block = atoi(ch);
	printf("block %d\n", block);
	if(fileDir == 0){
		removeDirectories(block);
	}else{
		removeFile(block);
	}	
	char *point2 = strstr (fileContent,"filename_to_inode_dict:{");
	pos1 = point - fileContent;
	pos2 = point2 - fileContent + 24;
	int pos3 = pos1;
	while(1){
		if(fileContent[pos3]=='}' || fileContent[pos3]==','){
			break;
		}pos3++;
	}
	strncpy(substr, fileContent+pos2, pos1-pos2-1);
	strcat(substr, fileContent+pos3);			
	fd = fopen(tmpPath,"w");
	fprintf(fd, "{size:%d,uid:1000,gid:1000,mode:16877,atime:%ld,ctime:%ld,mtime:%ld,linkcount:%d,filename_to_inode_dict:{%s",(sizeblock-1),seconds,seconds,seconds,(links-1),substr);			
	fclose(fd);
	free(fileContent);
	free(substr);	
	free(lastPath);
	free(tmpPath);
}	

static void renameDir(const char *from, const char *to){	
	char *p = strstr(from,"/.goutputstream");	
	int p1 =p-from;
	if(p1>=0){printf("renmae returned\n");return;}
	int rootblocknum = getThatBlock(from);
	char* lastPath  = (char *) malloc(20);
	int tmp =0;
	char ch[20];	
	int i = 0;
	while(from[i]!='\0'){
		if(from[i]=='/'){
			i++;
			for(tmp = 0 ; tmp < 20 ; tmp++){
				ch[tmp] = '\0';
			}
			tmp = 0;			
		}
		ch[tmp++]=from[i++];
	}		
	strcpy(lastPath,ch);	
	char str[5];
	char *tmpPath=(char *) malloc(30);
	sprintf(str, "%d", rootblocknum);		
	strcat(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	FILE *fd = NULL;
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);
	char *point = strstr (fileContent,"filename_to_inode_dict:{");
	int pos1 = point - fileContent ;
	char* substr  = (char *) malloc(BLOCK_SIZE);
	strncat(substr, fileContent, pos1+24);
	point = strstr (fileContent,lastPath);
	int pos2 = point - fileContent;	
	strncat(substr,  fileContent+pos1+24,pos2-pos1-24);
	while(fileContent[pos2++]!=':'){}
	tmp =0;
	char ch1[20];	
	i = 0;
	while(to[i]!='\0'){
		if(to[i]=='/'){
			i++;
			ch1[tmp++]='\0';
			tmp = 0;			
		}
		ch1[tmp++]=to[i++];
	}			
	strcat(substr,  ch1);
	strcat(substr,  fileContent+pos2-1);
	fd = fopen(tmpPath,"w");	
	fprintf(fd, "%s", substr);
	fclose(fd);
	free(fileContent);
	free(substr);	
	free(lastPath);
	free(tmpPath);	
}

static void makeFile(const char *path){	
	char *p = strstr(path,"/.");	
	int p1 =p-path;
	if(p1>=0){printf("freturned\n");return;}
	int block = getOneBlock();
	setFreeBlocks(block);
	int blockfile = getOneBlock();	
	setFreeBlocks(blockfile);
	int rootblocknum = getThatBlock(path);
	if(block == -1){
		printf("file system filled up");
		return;
	}
	char str[5];
	sprintf(str, "%d", block);
	FILE *fd = NULL;
	time_t seconds = time(NULL);
	char *tmpPath=(char *) malloc(30);
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	fd = fopen(tmpPath,"w");
	fprintf(fd,"{size:0,uid:1,gid:1,mode:33261,linkcount:2,atime:%ld,ctime:%ld,mtime:%ld,indirect:0,location:%d}",seconds,seconds,seconds,blockfile);
	fclose(fd);	
	sprintf(str, "%d", rootblocknum);		
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);
	char *point = strstr (fileContent,"size:");
	int pos1 = point - fileContent + 5;
	int pos2 = pos1;
	while(fileContent[pos2++]!=','){}
	char* substr  = (char *) malloc(BLOCK_SIZE);	
	strncpy(substr, fileContent+pos1, pos2-pos1-1);	
	int sizeblock = atoi(substr);	
	point = strstr (fileContent,"linkcount:");
	pos1 = point - fileContent + 10;
	pos2 = pos1;
	while(fileContent[pos2++]!=','){}
	strncpy(substr, fileContent+pos1, pos2-pos1-1);	
	int links = atoi(substr);
	point = strstr (fileContent,"filename_to_inode_dict:{");
	pos1 = point - fileContent + 24;
	pos2 = pos1;
	while(fileContent[pos2++]!='}'){}
	strcpy(substr,"");
	strncat(substr, fileContent+pos1, pos2-pos1-1);
	char* lastPath  = (char *) malloc(20);
	int tmp =0;
	char ch[20];
	int i = 0;
	while(path[i]!='\0'){
		if(path[i]=='/'){
			i++;
			for(tmp = 0 ; tmp < 20 ; tmp++){
				ch[tmp] = '\0';
			}		
			tmp = 0;			
		}
		ch[tmp++]=path[i++];
	}	
	strcpy(lastPath,ch);
	strcat(substr,",f:");
	strcat(substr,lastPath);
	strcat(substr,":");
	sprintf(str, "%d", block);
	strcat(substr,str);	
	fd = fopen(tmpPath,"w");
	fprintf(fd, "{size:%d,uid:1000,gid:1000,mode:16877,atime:%ld,ctime:%ld,mtime:%ld,linkcount:%d,filename_to_inode_dict:{%s}}",(sizeblock+1),seconds,seconds,seconds,(links+1),substr);
	fclose(fd);
	free(fileContent);
	free(substr);	
	free(lastPath);
	free(tmpPath);
}

static void writeToFile(const char *path){
	char *p = strstr(path,"/.goutputstream");	
	int p1 =p-path;
	if(p1>=0){printf("freturned\n");return;}
	printf("in write %s\n",buffer);
	int rootblocknum = getThatBlock(path);		
	printf("after rootblock\n");
	char str[5];
	sprintf(str, "%d", rootblocknum);	
	FILE *fd = NULL;
	char* lastPath  = (char *) malloc(20);	
	char *tmpPath=(char *) malloc(30);
	int tmp =2;
	char ch[20];
	ch[0] = 'f';
	ch[1] = ':';
	int i = 0;	
	while(path[i]!='\0'){
		if(path[i]=='/'){
			i++;
			for(tmp = 2 ; tmp < 20 ; tmp++){
				ch[tmp] = '\0';
			}
			tmp = 2;			
		}
		ch[tmp++]=path[i++];
	}		
	strcpy(lastPath,ch);	
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,str);
	char *fileContent = (char *) malloc(BLOCK_SIZE);	
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);	
	printf("%s lat %s\n", fileContent,lastPath);
	char* filenum  = (char *) malloc(5);	
	char *point = strstr (fileContent,lastPath);
	int pos1 = point - fileContent + strlen(lastPath)+1;
	int pos2 = pos1;
	while(1){
		if(fileContent[pos2]=='}' || fileContent[pos2]==','){
			break;
		}pos2++;
	}
	printf("after getting file block number\n");
	strcpy(filenum,"");
	strncat(filenum, fileContent+pos1, pos2-pos1);	
	printf("filenum %s\n",filenum);
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,filenum);
	printf("going to get size of buffer\n");
	int size = strlen(buffer);
	printf("size %d\n",size);
	int numFiles = size/BLOCK_SIZE +1;	
	printf("opeingn %s\n", tmpPath);
	fd = fopen(tmpPath,"r");
	fscanf(fd, "%[^\n]s", fileContent);
	fclose(fd);
	printf("%s\n",fileContent );
	point = strstr (fileContent,"indirect:");
	pos1 = point - fileContent + 9;
	if(numFiles <= 1){
		fileContent[pos1] = '0';
	}else{
		fileContent[pos1] = '1';
	}	
	fd = fopen(tmpPath,"w");
	fprintf(fd, "%s", fileContent);
	fclose(fd);
	point = strstr (fileContent,"location:");
    pos1 = point - fileContent + 9;
	pos2 = pos1;
	while(fileContent[pos2++]!='}'){}
	strcpy(filenum,"");
	strncat(filenum, fileContent+pos1, pos2-pos1-1);	
	strcpy(tmpPath,"FileSysData/fusedata.");
	strcat(tmpPath,filenum);
	printf("buffer %s\n", buffer);		
	if(numFiles == 1){
		fd = fopen(tmpPath,"w");
		fprintf(fd, "%s", buffer);
		fclose(fd);
	}else{
		fd = fopen(tmpPath,"r");
		fscanf(fd, "%[^\n]s", fileContent);
		fclose(fd);
		int tmpnum = 0;
		i = 0;
		while(fileContent[i]!='\0'){
			if(fileContent[i]==','){
				tmpnum++;
			}i++;	
		}
		int tmplistofFile[numFiles];
		fd = fopen(tmpPath,"r");
		fscanf(fd, "%[^\n]s", fileContent);
		fclose(fd);		
		i = 1;
		pos1 = 0;
		for(tmp = 0 ; tmp < 5 ; tmp++){
			str[tmp] = '\0';
		}tmp = 0;
		while(fileContent[i]!='}'){
			if(fileContent[i]==','){
				i++;
				tmplistofFile[pos1++] = atoi(str);
				for(tmp = 0 ; tmp < 5 ; tmp++){
					str[tmp] = '\0';
				}tmp=0;
				if(fileContent[i]=='}'){break;}
			}
			str [tmp++] = fileContent[i++];
		}
		for(i =0 ; i < pos1 ; i++){	
			addFreeBlocks(tmplistofFile[i]);
		}

		while(tmpnum <= numFiles){
			char* substr  = (char *) malloc(BLOCK_SIZE);			
			strcpy(substr,"");
			strncat(substr, fileContent,strlen(fileContent)-1);
			int block = getOneBlock();
			setFreeBlocks(block);
			sprintf(str, "%d", block);
			strcat(substr, str);
			strcat(substr, ",}");			
			fd = fopen(tmpPath,"w");
			fprintf(fd, "%s", substr);
			fclose(fd);
			free(substr);
			tmpnum++;
		}
		int listofFile[numFiles];
		fd = fopen(tmpPath,"r");
		fscanf(fd, "%[^\n]s", fileContent);
		fclose(fd);		
		i = 1;
		pos1 = 0;
		for(tmp = 0 ; tmp < 5 ; tmp++){
			str[tmp] = '\0';
		}tmp = 0;
		while(fileContent[i]!='}'){
			if(fileContent[i]==','){
				i++;
				listofFile[pos1++] = atoi(str);
				for(tmp = 0 ; tmp < 5 ; tmp++){
					str[tmp] = '\0';
				}tmp=0;
				if(fileContent[i]=='}'){break;}
			}
			str [tmp++] = fileContent[i++];
		}		
		for(i =0 ; i < pos1 ; i++){	
			char *filePath=(char *) malloc(30);
			strcpy(filePath,"FileSysData/fusedata.");
			strcat(filePath,listofFile[i]);
			strncpy(fileContent,(buffer + (4096*i)),4096);
			fd = fopen(filePath,"w");
			fprintf(fd, "%s", fileContent);
			fclose(fd);	
			free(filePath);
			free(fileContent);			
		}				
	}
	free(fileContent);
	free(filenum);	
	free(lastPath);
	free(tmpPath);
	free(buffer);
}

static int my_getattr(const char *path, struct stat *stbuf){
	int res;	
	if(path[0] == '/' && path[1] == 't' && path[2] == 'm' && path[3] == 'p'){
		printf("%s\n",path);
	}
	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;
	return 0;
}
struct amay_dirp {
	DIR *dp;
	struct dirent *entry;
	off_t offset;
};

static int my_opendir(const char *path, struct fuse_file_info *fi){
	int res;
	struct amay_dirp *d = malloc(sizeof(struct amay_dirp));
	if (d == NULL)
		return -ENOMEM;
	d->dp = opendir(path);
	if (d->dp == NULL) {
		res = -errno;
		free(d);
		return res;
	}
	d->offset = 0;
	d->entry = NULL;
	fi->fh = (unsigned long) d;
	return 0;
}

static inline struct amay_dirp *get_dirp(struct fuse_file_info *fi){
	return (struct amay_dirp *) (uintptr_t) fi->fh;
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi){
	struct amay_dirp *d = get_dirp(fi);
	(void) path;
	if (offset != d->offset) {
		seekdir(d->dp, offset);
		d->entry = NULL;
		d->offset = offset;
	}
	while (1) {
		struct stat st;
		off_t nextoff;

		if (!d->entry) {
			d->entry = readdir(d->dp);
			if (!d->entry)
				break;
		}
		memset(&st, 0, sizeof(st));
		st.st_ino = d->entry->d_ino;
		st.st_mode = d->entry->d_type << 12;
		nextoff = telldir(d->dp);
		if (filler(buf, d->entry->d_name, &st, nextoff))
			break;
		d->entry = NULL;
		d->offset = nextoff;
	}
	return 0;
}

static int my_mkdir(const char *path, mode_t mode){
	int res;
	if(path[0] == '/' && path[1] == 't' && path[2] == 'm' && path[3] == 'p'){
		printf("Making Dir %s\n",path);	
		makedir(path);
		res = mkdir(path, mode);
		if (res == -1)
			return -errno;
	}else{
		printf("==CANNOT DO THAT NOT OUTSIDE /tmp Dir\n");		
	}
	return 0;
}

static int my_unlink(const char *path){
	int res;		
	res = unlink(path);	
	printf("unlinked\n");
	removeDir(path,1);	
	if (res == -1)
		return -errno;
	return 0;
}

static int my_rmdir(const char *path){
	int res;
	if(path[0] == '/' && path[1] == 't' && path[2] == 'm' && path[3] == 'p'){
		printf("Removing Dir %s\n",path);	
		removeDir(path,0);
		res = rmdir(path);
		if (res == -1)
			return -errno;
	}else{
		printf("CANNOT DO THAT NOT OUTSIDE /tmp Dir\n");
		return 0;
	}
}

static int my_rename(const char *from, const char *to){
	int res;
	printf("Renaming %s to %s\n",from,to);	
	if(write_buf_flag == 0){
		renameDir(from, to);	
	}else{
		writeToFile(to);
	}		
	res = rename(from, to);
	if (res == -1)
		return -errno;
	return 0;
}

static int my_create(const char *path, mode_t mode, struct fuse_file_info *fi){	
	if(path[0] == '.'){
		printf("Don't put in dot(.)\n");	
		return;	
	}
	int fd;
	fd = open(path, fi->flags, mode);	
	makeFile(path);
	if (fd == -1)
		return -errno;
	fi->fh = fd;
	return 0;
}

static int my_open(const char *path, struct fuse_file_info *fi){
	int fd;	
	fd = open(path, fi->flags);
	if (fd == -1)
		return -errno;
	fi->fh = fd;
	return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi){
	int res;
	(void) path;
	res = pread(fi->fh, buf, size, offset);
	if (res == -1)
		res = -errno;
	return res;
}

static int my_read_buf(const char *path, struct fuse_bufvec **bufp,
			size_t size, off_t offset, struct fuse_file_info *fi){
	struct fuse_bufvec *src;
	(void) path;
	src = malloc(sizeof(struct fuse_bufvec));
	if (src == NULL)
		return -ENOMEM;
	*src = FUSE_BUFVEC_INIT(size);
	src->buf[0].flags = FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK;
	src->buf[0].fd = fi->fh;
	src->buf[0].pos = offset;
	*bufp = src;
	return 0;
}

static int my_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi){
	int res;
	(void) path;	
	res = pwrite(fi->fh, buf, size, offset);
	printf("in %s\n", buf);
	buffer = (char *) malloc(strlen(buf));	
	strcpy(buffer,buf);	
	printf("going to write %s",buf);	
	writeToFile(path);	
	write_buf_flag=1;
	if (res == -1)
		res = -errno;
	return res;
}

static int my_statfs(const char *path, struct statvfs *stbuf){
	int res;
	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;
	return 0;
}

static struct fuse_operations my_oper = {
	.getattr	= my_getattr,
	.opendir	= my_opendir,
	.readdir	= my_readdir,
	.mkdir		= my_mkdir,
	.unlink		= my_unlink,
	.rmdir		= my_rmdir,
	.rename		= my_rename,
	.create		= my_create,
	.open		= my_open,
	.read		= my_read,
	.write		= my_write,	
	.statfs		= my_statfs,
	.flag_nullpath_ok = 0,       
};

int main(int argc, char *argv[]){
	
	return fuse_main(argc, argv, &my_oper, NULL);
}


