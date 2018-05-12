// Name: Ori Ben Zaken  ID: 311492110

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>

#define STDERR_FD 2
#define CONFIG_LINE_LENGTH 160
/**
 * prints error in sys call to stderr.
 */
void printErrorInSysCallToSTDERR() {
    char error_msg[] = "Error in system call\n";
    write(STDERR_FD, error_msg, sizeof(error_msg));
}

int main(int argc, char *argv[]) {
    printf("OS EX_3_B!\n");

    if (argc < 2) {
        printf("Missing configuration file path or configuration file name\n");
        return 0;
    }

    char config_file_buffer[CONFIG_LINE_LENGTH * 3];
    int config_file = open(argv[1], O_RDONLY);
    int read_bytes;
    if (config_file == -1) {
        printf("Couldn't open configuration file\n");
        printErrorInSysCallToSTDERR();
        return 0;
    }

    if ((read_bytes = read(config_file, config_file_buffer, CONFIG_LINE_LENGTH * 3)) <= 0 ) {
        printf("Error while reading configuration file\n");
        printErrorInSysCallToSTDERR();
        return 0;
    } else {
        config_file_buffer[read_bytes] = '\0';
    }

    char* input_file_path;
    char* correct_output_file_path;
    char* students_directory_path;

    students_directory_path = strtok(config_file_buffer, "\n");
    input_file_path = strtok(NULL, "\n");
    correct_output_file_path =  strtok(NULL, "\n");

    printf("%s\n", students_directory_path);
    printf("%s\n", input_file_path);
    printf("%s\n", correct_output_file_path);

    return 0;
}