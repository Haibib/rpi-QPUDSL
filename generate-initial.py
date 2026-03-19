import argparse
import random
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("outfile", help="Path to the output .txt file")
    parser.add_argument("N", type=int, help="Size of the square grid")
    args = parser.parse_args()

    n = args.N
    path = Path(args.outfile)

    with path.open("w", encoding="utf-8") as f:
        for _ in range(n):
            row = "".join(str(random.randint(0, 1)) for _ in range(n))
            f.write(row + "\n")


if __name__ == "__main__":
    main()