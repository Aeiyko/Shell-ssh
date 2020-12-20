#ifndef MYLS_HEAD
#define MYLS_HEAD

#define BLOCK 512
#define ERR -1
#define ALLOC 30
#define SIX_MONTH 6*31*24*3600
#define DATE_LEN 13
#define OPEN_ERR 1
#define WRITE_ERR 2
#define STAT_ERR 3
#define SCN_ERR 4
#define LNK_ERR 5
#define RLN_ERR 6

char* msg_error[] = {
  "ls : open error on file %s : %s\n",
  "ls : write error on %c : %s\n",
  "ls : stat error on %s : %s\n",
  "ls : scandir error on %s : %s\n",
  "ls : link error on %s : %s\n",
  "ls : readlink error on %s : %s\n"
};

#define syserror(n, s) fprintf(stderr, msg_error[n-1], s, strerror(errno));

typedef struct _fileInfo{
  char* entirePath;
  char* fileName;
  char type;
  char* usr_name;
  char* grp_name;
  uid_t usr;
  gid_t grp;
  char* link;
  int type_link;
  mode_t mode_link;
  unsigned int min;
  unsigned int maj;
  off_t size;
  mode_t mode;
  nlink_t nlinks;
  time_t time;
  int quotes;
  int spaces;
  int len_usr;
  int len_grp;
}fileInfo;

typedef struct _sizeSpace{
  blkcnt_t nb_blocks;
  int len_link;
  int len_usr;
  int len_grp;
  int len_size;
  int len_min; //minor si fichier special
  int len_maj; //major si fichier special
  int len_tab;
  int quotes; //si un nom de fichier contient un espace
}sizeSpace;

typedef struct _dirContent{
    fileInfo* files;
    char* dirName;
    int size;
    sizeSpace spaces;
}dirContent;



//free_fileInfo(fileInfo f); //libère le contenu d'un fileInfo


#endif
