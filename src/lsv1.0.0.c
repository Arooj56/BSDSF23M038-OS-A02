/*
* Programming Assignment 02: ls v1.5.0
* Feature 6: Reverse Order (-r)
* Adds -r option to show files Z–A.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/ioctl.h>

extern int errno;

void do_ls(const char *dir, int long_listing, int horizontal, int reverse);
void print_in_columns(char **filenames, int count);
void print_horizontal(char **filenames, int count);
int compare_names(const void *a, const void *b);

int main(int argc, char *argv[])
{
    int opt;
    int long_listing = 0, horizontal = 0, reverse = 0;

    while ((opt = getopt(argc, argv, "lxr")) != -1)
    {
        switch (opt)
        {
        case 'l': long_listing = 1; break;
        case 'x': horizontal = 1; break;
        case 'r': reverse = 1; break;
        default:
            fprintf(stderr, "Usage: %s [-l] [-x] [-r] [directory]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
        do_ls(".", long_listing, horizontal, reverse);
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_listing, horizontal, reverse);
            puts("");
        }
    }
    return 0;
}

void do_ls(const char *dir, int long_listing, int horizontal, int reverse)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (!dp) { perror("opendir"); return; }

    struct stat fileStat;
    struct passwd *pw; struct group *gr;
    char path[1024];
    char **files = NULL; int count = 0;

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (long_listing)
        {
            if (stat(path, &fileStat) == -1) { perror("stat"); continue; }

            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r":"-");
            printf((fileStat.st_mode & S_IWUSR) ? "w":"-");
            printf((fileStat.st_mode & S_IXUSR) ? "x":"-");
            printf((fileStat.st_mode & S_IRGRP) ? "r":"-");
            printf((fileStat.st_mode & S_IWGRP) ? "w":"-");
            printf((fileStat.st_mode & S_IXGRP) ? "x":"-");
            printf((fileStat.st_mode & S_IROTH) ? "r":"-");
            printf((fileStat.st_mode & S_IWOTH) ? "w":"-");
            printf((fileStat.st_mode & S_IXOTH) ? "x":"-");

            pw = getpwuid(fileStat.st_uid);
            gr = getgrgid(fileStat.st_gid);
            printf(" %3ld %-8s %-8s %8ld ",
                   fileStat.st_nlink,
                   pw ? pw->pw_name:"?", gr?gr->gr_name:"?",
                   fileStat.st_size);

            char *t = ctime(&fileStat.st_mtime);
            t[strlen(t)-1] = '\0';
            printf("%s %s\n", t, entry->d_name);
        }
        else
        {
            files = realloc(files, sizeof(char*)*(count+1));
            files[count++] = strdup(entry->d_name);
        }
    }

    if (errno != 0) perror("readdir");

    if (!long_listing && count>0)
    {
        qsort(files, count, sizeof(char*), compare_names);
        if (reverse)
        {
            for (int i=0; i<count/2; i++)
            {
                char *tmp = files[i];
                files[i] = files[count-1-i];
                files[count-1-i] = tmp;
            }
        }
        if (horizontal) print_horizontal(files, count);
        else print_in_columns(files, count);
    }

    for (int i=0;i<count;i++) free(files[i]);
    free(files);
    closedir(dp);
}

int compare_names(const void *a, const void *b)
{
    char *const *sa = a; char *const *sb = b;
    return strcasecmp(*sa, *sb);
}

void print_in_columns(char **f,int n)
{
    if (n==0) return;
    int max=0; for(int i=0;i<n;i++) if(strlen(f[i])>max) max=strlen(f[i]);
    struct winsize w; int width=80;
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&w)!=-1) width=w.ws_col;
    int cw=max+2, cols=width/cw; if(cols<1)cols=1;
    int rows=(n+cols-1)/cols;
    for(int r=0;r<rows;r++){ for(int c=0;c<cols;c++){
        int idx=c*rows+r; if(idx<n) printf("%-*s",cw,f[idx]); }
        printf("\n"); }
}

void print_horizontal(char **f,int n)
{
    if(n==0) return;
    int max=0; for(int i=0;i<n;i++) if(strlen(f[i])>max) max=strlen(f[i]);
    struct winsize w; int width=80;
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&w)!=-1) width=w.ws_col;
    int cw=max+2,pos=0;
    for(int i=0;i<n;i++){
        if(pos+cw>width){printf("\n");pos=0;}
        printf("%-*s",cw,f[i]); pos+=cw;
    }
    printf("\n");
}
