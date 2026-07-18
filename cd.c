#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int cp_main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    int source_fd = open(argv[1], O_RDONLY);
    if (source_fd < 0) {
        perror("Failed to open source file");
        return 1;
    }

    int dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dest_fd < 0) {
        perror("Failed to open destination file");
        close(source_fd);
        return 1;
    }

    char buffer[4096];
    long bytes_read;
    long bytes_written;

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        
        if (bytes_written != bytes_read) {
            perror("Failed to write data");
            close(source_fd);
            close(dest_fd);
            return 1;
        }
    }

    if (bytes_read < 0) {
        perror("Failed to read data");
        close(source_fd);
        close(dest_fd);
        return 1;
    }

    if (close(source_fd) < 0) {
        perror("Failed to close source file");
        return 1;
    }
    
    if (close(dest_fd) < 0) {
        perror("Failed to close destination file");
        return 1;
    }

    return 0;
}
