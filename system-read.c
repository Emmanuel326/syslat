#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static inline uint64_t nsec_now(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0) {
        perror("clock_gettime");
        exit(1);
    }
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr,
                "usage: %s <file> <read_size_bytes> <iterations>\n",
                argv[0]);
        return 1;
    }

    const char *path = argv[1];
    size_t read_size = strtoull(argv[2], NULL, 10);
    size_t iters = strtoull(argv[3], NULL, 10);

    if (read_size == 0 || iters == 0) {
        fprintf(stderr, "read_size and iterations must be > 0\n");
        return 1;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    void *buf;
    if (posix_memalign(&buf, 4096, read_size) != 0) {
        perror("posix_memalign");
        return 1;
    }

    for (size_t i = 0; i < iters; i++) {
        uint64_t start = nsec_now();

        ssize_t n = pread(fd, buf, read_size, 0);
        if (n < 0) {
            perror("pread");
            return 1;
        }
        if ((size_t)n != read_size) {
            fprintf(stderr, "short read: %zd bytes\n", n);
            return 1;
        }

        uint64_t end = nsec_now();
        printf("%lu\n", end - start);
    }

    free(buf);
    close(fd);
    return 0;
}

