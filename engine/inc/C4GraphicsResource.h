/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Loads all standard graphics from Graphics.c4g */

class C4GraphicsResource {
public:
  C4GraphicsResource();
  ~C4GraphicsResource();

protected:
  C4Surface sfcGfxResource;
  C4Surface sfcControl;

public:
  C4Group File;
  BYTE GamePalette[256 * 3];
  C4FacetEx fctPlayer;
  C4FacetEx fctSurrender;
  C4FacetEx fctFlag;
  C4FacetEx fctCrew;
  C4FacetEx fctRank;
  C4FacetEx fctFire;
  C4FacetEx fctBackground;
  C4FacetEx fctEntry;
  C4FacetEx fctCaptain;
  C4FacetEx fctMouseCursor;
  C4FacetEx fctSelectMark;
  C4FacetEx fctMenuSymbol;
  C4FacetEx fctMenu;
  C4FacetEx fctFogOfWar;
  C4Facet fctCursor;
  C4Facet fctDropTarget;
  C4Facet fctInsideSymbol;
  C4Facet fctEnergy;
  C4Facet fctConSign;
  C4Facet fctWealthSymbol;
  C4Facet fctValueSymbol;
  C4Facet fctKeyboard;
  C4Facet fctMouse;
  C4Facet fctGamepad;
  C4Facet fctCommand;
  C4Facet fctKey;
  C4Facet fctMagicSymbol;
  C4Facet fctOKCancel;
  C4Facet fctMenuScroll;

public:
  void Default();
  void Clear();
  BOOL Init();
};