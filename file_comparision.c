#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include<string.h>
#include <unistd.h>

int main()
{
    int fd1 = open("data",O_RDONLY);
    if(fd1 <0)
    {   
        perror("\n\n!Open System Call Error!\n\n");
        exit(0);
    }
    int fd2 = open("shared_mem_content",O_RDONLY);
    if(fd1 <0)
    {
        perror("\n\n!Open System Call Error!\n\n");
        exit(0);
    }

    char buf1[100],buf2[100];
    ssize_t rb1,rb2;

    while((rb1=read(fd1,buf1,sizeof(buf1)))>0 && (rb2=read(fd2,buf2,sizeof(buf2)))>0)
    {
        if(strcmp(buf1,buf2)!=0)
        break;
    }
    if(rb1 >0 || rb2>0)
        printf("\nNot Same Content\n");
    else
    printf("\nSame Content on both files\n");
    
    return 0;

}