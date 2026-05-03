//
// Created by Natalie Wagner on 4/17/26.
//

struct Move {
    short move_value;
    struct Move *next;
};

struct Coordinate {
    int rank;
    int file;
};

struct Board {
    int squares[8][8];
    int isWhite;
    struct Move *move_list;
};

void *malloc(unsigned int size);

struct Coordinate indexToCoord(const int index) {
    const struct Coordinate coord = {index % 8, index / 8};
    return coord;
}

struct Board *createBoard() {
    struct Board *board = (struct Board *)malloc(sizeof(struct Board));
    board->squares[0][0] = 0x1 << 2;
    return board;
}

struct Move *createMove(int startSquare, int endSquare, int flags) {
    struct Move *move = (struct Move *)malloc(sizeof(struct Move));
    move->move_value = startSquare & 0x3f | (endSquare & 0x3f) << 6 | (flags & 0xf) << 12;
    move->next = 0;
    return move;
}

void makeMove(struct Board *board, struct Move *move) {
    move->next = board->move_list;
    board->move_list = move;
    int startSquare = move->move_value & 0x3f;
    int endSquare = (move->move_value >> 6) & 0x3f;

    struct Coordinate startCoord = indexToCoord(startSquare);
    struct Coordinate endCoord = indexToCoord(endSquare);

    int piece = board->squares[startCoord.rank][startCoord.file];
    board->squares[startCoord.rank][startCoord.file] = 0;
    board->squares[endCoord.rank][endCoord.file] = piece;
}