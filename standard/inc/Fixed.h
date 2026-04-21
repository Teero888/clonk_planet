/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Fixed point math extracted from ALLEGRO by Shawn Hargreaves */

/* The Clonk engine uses fixed point math for exact object positions.
   This is rather silly. Nowadays we should simply use floats. However,
	 I never dared changing the whole thing. */

typedef long fixed;

#define FIXED fixed

inline fixed itofix(int x) 
  { 
  return x << 16;
  }

inline int fixtoi(fixed x) 
  { 
  return (x >> 16) + ((x & 0x8000) >> 15);
  }

inline fixed ftofix(double x) 
  { 
  if (x >  32767) return   (long) 0x7FFFFFFF; // Error
  if (x < -32768) return - (long) 0x80000000; // Error
  
  return (long)(x * 65536 + (x < 0 ? -0.5 : 0.5)); 
  }

inline double fixtof(fixed x) 
  { 
  return (double)x / 65536; 
  }

inline fixed fmul(fixed x, fixed y) // This is the slow one...
  {
  return ftofix(fixtof(x)*fixtof(y));
  }

inline fixed fdiv(fixed x, fixed y) // This is the slow one...
  { 
  return ftofix(fixtof(x)/fixtof(y));
  }
