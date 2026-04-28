/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some wrappers to runtime-library random */

extern int RandomCount;

inline void Randomize()
  {
  srand((unsigned)time(NULL));
	RandomCount=0;
  }

inline void FixedRandom(DWORD dwSeed)
	{
	srand(dwSeed);
	RandomCount=0;
	}

inline int Random(int iRange)
  {
	RandomCount++;
  if (iRange==0) return 0;
  return (rand()%iRange);
  }
