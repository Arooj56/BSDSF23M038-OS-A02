/*
* Programming Assignment 02: ls v1.6.1
* Feature 7: Human-Readable Size (-h)
* Fix: Reverse (-r) now works with both short and long listings.
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

// Function prototypes
void do_ls(const char *dir, int long_listing, int horizontal, int reverse, int human);
void print_in_columns(char **filenames, int count);
void print_horizontal(char **filenames, int count);
int compare_names(const void *a, const void *b);
const char* human_size(off_t size);

int main(int argc, char *argv[])
{
    int opt;
    int long_listing = 0, horizontal = 0, reverse = 0, human = 0;

    while ((opt = getopt(argc, argv, "lxrh")) != -1)
    {
        switch (opt)
        {
            case 'l': long_listing = 1; break;
            case 'x': horizontal = 1; break;
            case 'r': reverse = 1; break;
            case 'h': human = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-r] [-h] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
        do_ls(".", long_listing, horizontal, reverse, human);
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_listing, horizontal, reverse, human);
            puts("");
        }
    }
    return 0;
}

void do_ls(const char *dir, int long_listing, int horizontal, int reverse, int human)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (!dp) { perror("opendir"); return; }

    char **files = NULL; 
    int count = 0;

    // Step 1: Collect all filenames
    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        files = realloc(files, sizeof(char*) * (count + 1));
        files[count++] = strdup(entry->d_name);
    }

    if (errno != 0) perror("readdir");
    closedir(dp);

    if (count == 0) return;

    // Step 2: Sort alphabetically
    qsort(files, count, sizeof(char*), compare_names);

    // Step 3: Reverse if needed
    if (reverse)
    {
        for (int i = 0; i < count / 2; i++)
        {
            char *tmp = files[i];
            files[i] = files[count - 1 - i];
            files[count - 1 - i] = tmp;
        }
    }

    // Step 4: Print according to mode
    if (long_listing)
    {
        struct stat fileStat;
        struct passwd *pw; 
        struct group *gr;
        char path[1024];

        for (int i = 0; i < count; i++)
        {
            snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
            if (stat(path, &fileStat) == -1) { perror("stat"); continue; }

            // Permissions
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

            printf(" %3ld %-8s %-8s %8s ",
                fileStat.st_nlink,
                pw ? pw->pw_name : "?",
                gr ? gr->gr_name : "?",
                human ? human_size(fileStat.st_size) : "bytes"
            );

            char *t = ctime(&fileStat.st_mtime);
            t[strlen(t) - 1] = '\0';
            printf("%s %s\n", t, files[i]);
        }
    }
    else
    {
        if (horizontal) print_horizontal(files, count);
        else print_in_columns(files, count);
    }

    // Step 5: Free memory
    for (int i = 0; i < count; i++) free(files[i]);
    free(files);
}

int compare_names(const void *a, const void *b)
{
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcasecmp(sa, sb);
}

const char* human_size(off_t size)
{
    static char output[20];
    const char *units[] = {"B", "K", "M", "G", "T"};
    int i = 0;
    double s = size;
    while (s >= 1024 && i < 4)
    {
        s /= 1024;
        i++;
    }
    snprintf(output, sizeof(output), "%.1f%s", s, units[i]);
    return output;
}

void print_in_columns(char **f, int n)
{
    if (n == 0) return;
    int max = 0; 
    for (int i = 0; i < n; i++) if (strlen(f[i]) > max) max = strlen(f[i]);

    struct winsize w; 
    int width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) width = w.ws_col;

    int cw = max + 2, cols = width / cw; 
    if (cols < 1) cols = 1;
    int rows = (n + cols - 1) / cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = c * rows + r;
            if (idx < n) printf("%-*s", cw, f[idx]);
        }
        printf("\n");
    }
}

void print_horizontal(char **f, int n)
{
    if (n == 0) return;
    int max = 0; 
    for (int i = 0; i < n; i++) if (strlen(f[i]) > max) max = strlen(f[i]);

    struct winsize w; 
    int width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) width = w.ws_col;

    int cw = max + 2, pos = 0;
    for (int i = 0; i < n; i++) {
        if (pos + cw > width) { printf("\n"); pos = 0; }
        printf("%-*s", cw, f[i]);
        pos += cw;
    }
    printf("\n");
}
