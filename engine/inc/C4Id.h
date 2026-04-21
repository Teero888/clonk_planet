/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* 32-bit value to identify object definitions */

typedef DWORD C4ID;

const C4ID C4ID_None       = 0x0,
           C4ID_Clonk      = 0x4B4E4C43, // CLNK
           C4ID_Flag       = 0x47414C46, // FLAG
           C4ID_Conkit     = 0x544B4E43, // CNKT
           C4ID_Loam       = 0x4D414F4C, // LOAM
           C4ID_Metal      = 0x4C54454D, // METL
           C4ID_Gold       = 0x444C4F47, // GOLD
           C4ID_Lorry      = 0x59524F4C, // LORY
           C4ID_Wood       = 0x444F4F57, // WOOD
           C4ID_Meteor     = 0x4F54454D, // METO
           C4ID_Elevator   = 0x56454C45, // ELEV
           C4ID_Linekit		 = 0x544B4E4C, // LNKT
           C4ID_PowerLine  = 0x4C525750, // PWRL
           C4ID_SourcePipe = 0x50495053, // SPIP
           C4ID_DrainPipe  = 0x50495044, // DPIP
					 C4ID_Monster		 = 0x534E4F4D, // MONS
					 C4ID_Shark 		 = 0x4B524853; // SHRK

C4ID C4Id(const char *szId);
void GetC4IdText(C4ID id, char *sBuf);
const char *C4IdText(C4ID id);
BOOL LooksLikeID(const char *szText);