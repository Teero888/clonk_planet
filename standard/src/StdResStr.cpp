/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Load bi-language strings from the resource string table */

#include <Windows.h>

#include <Standard.h>
#include <StdResStr.h>

char ResourceString[500+1];
int ResourceStringExtension=0;
HINSTANCE ResourceInstance=NULL;
void (*ResourceStringUnscramble)(char*)=NULL;

void SetStringResource(HINSTANCE hResInst, BOOL fExtended)
  {
  ResourceInstance=hResInst;
	if (fExtended) ResourceStringExtension = 5000;
	else ResourceStringExtension = 0;
  }

void SetResourceStringUnscramble(void (*pResourceStringUnscramble)(char*))
	{
	ResourceStringUnscramble = pResourceStringUnscramble;
	}

char *LoadResStr(WORD id)
  {
  // Set empty string
  ResourceString[0]=0;
	// Load string (try to load extended string / second language)
  if (!LoadString(ResourceInstance,
									id+ResourceStringExtension,
									ResourceString,500))
		// Load normal string
		LoadString(ResourceInstance,
							 id,
							 ResourceString,500);
	// String is scrambled
	if (ResourceString[0]=='£')
		// Unscrambled with external function if provided
		if (ResourceStringUnscramble)
			{
			ResourceStringUnscramble(ResourceString+1);
			return ResourceString+1;
			}
	// Return string
  return ResourceString;
  }

