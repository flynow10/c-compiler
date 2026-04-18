int getColor(int x, int y, int round);

static int WIDTH = 80;
static int HEIGHT = 60;

int main() {
  int round = 0;
  while (1)
  {
    for (int row = 0; row < HEIGHT; row++)
    {
      for (int col = 0; col < WIDTH; col++)
      {
        printInt(round, 0xffffff);
        newLine();
        if(round >= 0) {
          printInt(round, 0xffffff);
        } else {
          printInt(~round + 1, 0xffffff);
        }
        reset();
        int roundAbs = round;
        if(round < 0) {
          roundAbs = -round;
        }
        printCharPos(0x1, row * WIDTH + col, getColor(col, row, roundAbs));
      }
    }
    round++;
    if (round >= 256) {
      round = -255;
    }
  }
}