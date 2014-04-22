#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rtorrent.h"
#include "sha1.h"


#define DEBUG

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

int findkey(char *key, long *pos){
    long i;

    *pos = -1;
    if (key == NULL){
        return 0;
    }
    for (i=0; i<torrent_length-strlen(key); i++){
        if(memcmp(&torrent_content[i], key, strlen(key)) == 0){
            *pos = i;
            return 1;
        }
    }

    return 0;

}

int getann(){
    annlist *node = NULL,*p = NULL;
    int len = 0;
    long i = 0;

    if (findkey("13:announce-list",&i) == 0){                 //没有找到13:annouce-list
        if (findkey("8:announce",&i) == 1) {
            i = i + strlen("8:announce");
            while (isdigit(torrent_content[i])){               //ctype所包含的库函数
                len = len * 10 + ( torrent_content[i]-'0');    
                i++;
            }
            i++;

            node = (annlist *)malloc(sizeof(annlist));
            strncpy(node->ann,&torrent_content[i],len);
            node->ann[len] = '\0';
            node->next = NULL;
            annlist_head = node;

        }
    }
    else{
        i = i + strlen("13:announce-list");
        i++;
        while (torrent_content[i] != 'e'){
            i++;
            while (isdigit(torrent_content[i])){
                len = len*10 + (torrent_content[i]-'0');
                i++;
            }
            if(torrent_content[i]==':') i++;
            else return -1;

            if (memcmp(&torrent_content[i],"http",4) == 0){
                node = (annlist *)malloc(sizeof(annlist));
                strncpy(node->ann,&torrent_content[i],len);
                node->ann[len] = '\0';
                node->next = NULL;
                if (annlist_head == NULL){
                    node->next = NULL;
                    annlist_head = node;

                }
                else {
                    p = annlist_head;
                    while ( p->next != NULL){
                        p =  p->next;
                    }
                    p->next = node;

                }
            }
            i = i + len;
            len = 0;
            i++;
            if (i >= torrent_length)
                return -1;

        }
    }


#ifdef DEBUG
    p = annlist_head;
    while (p != NULL){
        printf("%s\n",p->ann);
        p = p->next;
    }
#endif
    return 0;
}

/*获取重定向的url*/
int addann(char *url){
    annlist *p,*q;
    p = annlist_head;

    while (p != NULL){
        if(strcmp(p->ann,url) == 0)   //找到了和url相匹配的tarcker
            break;
        p = p -> next;
    }

    if (p != NULL)                    //如果p不为NULL，则说明url和已有的tracker重叠了
        return 0;

    q = (annlist *)malloc(sizeof(annlist)); //为放入annlist_head中url申请空间
    strcpy(q->ann,url);
    q->next = NULL;

    p = annlist_head;
    if (p == NULL){                   //如果annlist_head不存在，则将p赋予它
        annlist_head = q;
        return 1;
    }
    
    while (p->next != NULL)           //存在则遍历到最后，将p加在annlist_head的后面
        p = p->next;

    p->next = q;

    return 1;

}

int ismulti(){
    long i;

    if (findkey("5:files",&i) == 1){                //找到5:files关键字表面是多文件
        ismultifile = 1;
        return 1;
    }

#ifdef DEBUG
    printf("is multi files:%d\n",ismultifile);     //此调试语句只在是单文件是有效
#endif

    return 0;
}

/*获取piece的长度*/
int getpilen(){
    long i;

    if (findkey("12:piece length",&i) == 1){
        i = i + strlen("12:piece length");
        i++;
        while(torrent_content[i] != 'e'){
            piece_length = piece_length * 10 + (torrent_content[i] - '0');
            i++;
        }
    }else{
        return -1;
    }

#ifdef DEBUG
    printf("piece length:%d\n",piece_length);
#endif
    return 0;
}

/*获取piece的hash直*/
int getpihash(){
    long i;

    if (findkey("6:pieces", &i) == 1){
        i = i + strlen("6:pieces");
        while(torrent_content[i] != ':'){
            pieces_length = pieces_length * 10 + (torrent_content[i] - '0');
            i++;
        }
        i++;
        pieces_hash = (char *)malloc(pieces_length + 1);
        memcpy(pieces_hash,&torrent_content[i],pieces_length);
        pieces_hash[pieces_length] = '\0';
    }
    else {
        return -1;
    }

#ifdef DEBUG
    printf("get pieces hash ok\n");
#endif
    return 0;
}


/*获取文件的名字*/
int getfname(){
    long i;
    int count = 0;

    if (findkey("4:name", &i) == 1){
        i = i + strlen("4:name");
        while(torrent_content[i] != ':'){
            count = count * 10 + (torrent_content[i] - '0');
            i++;
        }
        i++;

        files = (char *)malloc(count+1);
        memcpy(files,&torrent_content[i],count);
        files[count] = '\0';
    }else{
        return -1;
    }

#ifdef DEBUG
    printf("files name:%s\n",files);
#endif

    return 0;
}

/*获取文件的长度*/
int getflen(){
    long i;
    file *p;

    if (ismulti() == 1){
        if (file_head == NULL){
           getflap(); 
        }
        p = file_head;
        while (p != NULL){
            file_size += p->len;
            p = p -> next;
        }
    }else {
        if (findkey("6:length",&i) == i) {
            i = i + strlen("6:length");
            i++;                        //跳过i
            while(torrent_content[i] != 'e'){
                file_size = file_size * 10 + torrent_content[i] - '0';
                i++;
            }
        }
    }

#ifdef DEBUG
    printf("file length:%lld\n",file_size);
#endif

    return 0;
}

/*对于多文件，获取文件的长度和名字*/
int getflap(){

    return 0;
}

int rtorrent(char *torrent){
    int ret;

    ret = readtorr(torrent);
    if (ret < 0) {
        printf("%s:%d wrong:",__FILE__,__LINE__);
        return -1;
    }

    ret = getann();
    if (ret < 0) {
        printf("%s:%d wrong:",__FILE__,__LINE__);
        return -1;
    }

    ret = ismulti();
    if (ret < 0) {
        printf("%s:%d wrong:",__FILE__,__LINE__);
        return -1;
    }

    ret = getpilen();
    if (ret < 0) {
        printf("%s:%d wrong:",__FILE__,__LINE__);
        return -1;
    }

    ret = getpihash();
    if (ret < 0) {
        printf("%s:%d wrong:",__FILE__,__LINE__);
        return -1;
    }

    ret = getfname();
    if (ret < 0) {
        printf("%s:%d wrong:",__FILE__,__LINE__);
        return -1;
    }

    return 0;
}
