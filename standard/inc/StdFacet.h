/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A very primitive piece of surface */

class CFacet  
	{
	public:
		CFacet();
		~CFacet();
  public: 
    SURFACE Surface; 
    int X,Y,Wdt,Hgt; 
  public:
	  void Draw(SURFACE sfcSurface, int iX, int iY, int iPhaseX=0, int iPhaseY=0);
	  void Default();
		void Clear();
    void Set(SURFACE nsfc, int nx, int ny, int nwdt, int nhgt);
	};
