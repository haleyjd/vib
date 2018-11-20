/** @file classVIBError.h
 *
 *  VIB::Error Class
 *  @author James Haley
 */

#ifndef CLASSVIBERROR_H__
#define CLASSVIBERROR_H__

#include <string>

namespace VIB
{
   /**
    * Error is the root class for all VIB exceptions.
    * It is thrown directly when library-internal errors occur.
    * @note Replaces most Borland exception types.
    */
   class Error
   {
   protected:
      std::string errormsg; //!< Message associated with the error.
      
   public:
      /**
       * Default constructor, call when there is no message to give.
       */
      Error() : errormsg("Unknown error")
      {
      }

      /**
       * Construct an error with a C string error message.
       */
      Error(const char *msg) : errormsg(msg)
      {
      }

      /**
       * Construct an error with a C++ string error message.
       */
      Error(const std::string &msg) : errormsg(msg)
      {
      }

      /**
       * Obtain the message associated with the exception.
       */
      const std::string &getErrorMsg() const { return errormsg; }
   };

   /**
    * IBError is thrown when InterBase errors occur.
    * @note Replaces all InterBase API exception types. Borland exceptions are
    *   never propagated by the VisualIB C API layer; instead, the VIB class
    *   layer checks for failures in the C API and will throw this new exception
    *   object in most cases where the Borland InterBase classes would have thrown.
    */
   class IBError : public Error
   {
   protected:
      int IBErrorCode; //!< InterBase API error code
      int SQLCode;     //!< SQL error code, if the exception is a standard SQL error

   public:
      /**
       * Default constructor, invoked if an unexpected error occurs.
       */
      IBError() : Error("Unknown InterBase error"), IBErrorCode(-1), SQLCode(-1)
      {
      }

      /**
       * C string constructor
       */
      IBError(const char *msg, int pIBErrorCode, int pSQLCode)
         : Error(msg), IBErrorCode(pIBErrorCode), SQLCode(pSQLCode)
      {
      }

      /**
       * C++ string object constructor
       */
      IBError(const std::string &msg, int pIBErrorCode, int pSQLCode)
         : Error(msg), IBErrorCode(pIBErrorCode), SQLCode(pSQLCode)
      {
      }

      /** Obtain the InterBase API error code for the exception */
      int getIBErrorCode() const { return IBErrorCode; }
      /** Obtain the standard SQL error code for the exception */
      int getSQLCode()     const { return SQLCode;     }
   };
}

#endif

// EOF

