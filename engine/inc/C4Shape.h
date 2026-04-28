/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Basic classes for rectangles and vertex outlines */

class C4Rect 
  { 
  public:
    int x,y,Wdt,Hgt; 
	public:
		void Set(int iX, int iY, int iWdt, int iHgt);
		void Default();
		BOOL Overlap(C4Rect &rTarget);
  };

class C4TargetRect: public C4Rect
  {
  public:
    int tx,ty;
  public:
	  void Set(int iX, int iY, int iWdt, int iHgt, int iTX, int iTY);
	  void Default();
  };

class C4Shape: public C4Rect
  { 
  public:
	  C4Shape();
  public:
    int FireTop;
    int VtxNum;
    int VtxX[C4D_MaxVertex];
    int VtxY[C4D_MaxVertex];  
    int VtxCNAT[C4D_MaxVertex];
    int VtxFriction[C4D_MaxVertex];
		int ContactCNAT;
		int ContactCount;
		int AttachMat;
    int VtxContactCNAT[C4D_MaxVertex];
    int VtxContactMat[C4D_MaxVertex];
  public:
	  void Default();
    void Clear();
		void Rotate(int iAngle);
		void Stretch(int iPercent);
		void Jolt(int iPercent);
		void GetVertexOutline(C4Rect &rRect);
	  int GetVertexY(int iVertex);
	  int GetVertexX(int iVertex);
	  BOOL AddVertex(int iX, int iY);
		BOOL CheckContact(int cx, int cy);
		BOOL ContactCheck(int cx, int cy);
		BOOL Attach(int &cx, int &cy, BYTE cnat_pos);
		BOOL LineConnect(int tx, int ty, int cvtx, int ld);
		BOOL InsertVertex(int iPos, int tx, int ty);
		BOOL RemoveVertex(int iPos);
  };         
