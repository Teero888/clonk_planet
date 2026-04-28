/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Text messages drawn inside the viewport */

const int C4GM_MaxText = 256;

class C4GameMessage  
	{
	friend C4GameMessageList;
	public:
		void Draw(C4FacetEx &cgo, int iPlayer);
		C4GameMessage();
		~C4GameMessage();
	protected:
		int X,Y,Hgt;
		int Delay;
		int Color;
		int Player;
		C4Object *Target;
		char Text[C4GM_MaxText+1];
		C4GameMessage *Next;
	protected:
		void Init(const char *szText, C4Object *pTarget, int iPlayer, int iX, int iY, BYTE bCol);
		BOOL Execute();
	};

class C4GameMessageList
	{
	public:
		C4GameMessageList();
		~C4GameMessageList();
	protected:
		C4GameMessage *First;
	public:
		void Default();
		void Clear();
		void Execute();
		void Draw(C4FacetEx &cgo, int iPlayer);
		void ClearPlayers(int iPlayer);
		void ClearPointers(C4Object *pObj);
		BOOL New(const char *szText, C4Object *pTarget, int iPlayer, int iX, int iY, BYTE bCol);
	};	
