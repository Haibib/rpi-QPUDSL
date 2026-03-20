// grayscale-unix.c
#include "libunix.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "put-code.h"
#define ROWS 446
#define COLS 576
#define TOTAL_CELLS  (ROWS * COLS)
#define READY_SIGNAL 0xAB
#define FRAME_PERIOD_USEC 100000

uint8_t grid_r[ROWS][COLS];
uint8_t grid_g[ROWS][COLS];
uint8_t grid_b[ROWS][COLS];
static int visualizer_fd = -1;

static void usage(const char *msg) {
    output("%s\n", msg);
    output("usage: input-program --fd <n> --input <file>\n");
    exit(1);
}

static void read_channel(const char *path, uint8_t grid[ROWS][COLS]) {
    FILE *f = fopen(path, "r");
    if (!f)
        sys_die(fopen, "open channel file %s", path);
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int v;
            if (fscanf(f, "%d", &v) != 1)
                panic("read_channel: unexpected EOF at [%d][%d] in %s\n", r, c, path);
            if (v < 0 || v > 255)
                panic("read_channel: value %d out of range at [%d][%d]\n", v, r, c);
            grid[r][c] = (uint8_t)v;
        }
    }
    fclose(f);
}

static void read_all_channels(void) {
    read_channel("initial_red.txt",   grid_r);
    read_channel("initial_green.txt", grid_g);
    read_channel("initial_blue.txt",  grid_b);
}

static void send_raw(int fd) {
    write_exact(fd, (uint8_t *)grid_r, TOTAL_CELLS);
    write_exact(fd, (uint8_t *)grid_g, TOTAL_CELLS);
    write_exact(fd, (uint8_t *)grid_b, TOTAL_CELLS);
}


void read_pi_output(int pi_fd) {
    // The Pi sends back exactly one grey plane (TOTAL_CELLS bytes).
    // Read it into grid_r; we'll replicate it to g and b for the visualizer.
    uint8_t *grey = (uint8_t *)grid_r;
    uint32_t total = 0;
    while (total < TOTAL_CELLS) {
        int n = read(pi_fd, grey + total, TOTAL_CELLS - total);
        if (n < 0)
            sys_die(read, "read failed at byte %d", total);
        total += n;
    }

    fprintf(stderr, "[debug] grayscale spot-check:\n");
    fprintf(stderr, "  [0][0]           = %3u\n", grey[0]);
    fprintf(stderr, "  [%d][%d] (center) = %3u\n",
            ROWS/2, COLS/2, grey[(ROWS/2)*COLS + COLS/2]);
    fprintf(stderr, "  [%d][%d] (end)    = %3u\n",
            ROWS-1, COLS-1, grey[(ROWS-1)*COLS + COLS-1]);
    fprintf(stderr, "  total bytes read: %u\n", total);

    // Forward to visualizer as interleaved R,G,B with all channels equal
    // (greyscale), built into one buffer to avoid per-pixel write overhead.
    if (visualizer_fd >= 0) {
        uint8_t *interleaved = malloc(TOTAL_CELLS * 3);
        if (!interleaved)
            panic("read_pi_output: out of memory\n");
        for (uint32_t i = 0; i < TOTAL_CELLS; i++) {
            interleaved[i * 3 + 0] = grey[i];
            interleaved[i * 3 + 1] = grey[i];
            interleaved[i * 3 + 2] = grey[i];
        }
        write_exact(visualizer_fd, interleaved, TOTAL_CELLS * 3);
        free(interleaved);
    }
}

static void start_visualizer(char* visualize_file) {
    int pipefd[2];
    if (pipe(pipefd) < 0)
        sys_die(pipe, "can't create visualizer pipe");

    pid_t pid = fork();
    if (pid < 0)
        sys_die(fork, "can't fork visualizer");

    if (pid == 0) {
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) < 0)
            sys_die(dup2, "dup2 failed");
        close(pipefd[0]);
 
        char rows_s[16], cols_s[16];
        snprintf(rows_s, sizeof rows_s, "%d", ROWS);
        snprintf(cols_s, sizeof cols_s, "%d", COLS);
 
        char *argv[] = {
            "python3", visualize_file,
            "--rows", rows_s,
            "--cols", cols_s,
            "--rgb",
            NULL
        };
        execvp("python3", argv);
        sys_die(execvp, "can't exec visualize.py");
    }
 
    close(pipefd[0]);
    visualizer_fd = pipefd[1];
}
 
int main(int argc, char **argv) {
    char *visualize_file = "visualize.py";
    char *portname = argv[0];
    int visualize = 0;
 
    for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--visualize") == 0) {
            if(++i >= argc) usage("missing --visualize arg");
            visualize = 1;
            visualize_file = argv[i];
        }
    }
    if (visualize) {
        start_visualizer(visualize_file);
    }
    int fd = TRACE_FD;
    read_all_channels();
    printf("got_initial_state \n");
    uint8_t sig;
    int n;
    do {
        n = read(fd, &sig, 1);
        if (n < 0)
            sys_die(read, "waiting for READY_SIGNAL");
    } while (n != 1 || sig != READY_SIGNAL);

    send_raw(fd);
    read_pi_output(fd);
    clean_exit("\nbootloader: pi exited.  cleaning up\n");
    notreached();
}