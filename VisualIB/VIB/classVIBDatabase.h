/** @file classVIBDatabase.h
 *
 *  VIB::Database Class
 *  @author James Haley
 */

#ifndef CLASSVIBDATABASE_H__
#define CLASSVIBDATABASE_H__

#include "../vibtypes.h"
#include "VIBProperties.h"

struct VIBDatabase;

namespace VIB
{
   /**
    * VIB::Database wraps the VisualIB VIBDatabase C structure to provide a C++11 
    * object with semantics compatible with those of Borland TIBDatabase.
    */
   class Database
   {
   protected:
      VIBDatabase *vdb; //!< Wrapped instance of VIBDatabase structure

   public:
      /**
       * Construct a VIB::Database instance.
       * @param[in] pvdb Optional pointer to an existing VIBDatabase structure 
       *   of which this class instance should take ownership.
       */
      Database(VIBDatabase *pvdb = NULL);
      /**
       * Destroy the VIB::Database. Any connections still open on the wrapped 
       * VIBDatabase instance will be closed, and then it will be destroyed as
       * well.
       */
      virtual ~Database();

      /**
       * Get a pointer to the wrapped VIBDatabase structure instance, for use 
       * with the VisualIB C API.
       * @returns Pointer to the wrapped and owned VIBDatabase instance.
       */
      VIBDatabase *getVIBDatabase() const { return vdb; }

      /**
       * Internal method used to make calls to the InterBase API. Gives the 
       * option of raising an exception or returning an error based on the
       * value of RaiseError.
       * @note Reimplements TIBDatabase\::Call
       * @param[in] ErrCode InterBase API error code.
       * @param[in] RaiseError If true, an exception should be raised.
       * @returns   Unknown; InterBase API code?
       */
      int  Call(int ErrCode, bool RaiseError);
      /**
       * Call CheckActive to raise an error if the connection to the database
       * server is inactive.
       * @note Reimplements TIBDatabase\::CheckActive
       */
      void CheckActive();
      /**
       * Call CheckDatabaseName to check if the DatabaseName property is empty,
       * and raise an error if it is.
       * @note Reimplements TIBDatabase\::CheckDatabaseName
       */
      void CheckDatabaseName();
      /**
       * Call CheckInactive to raise an error if the connection to the database
       * server is active.
       * @note Reimplements TIBDatabase\::CheckInactive
       */
      void CheckInactive();
      /**
       * Call Close to disconnect from the source of database information.
       * Before the connection component is deactivated, all associated datasets
       * are closed. Calling Close is the same as setting the Connected property
       * to @b false. In most cases, closing a connection frees system resources
       * allocated to the connection.
       * @note If a previously active connection is closed and then reopened, any
       * associated datasets must be individually reopened.
       * @note Reimplements TCustomConnection\::Close
       */
      void Close();
      /**
       * Call CloseDataSets to close all active datasets without disconnecting 
       * from the database server.
       * @note Reimplements TIBDatabase\::CloseDataSets
       */
      void CloseDataSets();
      /**
       * Call CreateDatabase to create a database using Params as the rest of
       * the <tt>CREATE DATABASE</tt> command.
       * @note Reimplements TIBDatabase\::CreateDatabase
       */
      void CreateDatabase();
      /**
       * Call DropDatabase to drop a database, which removes the database file
       * from the server.
       * @warning Does exactly what it says. Data cannot be recovered without a backup!
       * @note Reimplements TIBDatabase\::DropDatabase
       */
      void DropDatabase();
      /**
       * Use ForceClose to force the database connection to close; even if the 
       * underlying API call fails, the database handle is reset to NULL.
       * @note Reimplements TIBDatabase\::ForceClose
       */
      void ForceClose();
      /**
       * Undocumented TIBDatabase method.
       * @note Reimplements TIBDatabase\::FlushSchema
       */
      void FlushSchema();
      /**
       * Undocumented TIBDatabase method.
       * @param[in] Relation Name of a database relation (table, view, etc).
       * @param[in] Field    Name of a database field.
       * @returns True or false depending if the field of this relation has computed BLR?
       * @note Reimplements TIBDatabase\::Has_COMPUTED_BLR
       */
      bool Has_COMPUTED_BLR(const std::string &Relation, const std::string &Field);
      /**
       * Undocumented TIBDatabase method.
       * @param[in] Relation Name of a database relation (table, view, etc).
       * @param[in] Field    Name of a database field.
       * @returns True or false depending if the field of this relation has a default value?
       * @note Reimplements TIBDatabase\::Has_DEFAULT_VALUE
       */
      bool Has_DEFAULT_VALUE(const std::string &Relation, const std::string &Field);
      /**
       * Use IndexOfDBConst to locate a parameter in the database parameters
       * list.
       * @returns Parameter index, or -1 if the parameter is not found.
       * @note Reimplements TIBDatabase\::IndexOfDBConst
       */
      int  IndexOfDBConst(const char *st);
      /**
       * Call Open to establish a connection to the source of database
       * information. Open sets the Connected property to \b true.
       * @note Reimplements TCustomConnection\::Open
       */
      void Open();
      /**
       * Use TestConnected to determine whether a database is connected to the
       * server.
       * @returns True if the connection is good, and false otherwise.
       * @note Reimplements TIBDatabase\::TestConnected
       */
      bool TestConnected();
      /**
       * Call RemoveTransaction to disassociate a specified transaction from 
       * the database.
       * @param[in] Idx Index of the transaction to remove.
       */
      void RemoveTransaction(int Idx);
      /**
       * Call RemoveTransactions to disassociate all transactions from the database.
       * @note Reimplements TIBDatabase\::RemoveTransactions
       */
      void RemoveTransactions();

      /* TODO:
      int AddTransaction(Transaction *vtr);
      int FindTransaction(Transaction *vtr);
      Transaction *FindDefaultTransaction();     
      */

      // Property classes

      /**
       * Class implementing the Params property, which reimplements the
       * <tt>TStrings *Params</tt> property of TIBDatabase.
       */
      class ParamClass
      {
      protected:
         Database *parent; //!< Pointer to containing VIB\::Database instance.
      public:
         /** 
          * Construct the Params wrapper object with a pointer to its parent
          * VIB::Database instance.
          */
         ParamClass(Database *pParent) : parent(pParent) {}

         /**
          * Call Params->Add to add a parameter value to the database.
          * @param[in] str Parameter string to add to the database, such as
          *   <tt>user_name=sysdba</tt>.
          * @note Reimplements TStrings::Add
          */
         void Add(const std::string &str);
      };
      friend class ParamClass;

      /**
       * Class implementing the DBParamByDPB property, which reimplements the
       * <tt>AnsiString DBParamByDPB[int Idx]</tt> property of TIBDatabase.
       */
      class DBParamClass
      {
      protected:
         Database *parent; //!< Pointer to containing VIB\::Database instance.
      public:
         DBParamClass(Database *pParent) : parent(pParent) {}

         /**
          * Return the DBParam at the indicated DPB index. The index is an
          * InterBase API-specific value such as <tt>isc_dpb_user_name</tt>.
          */
         std::string operator [] (int i);
      };

   protected:
      // Property implementor objects
      ParamClass   paramsObj;  //!< Protected instance of Params wrapper object.
      DBParamClass dbParamObj; //!< Protected instance of DBParamByDPB wrapper object.

   public:
      // Properties
      Property<bool>          AllowStreamedConnected; //!< Undocumented property.
      /**
       * Set to true to establish a database connection without opening a dataset.
       * Set to false to close a database connection. An application can check
       * Connected to determine the current status of a database connection.
       * @note Reimplements TIBDatabase\::Connected
       */
      Property<bool>          Connected;
      /**
       * Use to specify the name of the database to use with a database component.
       * For local InterBase databases, this can be a filename. Otherwise, use the
       * standard InterBase server_name:filename syntax.
       * @note Reimplements TIBDatabase\::DatabaseName
       */
      Property<std::string>   DatabaseName;
      /**
       * Use to determine the number of datasets listed by the DataSets property.
       * DataSets may include only the active datasets that use the connection
       * component, or it may list all datasets. Use as an upper bound when
       * iterating through the DataSets property.
       * @note Read-only property.
       * @note Reimplements TCustomConnection\::DataSetCount
       */
      Property<int>           DataSetCount;
      /**
       * Get the database SQL dialect.
       * @note Read-only property.
       * @note Reimplements TIBDatabase\::DBSQLDialect
       */
      Property<int>           DBSQLDialect;
      /**
       * Use to make calls directly to the InterBase API.
       * @note Read-only property.
       * @note Reimplements TIBDatabase\::Handle
       */
      Property<void **>       Handle;
      /**
       * Read to determine if the handle to the database is shared.
       * @note Read-only property.
       * @note Reimplements TIBDatabase\::HandleIsShared
       */
      Property<bool>          HandleIsShared;
      /**
       * Specifies how long the database should wait before disconnecting an
       * idle connection.
       * @note Reimplements TIBDatabase\::IdleTimer
       */
      Property<int>           IdleTimer;
      /**
       * Indicates whether or not the database is set to read-only.
       * @note Reimplements TIBDatabase\::IsReadOnly
       */
      Property<bool>          IsReadOnly;
      /**
       * Specifies whether a login dialog appears immediately before opening
       * a new connection. When set to false, the application must supply
       * username and password values programmatically.
       * @note Reimplements TCustomConnection\::LoginPrompt
       */
      Property<bool>          LoginPrompt;
      /**
       * Specifies the database parameters to pass to the InterBase server.
       * Use the Add method of the property object to add string parameters.
       * @note Reimplements TIBDatabase\::Params
       */
      Property<ParamClass *>  Params;
      /**
       * Sets or returns the SQL dialect used by the client. If the connection is
       * active, the SQLDialect property canot be set to a value greater than the
       * database SQL dialect. If the connection is inactive, then on connect an
       * OnDialectDowngradeWarning event may be fired and the SQLDialect property
       * will be downgraded to match the database SQL dialect.
       * @note Reimplements TIBDatabase\::SQLDialect
       */
      Property<int>           SQLDialect;
      /**
       * Returns the number of SQL objects in the database. SQL objects are usually
       * defined as InterBase datasets, IBSQL, and blobs.
       * @note Read-only property.
       * @note Reimplements TIBDatabase\::SQLObjectCount
       */
      Property<int>           SQLObjectCount;
      /**
       * Specifies the database operations to track with the SQL Monitor at runtime.
       * TraceFlags is only meaningful for the SQL Monitor, which is provided to 
       * enable performance tuning and SQL debugging when working with remote SQL
       * database servers.
       * @see VIBTraceFlags
       * @note Reimplements TIBDatabase\::TraceFlags
       */
      Property<VIBTraceFlags> TraceFlags;
      /**
       * Returns the number of transactions associated with the database component.
       * @note Read-only property.
       * @note Reimplements TIBDatabase\::TransactionCount
       */
      Property<int>           TransactionCount;

      /**
       * Use to inspect and set DPB parameters without looking at the Params string list.
       * @note Reimplements TIBDatabase\::DBParamByDPB
       */
      ArrayProperty<DBParamClass *, std::string> DBParamByDPB;
      
      /* TODO:
      VIBDLLFUNC VIBTransaction *VIBCALL VIBDatabase_Transactions(VIBDatabase *vdb, int Index);
      VIBDLLFUNC VIBTransaction *VIBCALL VIBDatabase_InternalTransaction(VIBDatabase *vdb);
      VIBDLLFUNC VIBTransaction *VIBCALL VIBDatabase_GetDefaultTransaction(VIBDatabase *vdb);
      VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetDefaultTransaction(VIBDatabase *vdb, VIBTransaction *vtr);
      VIBDLLFUNC VIBDataSet *    VIBCALL VIBDatabase_DataSets(VIBDatabase *vdb, int Index);
      */
   };
}

#endif

// EOF

