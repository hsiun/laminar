#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "rtorrent.h"

int main(int argc, char *argv[])
{
    int ret;

    if (argc !=2){
        printf("usage:%s metafile\n",argv[0]);
        exit(-1);
    }

    ret = rtorrent(argv[1]);
    if (ret != 0){
        printf("%s:%d error\n",__FILE__,__LINE__);
        return -1;
    }

    return 0;
}
