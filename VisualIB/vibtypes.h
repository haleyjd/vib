/** @file vibtypes.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Types suitable for inclusion into C code and across compiler boundaries.
 *
 * We explicitly do not rely on any of the following:
 * - sizeof(bool)
 * - sizeof(enum)
 *
 * @author James Haley
 *
 */

#ifndef VIBTYPES_H__
#define VIBTYPES_H__

/** Portable boolean type */
typedef int VIBBOOL;

/** False value for VIBBOOL */
#define VIBFALSE 0

/** True value for VIBBOOL */
#define VIBTRUE  1

// Enumerations

/**
 * Trace flags as used by VIBDatabase. Use to specify which database operations
 * the SQL Monitor should track in an application at runtime. Provided for
 * performance tuning and SQL debugging when working with remote SQL databases.
 * @note Reimplements TTraceFlag
 */
enum VIBTraceFlag
{
   vib_tfQPrepare = 0x00000001, //!< Monitor Prepare statements
   vib_tfQExecute = 0x00000002, //!< Monitor ExecSQL statements
   vib_tfQFetch   = 0x00000004, //!< Undocumented
   vib_tfError    = 0x00000008, //!< Monitor server error messages
   vib_tfStmt     = 0x00000010, //!< Monitor all SQL statements
   vib_tfConnect  = 0x00000020, //!< Monitor database connect and disconnect operations, including handle allocation
   vib_tfTransact = 0x00000040, //!< Monitor transaction statements such as StartTransaction, Commit, and Rollback
   vib_tfBlob     = 0x00000080, //!< Monitor operations on blob data types
   vib_tfService  = 0x00000100, //!< Undocumented
   vib_tfMisc     = 0x00000200  //!< Monitor any statements not covered by other flag options
};
typedef int VIBTraceFlags;

/**
 * Actions for VIBTransaction. Specifies what action a transaction should take
 * upon timing out.
 * @note Reimplements TTransactionAction
 */
enum VIBTransactionAction_e 
{ 
   vib_TARollback,           //!< Rollback the transaction
   vib_TACommit,             //!< Commit the transaction
   vib_TARollbackRetaining,  //!< Rollback the transaction, retaining the context
   vib_TACommitRetaining     //!< Commit the transaction, retaining the context
};
typedef int VIBTransactionAction;

/**
 * AutoStop actions for VIBTransaction. Undocumented by Borland, so, you are
 * on your own using this.
 */
enum VIBAutoStopAction_e 
{ 
   vib_saNone,               //!< Take no action?
   vib_saRollback,           //!< Rollback the transaction?
   vib_saCommit,             //!< Commit the transaction?
   vib_saRollbackRetaining,  //!< Rollback the transaction and retain the context?
   vib_saCommitRetaining     //!< Commit the transaction and retain the context?
};
typedef int VIBAutoStopAction;

/**
 * SQL query types as used by VIBSQL. Determines the type of query to be
 * executed
 * @note Reimplements TIBSQLTypes
 */
enum VIBSQLTypes_e 
{ 
   vib_SQLUnknown,          //!< Unknown SQL type
   vib_SQLSelect,           //!< Retrieves data from one or more tables
   vib_SQLInsert,           //!< Adds one or more new rows to a specified table
   vib_SQLUpdate,           //!< Changes data in all or part of an existing table, view, or active set
   vib_SQLDelete,           //!< Removes rows in a table or active set of a cursor
   vib_SQLDDL,              //!< Modifies the database metadata
   vib_SQLGetSegment,       //!< Reads a segment from an open Blob
   vib_SQLPutSegment,       //!< Writes a Blob segment
   vib_SQLExecProcedure,    //!< Calls a stored procedure
   vib_SQLStartTransaction, //!< Starts a new transaction against one or more databases
   vib_SQLCommit,           //!< Commits an active transaction
   vib_SQLRollback,         //!< Restores the database to its state prior to start of the current transaction
   vib_SQLSelectForUpdate,  //!< Used for positioned updates
   vib_SQLSetGenerator      //!< Sets a new value for an existing generator
};
typedef int VIBSQLTypes;

/**
 * TField Data Types, for VIBDataSet::DataType.
 * @note Reimplements TFieldType
 */
enum VIBFieldType_e 
{ 
   vib_ftUnknown,     //!< Unknown or undetermined
   vib_ftString,      //!< Character or string field
   vib_ftSmallint,    //!< 16-bit integer field
   vib_ftInteger,     //!< 32-bit integer field
   vib_ftWord,        //!< 16-bit unsigned integer field
   vib_ftBoolean,     //!< Boolean field
   vib_ftFloat,       //!< Floating-point numeric field
   vib_ftCurrency,    //!< Money field
   vib_ftBCD,         //!< Binary-coded decimal field
   vib_ftDate,        //!< Date field
   vib_ftTime,        //!< Time field
   vib_ftDateTime,    //!< Date and time field (timestamp)
   vib_ftBytes,       //!< Fixed number of bytes (binary storage)
   vib_ftVarBytes,    //!< Variable number of bytes (binary storage)
   vib_ftAutoInc,     //!< Auto-incrementing 32-bit integer counter field
   vib_ftBlob,        //!< Binary Large Object field
   vib_ftMemo,        //!< Text memo field
   vib_ftGraphic,     //!< Bitmap field
   vib_ftFmtMemo,     //!< Formatted text memo field
   vib_ftParadoxOle,  //!< Paradox OLE field
   vib_ftDBaseOle,    //!< dBASE OLE field
   vib_ftTypedBinary, //!< Typed binary field
   vib_ftCursor,      //!< Output cursor from an Oracle stored procedure
   vib_ftFixedChar,   //!< Fixed character field
   vib_ftWideString,  //!< Wide string field
   vib_ftLargeint,    //!< Large integer field
   vib_ftADT,         //!< Abstract Data Type field
   vib_ftArray,       //!< Array field
   vib_ftReference,   //!< REF field
   vib_ftDataSet,     //!< DataSet field
   vib_ftOraBlob,     //!< BLOB fields in Oracle 8 tables
   vib_ftOraClob,     //!< CLOB fields in Oracle 8 tables
   vib_ftVariant,     //!< Data of unknown or undetermined type
   vib_ftInterface,   //!< References to COM interfaces (IUnknown)
   vib_ftIDispatch,   //!< References to COM IDispatch interfaces
   vib_ftGuid         //!< Globally Unique Identifier value
};
typedef int VIBFieldType;

/**
 * TField Kinds, for VIBDataSet::FieldKind.
 * @note Reimplements TFieldKind
 */
enum VIBFieldKind_e
{
   vib_fkData,         //!< Field represents a physical field in a database table
   vib_fkCalculated,   //!< Field is calculated in an OnCalcFields event handler
   vib_fkLookup,       //!< Field is a lookup field
   vib_fkInternalCalc  //!< Field is calculated but values are stored in the dataset
};
typedef int VIBFieldKind;


#endif

// EOF

