/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* 32-bit value to identify object definitions */

#include <C4Include.h>

C4ID C4Id(const char *szId) 
  {
  if (!szId) return C4ID_None;	
	// Numerical id
	if (Inside(szId[0],'0','9') && Inside(szId[1],'0','9') && Inside(szId[2],'0','9') && Inside(szId[3],'0','9'))
		{
		int iResult;
		sscanf(szId,"%d",&iResult);
		return iResult;
		}
	// Literal id
	return (((DWORD)szId[3])<<24) + (((DWORD)szId[2])<<16) + (((DWORD)szId[1])<<8) + ((DWORD)szId[0]);
  }

static char C4IdTextBuffer[5];

const char *C4IdText(C4ID id)
	{
	GetC4IdText(id,C4IdTextBuffer);
	return C4IdTextBuffer;
	}

void GetC4IdText(C4ID id, char *sBuf)
  {
	// Invalid parameters
  if (!sBuf) return;
	// No id
	if (id==C4ID_None) { SCopy("NONE",sBuf); return; }
	// Numerical id
	if (Inside(id,0,9999))
		{
		sprintf(sBuf,"%04i",id);
		}
	// Literal id
	else
		{
		sBuf[0]= (char) ((id & 0x000000FF) >>  0);
		sBuf[1]= (char) ((id & 0x0000FF00) >>  8);
		sBuf[2]= (char) ((id & 0x00FF0000) >> 16);
		sBuf[3]= (char) ((id & 0xFF000000) >> 24);
		sBuf[4]= 0;
		}
  }

BOOL LooksLikeID(const char *szText)
  {
  int cnt;
  if (SLen(szText)!=4) return FALSE;
  for (cnt=0; cnt<4; cnt++)
    if (!(Inside(szText[cnt],'A','Z') || Inside(szText[cnt],'0','9') || Inside(szText[cnt],'_','_'))) 
      return FALSE;
  return TRUE;
  }

