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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <string>
#include <vector>
#include <cstdarg>

#if WENDY_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

///////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

// Don't consider the libc to be obsolete
#pragma warning( disable: 4996 )

#define snprintf _snprintf
#define strcasecmp stricmp

// This is only needed for versions below Visual C++ 2008
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif

float log2f(float x);

#endif /*_MSC_VER*/

///////////////////////////////////////////////////////////////////////

#if !WENDY_HAVE_VASPRINTF
int vasprintf(char** result, const char* format, va_list vl);
#endif /*WENDY_HAVE_VASPRINTF*/

#if !WENDY_HAVE_STRTOF
float strtof(const char* nptr, char** endptr);
#endif /*WENDY_HAVE_STRTOF*/

#if !WENDY_HAVE_STRLCAT
size_t strlcat(char* target, const char* source, size_t size);
#endif /*WENDY_HAVE_STRLCAT*/

#if !WENDY_HAVE_STRLCPY
size_t strlcpy(char* target, const char* source, size_t size);
#endif /*WENDY_HAVE_STRLCPY*/

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! Byte value.
 */
typedef uint8_t Byte;

/*! Time value, in seconds.
 */
typedef double Time;

/*! String type.
 */
typedef std::string String;

/*! String hash type.
 */
typedef uint32_t StringHash;

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
 *  @param format [in] The formatting string for the log entry.
 */
void logError(const char* format, ...);

/*! Writes a warning message log entry to the log consumers,
 *  or to stderr if there are no log consumers.
 *  @param format [in] The formatting string for the log entry.
 */
void logWarning(const char* format, ...);

/*! Writes an informational  message log entry to the log consumers,
 *  or to stderr if there are no log consumers.
 *  @param format [in] The formatting string for the log entry.
 */
void log(const char* format, ...);

///////////////////////////////////////////////////////////////////////

/*! Base class for exceptions.
 */
class Exception : public std::exception
{
public:
  /*! Constructor.
   *  @param initMessage The message for this exception.
   */
  Exception(const char* initMessage);
  /*! @return The massage for this exception.
   */
  const char* what(void) const throw();
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
  inline explicit Ptr(T* object = NULL);
  /*! Destructor
   */
  inline virtual ~Ptr(void);
  /*! Detaches (orphans) the currently owned object.
   * @return The currently owned object.
   * @remarks Use with care.
   */
  inline T* detachObject(void);
  /*! Cast operator.
   */
  inline operator T* (void);
  /*! Cast operator.
   */
  inline operator const T* (void) const;
  /*! Member operator.
   */
  inline T* operator -> (void);
  /*! Member operator.
   */
  inline const T* operator -> (void) const;
  /*! Object assignment operator.
   */
  inline Ptr<T>& operator = (T* newObject);
  /*! @return The currently owned object.
   */
  inline T* getObject(void);
  /*! @return The currently owned object.
   */
  inline const T* getObject(void) const;
private:
  inline Ptr(const Ptr<T>& source);
  inline Ptr<T>& operator = (const Ptr<T>& source);
  T* object;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for references.
 */
class RefBase
{
protected:
  bool unreferenced(RefObject* object);
  void increment(RefObject* object);
  void decrement(RefObject* object);
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
  inline Ref(T* object = NULL);
  /*! Copy constructor.
   *  @param source The pointer object to inherit from.
   */
  inline Ref(const Ref<T>& source);
  /*! Destructor
   */
  inline ~Ref(void);
  /*! Detaches the currently referenced object.
   * @return The currently reference object.
   * @remarks Use with care.
   */
  inline Ref<T> detachObject(void);
  /*! Cast operator.
   */
  inline operator T* (void) const;
  /*! Member operator.
   */
  inline T* operator -> (void) const;
  /*! Object assignment operator.
   */
  inline Ref<T>& operator = (T* newObject);
  /*! Assignment operator.
   */
  inline Ref<T>& operator = (const Ref<T>& source);
  /*! @return The currently owned object.
   */
  inline T* getObject(void) const;
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
  RefObject(void);
  /*! Copy constructor.
   */
  RefObject(const RefObject& source);
  /*! Destructor.
   */
  virtual ~RefObject(void);
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
  virtual ~Singleton(void);
  /*! Deletes the singleton instance.
   */
  static inline void destroySingleton(void);
  /*! @return The singleton instance if available, otherwise @c NULL.
   */
  static inline T* getSingleton(void);
protected:
  /*! Sets the singleton instance.
   *  @param newObject [in] The instance to set.
   */
  static inline void set(T* newObject);
private:
  static Ptr<T> object;
};

///////////////////////////////////////////////////////////////////////

class LogConsumer
{
public:
  LogConsumer(void);
  virtual ~LogConsumer(void);
  virtual void onLogEntry(LogEntryType type, const char* message) = 0;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Ptr<T>::Ptr(T* initObject):
  object(initObject)
{
}

template <typename T>
inline Ptr<T>::~Ptr(void)
{
  if (object)
    delete object;
}

template <typename T>
inline T* Ptr<T>::detachObject(void)
{
  T* temp = object;
  object = NULL;
  return temp;
}

template <typename T>
inline Ptr<T>::operator T* (void)
{
  return object;
}

template <typename T>
inline Ptr<T>::operator const T* (void) const
{
  return object;
}

template <typename T>
inline T* Ptr<T>::operator -> (void)
{
  return object;
}

template <typename T>
inline const T* Ptr<T>::operator -> (void) const
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
inline T* Ptr<T>::getObject(void)
{
  return object;
}

template <typename T>
inline const T* Ptr<T>::getObject(void) const
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
inline Ref<T>::~Ref(void)
{
  operator = (NULL);
}

template <typename T>
inline Ref<T> Ref<T>::detachObject(void)
{
  Ref<T> result = object;
  operator = (NULL);
  return result;
}

template <typename T>
inline Ref<T>::operator T* (void) const
{
  return object;
}

template <typename T>
inline T* Ref<T>::operator -> (void) const
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
inline T* Ref<T>::getObject(void) const
{
  return object;
}

///////////////////////////////////////////////////////////////////////

template <typename T>
inline Singleton<T>::~Singleton(void)
{
}

template <typename T>
inline void Singleton<T>::destroySingleton(void)
{
  object = NULL;
}

template <typename T>
inline T* Singleton<T>::getSingleton(void)
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
