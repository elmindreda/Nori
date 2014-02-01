///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_SIGNAL_HPP
#define WENDY_SIGNAL_HPP
///////////////////////////////////////////////////////////////////////

#include <algorithm>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class SignalSlotBase;

template <typename R, typename... A>
class Signal;

////////////////////////////////////////////////////////////////////////

/*! @brief Mixin for automatic slot destruction.
 *
 *  @remarks Inherit from this to automatically be disconnected from all
 *  attached signals on destruction.
 */
class Trackable
{
  friend class SignalSlotBase;
public:
  Trackable();
  Trackable(const Trackable& source);
  virtual ~Trackable();
  Trackable& operator = (const Trackable& source);
private:
  std::vector<SignalSlotBase*> slots;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Abstract base class for signal slots.
 *
 *  @remarks If you wish to keep track of a connection for explicit
 *  disconnection, you usually want to do it by keeping a pointer to a
 *  SignalSlot object.
 */
class SignalSlotBase
{
public:
  /*! Constructor.
   *  @param[in] object The trackable object this slot connects to, or @c
   *  nullptr if it is connected to a function or a non-trackable object.
   */
  SignalSlotBase(Trackable* object = nullptr);
  /*! Destructor.
   *  If this slot is attached to a trackable object, it will sever the
   *  connection to it.
   */
  virtual ~SignalSlotBase() = 0;
private:
  Trackable* object;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Base class for typed signal slots.
 */
template <typename R, typename... A>
class SignalSlot : public SignalSlotBase
{
  friend class Signal<R,A...>;
public:
  /*! Destructor.
   */
  ~SignalSlot()
  {
    if (signal)
    {
      auto& slots = signal->slots;
      slots.erase(std::find(slots.begin(), slots.end(), this));
    }
  }
  /*! Calls the target for this slot.
   */
  virtual R operator () (A... args) = 0;
protected:
  /*! Constructor.
   */
  SignalSlot(Trackable* object = nullptr):
    SignalSlotBase(object),
    signal(nullptr)
  {
  }
private:
  Signal<R,A...>* signal;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Typed signal slot for functions and static methods.
*/
template <typename R, typename... A>
class FunctionSlot : public SignalSlot<R,A...>
{
public:
  /*! Function pointer type for this slot.
   */
  typedef R (*Function)(A...);
  /*! Constructor.
   */
  FunctionSlot(Function function):
    function(function)
  {
  }
  /*! Calls the target for this slot.
   */
  R operator () (A... args)
  {
    return (*function)(args...);
  }
private:
  Function function;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Typed signal slot for member functions.
*/
template <typename T, typename R, typename... A>
class MethodSlot : public SignalSlot<R,A...>
{
public:
  /*! Method pointer type for this slot.
   */
  typedef R (T::*Function)(A...);
  /*! Constructor.
   */
  MethodSlot(T& object, Function function):
    SignalSlot<R,A...>(&dynamic_cast<Trackable&>(object)),
    object(object),
    function(function)
  {
    static_assert(std::is_base_of<Trackable, T>(),
                  "Method slots require the Trackable mixin class");
  }
  /*! Calls the target for this slot.
   */
  R operator () (A... args)
  {
    return (object.*function)(args...);
  }
private:
  T& object;
  Function function;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Typed signal object.
*/
template <typename R, typename... A>
class Signal
{
friend class SignalSlot<R,A...>;
public:
  /*! Slot base class for this signal.
   */
  typedef SignalSlot<R,A...> Slot;
  /*! Constructor.
   */
  Signal() { }
  /*! Copy contructor.
   *  @note This does not copy any existing connections to the source object.
   */
  Signal(const Signal<R,A...>& source) { }
  /*! Destructor.
   */
  ~Signal()
  {
    while (!slots.empty())
      delete slots.back();
  }
  /*! Adds the specified generic slot to this signal.
   */
  Slot* connect(Slot* slot)
  {
    slots.push_back(slot);
    slot->signal = this;
    return slot;
  }
  /*! Connects the specified instance and member function to this signal.
   */
  template <typename T>
  Slot* connect(T& object, typename MethodSlot<T,R,A...>::Function function)
  {
    return connect(new MethodSlot<T,R,A...>(object, function));
  }
  /*! Connects the specified function to this signal.
   */
  Slot* connect(typename FunctionSlot<R,A...>::Function function)
  {
    return connect(new FunctionSlot<R,A...>(function));
  }
  /*! Calls the targets for all slots in this signal.
   */
  void operator () (A... args) const
  {
    for (auto s : slots)
      (*s)(args...);
  }
  /*! Assignment operator.
   *  @note This does not copy any existing connections to the source object.
   */
  Signal<R,A...>& operator = (const Signal<R,A...>& source)
  {
    return *this;
  }
private:
  std::vector<Slot*> slots;
};

///////////////////////////////////////////////////////////////////////

/*! Proxy object for signal objects.
 *  Allows public connections to private signal objects.
 *  Concept taken from gtkmm.
 */
template <typename R, typename... A>
class SignalProxy
{
public:
  /*! Slot type.
   */
  typedef SignalSlot<R,A...> Slot;
  /*! Constructor.
   */
  SignalProxy(Signal<R,A...>& signal):
    signal(signal)
  {
  }
  /*! Connects the specified slot to the proxied signal.
   */
  Slot* connect(Slot* slot)
  {
    return signal.connect(slot);
  }
  /*! Connects the specified function to the proxied signal.
   */
  Slot* connect(typename FunctionSlot<R,A...>::Function function)
  {
    return signal.connect(function);
  }
  /*! Connects the specified instance and member function to the proxied signal.
   */
  template <typename T>
  Slot* connect(T& object, typename MethodSlot<T,R,A...>::Function function)
  {
    return signal.connect(object, function);
  }
private:
  Signal<R,A...>& signal;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SIGNAL_HPP*/
///////////////////////////////////////////////////////////////////////
