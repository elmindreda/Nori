///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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

#pragma once

#include <string>
#include <vector>

#include <cstdarg>
#include <cstddef>
#include <cassert>

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_precision.hpp>

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

#endif /*_MSC_VER*/

namespace pugi { class xml_node; }

namespace wendy
{

using namespace glm;

/*! String type.
 */
typedef std::string String;

/*! String hash type.
 */
typedef uint32 StringHash;

/*! @brief Converts the specified value to a string.
 */
String stringCast(const vec2& v);
String stringCast(const vec3& v);
String stringCast(const vec4& v);
String stringCast(const mat2& v);
String stringCast(const mat3& v);
String stringCast(const mat4& v);
String stringCast(const quat& v);

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

String vlformat(const char* format, va_list vl);

class RefObject;

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
  INFO_LOG_ENTRY
};

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

/*! @brief Base class for references.
 *  @remarks Concept taken from MoSync.
 */
class RefBase
{
protected:
  static bool unreferenced(RefObject* object);
  static void increment(RefObject* object);
  static void decrement(RefObject* object);
};

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
  uint count;
};

/*! @brief Smart reference.
 *
 *  Pointer to objects that inherit from RefObject.
 */
template <typename T>
class Ref : public RefBase
{
public:
  /*! Swaps the specified references.
   */
  friend void swap(Ref<T>& first, Ref<T> second)
  {
    using std::swap;

    swap(first.m_object, second.m_object);
  }
  /*! Default constructor.
   */
  Ref(T* object = nullptr):
    m_object(nullptr)
  {
    operator = (object);
  }
  /*! Copy constructor.
   *  @param source The pointer object to inherit from.
   */
  Ref(const Ref<T>& source):
    m_object(nullptr)
  {
    operator = (source);
  }
  /*! Destructor
   */
  ~Ref()
  {
    operator = (nullptr);
  }
  /*! Cast operator.
   */
  operator T* () const
  {
    return m_object;
  }
  /*! Member operator.
   */
  T* operator -> () const
  {
    return m_object;
  }
  /*! Object assignment operator.
   */
  Ref<T>& operator = (T* newObject)
  {
    if (newObject)
      increment(newObject);

    if (m_object)
    {
      decrement(m_object);
      if (unreferenced(m_object))
        delete static_cast<RefObject*>(m_object);
    }

    m_object = newObject;
    return *this;
  }
  /*! Assignment operator.
   */
  Ref<T>& operator = (const Ref<T>& source)
  {
    return operator = (source.m_object);
  }
  /*! @return The currently owned object.
   */
  T* object() const
  {
    return m_object;
  }
private:
  T* m_object;
};

/*! @brief Interface for receiving %log messages.
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

} /*namespace wendy*/

