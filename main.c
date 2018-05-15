// Name: Ori Ben Zaken  ID: 311492110

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <wait.h>

#define STDERR_FD 2
#define CONFIG_LINE_LENGTH 160
#define DIRECTORY_PATH_LENGTH 200
#define STUDENT_EXECUTE_FILE_NAME "prog_s"
#define COMPILATION_FAILED 4
#define SYS_CALL_FAILURE 10

/**
 * prints error in sys call to stderr.
 */
void printErrorInSysCallToSTDERR() {
    char error_msg[] = "Error in system call\n";
    write(STDERR_FD, error_msg, sizeof(error_msg));
}

int isCFile(char* filename) {
    filename = strrchr(filename, '.');

    if( filename != NULL )
        if (!strcmp(filename, ".c")) {
            return 1;
        }
    return 0;
}

int compileAndRunCFile(char* path, char* input_file_path, char* correct_output_file_path) {
    char compile_command[] = "gcc";
    char o_flag[] = "-o";
    char excute_file_name[] = STUDENT_EXECUTE_FILE_NAME;
    unlink(excute_file_name);
    int result = 0;
    char* args[] = {compile_command, o_flag, excute_file_name, path, NULL};
    int i = 0;
    int pid;
    if ((pid = fork()) < 0) {
        printf("Error in fork command in order to compile .c file\n");
        printErrorInSysCallToSTDERR();
        // son process
    }
    if (pid == 0) {
        execvp(args[0], &args[0]);
        exit(COMPILATION_FAILED);
    } else {
        // waiting for son process to finish compiling
        waitpid(pid, NULL, 0);
        if (!checkCompileSuccess()) {
            printf("Compiled failed\n");
            result = COMPILATION_FAILED;
        } else {
            result = getStudentProgramResult(input_file_path, correct_output_file_path);
        }
    }
    return result;
}

int getStudentProgramResult(char* input_file_path, char* correct_output_file_path) {
    int status;
    int in, out;
    int pid;

    if ((pid = fork()) < 0) {
        printf("Failed to create cjild process for running student's program\n");
        printErrorInSysCallToSTDERR();
        exit(-1);
    }
     //child process
    if (pid == 0) {
        in = open(input_file_path, O_RDONLY);
        if (in == -1) {
            printf("Failed to open input file\n");
            printErrorInSysCallToSTDERR();
            exit(SYS_CALL_FAILURE);
        }
        unlink("output");
        out = open("output",  O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (out == -1) {
            printf("Failed to open output file\n");
            printErrorInSysCallToSTDERR();
            exit(SYS_CALL_FAILURE);
        }
        // replace standard input with input file
        if (dup2(in, STDIN_FILENO) == -1) {
            printf("Failed to operate dup2 for in\n");
            printErrorInSysCallToSTDERR();
            exit(SYS_CALL_FAILURE);
        }
        // replace standard output with output file
        if (dup2(out, STDOUT_FILENO) == -1) {
            printf("Failed to operate dup2 for out\n");
            printErrorInSysCallToSTDERR();
            exit(SYS_CALL_FAILURE);
        }
        printf("Hi\n");

        char prog_name[] = "./prog_s";
        char* args[] = {prog_name, NULL};
        execvp(args[0], args);
        printf("Failed to run student program\n");
        // main process
    } else {
        waitpid(pid, &status, 0);
        return 0;
    }
}

int checkCompileSuccess() {
    DIR* pDir;
    struct dirent*  entry;
    struct stat stat_p;
    char path[DIRECTORY_PATH_LENGTH];
    getcwd(path, DIRECTORY_PATH_LENGTH);
    int found_execute_file = 0;
    if ((pDir = opendir(path)) == NULL) {
        printf("Couldn't open current directory\n");
        exit(1);
    }
    while ((entry = readdir(pDir)) != NULL) {
        if (!strcmp(entry->d_name, STUDENT_EXECUTE_FILE_NAME)) {
            found_execute_file = 1;
            break;
        }
    }
    closedir(pDir);
    return found_execute_file;
}
void buildPath(char* dest, char* curr_path, char* filename) {
    strcpy(dest, curr_path);
    strcat(dest, "/");
    strcat(dest, filename);
}

int handleStudentDirectory(char* path, char* input_file_path, char* correct_output_file_path) {
    DIR* pDir;
    struct dirent*  entry;
    struct stat stat_p;
    char entry_path[DIRECTORY_PATH_LENGTH];
    int found_c_file = 0;
    int result = -1;

    if ((pDir = opendir(path)) == NULL) {
        printf("Couldn't open student directory\n");
    }
    while ((entry = readdir(pDir)) != NULL) {
        // go through all sub directories except "." (current dir) and ".." (father dir)
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            // building entry path
            printf("\t%s\n", entry->d_name);
            if (isCFile(entry->d_name)) {
                printf("\t\t%s is a C file!\n", entry->d_name);
                found_c_file = 1;
                buildPath(entry_path, path, entry->d_name);
                compileAndRunCFile(entry_path, input_file_path, correct_output_file_path);
            }
        }
    }
    closedir(pDir);

    // check if found a c file. if so - try to compile and run it.
    if (found_c_file) {
        // call to function which compiles and runs the c file.
        return 1;
    } else {
        // search for a .c file recursively
        if ((pDir = opendir(path)) == NULL) {
            printf("Couldn't open student directory\n");
        }
        while ((entry = readdir(pDir)) != NULL) {
            // go through all sub directories except "." (current dir) and ".." (father dir)
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
                continue;
            }
                // building entry path
            buildPath(entry_path, path, entry->d_name);
            if (stat(entry_path, &stat_p) == -1) {
                printf("Error occurred attempting to stat %s\n", entry->d_name);
            } else {
                if (S_ISDIR(stat_p.st_mode)) {
                    printf("####### Going recursively to a directory: %s #######\n", entry->d_name);
                    buildPath(entry_path, path, entry->d_name);
                    result =handleStudentDirectory(entry_path, input_file_path, correct_output_file_path);
                    if (result != -1) {
                        closedir(pDir);
                        return result;
                    }
                }
            }

        }
    }

    return -1;
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
    if (config_file < 0) {
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

    if (close(config_file) == -1) {
        printf("Failed to close configuration file\n");
        printErrorInSysCallToSTDERR();
    }

    char* input_file_path;
    char* correct_output_file_path;
    char* students_directory_path;

    // get all paths from the configuration file

    students_directory_path = strtok(config_file_buffer, "\n");
    input_file_path = strtok(NULL, "\n");
    correct_output_file_path =  strtok(NULL, "\n");

    printf("%s\n", students_directory_path);
    printf("%s\n", input_file_path);
    printf("%s\n", correct_output_file_path);

    // go through all directories in students directory
    struct dirent* sub_dir;
    DIR *p_student_dir;
    if ((p_student_dir = opendir(students_directory_path)) == NULL) {
        printf("Couldn't open student directory\n");
    }

    char path[DIRECTORY_PATH_LENGTH];
    int result;
    while ((sub_dir = readdir(p_student_dir)) != NULL) {
        // go through all sub directories except "." (current dir) and ".." (father dir)
        if (strcmp(sub_dir->d_name, ".") && strcmp(sub_dir->d_name, "..")) {
            printf("<-------- Now in: %s --------->\n", sub_dir->d_name);
            strcpy(path, students_directory_path);
            strcat(path, "/");
            strcat(path, sub_dir->d_name);
            result = handleStudentDirectory(path, input_file_path, correct_output_file_path);
            printf("$ Result for student %s is %d\n", sub_dir->d_name, result);
        }
    }

    closedir(p_student_dir);
    return 0;
}