#!/usr/bin/env python3
"""
Reads raw cell frames from stdin and renders them with pygame.
Protocol: exactly ROWS * COLS bytes per frame, one byte per cell (0 or 1).
Extensible: pass --rgb to instead expect ROWS * COLS * 3 bytes per frame (R,G,B).
"""
import sys
import numpy as np
import pygame

ROWS = 446
COLS = 576

def main():
    args = sys.argv[1:]
    rows  = int(args[args.index("--rows") + 1])
    cols  = int(args[args.index("--cols") + 1])
    cell_size = int(args[args.index("--cell") + 1]) if "--cell" in args else 1
    rgb_mode  = "--rgb" in args

    bytes_per_frame = rows * cols * (3 if rgb_mode else 1)

    pygame.init()
    screen = pygame.display.set_mode((cols * cell_size, rows * cell_size))
    pygame.display.set_caption("Conway's Game of Life")

    buf = sys.stdin.buffer

    # Read exactly one frame from the pipe.
    data = bytearray()
    while len(data) < bytes_per_frame:
        chunk = buf.read(bytes_per_frame - len(data))
        if not chunk:
            pygame.quit()
            sys.exit(0)
        data.extend(chunk)

    raw = np.frombuffer(data, dtype=np.uint8)

    if rgb_mode:
        pixels = raw.reshape(rows, cols, 3)
    else:
        grey   = (raw * 255).reshape(rows, cols)
        pixels = np.stack([grey, grey, grey], axis=-1)

    surface = pygame.surfarray.make_surface(
        np.ascontiguousarray(pixels.transpose(1, 0, 2))
    )
    if cell_size != 1:
        surface = pygame.transform.scale(surface, (cols * cell_size, rows * cell_size))

    screen.blit(surface, (0, 0))
    pygame.display.flip()

    # Static display — just wait for the window to be closed.
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit(0)

if __name__ == "__main__":
    main()