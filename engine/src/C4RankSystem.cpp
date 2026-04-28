/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Rank list for players or crew members */

#include <C4Include.h>

C4RankSystem::C4RankSystem()
  {
	Register[0]=0;
	RankName[0]=0;
	RankBase=1000;
  }

int C4RankSystem::Init(const char *szRegister, 
											 const char *szDefRanks,
											 int iRankBase)
  {
	int crank=0;
	
	// Init
	SCopy(szRegister,Register,256);
	RankBase=iRankBase;

	// Check registry for present rank names and set defaults
	char rankname[C4MaxName+1],keyname[30];
	BOOL Checking=TRUE;
	while (Checking)
		{
		sprintf(keyname,"Rank%03d",crank+1);
		if (GetRegistryString(Register,keyname,rankname,C4MaxName+1))
			{
			// Rank present
			crank++;
			}
		else
			{
			// Rank not defined, check for default
			if (SCopySegment(szDefRanks,crank,rankname,'|',C4MaxName)
			 && SetRegistryString(Register,keyname,rankname))
				crank++;
			else
				Checking=FALSE;
			}
		}

  return crank;
  }

const char *C4RankSystem::Name(int iRank)
  {
  if (iRank<0) return NULL;
	char keyname[30];
	sprintf(keyname,"Rank%03d",iRank+1);
	if (GetRegistryString(Register,keyname,RankName,C4MaxName+1))
		return RankName;
  return NULL;	
	}

int C4RankSystem::Experience(int iRank)
  {
  if (iRank<0) return 0;
  return (int) ( pow (iRank, 1.5) * RankBase );
  }

BOOL C4RankSystem::Check(int iRank, const char  *szDefRankName)
	{
	char rankname[C4MaxName+1],keyname[30];
	sprintf(keyname,"Rank%03d",iRank);
	if (GetRegistryString(Register,keyname,rankname,C4MaxName+1))
		return FALSE;
	if (!szDefRankName || (SLen(szDefRankName)>C4MaxName))
		return FALSE;
	return SetRegistryString(Register,keyname,szDefRankName);
	}

void C4RankSystem::Clear()
	{

	}

void C4RankSystem::Default()
	{

	}
