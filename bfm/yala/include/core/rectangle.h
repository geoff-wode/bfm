#if ! defined(__RECTANGLE__)
#define __RECTANGLE__

#include <glm/glm.hpp>

class Rectangle
{
public:
  Rectangle()
    : top(0), left(0), width(0), height(0)
  { }

  Rectangle(const Rectangle& other)
    : top(other.top), left(other.left), width(other.width), height(other.height)
  { }

  Rectangle(const glm::ivec2& topLeft, const glm::ivec2& size)
    : top(topLeft.y), left(topLeft.x), width(size.x), height(size.y)
  { }

  Rectangle(int x, int y, int w, int h)
    : top(y), left(x), width(w), height(h)
  { }

  int Bottom() const { return top + height; }
  
  int Right() const { return left + width; }

  int top;
  int left;
  int width;
  int height;
};

static bool operator==(const Rectangle& a, const Rectangle& b)
{
  return (a.top == b.top) && (a.left == b.left) && (a.width == b.width) && (a.height == b.height);
}

static bool operator!=(const Rectangle& a, const Rectangle& b)
{
  return !((a.top == b.top) || (a.left == b.left) || (a.width == b.width) || (a.height == b.height));
}

#endif // __RECTANGLE__
