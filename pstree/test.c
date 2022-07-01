/*#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
*/
/*
int main(int argc, char *argv[]){
	DIR *dir;
	char s[8] = "ro\0";
	printf("That's OK\n");
	char pre[50] = "/p\0";
	printf("That's OK\n");
	char su[3] = "c\0";
	printf("That's OK\n");
	strcat(pre, s);
	printf("That's OK\n");
	printf("Now DIR is %s\n",pre);
	strcat(pre, su);

	printf("Dir is %s", pre);
	dir = opendir(pre);
	struct dirent* ptr;
	while((ptr = readdir(dir)) != NULL)
		printf("Name : %s\n", ptr -> d_name);
	closedir(dir);
	return 0;
}
*/