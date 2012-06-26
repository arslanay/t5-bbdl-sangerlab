#ifndef __ExampleLib_h
#define __ExampleLib_h

#include <math.h>
#include <GL/glut.h>

const int iNrSegments = 10;
const double AxisLength = 0.25; //meter

const double LeftSideArc = -0.5; //radians
const double RightSideArc = 0.5; //radians

const double MinRadius = 0.28; //meter
const double MaxRadius = 0.64; //meter

const double CenterOffset = -0.45; //meter

const double TotalHeight = 0.4; //meter
const double PosHeigth = 0.2; //meter
const double NegHeight = -0.2; //meter

#ifndef Pi
#define Pi 3.14159265358979
#endif

#ifndef Pi2
#define Pi2 6.28318530717958
#endif

#ifndef DegPerRad
#define DegPerRad 180/Pi
#endif

#ifndef RadPerDeg
#define RadPerDeg Pi/180
#endif

typedef enum FcsAxes{X=0, Y, Z};

void DrawAxes(void);
void DrawWorkspace(void);

#endif


