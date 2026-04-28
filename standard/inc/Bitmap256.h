/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A structure for handling 256-color bitmap files */

#pragma pack( push, def_pack )

#pragma pack(1)

class CBitmap256Info
	{
	public:
		CBitmap256Info();
	public:
		BITMAPFILEHEADER Head;
		BITMAPINFOHEADER Info;
		RGBQUAD Colors[256];
	public:
		void Default();
		void Set(int iWdt, int iHgt, BYTE *bypPalette);
		BOOL Valid();
		int FileBitsOffset();
	};

#pragma pack( pop, def_pack )

