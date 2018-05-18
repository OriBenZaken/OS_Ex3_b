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
#define COMPILATION_ERROR 4
#define SYS_CALL_FAILURE 10
#define TIMEOUT 5
#define FAILURE 11
#define NO_C_FILE 6
#define RESULT_LINE_LENGTH 200
#define GREAT_JOB 3
#define SIMILAR_OUTPUT 2
#define BAD_OUTPUT 1

/**
 * prints error in sys call to stderr.
 */
void printErrorInSysCallToSTDERR() {
    char error_msg[] = "Error in system call\n";
    write(STDERR_FD, error_msg, sizeof(error_msg));
}

/**
 * confirms that the input path is a path of .c file.
 * @param filename
 * @return 1- filename is .c file, 0 - else.
 */
int isCFile(char* filename) {
    filename = strrchr(filename, '.');

    if( filename != NULL )
        if (!strcmp(filename, ".c")) {
            return 1;
        }
    return 0;
}

/**
 * compile and run a .c file
 * @param path path of the .c file
 * @param input_file_path  path to the file that contains input to the program
 * @param correct_output_file_path path to the file that contains correct output
 * @return result of the compile and program output
 */
int compileAndRunCFile(char* path, char* input_file_path, char* correct_output_file_path) {
    char compile_command[] = "gcc";
    char o_flag[] = "-o";
    char excute_file_name[] = STUDENT_EXECUTE_FILE_NAME;
    // remove previous STUDENT_EXECUTE_FILE_NAME file from the working directory.
    unlink(excute_file_name);
    int result;
    char* args[] = {compile_command, o_flag, excute_file_name, path, NULL};
    int i = 0;
    int pid;
    if ((pid = fork()) < 0) {
        printf("Error in fork command in order to compile .c file\n");
        printErrorInSysCallToSTDERR();
        // son process
    }
    if (pid == 0) {
        // compile student's code
        execvp(args[0], &args[0]);
        exit(COMPILATION_ERROR);
    } else {
        // waiting for son process to finish compiling
        waitpid(pid, NULL, 0);
        if (!checkCompileSuccess()) {
            printf("Compiled failed\n");
            result = COMPILATION_ERROR;
        } else {
            result = getStudentProgramResult(input_file_path, correct_output_file_path);
        }
    }
    return result;
}


/**
 * runs the student program with input and compare between the program output to the correct output.
 * student's grade is determined by it's program output, and returned by this function.
 * @param input_file_path path to the file that contains input to the program
 * @param correct_output_file_path  path to the file that contains correct output
 * @return student's grade
 */
int getStudentProgramResult(char* input_file_path, char* correct_output_file_path) {
    int status;
    int in, out;
    int pid;

    if ((pid = fork()) < 0) {
        printf("Failed to create child process for running student's program\n");
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
        // remove previous output file if exists
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

        char prog_name[] = "./prog_s";
        char* args[] = {prog_name, NULL};
        // run the student's program
        execvp(args[0], args);
        printf("Failed to run student program\n");
        exit(1);
        // main process - checks for time out
    } else {
        sleep(5);
        if (waitpid(pid, &status, WNOHANG) == 0) {
            // program is running more than 5 seconds
            return TIMEOUT;
        }
    }
    char prog_output[] = "output";
    return compareFiles(prog_output, correct_output_file_path);
}

/**
 * comparing between two files with comp.out and returns comp.out exit code.
 * @param prog_output path of file contains the student's program output
 * @param correct_output_file_path path of file that contains the correct output
 * @return 3 - identical files. 2 - similar files. 3- different files.
 */
int compareFiles(char* prog_output,char* correct_output_file_path) {
    char prog_name[] = "./comp.out";
    char* args[] = {prog_name, correct_output_file_path, prog_output, NULL};
    int pid;
    int status, result;
    if ((pid = fork()) < 0) {
        printf("Failed to create child process for running student's program\n");
        printErrorInSysCallToSTDERR();
        exit(-1);
    }
    // child process - runs comp.out
    if (pid == 0) {
        execvp(args[0], args);
        printf("Failed execute comp.out\n");
        exit(FAILURE);
    }
    // main process - waiting for the comparison result
    if (pid > 0) {
        waitpid(pid, &status, 0);
        result = WEXITSTATUS(status);
        if (result == FAILURE) {
            printf("Program stopped because it failed to execute comp.out\n");
            exit(FAILURE);
        }
        return result;
    }
}

/**
 * checks if STUDENT_EXECUTE_FILE_NAME file exists in the working directory
 * @return 1 - compilation succeeded. 0 - else.
 */
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
    int result = NO_C_FILE;

    if ((pDir = opendir(path)) == NULL) {
        printf("Couldn't open student directory\n");
    }
    while ((entry = readdir(pDir)) != NULL) {
        // go through all sub directories except "." (current dir) and ".." (father dir)
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            // building entry path
            printf("\t%s\n", entry->d_name);
            buildPath(entry_path, path, entry->d_name);
            if (stat(entry_path, &stat_p) == -1) {
                printf("Error occurred attempting to stat %s\n", entry->d_name);
                exit(FAILURE);
            }
            // file has suffix .c and is not a directory
            if (isCFile(entry->d_name) && !S_ISDIR(stat_p.st_mode)) {
                printf("\t\t%s is a C file!\n", entry->d_name);
                found_c_file = 1;
                result = compileAndRunCFile(entry_path, input_file_path, correct_output_file_path);
            }
        }
    }
    closedir(pDir);

    // check if found a c file. if so - try to compile and run it.
    if (found_c_file) {
        // call to function which compiles and runs the c file.
        return result;
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
                exit(FAILURE);
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

    return NO_C_FILE;
}

void writeGradeToResultsFile(int result_file, char* name, int result) {
    char result_line[RESULT_LINE_LENGTH] ={0};
    strcpy(result_line, name);
    strcat(result_line, ",");
    switch (result) {
        case GREAT_JOB:
            strcat(result_line, "100,GREAT_JOB\n");
            break;
        case SIMILAR_OUTPUT:
            strcat(result_line, "80,SIMILAR_OUTPUT\n");
            break;
        case BAD_OUTPUT:
            strcat(result_line, "60,BAD_OUTPUT\n");
            break;
        case TIMEOUT:
            strcat(result_line, "0,TIMEOUT\n");
            break;
        case COMPILATION_ERROR:
            strcat(result_line, "0,COMPILATION_ERROR\n");
            break;
        case NO_C_FILE:
            strcat(result_line, "0,NO_C_FILE\n");
            break;
        default:
            break;
    }
    if (write(result_file, result_line, strlen(result_line)) < 0) {
        printf("Failed to write to result.csv\n");
        printErrorInSysCallToSTDERR();
        exit(FAILURE);
    }
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
    int result_file = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (result_file < 0) {
        printf("Couldn't open result.csv file\n");
        printErrorInSysCallToSTDERR();
        exit(FAILURE);
    }
    while ((sub_dir = readdir(p_student_dir)) != NULL) {
        // go through all sub directories except "." (current dir) and ".." (father dir)
        if (strcmp(sub_dir->d_name, ".") && strcmp(sub_dir->d_name, "..")) {
            printf("<-------- Now in: %s --------->\n", sub_dir->d_name);
            strcpy(path, students_directory_path);
            strcat(path, "/");
            strcat(path, sub_dir->d_name);
            result = handleStudentDirectory(path, input_file_path, correct_output_file_path);
            printf("$ Result for student %s is %d\n", sub_dir->d_name, result);
            if (result == FAILURE || result == SYS_CALL_FAILURE) {
                printf("Got unexpected result: %d\n", result);
                exit(FAILURE);
            }
            writeGradeToResultsFile(result_file, sub_dir->d_name,result);
        }
    }
    closedir(p_student_dir);
    if (close(result_file) < 0 ) {
        printf("Error while closing result.csv file\n");
        printErrorInSysCallToSTDERR();
    }
    return 0;
}