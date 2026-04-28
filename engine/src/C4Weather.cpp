/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Controls temperature, wind, and natural disasters */

#include <C4Include.h>

C4Weather::C4Weather() { Default(); }

C4Weather::~C4Weather() { Clear(); }

void C4Weather::ScenarioInit() {
  // Season
  Season = Game.C4S.Weather.StartSeason.Evaluate();
  YearSpeed = Game.C4S.Weather.YearSpeed.Evaluate();
  // Temperature
  Climate = 100 - Game.C4S.Weather.Climate.Evaluate() - 50;
  Temperature = Climate;
  // Wind
  Wind = TargetWind = Game.C4S.Weather.Wind.Evaluate();
  // Precipitation
  if (!Game.C4S.Head.NoInitialize)
    if (Game.C4S.Weather.Rain.Evaluate())
      for (int iClouds = Min(GBackWdt / 500, 5); iClouds > 0; iClouds--)
        LaunchCloud(Random(GBackWdt), -1, GBackWdt / 15 + Random(320),
                    Game.C4S.Weather.Rain.Evaluate(),
                    Game.C4S.Weather.Precipitation);
  // Lightning
  LightningLevel = Game.C4S.Weather.Lightning.Evaluate();
  // Disasters
  MeteoriteLevel = Game.C4S.Disasters.Meteorite.Evaluate();
  VolcanoLevel = Game.C4S.Disasters.Volcano.Evaluate();
  EarthquakeLevel = Game.C4S.Disasters.Earthquake.Evaluate();
}

void C4Weather::Execute() {
  // Season
  if (!Tick35) {
    SeasonDelay += YearSpeed;
    if (SeasonDelay >= 200) {
      SeasonDelay = 0;
      Season++;
      if (Season > Game.C4S.Weather.StartSeason.Max)
        Season = Game.C4S.Weather.StartSeason.Min;
    }
  }
  // Temperature
  if (!Tick35) {
    int iTemperature =
        Climate - (int)(TemperatureRange * cos(6.28 * (float)Season / 100.0));
    if (Temperature < iTemperature)
      Temperature++;
    else if (Temperature > iTemperature)
      Temperature--;
  }
  // Wind
  if (!Tick1000)
    TargetWind = Game.C4S.Weather.Wind.Evaluate();
  if (!Tick3)
    Wind = BoundBy(Wind + Sign(TargetWind - Wind), Game.C4S.Weather.Wind.Min,
                   Game.C4S.Weather.Wind.Max);
  if (!Tick10)
    SoundLevel("Wind", Max(Abs(Wind) - 30, 0) * 2);
  // Disaster launch
  if (!Tick10) {
    // Meteorite
    if (!Random(60))
      if (Random(100) < MeteoriteLevel) {
        C4Object *meto;
        meto =
            Game.CreateObject(C4ID_Meteor, NO_OWNER, Random(GBackWdt), -10, 0,
                              ftofix((float)(Random(100 + 1) - 50) / 10.0),
                              ftofix(0.0), ftofix(0.2));
      }
    // Lightning
    if (!Random(35))
      if (Random(100) < LightningLevel)
        LaunchLightning(Random(GBackWdt), 0, -20, 41, +5, 15);
    // Earthquake
    if (!Random(50))
      if (Random(100) < EarthquakeLevel)
        LaunchEarthquake(Random(GBackWdt), Random(GBackHgt));
    // Volcano
    if (!Random(60))
      if (Random(100) < VolcanoLevel)
        LaunchVolcano(Game.Material.Get("Lava"), Random(GBackWdt), GBackHgt - 1,
                      BoundBy(15 * GBackHgt / 500 + Random(10), 10, 60));
  }
}

void C4Weather::Clear() {}

BOOL C4Weather::LaunchLightning(int x, int y, int xdir, int xrange, int ydir,
                                int yrange) {
  C4Object *pObj;
  if (pObj = Game.CreateObject(C4Id("FXL1")))
    pObj->Call(PSF_Activate, x, y, xdir, xrange, ydir, yrange);
  return TRUE;
}

int C4Weather::GetWind(int x, int y) {
  if (GBackIFT(x, y))
    return 0;
  return Wind;
}

int C4Weather::GetTemperature() { return Temperature; }

BOOL C4Weather::LaunchVolcano(int mat, int x, int y, int size) {
  C4Object *pObj;
  if (pObj = Game.CreateObject(C4Id("FXV1")))
    pObj->Call(PSF_Activate, x, y, size, mat);
  return TRUE;
}

void C4Weather::Default() {
  Season = 0;
  YearSpeed = 0;
  SeasonDelay = 0;
  Wind = TargetWind = 0;
  Temperature = Climate = 0;
  TemperatureRange = 30;
  MeteoriteLevel = VolcanoLevel = EarthquakeLevel = LightningLevel = 0;
}

BOOL C4Weather::LaunchEarthquake(int iX, int iY) {
  C4Object *pObj;
  if (pObj = Game.CreateObject(C4Id("FXQ1"), NO_OWNER, iX, iY))
    if (pObj->Call(PSF_Activate))
      return TRUE;
  return FALSE;
}

BOOL C4Weather::LaunchCloud(int iX, int iY, int iWidth, int iStrength,
                            const char *szPrecipitation) {
  if (Game.Material.Get(szPrecipitation) == MNone)
    return FALSE;
  C4Object *pObj;
  if (pObj = Game.CreateObject(C4Id("FXP1"), NO_OWNER, iX, iY))
    if (pObj->Call(PSF_Activate, Game.Material.Get(szPrecipitation), iWidth,
                   iStrength))
      return TRUE;
  return FALSE;
}

void C4Weather::SetWind(int iWind) {
  Wind = BoundBy(iWind, -100, +100);
  TargetWind = BoundBy(iWind, -100, +100);
}

void C4Weather::SetTemperature(int iTemperature) {
  Temperature = BoundBy(iTemperature, -100, 100);
}

void C4Weather::SetSeason(int iSeason) { Season = BoundBy(iSeason, 0, 100); }

int C4Weather::GetSeason() { return Season; }

void C4Weather::SetClimate(int iClimate) {
  Climate = BoundBy(iClimate, -50, +50);
}

int C4Weather::GetClimate() { return Climate; }
