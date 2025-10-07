/*
* Programming Assignment 02: ls v1.1.0
* Feature 2: Long Listing (-l)
* Implements -l option using stat(), getpwuid(), getgrgid(), and ctime()
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>      // For getpwuid()
#include <grp.h>      // For getgrgid()
#include <time.h>     // For ctime()

extern int errno;

// Function prototype
void do_ls(const char *dir, int long_listing);

// -----------------------------
// Main function
// -----------------------------
int main(int argc, char *argv[])
{
    int opt;
    int long_listing = 0;

    // Handle -l flag
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

    // If no directory given, use current directory
    if (optind == argc)
    {
        do_ls(".", long_listing);
    }
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

// -----------------------------
// Function: do_ls
// -----------------------------
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

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue; // skip hidden files

        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (long_listing)
        {
            if (stat(path, &fileStat) == -1)
            {
                perror("stat");
                continue;
            }

            // File type (d = directory, - = file)
            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");

            // Permissions
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

            // Owner, group, size
            pw = getpwuid(fileStat.st_uid);
            gr = getgrgid(fileStat.st_gid);

            printf(" %3ld %-8s %-8s %8ld ",
                   fileStat.st_nlink,
                   pw ? pw->pw_name : "unknown",
                   gr ? gr->gr_name : "unknown",
                   fileStat.st_size);

            // Time
            char *time_str = ctime(&fileStat.st_mtime);
            time_str[strlen(time_str) - 1] = '\0'; // remove newline
            printf("%s ", time_str);

            // File name
            printf("%s\n", entry->d_name);
        }
        else
        {
            // Normal listing
            printf("%s\n", entry->d_name);
        }
    }

    if (errno != 0)
    {
        perror("readdir failed");
    }

    closedir(dp);
}
