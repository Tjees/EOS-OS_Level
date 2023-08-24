#include "shell.hh"

void new_file() {
    std::string filename, text;
    std::cout << "Bestandsnaam: ";
    std::getline(std::cin, filename);

    std::cout << "Tekst (<EOF> om te eindigen):\n";
    while (true) {
        std::string line;
        std::getline(std::cin, line);
        if (line == "<EOF>") {
            break;
        }
        text += line + '\n';
    }

    int fd = syscall(SYS_creat, filename.c_str(), S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("creat");
        return;
    }

    syscall(SYS_write, fd, text.c_str(), text.size());
    syscall(SYS_close, fd);

    std::cout << "Bestand '" << filename << "' aangemaakt." << std::endl;
}

void list() {
    pid_t pid = syscall(SYS_fork);
    if (pid == 0) {
        // Child process
        const char* args[] = {"ls", "-la", nullptr};
        syscall(SYS_execve, "/bin/ls", (char* const*)args, nullptr);
        perror("execve");
        _exit(1);
    } else if (pid > 0) {
        // Parent process
        syscall(SYS_wait4, pid, nullptr, 0, nullptr);
    } else {
        perror("fork");
    }
}

void find() {
        std::string searchString;
    std::cout << "Zoekstring: ";
    std::getline(std::cin, searchString);

    int pipefd[2];
    if (syscall(SYS_pipe, pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = syscall(SYS_fork);
    if (pid == 0) {
        // Child process - find
        syscall(SYS_close, pipefd[0]);
        syscall(SYS_dup2, pipefd[1], STDOUT_FILENO);
        syscall(SYS_close, pipefd[1]);

        const char* args[] = {"find", ".", nullptr};
        syscall(SYS_execve, "/usr/bin/find", (char* const*)args, nullptr);
        perror("execve");
        _exit(1);
    } else if (pid > 0) {
        // Parent process
        syscall(SYS_close, pipefd[1]);
        
        pid_t pid2 = syscall(SYS_fork);
        if (pid2 == 0) {
            // Second child process - grep
            syscall(SYS_close, pipefd[1]);
            syscall(SYS_dup2, pipefd[0], STDIN_FILENO);
            syscall(SYS_close, pipefd[0]);

            const char* args[] = {"grep", searchString.c_str(), nullptr};
            syscall(SYS_execve, "/bin/grep", (char* const*)args, nullptr);
            perror("execve");
            _exit(1);
        } else if (pid2 > 0) {
            // Parent process
            syscall(SYS_close, pipefd[0]);
            syscall(SYS_wait4, pid, nullptr, 0, nullptr); // Wait for the first child (find) to finish
            syscall(SYS_wait4, pid2, nullptr, 0, nullptr); // Wait for the second child (grep) to finish
        } else {
            perror("fork");
        }
    } else {
        perror("fork");
    }
}

void seek() {
    const char* seekFilename = "seek";
    const char* loopFilename = "loop";

    int seekFd = syscall(SYS_open, seekFilename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (seekFd == -1) {
        perror("open");
        return;
    }

    int loopFd = syscall(SYS_open, loopFilename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (loopFd == -1) {
        perror("open");
        return;
    }

    const int dataSize = 5000000;
    char data[dataSize];
    memset(data, '\0', dataSize);

    syscall(SYS_write, seekFd, "x", 1);
    syscall(SYS_lseek, seekFd, dataSize, SEEK_CUR);
    syscall(SYS_write, seekFd, "x", 1);
    syscall(SYS_close, seekFd);

    syscall(SYS_write, loopFd, "x", 1);
    for (int i = 0; i < dataSize; ++i) {
        syscall(SYS_write, loopFd, "\0", 1);
    }
    syscall(SYS_write, loopFd, "x", 1);
    syscall(SYS_close, loopFd);

    std::cout << "Bestanden aangemaakt: " << seekFilename << ", " << loopFilename << std::endl;
}

int main() {
    while (true) {
        std::cout << "Prompt: ";

        std::string input;
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        } else if (input == "new_file") {
            new_file();
        } else if (input == "ls") {
            list();
        } else if (input == "find") {
            find();
        } else if (input == "seek") {
            seek();
        } else {
            std::cout << "Onbekend commando." << std::endl;
        }
    }
    return 0;
}