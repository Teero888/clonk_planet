/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A primitive list to store one amount value per mapped material */

class C4MaterialList  
	{
	public:
		C4MaterialList();
		~C4MaterialList();
	public:
		int Amount[C4MaxMaterial];
	public:
		void Default();
		void Clear();
		void Reset();
		int Get(int iMaterial);
		void Add(int iMaterial, int iAmount);
		void Set(int iMaterial, int iAmount);
	};
