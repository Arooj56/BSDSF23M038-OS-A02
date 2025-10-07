/*
* Programming Assignment 02: ls v1.3.0
* Feature 4: Horizontal Display (-x)
* Adds -x flag to print files row-wise instead of down-then-across.
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
#include <sys/ioctl.h>   // For terminal width

extern int errno;

// Function prototypes
void do_ls(const char *dir, int long_listing, int horizontal);
void print_in_columns(char **filenames, int count);
void print_horizontal(char **filenames, int count);

int main(int argc, char *argv[])
{
    int opt;
    int long_listing = 0;
    int horizontal = 0;

    // Parse -l and -x flags
    while ((opt = getopt(argc, argv, "lx")) != -1)
    {
        switch (opt)
        {
        case 'l':
            long_listing = 1;
            break;
        case 'x':
            horizontal = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l] [-x] [directory]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
        do_ls(".", long_listing, horizontal);
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_listing, horizontal);
            puts("");
        }
    }

    return 0;
}

void do_ls(const char *dir, int long_listing, int horizontal)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    struct stat fileStat;
    struct passwd *pw;
    struct group *gr;
    char path[1024];

    char **filenames = NULL;
    int count = 0;

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (long_listing)
        {
            if (stat(path, &fileStat) == -1)
            {
                perror("stat");
                continue;
            }

            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

            pw = getpwuid(fileStat.st_uid);
            gr = getgrgid(fileStat.st_gid);

            printf(" %3ld %-8s %-8s %8ld ",
                   fileStat.st_nlink,
                   pw ? pw->pw_name : "unknown",
                   gr ? gr->gr_name : "unknown",
                   fileStat.st_size);

            char *time_str = ctime(&fileStat.st_mtime);
            time_str[strlen(time_str) - 1] = '\0';
            printf("%s ", time_str);

            printf("%s\n", entry->d_name);
        }
        else
        {
            filenames = realloc(filenames, sizeof(char *) * (count + 1));
            filenames[count] = strdup(entry->d_name);
            count++;
        }
    }

    if (errno != 0)
        perror("readdir failed");

    if (!long_listing)
    {
        if (horizontal)
            print_horizontal(filenames, count);
        else
            print_in_columns(filenames, count);
    }

    for (int i = 0; i < count; i++)
        free(filenames[i]);
    free(filenames);

    closedir(dp);
}

// --------------------------------------------------
// Print filenames down-then-across (default display)
// --------------------------------------------------
void print_in_columns(char **filenames, int count)
{
    if (count == 0)
        return;

    int maxlen = 0;
    for (int i = 0; i < count; i++)
    {
        int len = strlen(filenames[i]);
        if (len > maxlen)
            maxlen = len;
    }

    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
        term_width = w.ws_col;

    int col_width = maxlen + 2;
    int cols = term_width / col_width;
    if (cols < 1)
        cols = 1;

    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int index = c * rows + r;
            if (index < count)
                printf("%-*s", col_width, filenames[index]);
        }
        printf("\n");
    }
}

// --------------------------------------------------
// Print filenames left-to-right (horizontal mode)
// --------------------------------------------------
void print_horizontal(char **filenames, int count)
{
    if (count == 0)
        return;

    int maxlen = 0;
    for (int i = 0; i < count; i++)
    {
        int len = strlen(filenames[i]);
        if (len > maxlen)
            maxlen = len;
    }

    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
        term_width = w.ws_col;

    int col_width = maxlen + 2;
    int pos = 0;

    for (int i = 0; i < count; i++)
    {
        if (pos + col_width > term_width)
        {
            printf("\n");
            pos = 0;
        }
        printf("%-*s", col_width, filenames[i]);
        pos += col_width;
    }
    printf("\n");
}
