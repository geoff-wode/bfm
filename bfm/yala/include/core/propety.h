#if ! defined(__PROPERTY_BASE__)
#define __PROPERTY_BASE__

template <typename type>
class Property
{
public:
  Property() : modified(false) { }
  Property(const type& value) : modified(false), value(value) { }

  virtual ~Property() { }

  bool IsModified() const { return modified; }
  void Reset() { modified = false; }
  
  virtual void Set(const type& value)
  {
    this->value = value;
    modified = true;
  }

  virtual type Get() const { return value; }

protected:
  bool modified;
  type value;
};


#endif // __PROPERTY_BASE__
