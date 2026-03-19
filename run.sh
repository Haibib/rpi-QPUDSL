if [[ -z "$1" ]]; then
  echo "Error: Requires program name as argument" >&2
  exit 1
fi

PROGRAM="$1"
CELL_SIZE="${2:-"5"}"
ROOT_DIR=$PWD
PROGRAM_DIR="$ROOT_DIR/$PROGRAM"

make -C "$ROOT_DIR/libpi/"
make -C "$PROGRAM_DIR"
make -C "$PROGRAM_DIR/input-program/"

./my-install "$PROGRAM_DIR/run-$PROGRAM.bin"  \
    --baud 460800  \
    --exec "$PROGRAM_DIR/input-program/$PROGRAM-unix" \
    --input-file "$PROGRAM_DIR/initial_state.txt" \
    --cell-size $CELL_SIZE \
    --visualize "$PROGRAM_DIR/visualize.py"