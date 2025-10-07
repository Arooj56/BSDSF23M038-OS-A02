/*
 * ls-v1.5.0  –  Colorized Output
 * Shows colors by file type while keeping column (down-then-across) layout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>

extern int errno;

/* ---------- ANSI COLOR CODES ---------- */
#define RESET   "\033[0m"
#define BLUE    "\033[0;34m"
#define GREEN   "\033[0;32m"
#define RED     "\033[0;31m"
#define PINK    "\033[0;35m"
#define REVERSE "\033[7m"

void do_ls(const char *dir);
void print_down_then_across(char **names,int n,const char *dir);
void print_color_name(const char *path,const char *name);

int main(int argc,char *argv[])
{
    if(argc==1)
        do_ls(".");
    else
        for(int i=1;i<argc;i++){
            printf("%s:\n",argv[i]);
            do_ls(argv[i]);
            if(i+1<argc) puts("");
        }
}

/* read all entries, then print in columns */
void do_ls(const char *dir)
{
    DIR *dp=opendir(dir);
    if(!dp){ perror("opendir"); return; }

    struct dirent *e;
    char **names=NULL; int count=0;

    while((e=readdir(dp))){
        if(e->d_name[0]=='.') continue;
        names=realloc(names,sizeof(char*)*(count+1));
        names[count++]=strdup(e->d_name);
    }
    closedir(dp);

    if(count>0)
        print_down_then_across(names,count,dir);

    for(int i=0;i<count;i++) free(names[i]);
    free(names);
}

/* ---------- COLOR LOGIC ---------- */
void print_color_name(const char *path,const char *name)
{
    struct stat st;
    char full[1024];
    snprintf(full,sizeof(full),"%s/%s",path,name);

    if(lstat(full,&st)==-1){
        printf("%s ",name);
        return;
    }

    const char *color=RESET;

    if(S_ISDIR(st.st_mode))
        color=BLUE;
    else if(S_ISLNK(st.st_mode))
        color=PINK;
    else if(S_ISCHR(st.st_mode)||S_ISBLK(st.st_mode)||
            S_ISSOCK(st.st_mode)||S_ISFIFO(st.st_mode))
        color=REVERSE;
    else if((st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)))
        color=GREEN;
    else if(strstr(name,".tar")||strstr(name,".gz")||strstr(name,".zip"))
        color=RED;

    printf("%s%s%s",color,name,RESET);
}

/* ---------- COLUMN DISPLAY (down-then-across) ---------- */
void print_down_then_across(char **names,int n,const char *dir)
{
    if(n==0) return;

    int max=0;
    for(int i=0;i<n;i++)
        if(strlen(names[i])>max) max=strlen(names[i]);

    struct winsize w; int width=80;
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&w)!=-1) width=w.ws_col;

    int cw=max+2;
    int cols=width/cw; if(cols<1) cols=1;
    int rows=(n+cols-1)/cols;

    for(int r=0;r<rows;r++){
        for(int c=0;c<cols;c++){
            int idx=c*rows+r;                 // down then across
            if(idx<n){
                print_color_name(dir,names[idx]);
                int pad=cw-strlen(names[idx]);
                for(int p=0;p<pad;p++) putchar(' ');
            }
        }
        putchar('\n');
    }
}
