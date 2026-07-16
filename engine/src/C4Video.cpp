/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Captures uncompressed AVI from game view */

#include <C4Include.h>

C4Video::C4Video() { Default(); }

C4Video::~C4Video() { Clear(); }

void C4Video::Default() {
  Active = false;
  pAviFile = NULL;
  pAviStream = NULL;
  AviFrame = 0;
  AspectRatio = 1.333;
  X = 0;
  Y = 0;
  Height = 100;
  Width = (int)(Height * AspectRatio);
  Buffer = NULL;
  BufferSize = 0;
  InfoSize = 0;
  Recording = false;
  Surface = NULL;
  ShowFlash = 0;
}

void C4Video::Clear() {}

void C4Video::Init(SURFACE sfcSource, int iWidth, int iHeight) {
  Active = false;
}

void C4Video::Enlarge() {}

void C4Video::Reduce() {}

void C4Video::Execute() {}

bool C4Video::Toggle() { return false; }

bool C4Video::Stop() { return true; }

bool C4Video::Start() { return false; }

bool C4Video::Start(const char *szFilename) { return false; }

void C4Video::Resize(int iChange) {}

void C4Video::Draw(C4FacetEx &cgo) {}

bool C4Video::AdjustPosition() { return false; }

bool C4Video::RecordFrame() { return false; }

void C4Video::Draw() {}
