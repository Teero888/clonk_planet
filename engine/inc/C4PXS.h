/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Pixel Sprite system for tiny bits of moving material */

class C4PXS
  {
	friend C4PXSSystem;
  protected:   
    int Mat;
    double x,y,xdir,ydir;
  protected:
    void Execute();
    void Deactivate();
  };

const int PXSChunkSize=500,PXSMaxChunk=20;

class C4PXSSystem  
	{
	public:
		C4PXSSystem();
		~C4PXSSystem();
	public:
		int Count;
	protected:
		C4PXS *Chunk[PXSMaxChunk];
	public:
		void Default();
		void Clear();
		void Execute();
		void Draw(C4FacetEx &cgo);
		void Synchronize();
		void Cast(int mat, int num, int tx, int ty, int level);
		BOOL Create(int mat, double ix, double iy, double ixdir=0, double iydir=0);
		BOOL Load(C4Group &hGroup);
		BOOL Save(C4Group &hGroup);
	protected:
		C4PXS *New();
	};

