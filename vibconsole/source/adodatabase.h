/** @file adodatabase.h

   NDWCoalesce

   ADO Database Abstraction
*/

#ifndef DATABASE_H__
#define DATABASE_H__

#ifndef VIBC_NO_ADODB

#include <Windows.h>
#include <comdef.h>

#include <map>
#include <string>
#include <vector>

class ADOReadOnlyDB;
class ADORecordSetPimpl;

/**
 * ADORecordSet
 * Record set returned by an ADOReadOnlyDB. Allows use of ordinary SQL queries only.
 * For prepared queries, use an ADOCommand.
*/
class ADORecordSet
{
private:
   ADORecordSetPimpl *pImpl; //!< Private implementation object

protected:
   friend class ADOCommand;
   friend class ADOReadOnlyDB;
   ADORecordSet(ADOReadOnlyDB *db); //!< Protected constructor. Only ADOReadOnlyDB may use this.
   ADORecordSet();                  //!< Protected constructor. Only ADOCommand may use this.

public:
   ~ADORecordSet(); //!< Closes the record set and frees all related resources.

   /**
    * Open a new recordset using a SQL query.
    * The recordset is always unidirectional and read-only.
    * @param query SQL query to run against the associated database
    * @return COM HRESULT value; S_OK if successful, otherwise, an error code.
    */ 
   HRESULT open(const char *query);
   /**
    * Explicitly close an ADORecordSet. This will happen automatically if the object is destroyed.
    * @return COM HRESULT value; S_OK if successful, otherwise, an error code.
    */
   HRESULT close();
   /**
    * Test if the record set is at "end of file," or, past the last valid record.
    * @return True if at EOF, false otherwise
    */
   bool    atEOF();
   /**
    * Step the recordset cursor to the next record.
    * @return COM HRESULT value; S_OK if successful, otherwise, an error code.
    */
   HRESULT next();

   /**
    * Obtain a string representation of the field from the query with the given name,
    * from the row at the recordset's current cursor position.
    * @param fieldName Name of the field; should be same as used in SQL query minus any qualifiers.
    * @return String representation of the field, if successful. Empty string in case of any errors.
    */
   std::string getValue(const char *fieldName);

   /**
    * Obtain the recordset's current cursor-position row as a map of field names to values.
    * @param[out] map Destination for output in the form of field/value pairs.
    */
   void getMap(std::map<std::string, std::string> &map);

   /**
    * Obtain the entire recordset as a vector of string maps. Rows will be in the same order
    * as they were output by the database, with the map at each row of the vector containing
    * values keyed by the corresponding field name used in the SQL query.
    * @param[out] vecmap A vector of string maps to receive the output.
    */
   void getVecMap(std::vector<std::map<std::string, std::string>> &vecmap);
};

/**
 * Smart pointer to an ADORecordSet; this is returned by all functions which create an
 * ADORecordSet object, for proper RAII.
 */
typedef std::unique_ptr<ADORecordSet> ADORecordSetPtr;

class ADOCommandPimpl;

/**
 * ADOCommand
 * Command object allowing use of ordinary or prepared SQL queries. Returned by methods
 * of ADOReadOnlyDB.
 */
class ADOCommand
{
private:
   ADOCommandPimpl *pImpl; //!< Private implementation object

protected:
   friend class ADOReadOnlyDB;
   /**
    * Protected constructor; can only be created by ADOReadOnlyDB.
    * @param[in] db The parent database to which the command is attached
    * @param[in] cmdText SQL command text
    * @param[in] prepared True if should treat SQL as a prepared query; false otherwise.
    */
   ADOCommand(ADOReadOnlyDB *db, const char *cmdText, bool prepared);

public:
   ~ADOCommand(); //!< Frees all resources related to the command object.

   /**
    * Execute the SQL query stored in the command and return a record set.
    * @throws COM exception on failure
    */
   ADORecordSetPtr execute();
   /**
    * Add a boolean parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(bool);
   /**
    * Add a double floating-point parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(double);
   /**
    * Add an integer parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(int);
   /**
    * Add an unsigned integer parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(unsigned int);
   /**
    * Add a string parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(const char *);
   /**
    * Remove all parameters from a prepared query.
    */
   HRESULT clearParameters();
};

/**
 * Smart pointer to an ADOCommand object. Always returned by ADOReadOnlyDB,
 * for proper RAII.
 */
typedef std::unique_ptr<ADOCommand> ADOCommandPtr;


class ADOReadOnlyDBPimpl;

/**
 * ADOReadOnlyDB
 * Read-only connection to an ADO database.
 */
class ADOReadOnlyDB
{
private:
   friend class ADORecordSet;
   friend class ADOCommand;
   ADOReadOnlyDBPimpl *pImpl; //!< Private implementation object

protected:
   /**
    * Handles initialization of COM and ADO when any ADOReadOnlyDB
    * object is instantiated. This means you never need to worry
    * about doing it anywhere in your program.
    * @return True if initialization was successful; otherwise, false.
    */
   static bool InitADO();

public:
   /**
    * Call to construct an ADOReadOnlyDB instance. A connection is not opened
    * until the open method is called.
    */
   ADOReadOnlyDB();
   /**
    * When the ADOReadOnlyDB instance falls out of scope, it will
    * disconnect from the database if the connection is still active.
    * This will invalidate any commands or recordsets attached to the
    * database that may still be alive.
    */
   ~ADOReadOnlyDB();

   /**
    * Open a connection to a specific ADO database. The format of
    * the connection string is driver-dependent.
    * @param[in] connectionStr Database connection string (server, user, password, etc)
    * @return COM HRESULT; S_OK if connection succeeded, error code otherwise.
    */
   HRESULT open(const char *connectionStr);
   /**
    * Close the database connection if it is open.
    * @return COM HRESULT; S_OK if connection succeeded, error code otherwise.
    */
   HRESULT close();

   /**
    * Get a new record set associated with this database.
    * @return Smart pointer to an ADORecordSet object.
    * @pre The database must be connected.
    */
   ADORecordSetPtr getRecordSet();
   /**
    * Get a command object associated with this database.
    * @param[in] sqlCmd SQL command text
    * @param[in] prepared True if command text is a prepared query; false otherwise.
    * @return Smart pointer to an ADOCommand object.
    * @pre The database must be connected.
    */
   ADOCommandPtr   getCommand(const char *sqlCmd, bool prepared);
   /**
    * Get a command object associated with this database.
    * @param[in] filename Absolute path to a file to load as a SQL query
    * @param[in] prepared True if command text is a prepared query; false otherwise.
    * @return Smart pointer to an ADOCommand object.
    * @pre The database must be connected.
    */
   ADOCommandPtr   getCommandFile(const char *filename, bool prepared);
   /**
    * Get a command object associated with this database.
    * @param[in] str Absolute path to a file to load as a SQL query
    * @param[in] prepared True if command text is a prepared query; false otherwise.
    * @return Smart pointer to an ADOCommand object.
    * @pre The database must be connected.
    */
   ADOCommandPtr   getCommandFile(const std::string &str, bool prepared)
   {
      return getCommandFile(str.c_str(), prepared);
   }

   /**
    * Execute a generic SQL command against the connection to the database.
    * @param[in] str Command to run
    * @pre The database must be connected.
    */
   void execute(const char *str);

   /**
    * Utility function to print out all information related to a _com_error exception.
    * Output is to the standard output stream.
    * @param[in] e Reference to a _com_error exception thrown from any ADO-related function.
    */
   static void PrintError(_com_error &e);

   static void PrintError(const char *msg);
};

#endif // VIBC_NO_ADODB

#endif // DATABASE_H__

// EOF

