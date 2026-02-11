#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include "proc_reader.h"

int list_process_directories(void) {
    // Open the directory
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("Error opening /proc");
        return -1;
    }
    
    struct dirent *entry;
    int process_count = 0;

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");
    
    // Iterate through entries -- PID directories are named numbers
    while ((entry = readdir(dir)) != NULL) {
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "Process");
            process_count++;
        }
    }

    if (closedir(dir) == -1) {
        perror("Error closing /proc");
        return -1;
    }

    printf("\nTotal process directories found: %d\n", process_count);

    return 0; 
}

int read_process_info(const char* pid) {
    char filepath[256];
    
    //Construct path to process status & print 
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);

    printf("\n--- Process Information for PID %s ---\n", pid);

    if (read_file_with_syscalls(filepath) != 0) {
        fprintf(stderr, "Failed to read %s\n", filepath);
    }
    
    //Construct path to command line args
    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);

    printf("\n--- Command Line ---\n");

    if (read_file_with_syscalls(filepath) != 0) {
        fprintf(stderr, "Failed to read %s\n", filepath);
    }

    printf("\n"); // Add extra newline for readability

    return 0; 
}

int show_system_info(void) {
    int line_count = 0;
    const int MAX_LINES = 10;
    char buffer[256];

    //Read a few lines of CPU info
    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);
    FILE *cpu_file = fopen("/proc/cpuinfo", "r");
    if (!cpu_file) {
        perror("fopen /proc/cpuinfo");
        return -1;
    }

    while (line_count < MAX_LINES && fgets(buffer, sizeof(buffer), cpu_file)) {
        printf("%s", buffer);
        line_count++;
    }

    fclose(cpu_file);

    //Do the same for memory info
    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);
    FILE *mem_file = fopen("/proc/meminfo", "r");
    if (!mem_file) {
        perror("fopen /proc/meminfo");
        return -1;
    }

    line_count = 0; // Reset counter
    while (line_count < MAX_LINES && fgets(buffer, sizeof(buffer), mem_file)) {
        printf("%s", buffer);
        line_count++;
    }
    fclose(mem_file);

    return 0; 
}

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    //Direct Kernel Interface
    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    //C Standard Library
    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);
    
    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

int read_file_with_syscalls(const char* filename) {
    int fd;
    char buffer[1024];
    ssize_t bytes_read;

    //Low-level open syscall
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return -1;
    }
    
    //Read chunks until EOF
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        return -1;
    }

    return 0; 
}

int read_file_with_library(const char* filename) {
    FILE *fp;
    char buffer[1024];
    fp = fopen(filename, "r");

    if (fp == NULL) {
        return -1;
    }
  
    // Use fgets for buffered reading
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    if (fclose(fp) != 0) {
        return -1;
    }

    return 0; 
}

int is_number(const char* str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    
    //Check every char to make sure the string represents a valid PID
    const char *p = str;
    while (*p) {
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
        p++;
    }
    
    return 1;
}
