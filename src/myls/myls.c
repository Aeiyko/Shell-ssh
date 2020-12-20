#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <math.h>
#include <errno.h>
#include "color.h"
#include "myls.h"

dirContent *datas;
int indexData=0, nb_allocs=1, r=0, a=0, folders=0;
int exit_code = 0;
char* my_concat(char* fileName, char* subFileName){
  size_t size = strlen(fileName) + strlen(subFileName) + 1;
  char *path = (char*)calloc(sizeof(char), (size+1));
  sprintf(path, "%s%s%s", fileName, "/", subFileName);
  path[size] = '\0';
  return path;
}

void copy_chain(char** dest, char* src){
  *dest = (char*)malloc((strlen(src)+1)*sizeof(char));
  strcpy(*dest, src);
}

int nb_digits(int number){
    return (!number)?(1):(log10((double)number)+1);
}

int contain_symbol(char *chain, char symbol){
  char *s;
  for (s=chain; *s!='\0'; s++) if (*s == symbol) return 1;
  return 0;
}

char get_type(mode_t mode, const char* link){
  if (S_ISLNK(mode) || link) return 'l';
  if (S_ISREG(mode)) return '-';
  if (S_ISDIR(mode)) return 'd';
  if (S_ISCHR(mode)) return 'c';
  if (S_ISBLK(mode)) return 'b';
  if (S_ISFIFO(mode)) return 'p';
  if (S_ISSOCK(mode)) return 's';
  return ' ';
}

int maxi(int a, int b){
    return (a>b)?(a):(b);
}

void prepare_format(fileInfo file, sizeSpace* spaces){
  int nb_min, nb_maj, nb_size, nb_usr, nb_grp, nb_link;
  if (file.type=='b' || file.type == 'c'){
    nb_min=nb_digits(file.min), nb_maj=nb_digits(file.maj);
    if (nb_min > spaces->len_min) spaces->len_min = nb_min;
    if (nb_maj > spaces->len_maj) spaces->len_maj = nb_maj;
  }
  else{
    nb_size = nb_digits(file.size);
    if (nb_size>spaces->len_size) spaces->len_size = nb_size;
  }
  nb_usr = (file.usr_name)?(strlen(file.usr_name)):(nb_digits(file.usr));
  nb_grp = (file.grp_name)?(strlen(file.grp_name)):(nb_digits(file.grp));
  if (nb_usr > spaces->len_usr) spaces->len_usr = nb_usr;
  if (nb_grp > spaces->len_usr) spaces->len_usr = nb_grp;
  nb_link = nb_digits(file.nlinks);
  if (nb_link > spaces->len_link) spaces->len_link = nb_link;
}



void get_type_link(fileInfo* file, char* fatherName){
  struct stat s;
  int desc;
  char *path = (file->link[0]=='/')?(file->link):(my_concat(fatherName, file->link));
  if ((desc=open(path, O_RDONLY)) == ERR){
    exit_code = LNK_ERR;
    syserror(LNK_ERR, path);
    return;
  }
  if (fstat(desc, &s)==ERR) {
      file->type_link = 0;
      return;

  }else file->type_link = 1, file->mode_link = s.st_mode;
  close(desc);
  if (file->link[0]!='/') free(path);
}

void read_link(fileInfo* file, char* fatherName){
  file->link = (char*)calloc(sizeof(char), (BLOCK+1));
  ssize_t s_read = readlink(file->entirePath, file->link, BLOCK);

  if (s_read == ERR){
    if (errno == EINVAL){
      free(file->link);
      file->link = NULL;
      return;
    }
    else {
      exit_code = RLN_ERR;
      syserror(RLN_ERR, file->entirePath);
      return;
    }
  }
  get_type_link(file, fatherName);
}

void get_user(fileInfo* file, sizeSpace *spaces){
  struct passwd *usr = getpwuid(file->usr);
  if (usr) {
    copy_chain(&file->usr_name, usr->pw_name);
    file->len_usr = (int)strlen(file->usr_name);
  }else{
    file->usr_name = NULL;
    file->len_usr = nb_digits(file->usr);
  }
  if (file->len_usr > spaces->len_usr) spaces->len_usr = file->len_usr;
}

void get_group(fileInfo* file, sizeSpace *spaces){
  struct group *grp = getgrgid(file->grp);
  if (grp) {
    copy_chain(&file->grp_name, grp->gr_name);
    file->len_grp = (int)strlen(file->grp_name);
  }else{
    file->grp_name = NULL;
    file->len_grp = nb_digits(file->grp);
  }
  if (file->len_grp > spaces->len_grp) spaces->len_grp = file->len_grp;
}

void get_informations(char *fatherName, fileInfo* file, struct dirent infos, struct stat stats, sizeSpace* spaces){
  file->quotes = contain_symbol(file->fileName, '\'');
  file->spaces = contain_symbol(file->fileName, ' ');
  if (file->quotes || file->spaces) spaces->quotes++;
  file->mode = stats.st_mode;
  file->type = get_type(file->mode, file->link);
  file->min = minor(stats.st_rdev);
  file->maj = major(stats.st_rdev);
  file->size = stats.st_size;
  file->nlinks = stats.st_nlink;
  file->usr = stats.st_uid;
  file->grp = stats.st_gid;
  file->time = stats.st_mtim.tv_sec;
  file->type_link = 0;
  spaces->nb_blocks += stats.st_blocks;
  if (file->link) get_type_link(file, fatherName);
  get_user(file, spaces);
  get_group(file, spaces);
  prepare_format(*file, spaces);
}


void free_folder_infos(fileInfo file){
  free(file.entirePath), free(file.fileName);
  if (file.usr_name) free(file.usr_name);
  if (file.grp_name) free(file.grp_name);
  if (file.link) free(file.link);
}

void print_permissions(mode_t mode){
  printf("%c", (mode&S_IRUSR)?('r'):('-')); //user
  printf("%c", (mode&S_IWUSR)?('w'):('-'));
  printf("%c", (mode&S_ISUID)?('s'):((mode&S_IXUSR)?('x'):('-')));

  printf("%c", (mode&S_IRGRP)?('r'):('-')); //groupe
  printf("%c", (mode&S_IWGRP)?('w'):('-'));
  printf("%c", (mode&S_ISGID)?('s'):((mode&S_IXGRP)?('x'):('-')));

  printf("%c", (mode&S_IROTH)?('r'):('-')); //autre
  printf("%c", (mode&S_IWOTH)?('w'):('-'));
  printf("%c", (mode&S_ISVTX)?('t'):((mode&S_IXOTH)?('x'):('-')));
  printf(" ");
}

void print_user(fileInfo file, sizeSpace spaces){
  int i;
  (file.usr_name)?(printf("%s ", file.usr_name)):(printf("%d ", file.usr));
  for (i=0; i<spaces.len_usr-file.len_usr; i++) printf(" ");
}

void print_group(fileInfo file, sizeSpace spaces){
  int i;
  (file.grp_name)?(printf("%s ", file.grp_name)):(printf("%d ", file.grp));
  for (i=0; i<spaces.len_grp-file.len_grp; i++) printf(" ");
}

void print_major(unsigned int maj, sizeSpace spaces){
  int nb_maj=nb_digits(maj);
  printf("%*.*u, ", nb_maj+spaces.len_maj-nb_maj, nb_maj, maj);
}

void print_minor(unsigned int min, sizeSpace spaces){
   int nb_min = nb_digits(min);
   printf("%*.*u ", nb_min+spaces.len_min-nb_min, nb_min, min);
}

void print_size(off_t size, sizeSpace spaces){
  int n = nb_digits(size);
  printf("%*.*ld ",n+spaces.len_size-n, n, size);
}

void print_nlinks(nlink_t nlinks, sizeSpace spaces){
  int n = nb_digits(nlinks);
  printf("%*.*ld ", n+spaces.len_link-n, n, nlinks);
}

void print_date(time_t s){
  double diff = difftime(time(NULL), s);
  char *d = (char*)malloc(sizeof(char)*DATE_LEN);
  if (diff < SIX_MONTH){
      strftime(d, DATE_LEN,"%b %e %R", localtime(&s));
  }else {
     strftime(d, DATE_LEN, "%b %e  %Y", localtime(&s));
  }
  printf("%s ", d);
  free(d);
}

void print_file(char* fileName, mode_t mode, int quotes, int spaces){
  if (mode&S_ISUID) {printf(COLOR(SURROUGE,quotes,spaces), fileName); return;}
  if (mode&S_ISGID) {printf(COLOR(SURJAUN,quotes,spaces), fileName); return;}
  if (S_ISDIR(mode)) {printf(COLOR(BLEU,quotes,spaces), fileName);return;}
  if (S_ISCHR(mode)) {printf(COLOR(JAUNE,quotes,spaces), fileName);return;}
  if (S_ISBLK(mode)) {printf(COLOR(JAUNE,quotes,spaces), fileName);return;}
  if (S_ISFIFO(mode)) {printf(COLOR(MAGENTA,quotes,spaces), fileName);return;}
  if (S_ISSOCK(mode)) {printf(COLOR(ROUGE,quotes,spaces), fileName);return;}
  if (S_ISREG(mode)) {printf(COLOR(VERT,quotes,spaces), fileName);return;}
}

void print_lnk(fileInfo file, sizeSpace spaces){
  int desc;
  printf(COLOR(CYAN, file.quotes, file.spaces), file.fileName);
  if (file.type_link == 0) return;
  printf(" -> ");

  print_file(file.link, file.mode_link, contain_symbol(file.link, '\''), contain_symbol(file.link, ' '));

}

void print_name_in_color(fileInfo file, sizeSpace spaces){
    if (!file.spaces && !file.quotes && spaces.quotes) printf(" ");
    if (file.link) print_lnk(file, spaces);
    else print_file(file.fileName, file.mode, file.quotes, file.spaces);
}

void print_an_element(fileInfo file, sizeSpace spaces){
    printf("%c", file.type);
    print_permissions(file.mode);
    print_nlinks(file.nlinks, spaces);
    print_user(file, spaces), print_group(file, spaces);
    if (file.type=='c' || file.type=='b') print_major(file.maj, spaces), print_minor(file.min, spaces);
    else print_size(file.size, spaces);
    print_date(file.time);
    print_name_in_color(file, spaces);
    printf("\n");
}

void get_folder_infos(char *folderName){
    int fd, size, i, index=indexData;
    struct dirent** infos;
    struct stat* stats;

    char* path;
    if (indexData==nb_allocs*ALLOC) {
      datas=(dirContent*)realloc(datas, sizeof(dirContent)*(++nb_allocs)*ALLOC);
    }
    
    if ((fd=open(folderName, O_RDONLY))==ERR){
      exit_code = OPEN_ERR;
      syserror(OPEN_ERR, folderName);
      return;
    }

    if ((size=scandir(folderName, &infos, NULL, &alphasort))==ERR){
      exit_code = SCN_ERR;
      close(fd),syserror(SCN_ERR, folderName);
      return;
    }


    datas[indexData].files = (fileInfo*)calloc(sizeof(fileInfo), size);
    stats = (struct stat*)calloc(sizeof(struct stat), size);
    datas[indexData].size = size;

    copy_chain(&datas[indexData++].dirName, folderName);
    for (i=0; i<size; i++){
      if ((!a) && (infos[i]->d_name[0] == '.')) continue;

      copy_chain(&datas[index].files[i].fileName, infos[i]->d_name);
      datas[index].files[i].entirePath = my_concat(folderName, infos[i]->d_name);

      if (lstat(datas[index].files[i].entirePath, stats+i)==ERR){
        exit_code = STAT_ERR;
        syserror(STAT_ERR, infos[i]->d_name);
        continue;
      }
        //imprime erreur mais ne libère rien
      read_link(datas[index].files+i, folderName);
      datas[index].spaces.quotes = 1;

      get_informations(folderName, datas[index].files+i, *infos[i], stats[i], &(datas[index].spaces));

      if (r && S_ISDIR(stats[i].st_mode) && strcmp(".", infos[i]->d_name) && strcmp("..", infos[i]->d_name)) {
        get_folder_infos(datas[index].files[i].entirePath);
      }

      free(infos[i]);
    }
    free(stats);
    free(infos);
    datas[index].spaces.len_size = maxi(datas[index].spaces.len_min+2+datas[index].spaces.len_maj, datas[index].spaces.len_size);

    close(fd);
}

void print_folder_infos(int index){
  int i;
  if (r||folders) printf("%s:\n", datas[index].dirName);
  printf("total %d\n", (int)(datas[index].spaces.nb_blocks>>1));

  for (i=0; i<datas[index].size; i++){
    if ((!a) && datas[index].files[i].fileName == NULL) continue;
    print_an_element(datas[index].files[i], datas[index].spaces);
    free_folder_infos(datas[index].files[i]);
  }

  free(datas[index].files);
  if (indexData>1 && index < indexData-1) printf("\n");
}

void print_folders_infos(){
  int i;
  for (i=0; i<indexData; i++){
    print_folder_infos(i);
  }
}

int list_one_dir(char *fileName, int a, int r, int found){
  datas = (dirContent*)calloc(sizeof(dirContent), ALLOC);

  get_folder_infos(fileName);
  print_folders_infos();
  for (int i=0; i<indexData; i++){
    free(datas[i].dirName);
  }
  free(datas);
}

void ls(int argc, char* argv[]){
  int i;
  char *s;
  for (i=1; i<argc; i++){
    if (argv[i][0] == '-'){
      for (s=argv[i]+1;*s!='\0';s++){
        switch(*s){
          case 'R':
            r=1;
            break;
          case 'a':
            a=1;
            break;
          default:
            fprintf(stderr, "bad parameter : -%c\n", *s),  exit(1);
            break;
        }
      }
    }
    else folders++;
  }
  for (i=1; i<argc; i++) if (argv[i][0] != '-') list_one_dir(argv[i], a, r, folders);
  if (!folders) list_one_dir(".", a, r, 0);
}

int main(int argc, char **argv){
  ls(argc, argv);
  return exit_code;
}
