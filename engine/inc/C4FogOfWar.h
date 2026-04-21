/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A visibility map for each player */

class C4FogOfWar  
	{
	public:
		C4FogOfWar();
		~C4FogOfWar();
	protected:
		int Width,Height;
		BYTE *Visibility;
	public:
		void Default();
		void Clear();
		void Draw(C4FacetEx &cgo);
		void AddVisibility(int iX, int iY);
		void Init(int iLandscapeWidth, int iLandscapeHeight);
		void ClearVisibility();
		BOOL IsVisible(int iX, int iY);
	};
