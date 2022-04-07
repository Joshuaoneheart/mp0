#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int findFile (char filename[32], char path[256]) {
  struct stat st;
  struct dirent de;
  int fd;
  int flag = 0;
  if((fd = open(path, 0)) < 0){
    return 0;
  }

  if(fstat(fd, &st) < 0){
    close(fd);
    return 0;
  }
  char buf[512];
  switch(st.type){
  case T_FILE:
	  return 0;
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      break;
    }
    path[strlen(path) + 1] = '\0';
    path[strlen(path)] = '/';
    strcpy(buf, path);
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
	buf[strlen(buf) + strlen(de.name)] = '\0';
	memmove(&buf[strlen(buf)], de.name, strlen(de.name));

	int pid = getpid();
	if(!strcmp(filename, de.name)){
		printf("%d as Watson: %s\n", pid, buf);
		flag = 1;
	}
	if(!((strlen(de.name) == 0) || ((de.name[0] == '.') && (de.name[1] == '\0' || de.name[1] == '.'))))
	    flag = flag | findFile(filename, buf);
    	strcpy(buf, path);
    }
    break;
  }
  return flag;
}

int
main(int argc, char *argv[])
{
  char filename[32];
  strcpy(filename, argv[1]);
  int p[2];
  pipe(p);
  if(fork() == 0){
	close(p[0]);
	if(findFile(filename, ".")){
		write(p[1], "y", 2);
	}
	else {
		write(p[1], "n", 2);
	}
  }
  else
  {
	char buf[512];
	read(p[0], buf, 512);
	if(!strcmp(buf, "y")){
		printf("%d as Holmes: This is the evidence\n", getpid());
	}
	else printf("%d as Holmes: This is the alibi\n", getpid());
	close(p[1]);
  }
  wait(0);
  exit(0);
}
