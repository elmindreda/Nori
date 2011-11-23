///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////
#ifndef WENDY_CORE_H
#define WENDY_CORE_H
///////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include <cstdarg>
#include <cstddef>
#include <cassert>

#ifndef NULL
#define NULL 0
#endif

//////////////////////////////////////////////////////////////////////

#include <glm/gtc/type_precision.hpp>

///////////////////////////////////////////////////////////////////////

#if _MSC_VER
#define WENDY_NORETURN(x) __declspec(noreturn) x
#elif __GNUC__
#define WENDY_NORETURN(x) x __attribute__((noreturn))
#else
#define WENDY_NORETURN(x) x
#endif

#if __GNUC__
#define WENDY_CHECKFORMAT(i, x) x __attribute__((format(printf, i, i + 1)))
#else
#define WENDY_CHECKFORMAT(i, x) x
#endif

#ifdef _MSC_VER

// Don't consider the libc to be obsolete
#pragma warning( disable: 4996 )

// This is only needed for versions below Visual C++ 2008
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif

#endif /*_MSC_VER*/

#if !WENDY_HAVE_VASPRINTF
int vasprintf(char** result, const char* format, va_list vl);
#endif /*WENDY_HAVE_VASPRINTF*/

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

using namespace glm;

///////////////////////////////////////////////////////////////////////

/*! Time value, in seconds.
 */
typedef double Time;

/*! String type.
 */
typedef std::string String;

/*! String hash type.
 */
typedef uint32 StringHash;

///////////////////////////////////////////////////////////////////////

/*! @brief Converts the specified value to a string.
 */
template <typename T>
String stringCast(const T& value);

/*! @brief Converts the specified string to a vec2.
 */
vec2 vec2Cast(const String& string);
/*! @brief Converts the specified string to a vec3.
 */
vec3 vec3Cast(const String& string);
/*! @brief Converts the specified string to a vec4.
 */
vec4 vec4Cast(const String& string);

/*! @brief Converts the specified string to a mat2.
 */
mat2 mat2Cast(const String& string);
/*! @brief Converts the specified string to a mat3.
 */
mat3 mat3Cast(const String& string);
/*! @brief Converts the specified string to a mat4.
 */
mat4 mat4Cast(const String& string);

/*! @brief Converts the specified string to a quat.
 */
quat quatCast(const String& string);

/*! @brief Creates a string using printf formatting.
 */
WENDY_CHECKFORMAT(1, String format(const char* format, ...));

///////////////////////////////////////////////////////////////////////

class RefObject;

///////////////////////////////////////////////////////////////////////

/*! Log entry type enumeration.
  */
enum LogEntryType
{
  /*! The log entry is an error message.
    */
  ERROR_LOG_ENTRY,
  /*! The log entry is a warning, or a non-critical error message.
    */
  WARNING_LOG_ENTRY,
  /*! The log entry is an informational message.
    */
  INFO_LOG_ENTRY,
};

///////////////////////////////////////////////////////////////////////

/*! Returns a hash value of the specified string.
 */
StringHash hashString(const String& string);

/*! Returns a hash value of the specified string.
 */
StringHash hashString(const char* string);

/*! Writes an error message log entry to the log consumers,
 *  or to stderr if there are no log consumers.
 *  @param[in] format The formatting string for the log entry.
 */
WENDY_CHECKFORMAT(1, void logError(const char* format, ...));

/*! Writes a warning message log entry to the log consumers,
 *  or to stderr if there are no log consumers.
 *  @param[in] format The formatting string for the log entry.
 */
WENDY_CHECKFORMAT(1, void logWarning(const char* format, ...));

/*! Writes an informational message log entry to the log consumers,
 *  or to stderr if there are no log consumers.
 *  @param[in] format The formatting string for the log entry.
 */
WENDY_CHECKFORMAT(1, void log(const char* format, ...));

/*! Displays the specified message and terminates the program.
 */
WENDY_CHECKFORMAT(1, WENDY_NORETURN(void panic(const char* format, ...)));

///////////////////////////////////////////////////////////////////////

/*! Base class for exceptions.
 */
class Exception : public std::exception
{
public:
  /*! Constructor.
   *  @param message The message for this exception.
   */
  Exception(const char* message);
  /*! @return The massage for this exception.
   */
  const char* what() const throw();
private:
  const char* message;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Scoped automatic pointer.
 *
 *  Smart pointer. Manages a single object of type T.
 */
template <typename T>
class Ptr
{
public:
  /*! Default constructor.
   */
  explicit Ptr(T* object = NULL);
  /*! Destructor
   */
  virtual ~Ptr();
  /*! Detaches (orphans) the currently owned object.
   * @return The currently owned object.
   * @remarks Use with care.
   */
  T* detachObject();
  /*! Cast operator.
   */
  operator T* ();
  /*! Cast operator.
   */
  operator const T* () const;
  /*! Member operator.
   */
  T* operator -> ();
  /*! Member operator.
   */
  const T* operator -> () const;
  /*! Object assignment operator.
   */
  Ptr<T>& operator = (T* newObject);
  /*! @return The currently owned object.
   */
  T* getObject();
  /*! @return The currently owned object.
   */
  const T* getObject() const;
private:
  Ptr(const Ptr<T>& source);
  Ptr<T>& operator = (const Ptr<T>& source);
  T* object;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for references.
 */
class RefBase
{
protected:
  static bool unreferenced(RefObject* object);
  static void increment(RefObject* object);
  static void decrement(RefObject* object);
};

///////////////////////////////////////////////////////////////////////

/*! @brief Smart reference.
 *
 *  Pointer to objects that inherit from RefObject.
 */
template <typename T>
class Ref : public RefBase
{
public:
  /*! Default constructor.
   */
  Ref(T* object = NULL);
  /*! Copy constructor.
   *  @param source The pointer object to inherit from.
   */
  Ref(const Ref<T>& source);
  /*! Destructor
   */
  ~Ref();
  /*! Detaches the currently referenced object.
   * @return The currently reference object.
   * @remarks Use with care.
   */
  Ref<T> detachObject();
  /*! Cast operator.
   */
  operator T* () const;
  /*! Member operator.
   */
  T* operator -> () const;
  /*! Object assignment operator.
   */
  Ref<T>& operator = (T* newObject);
  /*! Assignment operator.
   */
  Ref<T>& operator = (const Ref<T>& source);
  /*! @return The currently owned object.
   */
  T* getObject() const;
private:
  T* object;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Super class for reference counted object.
 *
 *  @remarks No, there are no visible knobs on this class. Use the Ref class to
 *  point to objects derived from RefObject to enable reference counting.
 */
class RefObject
{
  friend class RefBase;
public:
  /*! Constructor.
   */
  RefObject();
  /*! Copy constructor.
   */
  RefObject(const RefObject& source);
  /*! Destructor.
   */
  virtual ~RefObject();
  /*! Assignment operator.
   */
  RefObject& operator = (const RefObject& source);
private:
  unsigned int count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Singleton template mixin.
 *
 *  Inherit from this to become a compatible singleton.
 */
template <typename T>
class Singleton
{
public:
  /*! Destructor.
   */
  virtual ~Singleton();
  /*! Deletes the singleton instance.
   */
  static void destroySingleton();
  /*! @return The singleton instance if available, otherwise @c NULL.
   */
  static T* getSingleton();
protected:
  /*! Sets the singleton instance.
   *  @param[in] newObject The instance to set.
   */
  static void set(T* newObject);
private:
  static Ptr<T> object;
};

///////////////////////////////////////////////////////////////////////

/*! @breif Interface for receiving %log messages.
 *
 *  All instances of this class are added to an internal list of consumers and
 *  are notified of %log messages until destroyed.
 */
class LogConsumer
{
public:
  /*! Constructor.
   */
  LogConsumer();
  /*! Destructor.
   */
  virtual ~LogConsumer();
  /*! Called for each message generated by log, logWarning and logError.
   */
  virtual void onLogEntry(LogEntryType type, const char* message) = 0;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Ptr<T>::Ptr(T* initObject):
  object(initObject)
{
}

template <typename T>
inline Ptr<T>::~Ptr()
{
  if (object)
    delete object;
}

template <typename T>
inline T* Ptr<T>::detachObject()
{
  T* temp = object;
  object = NULL;
  return temp;
}

template <typename T>
inline Ptr<T>::operator T* ()
{
  return object;
}

template <typename T>
inline Ptr<T>::operator const T* () const
{
  return object;
}

template <typename T>
inline T* Ptr<T>::operator -> ()
{
  return object;
}

template <typename T>
inline const T* Ptr<T>::operator -> () const
{
  return object;
}

template <typename T>
inline Ptr<T>& Ptr<T>::operator = (T* newObject)
{
  if (object)
    delete object;

  object = newObject;
  return *this;
}

template <typename T>
inline T* Ptr<T>::getObject()
{
  return object;
}

template <typename T>
inline const T* Ptr<T>::getObject() const
{
  return object;
}

template <typename T>
inline Ptr<T>::Ptr(const Ptr<T>& source):
  object(NULL)
{
}

template <typename T>
inline Ptr<T>& Ptr<T>::operator = (const Ptr<T>& source)
{
  operator = (source.detachObject());

  return *this;
}

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Ref<T>::Ref(T* initObject):
  object(NULL)
{
  operator = (initObject);
}

template <typename T>
inline Ref<T>::Ref(const Ref<T>& source):
  object(NULL)
{
  operator = (source);
}

template <typename T>
inline Ref<T>::~Ref()
{
  operator = (NULL);
}

template <typename T>
inline Ref<T> Ref<T>::detachObject()
{
  Ref<T> result = object;
  operator = (NULL);
  return result;
}

template <typename T>
inline Ref<T>::operator T* () const
{
  return object;
}

template <typename T>
inline T* Ref<T>::operator -> () const
{
  return object;
}

template <typename T>
inline Ref<T>& Ref<T>::operator = (T* newObject)
{
  if (newObject)
    increment(newObject);

  if (object)
  {
    decrement(object);
    if (unreferenced(object))
      delete static_cast<RefObject*>(object);
  }

  object = newObject;
  return *this;
}

template <typename T>
inline Ref<T>& Ref<T>::operator = (const Ref<T>& source)
{
  return operator = (source.object);
}

template <typename T>
inline T* Ref<T>::getObject() const
{
  return object;
}

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Singleton<T>::~Singleton()
{
}

template <typename T>
inline void Singleton<T>::destroySingleton()
{
  object = NULL;
}

template <typename T>
inline T* Singleton<T>::getSingleton()
{
  return object;
}

template <typename T>
inline void Singleton<T>::set(T* newObject)
{
  object = newObject;
}

template <typename T>
Ptr<T> Singleton<T>::object;

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_CORE_H*/
///////////////////////////////////////////////////////////////////////
