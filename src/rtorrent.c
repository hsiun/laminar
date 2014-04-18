#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rtorrent.h"
#include "sha1.h"


char *torrent_content = NULL;
long torrent_length = -1;

int piece_length = 0;
char *pieces_hash = NULL;
int pieces_length = 0;

int ismultifile = 0;
char *files = NULL;
long long file_size = 0;
file *file_head = NULL;

unsigned char info_hash[20];
unsigned char perr_id[20];

annlist *annlist_head = NULL;


int readtorr(char *torrent){
    long i;
    FILE *fp = fopen(torrent,"rb");
    if (fp == NULL){
        printf("%s:%d can not open file...\n",__FILE__,__LINE__);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);

    torrent_length = ftell(fp);

    if (torrent_length == -1){
        printf("%s:%d get file length failed...\n",__FILE__,__LINE__);
        return -1;
    }
    
    torrent_content = (char *)malloc(torrent_length+1);
    if (torrent_content == NULL){
        printf("%s:%d malloc failed...\n",__FILE__,__LINE__);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);
    for (i=0; i<torrent_length; i++)
        torrent_content[i] = fgetc(fp);
    torrent_content[i] = '\0';

    fclose(fp);

#ifdef DEBUG
    printf("torrent file size is: %ld\n",torrent_length);
#endif
    return 0;

}

int rtorrent(char *torrent){
    int ret;

    ret = readtorr(torrent);
    if (ret < 0) {
        printf("%s:%d wrong",__FILE__,__LINE__);
        return -1;
    }

    return 0;
}
