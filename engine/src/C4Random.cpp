/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Buffered fast and network-safe random */

#include <C4Include.h>

//--------------------------------------- Random3 -------------------------------------------------------

const int FRndRes = 500;

int FRndBuf3[FRndRes];
int FRndPtr3;

void Randomize3() {
  FRndPtr3 = 0;
  for (int cnt = 0; cnt < FRndRes; cnt++)
    FRndBuf3[cnt] = Random(3) - 1;
}

int Rnd3() {
  FRndPtr3++;
  if (FRndPtr3 == FRndRes)
    FRndPtr3 = 0;
  return FRndBuf3[FRndPtr3];
}

//------------------------------------ Safe Random --------------------------------------------------------

const int SafeRndRes = 500;

int SafeRndBuf[SafeRndRes];
int SafeRndPtr = 0;

void InitSafeRandom() {
  int cnt;
  for (cnt = 0; cnt < SafeRndRes; cnt++)
    SafeRndBuf[cnt] = Random(100);
}

int SafeRandom(int range) {
  SafeRndPtr++;
  if (SafeRndPtr == SafeRndRes)
    SafeRndPtr = 0;
  if (range == 0)
    return 0;
  return SafeRndBuf[SafeRndPtr] % range;
}
