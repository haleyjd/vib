/** @file VIBProperties.h
 *
 * VIB Class Library - C++11 Properties Implementation
 * 
 */

#ifndef VIBPROPERTIES_H__
#define VIBPROPERTIES_H__

#include <functional>

namespace VIB
{
   /**
    * Property implements Borland Delphi-style properties using standard C++11
    * mechanisms. Implementation of proper message forwarding to the parent
    * object of the property is accomplished through use of lambda functions as
    * get/set callbacks, which capture the "this" pointer by reference during
    * their construction. Both callbacks are optional; leaving one unimplemented
    * will cause a std::exception to be thrown if a call is attempted. This can
    * be used to create read-only or even write-only properties.
    * @tparam T Basic type or structure/class type to wrap.
    */
   template<typename T> class Property
   {
   protected:
      typedef std::function<T (void)>         Getter; //!< The type of a Property Getter method
      typedef std::function<void (const T &)> Setter; //!< The type of a Property Setter method

      Getter get; //!< The getter method; typically, a this-capturing lambda function.
      Setter set; //!< The setter method; typically, a this-capturing lambda function.

   public:
      /** 
       * Default constructor, you must initialize get and set later. Typical use case.
       */
      Property() : get(), set() {}

      /** 
       * Full constructor. Provide get and set callbacks at instantiation. 
       * @param[in] pGet Pointer to a T (void) getter function; typically, a 
       *   this-capturing lambda.
       * @param[in] pSet Pointer to a void (const T &) setter function; typically,
       *   a this-capturing lambda.
       */
      Property(const Getter &pGet, const Setter &pSet) : get(pGet), set(pSet)
      {
      }

      /** 
       * Late-initialize the callbacks, or change their values at runtime.
       * @param[in] pGet Pointer to a T (void) getter function; typically, a
       *  this-capturing lambda.
       * @param[in] pSet Pointer to a void (const T &) setter function; typically,
       *  a this-capturing lambda.
       */
      void initCallbacks(const Getter &pGet, const Setter &pSet)
      {
         get = pGet;
         set = pSet;
      }

      /** 
       * Cast operator, invokes get() and returns its value. Allows treating an
       * instance of Property like an rvalue of its underlying type in most contexts.
       * @returns Value returned by the getter callback function.
       */
      operator T() { return get(); }

      /** 
       * Assignment operator, invokes set with the provided value, returns this object. 
       * @param[in] nv New value of the underlying type to assign to the property via
       *  the setter callback function.
       * @returns Reference to the Property instance. Allows use in consecutive
       *  assignment expressions with the expected normal language semantics.
       */
      Property<T> &operator = (const T &nv) { set(nv); return *this; }

      /** 
       * Invokes get, assuming it returns a pointer that can be used with the
       * deref-access arrow operator. If not, you will cause a compile-time error
       * by trying to use it.
       * @returns Value returned by the getter callback function. Ensure that the
       *  underlying type is a pointer when using this.
       */
      T operator -> () { return get(); }
   };

   /**
    * ArrayProperty extends Property with an array access operator, for
    * properties which behave like C arrays or collections. It is possible
    * to use this recursively to create multidimensional property arrays.
    * @tparam T Basic type or structure/class type to wrap.
    * @param  A Basic type or structure/class type of elements in the array.
    */
   template<typename T, typename A> class ArrayProperty : public Property<T>
   {
   public:
      /** 
       * Default constructor, you must initialize get and set later. 
       */
      ArrayProperty<T, A>() : Property<T>() {}

      /** 
       * Provide get and set callbacks at instantiation. 
       * @param[in] pGet Pointer to a getter function; typically, a 
       *   this-capturing lambda.
       * @param[in] pSet Pointer to a setter function; typically, a
       *   this-capturing lambda.
       */
      ArrayProperty<T, A>(const Getter &pGet, const Setter &pSet) 
         : Property<T>(pGet, pSet)
      {
      }

      /**
       * Array access operator. Calls the getter callback of this 
       * object, and then invokes the underlying object's operator [] 
       * using the input index. It must return template type A.
       * @param[in] i Index of the item in the array to obtain.
       */
      A operator [] (int i) { return this->get()->operator[](i); }
   };
}

#endif

// EOF

