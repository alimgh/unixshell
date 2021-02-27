// C Program to design a shell in Linux
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<pthread.h>
#include<signal.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXCOM 512 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

volatile sig_atomic_t execFlag = 0;
//volatile int shmid;
//volatile key_t key;
//volatile pid_t p1;
//volatile pid_t p2;
//volatile pid_t pid;

// Greeting shell during startup
void init_shell()
{
    clear();
    printf("****unixshell project****");
    printf("\nAli Moghaddaszadeh,\t9712762385");
    char* username = getenv("USER");
    printf("\n\nUSER is: @%s", username);
    printf("\n");
    sleep(1);
    clear();
}

// Function to take input
int takeInput(char* str) {

    char* buf;

    buf = readline("\nprompt> ");

    if (buf == 0) {
//        printf("\nGoodbye\n");

        // remove named pipe
        unlink("/tmp/myfifo");

//        // destroy the shared memory
//        shmctl(shmid,IPC_RMID,NULL);

        exit(0);
    }

    if (strlen(buf) != 0) {

        // add command to history
        add_history(buf);

        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
}

// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork();
//    pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        if (execvp(parsed[0], parsed) < 0) {
//            printf("\nCould not execute command..");
            fprintf( stderr, "Could not execute command..\n");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        signal(SIGINT, SIG_DFL);

        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            fprintf( stderr, "Could not execute command 1\n");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();

        if (p2 < 0) {
            fprintf( stderr, "Could not fork\n");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            signal(SIGINT, SIG_DFL);

            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                fprintf( stderr, "Could not execute command 2\n");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            close(pipefd[0]);
            close(pipefd[1]);
            wait(NULL);
            wait(NULL);
        }
    }
}

// Help command built-in
void openHelp() {
    puts("\nList of Commands supported:"
         "\n>cd"
         "\n>history"
         "\n>msg (mkfifo)"
         "\n>quit"
         "\n>pipe handling (only for 2 cmd)");

    return;
}

//void write_msg() {
//
//    // shmat to attach to shared memory
//    char *str = (char*) shmat(shmid,(void*)0,0);
//
//    if (strlen(str) > 1){
//        printf("Data read from memory: %s\n",str);
//    }
//
//    printf("msg> ");
//    fgets(str, 512, stdin);
//
//    //detach from shared memory
//    shmdt(str);
//}

// Function to handle msg command
void write_msg() {
    int fd;

    // FIFO file path
    char *myfifo = "/tmp/myfifo";

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0666);

    char arr[80];
    // Open FIFO for write only
    fd = open(myfifo, O_WRONLY);

    // Take an input arring from user.
    // 80 is maximum length
    fgets(arr, 80, stdin);

    // Write the input arring on FIFO
    // and close it
    write(fd, arr, strlen(arr) + 1);
    close(fd);
}

// Function to execute builtin commands
int shellCmdHandler(char** parsed)
{
    int NoOfShellCmds = 5, i, switchShellArg = 0;
    char* ListOfShellCmds[NoOfShellCmds];

    ListOfShellCmds[0] = "quit";
    ListOfShellCmds[1] = "cd";
    ListOfShellCmds[2] = "help";
    ListOfShellCmds[3] = "history";
    ListOfShellCmds[4] = "msg";

    for (i = 0; i < NoOfShellCmds; i++) {
        if (strcmp(parsed[0], ListOfShellCmds[i]) == 0) {
            switchShellArg = i + 1;
            break;
        }
    }

    int entry = 1;
    switch (switchShellArg) {
        case 1:
//            printf("\nGoodbye\n");

            // remove named pipe
            unlink("/tmp/myfifo");
//            // destroy the shared memory
//            shmctl(shmid,IPC_RMID,NULL);
            exit(0);
        case 2:
            chdir(parsed[1]);
            return 1;
        case 3:
            openHelp();
            return 1;
        case 4:
            printf("Output:");
            for (; entry < history_length; entry++)
                printf("\n%d %s", entry, history_get(entry)->line);
            return 1;
        case 5:
            write_msg();
            return 1;
        default:
            break;
    }

    return 0;
}

// Function for tokenizing two commands with '|'
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char* trimwhitespace(char *str)
{
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

// Removing all occurrences of c in string
void remove_all_chars(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

// Function for parsing command words
void parseSpace(char* str, char** parsed)
{
    int i;

    trimwhitespace(str);
    remove_all_chars(str, '\"');

    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

// Function to process command and check if it need pipe
int processString(char* str, char** parsed, char** parsedpipe) {
    char* strpiped[2];
    int piped = 0;

    piped = parsePipe(str, strpiped);

    if (piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);

    } else {

        parseSpace(str, parsed);
    }

    if (shellCmdHandler(parsed))
        return 0; //no command of built-in
    else
        return 1 + piped;
}

// handling Ctrl+C interrupt
void interrupt_handler(int sig) {
//    if (execFlag == 1) {
//        kill(pid, sig);
//    } else if (execFlag == 2) {
//        kill(p1, sig);
//        kill(p2, sig);
//    }
    execFlag = 0;
}

// Function to read incoming msg
void* read_thread() {
    int fd;

    // FIFO file path
    char * myfifo = "/tmp/myfifo";

    char str[80];
    while (1) {
        fd = open(myfifo,O_RDONLY);
        // First open in read only and read
        while (read(fd, str, 80) != 0) {
            // Print the read string and close
            printf("\nmsg: %s\n", str);
        }

        close(fd);
    }
}

int main(int argc, char **argv) {

    // More than one arguments!!
    if (argc > 2) {
        fprintf( stderr, "Wrong arguments\n");
        return 0;
    }

    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    pthread_t msg_reader_id;
    size_t input_len = 0;
    init_shell();

    // batch mode
    FILE * batch_file;
    if (argc == 2) {
        batch_file = fopen(argv[1], "r");
        if(batch_file == NULL){
            perror("Error opening file"); // print error
            sleep(.1);
            return(-1);
        }
    }
    else
        // interactive mode
        signal(SIGINT, interrupt_handler);

    // batch mode
    if (argc == 2) {
//        while (fgets(inputString, 512, batch_file) != 0 && !feof(stdin)) {
        while (fgets(inputString, 512, batch_file) != 0) {
            printf("\n");
            if (input_len > 512) {
                fprintf( stderr, "Command is more than 512 char\n");
                sleep(.1);
                continue;
            }

            // process
            execFlag = processString(inputString,
                                     parsedArgs, parsedArgsPiped);
            // execflag returns zero if there is no command
            // or it is a builtin command,
            // 1 if it is a simple command
            // 2 if it is including a pipe.

            if (execFlag) {
                // execute
                if (execFlag == 1)
                    execArgs(parsedArgs);

                if (execFlag == 2)
                    execArgsPiped(parsedArgs, parsedArgsPiped);
            }
        }
        fclose(batch_file);
    }
    else { // interactive mode

//        // ftok to generate unique key
//        key = ftok("shmfile",65);
//
//        // shmget returns an identifier in shmid
//        shmid = shmget(key,1024,0666|IPC_CREAT);

        // FIFO file path
        char * myfifo = "/tmp/myfifo";

        // Creating the named file(FIFO)
        // mkfifo(<pathname>,<permission>)
        mkfifo(myfifo, 0666);

        // creating msg reader thread
        pthread_create(&msg_reader_id, NULL, read_thread, NULL);

        // interactive mode
        while (1) {

            // print shell line (directory)
            printDir();

            // take input
            if (takeInput(inputString)) {
                continue;
            }

            // command len is more than 512
            if (strlen(inputString) > 512) {
                fprintf( stderr, "Command is more than 512 char\n");
                sleep(.1);
//            printf("Command is more than 512 char\n");
                continue;
            }

            // process
            execFlag = processString(inputString,
                                     parsedArgs, parsedArgsPiped);
            // execflag returns zero if there is no command
            // or it is a builtin command,
            // 1 if it is a simple command
            // 2 if it is including a pipe.

            if (execFlag) {
                // execute
                if (execFlag == 1) {
                    execArgs(parsedArgs);
                }

                if (execFlag == 2) {
                    execArgsPiped(parsedArgs, parsedArgsPiped);
                }
            }

//            // shmat to attach to shared memory
//            char *mstr = (char*) shmat(shmid,(void*)0,0);
//            if (strlen(mstr) > 1){
//                printf("Data read from memory: %s\n",mstr);
//            }
//            //detach from shared memory
//            shmdt(mstr);
        }
    }
    return 0;
}
