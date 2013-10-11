#include <rterendergeometryeffect.h>

RTERenderGeometryEffect::RTERenderGeometryEffect()
{
}

RTERenderGeometryEffect::~RTERenderGeometryEffect()
{
}

void RTERenderGeometryEffect::Initialise()
{
  RTEWorldViewProjectionMatrix = &parameters["RTEWorldViewProjectionMatrix"];
  CameraPositionLow = &parameters["CameraPositionLow"];
  CameraPositionHigh = &parameters["CameraPositionHigh"];
  LogDepthConstant = &parameters["LogDepthConstant"];
  FarClipPlaneDistance = &parameters["FarClipPlaneDistance"];
}
