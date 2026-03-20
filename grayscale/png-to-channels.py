#!/usr/bin/env python3
import sys
from PIL import Image

def write_channel(pixels, path):
    """Write a 2-D numpy-like array (list of rows) to a space-separated text file."""
    with open(path, "w") as f:
        for row in pixels:
            f.write(" ".join(str(v) for v in row) + "\n")
    print(f"  wrote {path}")


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <input.png>")
        sys.exit(1)

    path = sys.argv[1]
    img  = Image.open(path).convert("RGB")
    width, height = img.size

    # Split into R, G, B 2-D lists
    r_rows, g_rows, b_rows = [], [], []
    for y in range(height):
        r_row, g_row, b_row = [], [], []
        for i in range(8):
            r_row.append(0)
            g_row.append(0)
            b_row.append(0)
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            r_row.append(r)
            g_row.append(g)
            b_row.append(b)
        for i in range(8):
            r_row.append(0)
            g_row.append(0)
            b_row.append(0)
        r_rows.append(r_row)
        g_rows.append(g_row)
        b_rows.append(b_row)

    print(f"Writing channel files for {width}×{height} image...")
    write_channel(r_rows, "initial_red.txt")
    write_channel(g_rows, "initial_green.txt")
    write_channel(b_rows, "initial_blue.txt")
    print("Done.")


if __name__ == "__main__":
    main()