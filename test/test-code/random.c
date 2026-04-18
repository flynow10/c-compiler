unsigned int lfsr32, lfsr31;


int shift_lfsr(unsigned int *lfsr, unsigned int polynomial_mask)
{
  int feedback;

  feedback = *lfsr & 1;
  *lfsr >>= 1;

  if (feedback == 1)
  {
    *lfsr ^= polynomial_mask;
  }

  return *lfsr;
}

void init_lfsrs(void)
{
  lfsr32 = 0xABCDE;
  lfsr31 = 0x23456789;
}

int get_random(void)
{
  shift_lfsr(&lfsr32, 0xB4BCD35C);
  return (shift_lfsr(&lfsr32, 0xB4BCD35C) ^ shift_lfsr(&lfsr31, 0x7A5BC2E3)) & 0xffff;
}

int main() {
  init_lfsrs();
  int i = get_random();
}
