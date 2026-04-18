// Sourced from https://gist.github.com/caiorss/133e91ba3732718cb228310173368674
// 17 April 2026


// ------ File: unix-process.cpp -----------------------------------------------------------//
// Description: Shows hows to launch processes on Unix with Exec and Fork syscall wrappers.
//-------------------------------------------------------------------------------------------//

#include "execute.hpp"

#include <iostream>
#include <vector>

// ----- Unix/Linux headers -----//
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <cstring> // Import: char* strerror(in errnum);

// -------------- Declarations --------------------------//

void print_errno_details(int err);

void execvp_test();

void execvp_cpp(std::string app, std::vector<std::string> args);

// ------------- Definitions ------------------------//

void print_errno_details(int err) {
    std::fprintf(stderr, "\n   =>    errno(int) = %d"
                 "\n   => errno message = %s \n"
                 , err, strerror(err));
    std::fflush(stderr);
}

// C++ wrapper for the exevp() library-call
// It replaces the current process image with a new one
// from other executable.
void execvp_cpp(std::string app
                , std::vector<std::string> args) {
    std::vector<const char *> pargs;
    pargs.reserve(args.size() + 1);
    pargs.push_back(app.c_str());
    for (auto const &a: args) { pargs.push_back(a.c_str()); }
    pargs.push_back(nullptr);

    // Signature: int execvp(const char *file, char *const argv[]);

    // execvp(app.c_str(), execvp(app.c_str(), (char* const *) pargs.data() )
    int status = execvp(app.c_str(), (char * const*) pargs.data());
    if (status == -1) {
        std::fprintf(stderr, " Error: unable to launch process");
        print_errno_details(errno);
        throw std::runtime_error("Error: failed to launch process");
    }
}

void fork_exec(std::string app, std::vector<std::string> args) {
    // std::printf(" [TRACE] <BEFORE FORK> PID of parent process = %d \n", getpid());

    // PID of child process (copy of this process)
    pid_t pid = fork();

    if (pid == -1) {
        std::fprintf(stderr, "Error: unable to launch process");
        print_errno_details(errno);
        throw std::runtime_error("Error: unable to launch process");
    }
    if (pid == 0) {
        // std::printf(" [TRACE] Running on child process => PID_CHILD = %d \n", getpid());

        // Close file descriptors, in order to disconnect the process from the terminal.
        // This procedure allows the process to be launched as a daemon (aka service).
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        close(STDIN_FILENO);

        // Execvp system call, replace the image of this process with a new one
        // from an executable.
        execvp_cpp(app, args);
        return;
    }

    // std::printf(" [TRACE] <AFTER FORK> PID of parent process = %d \n", getpid());

    // pid_t waitpid(pid_t pid, int *wstatus, int options);
    int status;

    // std::printf(" [TRACE] Waiting for child process to finish. ");

    // Wait for child process termination.
    // From header: #include <sys/wait.h>
    if (waitpid(pid, &status, 0) == -1) {
        print_errno_details(errno);
        throw std::runtime_error("Error: cannot wait for child process");
    }

    // std::printf(" [TRACE] Child process has been terminated Ok.");
    // -------- Parent process ----------------//
}
