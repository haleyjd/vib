/** @file classVIBSQL.h
 *
 *  VIB Class Library - VIBSQL Wrapper
 *  @author James Haley
 */

#ifndef CLASSVIBSQL_H__
#define CLASSVIBSQL_H__

#include "../vibsql.h"
#include "VIBProperties.h"

struct VIBDatabase;
struct VIBTransaction;

namespace VIB
{
   class Database;
   class Transaction;

   /**
    * VIB::SQL wraps the VisualIB VIBSQL C structure with a C++11 object
    * with semantics compatible with those of Borland TIBSQL.
    */
   class SQL
   {
   protected:
      VIBSQL *vsql; //!< Pointer to the wrapped VIBSQL instance.

   public:
      /**
       * Construct a VIB::SQL object.
       * @param[in] pvsql Pointer to a VIBSQL instance of which the VIB::SQL instance 
       *   should take ownership.
       */
      SQL(VIBSQL *pvsql = NULL);
      /**
       * Destroy the VIB::SQL instance, and the VIBSQL instance which it wraps.
       */
      virtual ~SQL();

      /**
       * Call is an internal method used to make calls to the InterBase API, and gives the
       * option of throwing an exception or returning an error.
       * @param[in] ErrCode InterBase API error code?
       * @param[in] RaiseError If true, throws an exception.
       * @returns   An error message based on the error code (???)
       * @note Reimplements TIBSQL\::Call
       */
      int  Call(int ErrCode, bool RaiseError);
      /**
       * Throws an exception if the query is not closed.
       * @note Reimplements TIBSQL\::CheckClosed
       */
      void CheckClosed();
      /**
       * Throws an exception if the query is not open.
       * @note Reimplements TIBSQL\::CheckOpen
       */
      void CheckOpen();
      /**
       * Throws an exception if the query does not have a valid statement.
       * @note Reimplements TIBSQL\::CheckValidStatement
       */
      void CheckValidStatement();
      /**
       * Closes the query.
       * @note Reimplements TIBSQL\::Close
       */
      void Close();
      /**
       * Executes an SQL query.
       * @note Reimplements TIBSQL\::ExecQuery
       */
      void ExecQuery();
      /**
       * Frees InterBase resources associated with the query.
       * @note Reimplements TIBSQL\::FreeHandle
       */
      void FreeHandle();
      /**
       * Prepares a query for execution.
       * @note Reimplements TIBSQL\::Prepare
       */
      void Prepare();
      /**
       * Undocumented method.
       * @param[in] FieldName Name of a field referenced by the query?
       * @returns   Index of the named field?
       */
      int  FieldIndex(const std::string &FieldName);
      
      /** Undocumented method. */
      std::string GetUniqueRelationName();

      // Property classes

      /**
       * Wrapper class for the SQL property.
       * Equivalent to a Borland TStrings object.
       */
      class SQLClass
      {
      protected:
         SQL *parent; //!< Pointer to the parent VIB::SQL instance
      public:
         /**
          * Construct a SQL property wrapper object.
          * @param[in] pParent Pointer to the parent VIB::SQL instance
          */
         SQLClass(SQL *pParent);

         /**
          * Lists the strings in the object as a single string with the
          * individual strings delimited by carriage returns and line
          * feeds. Use to get or set all the strings in the object with
          * a single call.
          * @note Reimplements TStrings\::Text
          */
         Property<std::string> Text;
      };
      friend class SQLClass;

      // Class for individual properties
      class ParamsSetClass;

      /**
       * Wrapper for a single Param object.
       */
      class ParamClass
      {
      protected:
         friend ParamsSetClass;
         std::string name; //!< Name of the parameter
         SQL *parent;      //!< Pointer to the parent VIB::SQL instance
      public:
         /**
          * Construct a ParamClass wrapper instance.
          * @param[in] pParent Pointer to parent VIB::SQL instance.
          */
         ParamClass(SQL *pParent);

         Property<std::string> AsString; //!< Obtain the value of the property as a string.
         Property<bool>        IsNull;   //!< Test whether or not the parameter is null.
         Property<int>         SQLType;  //!< Obtain the InterBase SQLType value for the parameter.
      };
      friend class ParamClass;

      /**
       * Wrapper class for Params property
       * Reimplements Borland TIBXSQLDA
       */
      class ParamsSetClass
      {
      protected:
         SQL *parent;   //!< Pointer to parent VIB::SQL instance
         ParamClass pc; //!< Instance of ParamClass
      public:
         /**
          * Construct a Params property wrapper object.
          * @param[in] pParent Pointer to parent VIB::SQL instance.
          */
         ParamsSetClass(SQL *pParent) : parent(pParent), pc(pParent) {}

         /**
          * Return a Param instance by parameter name.
          * @param[in] name Name of the parameter to retrieve.
          * @returns Pointer to a ParamClass instance.
          */
         ParamClass *ByName(const std::string &name);
      };

   protected:
      // Property instances
      SQLClass sc;        //!< Instance of SQLClass property wrapper object
      ParamsSetClass psc; //!< Instance of ParamsSet property wrapper object

   public:
      // Properties
      /**
       * Indicates whether or not a query is at the beginning of the dataset.
       * @note Read-only property.
       * @note Reimplements TIBSQL\::Bof
       */
      Property<bool>             Bof;
      /**
       * Sets or returns the database associated with the query.
       * @note Reimplements TIBSQL\::Database
       * @note API Difference - The value of the property is a VIBDatabase
       *   structure pointer. The corresponding TIBDatabase instance is
       *   hidden by the VisualIB C API abstraction.
       */
      Property<VIBDatabase *>    Database;
      /**
       * Specifies the database handle for the query.
       * @note Reimplements TIBSQL\::DBHandle
       * @note API Difference - the return value is equivalent to and could
       *   be cast to PISC_DB_HANDLE if the InterBase C API is visible in scope.
       */
      Property<void ***>         DBHandle;
      /**
       * Indicates wehther or not a query is at the end of the dataset. If this
       * property is true immediately after the query is opened, then it means
       * that the dataset is empty.
       * @note Reimplements TIBSQL\::Eof
       */
      Property<bool>             Eof;
      /**
       * Generates a list of parameter names for the query.
       * @note Reimplements TIBSQL\::GenerateParamNames
       */
      Property<bool>             GenerateParamNames;
      /**
       * Goes to the first record in the result set upon opening the query.
       * True by default except when used internally by TIBDataSet.
       * @note Reimplements TIBSQL\::GoToFirstRecordOnExecute
       */
      Property<bool>             GoToFirstRecordOnExecute;
      /**
       * Specifies the handle for the query.
       * @note Reimplements TIBSQL\::Handle
       * @note API Difference - void ** is equivalent to Ibexternals::PVoid
       */
      Property<void **>          Handle;
      /**
       * Determines if the dataset is open.
       * @note Read-only property.
       * @note Reimplements TIBSQL\::Open
       */
      Property<bool>             Open;
      /**
       * Specifies whether the parameter list for an SQL query is regenerated
       * if the SQL property changes at runtime. This property is useful for DDL
       * statements that contain parameters as part of the DDL statement and that
       * are not parameters for the TIBSQL query.
       * @note Reimplements TIBSQL\::ParamCheck
       */
      Property<bool>             ParamCheck;
      /**
       * Returns the XSQLDA parameters.
       * @note Reimplements TIBSQL\::Params
       */
      Property<ParamsSetClass *> Params;
      /**
       * Returns the plan for the query, once it has been prepared.
       * @note Reimplements TIBSQL\::Plan
       */
      Property<std::string>      Plan;
      /**
       * Indicates whether or not the query has been prepared.
       * @note Read-only property.
       * @note Reimplements TIBSQL\::Prepared
       */
      Property<bool>             Prepared;
      /**
       * Returns the current count of records from the query. The value of this
       * property changes as rows from the query are visited (it starts out 0 and
       * increases for each row access).
       * @note Reimplements TIBSQL\::RecordCount
       */
      Property<int>              RecordCount;
      /**
       * Returns the number of rows affected by the query. This property is
       * useful for <tt>INSERT</tt>, <tt>DELETE</tt>, and <tt>UPDATE</tt> statements.
       * @note Reimplements TIBSQL\::RowsAffected
       */
      Property<int>              RowsAffected;
      /**
       * Sets the SQL query to be executed.
       * @note Reimplements TIBSQL::SQL
       * @note API Difference - The name of this property is prefaced with an
       *   underscore not present in the TIBSQL class, due to the fact that a
       *   C++ class may not have a field with the same name as the class.
       */
      Property<SQLClass *>       _SQL;                      // NB: Not same name as in C++Builder!
      /**
       * Returns the type of query to be executed.
       * @note Reimplements TIBSQL\::SQLType
       * @see VIBSQLTypes
       */
      Property<VIBSQLTypes>      SQLType;
      /**
       * Sets or returns the transaction to be used by the query.
       * @note Reimplements TIBSQL\::Transaction
       * @note API Difference - The value of the property is a pointer to an
       *   instance of VIBTransaction. The corresponding TIBTransaction object
       *   is hidden by the VisualIB C API abstraction.
       */
      Property<VIBTransaction *> Transaction;
      /**
       * Specifies the transaction handle for the query.
       * @note Reimplements TIBSQL\::TRHandle
       * @note API Difference - The return value is equivalent to and could be
       *  cast to PISC_TR_HANDLE if the InterBase C API is visible in scope.
       */
      Property<void ***>         TRHandle;
      /**
       * Indicates the unique relation name for a query that involves only one
       * base table.
       * @note Read-only property.
       * @note Reimplements TIBSQL\::UniqueRelationName
       */
      Property<std::string>      UniqueRelationName;
   };
}

#endif

// EOF

