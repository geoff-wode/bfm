#if ! defined(__FRAME_BUFFER__)
#define __FRAME_BUFFER__

#include <texture2d.h>
#include <boost/shared_ptr.hpp>
#include <gl_loader/gl_loader.h>

//--------------------------------------------------------

class RenderTarget
{
public:
  virtual ~RenderTarget();

  // Sets this render target to be a source of data.
  void SetAsSource();

  // Sets this render target to be a destination of data.
  void SetAsDestination();
  Texture2D texture;

protected:
  RenderTarget(size_t width, size_t height);
};

//--------------------------------------------------------

class ColourRenderTarget : public RenderTarget
{
public:
  ColourRenderTarget(size_t width, size_t height);
  virtual ~ColourRenderTarget();
};

//--------------------------------------------------------

class DepthRenderTarget : public RenderTarget
{
public:
  DepthRenderTarget(size_t width, size_t height);
  virtual ~DepthRenderTarget();
};

//--------------------------------------------------------

class FrameBuffer
{
public:
  FrameBuffer();
  ~FrameBuffer();

  void Enable();
  void Disable();

  void AttachRenderTarget(boost::shared_ptr<DepthRenderTarget> depthRenderTarget);
  void AttachRenderTarget(size_t count, boost::shared_ptr<ColourRenderTarget> colourRenderTargets[]);
  void AttachRenderTarget(size_t slot, boost::shared_ptr<ColourRenderTarget> colourRenderTarget);

private:
  boost::shared_ptr<DepthRenderTarget> depthRenderTarget;

  static const size_t MaxColourRenderTargets = 4;
  boost::shared_ptr<ColourRenderTarget> colourRenderTargets[MaxColourRenderTargets];

private:
  GLuint handle;
};

#endif // __FRAME_BUFFER__
