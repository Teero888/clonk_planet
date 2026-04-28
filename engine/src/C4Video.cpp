/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Captures uncompressed AVI from game view */

#include <C4Include.h>

C4Video::C4Video()
	{
	Default();
	}

C4Video::~C4Video()
	{
	Clear();
	}

void C4Video::Default()
	{
	Active=false;
	pAviFile=NULL;
	pAviStream=NULL;
	AviFrame=0;
	AspectRatio=1.333;
	X=0; Y=0; Height=100; Width=(int)(Height*AspectRatio);
	Buffer=NULL;
	BufferSize=0;
	InfoSize=0;
	Recording=false;
	Surface=NULL;
	ShowFlash=0;
	}

void C4Video::Clear()
	{
	Stop();
	}

void C4Video::Init(SURFACE sfcSource, int iWidth, int iHeight)
	{
	// Activate
	Active=true;
	// Set source surface
	Surface=sfcSource;
	// Set video size
	Width=iWidth; Height=iHeight;
	AspectRatio = (double) iWidth / (double) iHeight;
	}

void C4Video::Enlarge()
	{
	Resize(+10);
	}

void C4Video::Reduce()
	{
	Resize(-10);
	}

void C4Video::Execute() // Record frame, draw status
	{
	// Not active
	if (!Active) return;
	// Flash time
	if (ShowFlash) ShowFlash--;
	// Record
	if (Recording) RecordFrame();
	// Draw
	Draw();
	}

bool C4Video::Toggle()
	{
	if (!Recording) return(Start());
	return(Stop());
	}

bool C4Video::Stop()
	{
	// Recording: close file
	if (Recording) AVICloseOutput(&pAviFile,&pAviStream);
	// Recording flag
	Recording = false;
	// Clear buffer
	if (Buffer) delete [] Buffer; Buffer=NULL;
	// Done
	return true;
	}

bool C4Video::Start()
	{
	// Determine new filename
	char szFilename[_MAX_PATH+1]; int iIndex=0;
	do { sprintf(szFilename,"Video%03d.avi",iIndex++); } while (ItemExists(szFilename));
	// Start recording
	return(Start(szFilename));
	}

bool C4Video::Start(const char *szFilename)
	{
	// Stop any recording
	Stop();
	// Open output file
	if (!AVIOpenOutput(szFilename,&pAviFile,&pAviStream,Width,Height))
		{
		Log("AVIOpenOutput failure");
		AVICloseOutput(&pAviFile,&pAviStream); 
		return false; 
		}
	// Create video buffer
	AviFrame=0;
	InfoSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;
	BufferSize = InfoSize + DWordAligned(Width)*Height;
	Buffer = new BYTE[BufferSize];
	// Set bitmap info
  void *pInfo = (void*) Buffer;
	ZeroMem((BYTE*)pInfo,sizeof(BITMAPINFOHEADER));
	( (BITMAPINFO*) pInfo)->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	( (BITMAPINFO*) pInfo)->bmiHeader.biPlanes=1;
	( (BITMAPINFO*) pInfo)->bmiHeader.biWidth=Width;
	( (BITMAPINFO*) pInfo)->bmiHeader.biHeight=Height;
	( (BITMAPINFO*) pInfo)->bmiHeader.biBitCount=8;
	( (BITMAPINFO*) pInfo)->bmiHeader.biCompression=0;
	( (BITMAPINFO*) pInfo)->bmiHeader.biSizeImage=DWordAligned(Width)*Height;
	for (int cnt=0; cnt<256; cnt++)
		{
		( (BITMAPINFO*) pInfo)->bmiColors[cnt].rgbRed = Game.GraphicsResource.GamePalette[cnt*3+0];
		( (BITMAPINFO*) pInfo)->bmiColors[cnt].rgbGreen = Game.GraphicsResource.GamePalette[cnt*3+1];
		( (BITMAPINFO*) pInfo)->bmiColors[cnt].rgbBlue = Game.GraphicsResource.GamePalette[cnt*3+2];
		}
	// Recording flag
	Recording=true;
	// Success
	return true;
	}

void C4Video::Resize(int iChange)
	{
	// Not while recording
	if (Recording) return;
	// Resize
	Width = BoundBy( Width+iChange, 50, 640 );
	Height = BoundBy( (int) ((double)Width/AspectRatio), 50, 480 );
	// Adjust position
	AdjustPosition();
	// Show flash
	ShowFlash=10;
	}

void C4Video::Draw(C4FacetEx &cgo)
	{
	// Not active
	if (!Active) return;
	// No show
	if (!ShowFlash) return;
	// Draw frame
	Engine.DDraw.DrawFrame(cgo.Surface, X+cgo.X,Y+cgo.Y,
												 X+cgo.X+Width-1,Y+cgo.Y+Height-1,
												 Recording ? CRed : CWhite);
	// Draw status
	if (Recording) sprintf(OSTR,"%dx%d Frame %d",Width,Height,AviFrame);
	else sprintf(OSTR,"%dx%d",Width,Height,AviFrame);
	Engine.DDraw.TextOut(OSTR,cgo.Surface,cgo.X+X,cgo.Y+Y-Engine.DDraw.TextHeight()-3,Recording ? FRed : FWhite);
	}

bool C4Video::AdjustPosition()
	{
	// Get source player & viewport
	C4Viewport *pViewport = Game.GraphicsSystem.GetViewport(0);
	if (!pViewport) return false;
	C4Player *pPlr = Game.Players.Get(0);
	if (!pPlr) return false;
	// Set camera position
	X = pPlr->ViewX - pViewport->ViewX + pViewport->DrawX - Width/2;
	X = BoundBy( X, 0, pViewport->ViewWdt - Width );
	Y = pPlr->ViewY - pViewport->ViewY + pViewport->DrawY - Height*2/3;
	Y = BoundBy( Y, 0, pViewport->ViewHgt - Height );
	// Success
	return true;
	}

bool C4Video::RecordFrame()
	{
	// No buffer
	if (!Buffer) return false;
	// Lock source
	int iPitch,iWidth,iHeight;
	BYTE *bypBits = LockSurface(Surface,iPitch,&iWidth,&iHeight);
	if (!bypBits) { Log("Video: lock surface failure"); Stop(); return false; }
	// Adjust source position
	if (!AdjustPosition()) { Log("Video: player/viewport failure"); Stop(); return false; }
	// Blit screen to buffer
	StdBlit(bypBits,iPitch,-iHeight,
					X,Y,Width,Height,
					Buffer+InfoSize,
					DWordAligned(Width),Height,
					0,0,Width,Height,
					1);
	// Unlock source
	UnLockSurface(Surface);
	// Write frame to file
	if (!AVIPutFrame(pAviStream,
									 AviFrame,
		               Buffer,InfoSize,
									 Buffer+InfoSize,BufferSize-InfoSize))
		{ Log("AVIPutFrame failure"); Stop(); return false; }
	// Advance frame
	AviFrame++;
	// Show flash
	ShowFlash=1;
	// Success
	return true;
	}

void C4Video::Draw()
	{
	// Not active
	if (!Active) return;
	// Get viewport
	C4Viewport *pViewport;
	if (pViewport = Game.GraphicsSystem.GetViewport(0))
		{
		C4FacetEx cgo; 
		cgo.Set(Surface,pViewport->DrawX,pViewport->DrawY,pViewport->ViewWdt,pViewport->ViewHgt,pViewport->ViewX,pViewport->ViewY);
		Draw(cgo);
		}
	}
