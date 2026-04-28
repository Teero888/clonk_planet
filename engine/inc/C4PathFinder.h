/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Finds the way through the Clonk landscape */

class C4PathFinderRay {
  friend C4PathFinder;

public:
  C4PathFinderRay();
  ~C4PathFinderRay();

public:
  void Clear();
  void Default();

protected:
  int Status;
  int X, Y, X2, Y2, TargetX, TargetY;
  int CrawlStartX, CrawlStartY, CrawlAttach, CrawlLength, CrawlStartAttach;
  int Direction, Depth;
  C4TransferZone *UseZone;
  C4PathFinderRay *From;
  C4PathFinderRay *Next;
  C4PathFinder *pPathFinder;

protected:
  void SetCompletePath();
  void TurnAttach(int &rAttach, int iDirection);
  void CrawlToAttach(int &rX, int &rY, int iAttach);
  void CrawlByAttach(int &rX, int &rY, int iAttach, int iDirection);
  void Draw(C4FacetEx &cgo);
  int FindCrawlAttachDiagonal(int iX, int iY, int iDirection);
  int FindCrawlAttach(int iX, int iY);
  BOOL IsCrawlAttach(int iX, int iY, int iAttach);
  BOOL CheckBackRayShorten();
  BOOL Execute();
  BOOL CrawlTargetFree(int iX, int iY, int iAttach, int iDirection);
  BOOL PointFree(int iX, int iY);
  BOOL Crawl();
  BOOL PathFree(int &rX, int &rY, int iToX, int iToY,
                C4TransferZone **ppZone = NULL);
};

class C4PathFinder {
  friend C4PathFinderRay;

public:
  C4PathFinder();
  ~C4PathFinder();

protected:
  BOOL (*PointFree)(int, int);
  BOOL (*SetWaypoint)(int, int, int, int);
  C4PathFinderRay *FirstRay;
  int WaypointParameter;
  BOOL Success;
  C4TransferZones *TransferZones;

public:
  void Draw(C4FacetEx &cgo);
  void Clear();
  void Default();
  void Init(BOOL (*fnPointFree)(int, int),
            C4TransferZones *pTransferZones = NULL);
  BOOL Find(int iFromX, int iFromY, int iToX, int iToY,
            BOOL (*fnSetWaypoint)(int, int, int, int), int iWaypointParameter);

protected:
  void Run();
  BOOL AddRay(int iFromX, int iFromY, int iToX, int iToY, int iDepth,
              int iDirection, C4PathFinderRay *pFrom,
              C4TransferZone *pUseZone = NULL);
  BOOL SplitRay(C4PathFinderRay *pRay, int iAtX, int iAtY);
  BOOL Execute();
};
