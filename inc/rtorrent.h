#ifndef RTORRENT_H
#define RTORRENT_H

typedef struct annlist{
    char ann[128];
    struct annlist *next;
}annlist;


typedef struct file{
    char path[256];
    long len;
    struct file *next;
}file;


int readtorr(char *torrent);
int findkey(char *key,long *pos);
int getann();
int addann(char *url);

int getpilen();
int getpihash();

int ismulti();
int getfname();
int getflen();
int getflap();


int getpeid();
int getinhash();

void freemtor();
int rtorrent(char *torrent);

#endif
