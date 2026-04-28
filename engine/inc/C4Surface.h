/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Extension to CSurface that handles bitmaps in C4Group files */

class C4Surface: public CSurface
	{
	public:
		BOOL Load(C4Group &hGroup, const char *szFilename);
		BOOL Save(C4Group &hGroup, const char *szFilename);
	protected:
		BOOL Read(C4Group &hGroup);
	};

