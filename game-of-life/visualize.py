#!/usr/bin/env python3
import sys
import numpy as np
import pygame

ALIVE_COLOR = (0, 0, 0)
DEAD_COLOR = (255, 255, 255)


def build_view(grid, cs, screen_w, screen_h, pan_col, pan_row):
    rows, cols = grid.shape

    vis_cols = screen_w // cs + 1
    vis_rows = screen_h // cs + 1

    c0, c1 = pan_col, min(cols, pan_col + vis_cols)
    r0, r1 = pan_row, min(rows, pan_row + vis_rows)
    sub = grid[r0:r1, c0:c1]
    sr, sc = sub.shape

    expanded = np.repeat(np.repeat(sub.astype(bool), cs, axis=0), cs, axis=1)
    canvas = np.where(expanded[..., None], ALIVE_COLOR, DEAD_COLOR).astype(np.uint8)
    canvas = canvas[:screen_h, :screen_w]
    if canvas.shape[:2] != (screen_h, screen_w):
        out = np.full((screen_h, screen_w, 3), DEAD_COLOR, dtype=np.uint8)
        out[:canvas.shape[0], :canvas.shape[1]] = canvas
        return out

    return canvas


def clamp_pan(pan_col, pan_row, cols, rows, screen_w, screen_h, cs):
    """Keep the viewport inside the grid (integer cell coords)."""
    max_col = max(0, cols - screen_w // cs)
    max_row = max(0, rows - screen_h // cs)
    return max(0, min(max_col, pan_col)), max(0, min(max_row, pan_row))


def blit_grid(screen, grid, cs, screen_w, screen_h, pan_col, pan_row):
    pixels  = build_view(grid, cs, screen_w, screen_h, pan_col, pan_row)
    surface = pygame.surfarray.make_surface(
        np.ascontiguousarray(pixels.transpose(1, 0, 2))
    )
    screen.blit(surface, (0, 0))


def main():
    args      = sys.argv[1:]
    rows      = int(args[args.index("--rows") + 1])
    cols      = int(args[args.index("--cols") + 1])
    base_cs   = int(args[args.index("--cell") + 1]) if "--cell" in args else 3
    rgb_mode  = "--rgb" in args

    bytes_per_frame = rows * cols * (3 if rgb_mode else 1)

    pygame.init()
    screen_w, screen_h = cols * base_cs, rows * base_cs
    screen = pygame.display.set_mode((screen_w, screen_h), pygame.RESIZABLE)
    pygame.display.set_caption("Conway's Game of Life")
    clock = pygame.time.Clock()

    buf = sys.stdin.buffer

    view_cs  = base_cs
    pan_col  = 0
    pan_row  = 0
    dragging   = False
    drag_start = (0, 0)
    drag_pan   = (0, 0)
    current_grid = None

    while True:
        need_redraw = False

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit(0)

            elif event.type == pygame.VIDEORESIZE:
                screen_w, screen_h = event.w, event.h
                screen = pygame.display.set_mode((screen_w, screen_h), pygame.RESIZABLE)
                need_redraw = True

            elif event.type == pygame.MOUSEWHEEL:
                mx, my  = pygame.mouse.get_pos()
                cell_col = pan_col + mx // view_cs
                cell_row = pan_row + my // view_cs
                view_cs  = max(1, min(32, view_cs + event.y))
                pan_col  = cell_col - mx // view_cs
                pan_row  = cell_row - my // view_cs
                pan_col, pan_row = clamp_pan(pan_col, pan_row, cols, rows,
                                             screen_w, screen_h, view_cs)
                need_redraw = True

            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 3:
                dragging   = True
                drag_start = event.pos
                drag_pan   = (pan_col, pan_row)

            elif event.type == pygame.MOUSEBUTTONUP and event.button == 3:
                dragging = False

            elif event.type == pygame.MOUSEMOTION and dragging:
                dx = (event.pos[0] - drag_start[0]) // view_cs
                dy = (event.pos[1] - drag_start[1]) // view_cs
                pan_col = drag_pan[0] - dx
                pan_row = drag_pan[1] - dy
                pan_col, pan_row = clamp_pan(pan_col, pan_row, cols, rows,
                                             screen_w, screen_h, view_cs)
                need_redraw = True

            elif event.type == pygame.KEYDOWN:
                step = max(1, screen_w // view_cs // 10)
                if event.key == pygame.K_LEFT:
                    pan_col -= step
                elif event.key == pygame.K_RIGHT:
                    pan_col += step
                elif event.key == pygame.K_UP:
                    pan_row -= step
                elif event.key == pygame.K_DOWN:
                    pan_row += step
                pan_col, pan_row = clamp_pan(pan_col, pan_row, cols, rows,
                                             screen_w, screen_h, view_cs)
                need_redraw = True

        if need_redraw and current_grid is not None and not rgb_mode:
            blit_grid(screen, current_grid, view_cs, screen_w, screen_h,
                      pan_col, pan_row)
            pygame.display.flip()

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
            surface = pygame.surfarray.make_surface(
                np.ascontiguousarray(pixels.transpose(1, 0, 2))
            )
            if view_cs != 1:
                surface = pygame.transform.scale(
                    surface, (cols * view_cs, rows * view_cs)
                )
            screen.blit(surface, (0, 0))
        else:
            current_grid = raw.reshape(rows, cols)
            blit_grid(screen, current_grid, view_cs, screen_w, screen_h,
                      pan_col, pan_row)

        pygame.display.flip()
        clock.tick(60)

if __name__ == "__main__":
    main()