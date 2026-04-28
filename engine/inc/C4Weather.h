/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Controls temperature, wind, and natural disasters */

class C4Weather {
public:
  C4Weather();
  ~C4Weather();

public:
  int Season, YearSpeed, SeasonDelay;
  int Wind, TargetWind;
  int Temperature, TemperatureRange, Climate;
  int MeteoriteLevel, VolcanoLevel, EarthquakeLevel, LightningLevel;

public:
  void Default();
  void Clear();
  void Execute();
  void SetClimate(int iClimate);
  void SetSeason(int iSeason);
  void SetTemperature(int iTemperature);
  void ScenarioInit();
  void SetWind(int iWind);
  int GetWind(int x, int y);
  int GetTemperature();
  int GetSeason();
  int GetClimate();
  BOOL LaunchLightning(int x, int y, int xdir, int xrange, int ydir, int yrange);
  BOOL LaunchVolcano(int mat, int x, int y, int size);
  BOOL LaunchEarthquake(int iX, int iY);
  BOOL LaunchCloud(int iX, int iY, int iWidth, int iStrength, const char *szPrecipitation);
};
