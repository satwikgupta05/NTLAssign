#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <pwd.h>

// Color Macros
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

void get_username() {
    const char *user = getenv("USER");
    if (user) {
        printf(YELLOW "User: " RESET "%s\n", user);
    } else {
        printf(YELLOW "User: " RESET "Unknown\n");
    }
}

void get_distro(char *distro, size_t size) {
    FILE *f = fopen("/etc/os-release", "r");
    if (!f) {
        strncpy(distro, "Unknown", size);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char *start = strchr(line, '=') + 1;
            if (start[0] == '\"') start++;
            char *end = strchr(start, '\"');
            if (end) *end = '\0';
            strncpy(distro, start, size - 1);
            break;
        }
    }

    fclose(f);
}

void get_kernel() {
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        printf(CYAN "Kernel: " RESET "%s\n", buffer.release);
    } else {
        printf(CYAN "Kernel: " RESET "Unknown\n");
    }
}

void get_uptime() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        int hours = info.uptime / 3600;
        int minutes = (info.uptime % 3600) / 60;
        printf(MAGENTA "Uptime: " RESET "%dh %dm\n", hours, minutes);
    }
}

void get_memory() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        long total = info.totalram / 1024 / 1024;
        long free = (info.totalram - info.freeram) / 1024 / 1024;
        printf(BLUE "Memory: " RESET "%ld MB / %ld MB\n", free, total);
    }
}

void get_disk_usage() {
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        unsigned long total = (stat.f_blocks * stat.f_frsize) / 1024 / 1024;
        unsigned long free = (stat.f_bavail * stat.f_frsize) / 1024 / 1024;
        printf(RED "Disk: " RESET "%lu MB free / %lu MB total\n", free, total);
    }
}

void get_package_count() {
    int count = 0;
    if (access("/usr/bin/dpkg", F_OK) == 0) {
        FILE *fp = popen("dpkg --get-selections | wc -l", "r");
        if (fp) {
            fscanf(fp, "%d", &count);
            pclose(fp);
        }
    } else if (access("/usr/bin/pacman", F_OK) == 0) {
        FILE *fp = popen("pacman -Q | wc -l", "r");
        if (fp) {
            fscanf(fp, "%d", &count);
            pclose(fp);
        }
    } else {
        printf(GREEN "Packages: " RESET "Package manager not detected\n");
        return;
    }

    printf(GREEN "Packages: " RESET "%d\n", count);
}

void get_cpu_info() {
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo) {
        printf(MAGENTA "CPU: " RESET "Unknown\n");
        return;
    }

    char line[256];
    char model[256] = "Unknown";
    int cores = 0;

    while (fgets(line, sizeof(line), cpuinfo)) {
        if (strstr(line, "model name")) {
            char *colon = strchr(line, ':');
            if (colon) {
                strncpy(model, colon + 2, sizeof(model) - 1);
                model[strcspn(model, "\n")] = '\0';
            }
        } else if (strstr(line, "cpu cores")) {
            char *colon = strchr(line, ':');
            if (colon) {
                cores = atoi(colon + 2);
            }
        }
    }

    fclose(cpuinfo);

    if (cores > 0) {
        printf(MAGENTA "CPU: " RESET "%s (%d cores)\n", model, cores);
    } else {
        printf(MAGENTA "CPU: " RESET "%s\n", model);
    }
}

void show_ascii_art(const char *distro) {
    if (strstr(distro, "Ubuntu")) {
        printf(GREEN
            "       .--.\n"
            "      |o_o |\n"
            "      |:_/ |\n"
            "     //   \\ \\\n"
            "    (|     | )  Ubuntu Linux\n"
            "   /'\\_   _/`\\\n"
            "   \\___)=(___/\n" RESET);
    } else if (strstr(distro, "Arch")) {
        printf(CYAN
            "       /\\\n"
            "      /  \\\n"
            "     /\\   \\\n"
            "    /      \\\n"
            "   /   ,,   \\\n"
            "  /   |  |  -\\  Arch Linux\n"
            "  \\_/|__|___/\n" RESET);
    } else if (strstr(distro, "Kali GNU")) {
        printf(BLUE
            "   _________\n"
            "  /         \\\n"
            " |        KALI|Linux\n"
            "  \\_________/\n" RESET);
    } else    {
    		printf(YELLOW
            "   _________\n"
            "  /         \\\n"
            " |    Unknown|Linux\n"
            "  \\_________/\n" RESET);
    }
}

int main() {
    char distro[128] = {0};

    get_distro(distro, sizeof(distro));

    printf(BOLD GREEN "\n==== My Machine Info ====\n\n" RESET);
    show_ascii_art(distro);

    get_username();
    printf(CYAN "Distro: " RESET "%s\n", distro);
    get_kernel();
    get_uptime();
    get_cpu_info();
    get_memory();
    get_disk_usage();
    get_package_count();

    printf("\n");

    return 0;}
