/** @file classVIBTransaction.h
 *
 *  VIB::Database Class
 *  @author James Haley
 */

#ifndef CLASSVIBTRANSACTION_H__
#define CLASSVIBTRANSACTION_H__

#include "../vibtypes.h"
#include "VIBProperties.h"

struct VIBTransaction;
struct VIBDatabase;

namespace VIB
{
   class Database;

   /**
    * VIB::Transaction wraps the VisualIB VIBTransaction structure with a C++11
    * object which emulates the semantics of Borland TIBTransaction.
    */
   class Transaction
   {
   protected:
      VIBTransaction *vtr; //!< Pointer to the wrapped VIBTransaction instance

   public:
      /**
       * Construct a new VIB::Transaction.
       * @param[in] pvtr Optional pointer to an existing VIBTransaction instance of which
       *   this object should take ownership.
       */
      Transaction(VIBTransaction *pvtr = NULL);
      /**
       * Destroy the VIB::Transaction object and the VIBTransaction it wraps.
       */
      virtual ~Transaction();

      /**
       * Returns an error message based on the error code. Internal method used to make
       * calls to the InterBase API, giving the option of throwing an exception or returning
       * an error.
       * @param[in] ErrCode    InterBase API error code?
       * @param[in] RaiseError If true, an exception will be thrown.
       * @returns   Error message?
       * @note Reimplements TIBTransaction\::Call
       */
      int  Call(int ErrCode, bool RaiseError);
      /**
       * Permanently stores updates, insertions, and deletions of data associated
       * with the current transaction, and ends the current transaction. If there is
       * no current transaction, an exception will be thrown.
       * @note Reimplements TIBTransaction\::Commit
       */
      void Commit();
      /**
       * Commits the active transaction and retains the transaction context.
       * @note Reimplements TIBTransaction\::CommitRetaining
       */
      void CommitRetaining();
      /**
       * Cancels all updates, insertions, and deletions for the current transaction
       * and ends the transaction. If there is no current transaction, an exception
       * will be thrown.
       * @note Reimplements TIBTransaction\::Rollback
       */
      void Rollback();
      /**
       * Cancels all updates, insertions, and deletions for the current transaction
       * and retains the transaction context.
       * @note Reimplements TIBTransaction\::RollbackRetaining
       */
      void RollbackRetaining();
      /**
       * Begins a new transaction against the database server. Before calling, an
       * application should check the status of the InTransaction property. Updates,
       * insertions, and deletions that take place after a call to StartTransaction
       * are held by the server until an application calls Commit to save the changes
       * or Rollback to cancel them.
       * @note Reimplements TIBDatabase\::StartTransaction
       */
      void StartTransaction();
      /**
       * Checks whether the transaction is active and whether there are any databases
       * in the transaction's database list. If either condition is false, an exception
       * is thrown.
       * @note Reimplements TIBTransaction\::CheckInTransaction
       */
      void CheckInTransaction();
      /**
       * Checks that the transaction is not active and that there are no databases in
       * the transaction's database list. If either condition is false, an exception
       * is thrown.
       * @note Reimplements TIBTransaction\::CheckNotInTransaction
       */
      void CheckNotInTransaction();
      /**
       * Undocumented method.
       * @note Reimplements TIBTransaction\::CheckAutoStop
       */
      void CheckAutoStop();
      /**
       * Associates a database to the transaction.
       * @param[in] db Pointer to a VIB::Database instance.
       * @returns Index of the database
       * @note Reimplements TIBTransaction\::AddDatabase
       * @note API Difference - The parameter taken is a VIB::Database instance.
       *  The corresponding TIBDatabase is wrapped within the VIB::Database's
       *  VIBDatabase structure.
       */
      int  AddDatabase(Database *db);
      /**
       * Finds the index of the associated database.
       * @param[in] db Pointer to a VIB::Database instance.
       * @returns Index of the associated database.
       * @note Reimplements TIBTransaction\::FindDatabase
       * @note API Difference - The parameter taken is a VIB::Database instance.
       *  The corresponding TIBDatabase is wrapped within the VIB::Database's
       *  VIBDatabase structure.
       */
      int  FindDatabase(Database *db);
      /**
       * Undocumented method.
       * @returns Pointer to the default database?
       * @note Reimplements TIBTransaction\::FindDefaultDatabase
       * @note API Difference - The return value is a pointer to a VIBDatabase
       *  instance. The corresponding TIBDatabase is hidden by the VisualIB
       *  C API abstraction.
       */
      VIBDatabase *FindDefaultDatabase();
      /**
       * Disassociates a database from the transaction.
       * @param[in] Idx Index of the database, as returned by FindDatabase.
       * @note Reimplements TIBTransaction\::RemoveDatabase
       */
      void RemoveDatabase(int Idx);
      /**
       * Disassociates all databases from the transaction.
       * @note Reimplements TIBTransaction\::RemoveDatabases
       */
      void RemoveDatabases();
      /**
       * Checks if there are any databases in the list. IF there are none,
       * an exception will be thrown.
       * @note Reimplements TIBTransaction\::CheckDatabasesInList
       */
      void CheckDatabasesInList();

      /**
       * Obtain the underlying VisualIB VIBTransaction C structure instance.
       * @returns Pointer to the wrapped VIBTransaction instance.
       */
      VIBTransaction *getVIBTransaction() const { return vtr; }

      // Property classes

      /**
       * Property implementation class for the Databases property
       */
      class DatabasesClass
      {
      protected:
         Transaction *parent; //!< Pointer to parent VIB::Transaction instance.
      public:
         /**
          * Construct the Databases property wrapper object.
          * @param[in] pParent Pointer to the parent VIB::Transaction instance.
          */
         DatabasesClass(Transaction *pParent) : parent(pParent) {}

         /**
          * Access the database at the given index, as would be returned by
          * FindDatabase.
          * @note Reimplements Databases::operator []
          * @note API Difference - The returned object is a VIBDatabase instance.
          *  The corresponding TIBDatabase is hidden by the VisualIB C API 
          *  abstraction.
          */
         VIBDatabase *operator [] (int i);
      };

      /**
       * Property implementation class for parameters
       */
      class ParamClass
      {
      protected:
         Transaction *parent; //!< Pointer to the parent VIB::Transaction instance
      public:
         /**
          * Construct the wrapper for an individual parameter.
          * @param[in] pParent Pointer to the parent VIB::Transaction instance.
          */
         ParamClass(Transaction *pParent) : parent(pParent) {}
         /**
          * Add a parameter to the list.
          * @param[in] str Parameter string value to add.
          * @note Reimplements TStrings\::Add
          */
         void Add(const std::string &str);
      };
      friend class ParamClass;

   protected:
      // Property class instances
      DatabasesClass dbObject;    //!< Instance of Databases property wrapper
      ParamClass     paramObject; //!< Instance of Params property wrapper

   public:
      // Properties 
      /**
       * Specifies whether or not a transaction is active.
       * @note Reimplements TIBTransaction\::Active
       */
      Property<bool>                 Active;
      /**
       * Undocumented property.
       * @note Reimplements TIBTransaction\::AutoStopAction
       * @see VIBAutoStopAction
       */
      Property<VIBAutoStopAction>    AutoStopAction;
      /**
       * Indicates the number of databases that are part of the transaction.
       * @note Read-only property.
       * @note Reimplements TIBTransaction\::DatabaseCount
       */
      Property<int>                  DatabaseCount;
      /**
       * Specifies what action a transaction should take upon timing out.
       * @note Reimplements TIBTransaction\::DefaultAction
       */
      Property<VIBTransactionAction> DefaultAction;
      /**
       * Sets or returns the default database for the transaction.
       * @note Reimplements TIBTransaction\::DefaultDatabase
       * @note API Difference - The property type is VIBDatabase.
       *  The corresponding TIBDatabase instance is hidden by the
       *  VisualIB C API abstraction.
       */
      Property<VIBDatabase *>        DefaultDatabase;
      /**
       * Returns the transaction handle.
       * @note Read-only property.
       * @note Reimplements TIBTransaction\::Handle.
       * @note API Difference - The property type is equivalent to
       *  Ibexternals::PVoid.
       */
      Property<void *>               Handle;
      /**
       * Indicates whether or not the handle is shared.
       * @note Read-only property.
       * @note Reimplements TIBTransaction\::HandleIsShared
       */
      Property<bool>                 HandleIsShared;
      /**
       * Specifies how long the transaction should wait before
       * automatically committing or rolling back, based on the
       * DefaultAction property.
       * @note Reimplements TIBTransaction\::IdleTimer
       */
      Property<int>                  IdleTimer;
      /**
       * Indicates whether a database transaction is in progress or not.
       * @note Read-only property. Call StartTransaction to begin a 
       *   transaction, and Commit or Rollback to end it.
       * @note Reimplements TIBTransaction\::InTransaction
       */
      Property<bool>                 InTransaction;
      /**
       * Returns the transaction parameter buffer. Use to examine and
       * set parameters in the buffer. Refer to the InterBase API Guide
       * for the names of the parameters to provide.
       * @note Reimplements TIBTransaction\::Params
       */
      Property<ParamClass *>         Params;
      /**
       * Returns the number of currently active datasets, SQL objects, and
       * blobs associated with the database.
       * @note Read-only property.
       * @note Reimplements TIBTransaction\::SQLObjectCount
       */
      Property<int>                  SQLObjectCount;
      /**
       * Provides a read-only view of the transaction parameter buffer.
       * @note Reimplements TIBTransaction\::TPB
       */
      Property<char *>               TPB;
      /**
       * Use to retrieve the length of the transaction parameter buffer.
       * @note Read-only property.
       * @note Reimplements TIBTransaction\::TPBLength
       */
      Property<short>                TPBLength;

      /**
       * Use to obtain the database at a given integer index.
       * @note Read-only property.
       * @note Reimplements TIBTransaction\::Databases.
       * @note API Difference - Each index of the array is a pointer to
       *  a VIBDatabase instance. The corresponding TIBDatabase instances
       *  are hidden by the VisualIB C API abstraction.
       */
      ArrayProperty<DatabasesClass *, VIBDatabase *> Databases;
   };
}

#endif

// EOF

