// Name: Ori Ben Zaken  ID: 311492110

#include <stdio.h>
#include <unistd.h>

#define STDERR_FD 2

/**
 * prints error in sys call to stderr.
 */
void printErrorInSysCallToSTDERR() {
    char error_msg[] = "Error in system call\n";
    write(STDERR_FD, error_msg, sizeof(error_msg));
}

int main() {
    printf("Hello, World!\n");
    return 0;
}