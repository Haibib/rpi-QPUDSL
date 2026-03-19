// game-of-life.c
#include "libunix.h"
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "put-code.h"
#define ROWS 258
#define COLS 258
#define TOTAL_CELLS  (ROWS * COLS)
#define PACKED_BYTES ((TOTAL_CELLS + 7) / 8)
#define READY_SIGNAL 0xAB
#define FRAME_PERIOD_USEC 100000

uint8_t game_of_life_grid[ROWS][COLS];
static int visualizer_fd = -1;

static void usage(const char *msg) {
    output("%s\n", msg);
    output("usage: input-program --fd <n> --input <file>\n");
    exit(1);
}

void read_initial_state(int pi_fd, char* input_file){
    if(!input_file)
        usage("missing required args");
    FILE *f = fopen(input_file, "r");
    if(!f)
        sys_die(fopen, "open input file %s", input_file);
    int character;
    int r = 0, c = 0;
    while((character = fgetc(f)) != EOF) {
        if(character == '\n' || character == 10) {
            r++;
            c = 0;
            continue;
        }
        if(character == '0') {
            game_of_life_grid[r][c] = 0;
        } else if (character == '1') {
            game_of_life_grid[r][c] = 1;
        } else {
            panic("invalid initial value: %d\n", (int)character);
        }
        c++;
    }
    fclose(f);
}

static void send_packed(int fd) {
    uint8_t packed[PACKED_BYTES];
    memset(packed, 0, sizeof packed);
    // treat matrix as flat
    uint8_t *flat = (uint8_t *)game_of_life_grid;
    for (uint32_t i = 0; i < TOTAL_CELLS; i++) {
        if (flat[i])
            packed[i / 8] |= (1 << (i % 8));
    }
    write_exact(fd, packed, PACKED_BYTES);
}

void read_pi_output(int pi_fd) {
    uint8_t packed[PACKED_BYTES];
    uint32_t total = 0;
    while (total < PACKED_BYTES) {
        int n = read(pi_fd, packed + total, PACKED_BYTES - total);
        if (n < 0) {
            sys_die(read, "read failed at byte %d", total);
        }
        total += n;
    }
    uint8_t *flat = (uint8_t *)game_of_life_grid;
    for (uint32_t i = 0; i < TOTAL_CELLS; i++) {
        flat[i] = (packed[i / 8] >> (i % 8)) & 1;
    }
    if (visualizer_fd >= 0) {
        write_exact(visualizer_fd, flat, TOTAL_CELLS);
    }
}

static void start_visualizer(char *cell_size, char* visualize_file) {
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
            "--cell", cell_size,
            NULL
        };
        execvp("python3", argv);
        sys_die(execvp, "can't exec visualize.py");
    }

    // parent: keep write-end, close read-end
    close(pipefd[0]);
    visualizer_fd = pipefd[1];
}

int main(int argc, char **argv) {
    char *input_file = "initial_state.txt";
    char *visualize_file = "visualize.py";
    char *portname = argv[0];
    int visualize = 0;
    char* cell_size = "3";

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--input-file") == 0) {
            if(++i >= argc) usage("missing --input arg");
            input_file = argv[i];
        } else if (strcmp(argv[i], "--visualize") == 0) {
            if(++i >= argc) usage("missing --visualize arg");
            visualize = 1;
            visualize_file = argv[i];
        } else if (strcmp(argv[i], "--cell-size") == 0) {
            if(++i >= argc) usage("missing --cell-size arg");
            cell_size = argv[i];
        } else {
            usage("unexpected argument");
        }
    }
    if (visualize) {
        start_visualizer(cell_size, visualize_file);
    }
    int fd = TRACE_FD;
    read_initial_state(fd, input_file);
    printf("got_initial_state \n");
    uint8_t sig = 0;
    while (read(fd, &sig, 1) != 1 || sig != READY_SIGNAL);
    printf("got start signal \n");
    send_packed(fd);
    while (1) {
        read_pi_output(fd);
    }
    clean_exit("\nbootloader: pi exited.  cleaning up\n");
    notreached();
}