/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Another C4Group bitmap-to-surface loader and saver */

class C4Group;

SURFACE GroupReadSurface(C4Group &hGroup, BYTE *bpPalette=NULL);

BOOL SaveSurface(const char *szFilename, SURFACE sfcSurface, BYTE *bpPalette);
