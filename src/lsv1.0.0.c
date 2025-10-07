/*
* Programming Assignment 02: ls v1.2.0
* Feature 3: Column Display (Down-Then-Across)
* Automatically arranges files in columns based on terminal width
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
#include <sys/ioctl.h>   // For terminal width (ioctl)

extern int errno;

// Function prototypes
void do_ls(const char *dir, int long_listing);
void print_in_columns(char **filenames, int count);

int main(int argc, char *argv[])
{
    int opt;
    int long_listing = 0;

    // Parse -l flag
    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        switch (opt)
        {
        case 'l':
            long_listing = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
        do_ls(".", long_listing);
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_listing);
            puts("");
        }
    }

    return 0;
}

void do_ls(const char *dir, int long_listing)
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

    // Array to hold filenames (for column display)
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
            // Save filenames for column printing
            filenames = realloc(filenames, sizeof(char *) * (count + 1));
            filenames[count] = strdup(entry->d_name);
            count++;
        }
    }

    if (errno != 0)
        perror("readdir failed");

    if (!long_listing)
        print_in_columns(filenames, count);

    // Cleanup
    for (int i = 0; i < count; i++)
        free(filenames[i]);
    free(filenames);

    closedir(dp);
}

// ----------------------------------------
// Helper function: print filenames in columns
// ----------------------------------------
void print_in_columns(char **filenames, int count)
{
    if (count == 0) return;

    // Find longest filename
    int maxlen = 0;
    for (int i = 0; i < count; i++)
    {
        int len = strlen(filenames[i]);
        if (len > maxlen) maxlen = len;
    }

    // Get terminal width
    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
        term_width = w.ws_col;

    int col_width = maxlen + 2;
    int cols = term_width / col_width;
    if (cols < 1) cols = 1;

    int rows = (count + cols - 1) / cols;

    // Print down-then-across
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
