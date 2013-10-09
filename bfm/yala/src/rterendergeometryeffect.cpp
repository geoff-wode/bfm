#include <rterendergeometryeffect.h>

RTERenderGeometryEffect::RTERenderGeometryEffect()
{
  RTEWorldViewProjectionMatrix = &parameters["RTEWorldViewProjectionMatrix"];
  CameraPositionLow = &parameters["CameraPositionLow"];
  CameraPositionHigh = &parameters["CameraPositionHigh"];
}

RTERenderGeometryEffect::~RTERenderGeometryEffect()
{
}

void RTERenderGeometryEffect::Initialise()
{
  RTEWorldViewProjectionMatrix = &parameters["RTEWorldViewProjectionMatrix"];
  CameraPositionLow = &parameters["CameraPositionLow"];
  CameraPositionHigh = &parameters["CameraPositionHigh"];
}
