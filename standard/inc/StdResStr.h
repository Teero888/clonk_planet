/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Load bi-language strings from the resource string table */

char *LoadResStr(WORD id);

void SetStringResource(HINSTANCE hResInst, BOOL fExtended=FALSE);

void SetResourceStringUnscramble(void (*pResourceStringUnscramble)(char*));
