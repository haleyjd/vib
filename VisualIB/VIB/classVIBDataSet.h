/** @file classVIBDataSet.h
 *
 *  VIB Class Library - VIBDataSet Wrapper
 *  @author James Haley
 */

#ifndef CLASSVIBDATASET_H__
#define CLASSVIBDATASET_H__

#include <string>
#include "VIBProperties.h"

struct VIBDataSet;
struct VIBDatabase;
struct VIBTransaction;

namespace VIB
{
   class Database;
   class Transaction;

   /**
    * VIB::DataSet wraps the VisualIB VIBDataSet C structure to provide a C++11
    * object with semantics compatible with those of Borland TIBDataSet.
    */
   class DataSet
   {
   protected:
      VIBDataSet *vds; //!< Pointer to the wrapped VIBDataSet instance.

   public:
      /**
       * Construct a VIB::DataSet instance.
       * @param[in] pvds Optional pointer to an existing VIBDataSet structure 
       *   of which this class instance should take ownership.
       */
      DataSet(VIBDataSet *pvds = NULL);
      /**
       * Destroy the VIB::DataSet and the VIBDataSet instance it wraps.
       */
      virtual ~DataSet();

      // Methods
      /**
       * Opens the dataset. Call to set the Active property to true, and enable
       * data to be read from and written to the database.
       * @note Reimplements TDataSet\::Open
       */
      void Open();
      /**
       * Positions the cursor on the first record in the dataset and makes it 
       * the active record. 
       * @note Reimplements TDataSet\::First
       */
      void First();
      /**
       * Positions the cursor on the next record in the dataset and makes it
       * the active record. Bof and Eof properties will be set to false. If the
       * cursor was already on the last record in the dataset, the Eof property
       * will be set to true.
       * @note Reimplements TDataSet\::Next
       */
      void Next();
      /**
       * Closes a dataset. The Active property will be set to false, and data
       * can no longer be read from or written to the database using this object.
       * @note Reimplements TDataSet\::Close
       */
      void Close();
      /**
       * Prepares all queries in the dataset to be executed.
       * @note Reimplements TIBDataSet\::Prepare
       */
      void Prepare();
      /**
       * Returns true or false if the dataset is at Eof.
       * @note API Difference - This is a property in TIBDataSet, not a method.
       */
      bool Eof();
      /**
       * Undocumented method.
       * @returns The SQL query plan for the SelectSQL statement?
       */
      std::string Plan();

      // Property classes

      /**
       * Class for VIB::DataSet::FieldByName method's return value.
       */
      class FieldByNameClass
      {
      protected:
         friend class DataSet;
         DataSet *parent;       //!< Pointer to containing VIB::DataSet
         std::string fieldname; //!< Name of the field
      public:
         /**
          * Construct a FieldByName property wrapper object.
          * @param[in] pParent Pointer to the parent VIB::DataSet instance.
          */
         FieldByNameClass(DataSet *pParent);

         /**
          * Obtain the value of the named field as a string.
          * @note Read-only property.
          * @note Reimplements TField\::AsString
          */
         Property<std::string> AsString;
      };
      friend class FieldByNameClass;

      
      class FieldProp;

      /**
       * Fields property wrapper object.
       * Actual value of a Fields[] index, equivalent to Borland TField.
       */
      class FieldClass
      {
      protected:
         friend class FieldProp;
         int index;   //!< Index of this field in the Fields array property
         DataSet *ds; //!< Pointer to the parent VIB::DataSet instance.

      public:
         /**
          * Construct a Fields value.
          * @param[in] pds Pointer to the parent VIB::DataSet instance.
          */
         FieldClass(DataSet *pds);

         /**
          * Obtain the value of the field as a string.
          * @note Reimplements TField\::AsString
          */
         Property<std::string>  AsString;
         /**
          * Obtain the data type of the field.
          * @note Reimplements TField\::DataType
          * @see VIBFieldType
          */
         Property<VIBFieldType> DataType;
         /**
          * Obtain the kind of the field.
          * @note Reimplements TField\::FieldKind
          * @see VIBFieldKind
          */
         Property<VIBFieldKind> FieldKind;
         /**
          * Obtain the name of the field as a string.
          * @note Reimplements TField\::FieldName
          */
         Property<std::string>  FieldName;
      };
      friend class FieldClass;
      
      /**
       * Intermediary for array-style access to fields, equivalent to the Borland 
       * TFields::Fields array.
       */
      class FieldProp
      {
      protected:
         FieldClass  fc; //!< FieldClass instance
         DataSet    *ds; //!< DataSet instance

      public:
         /**
          * Construct the Field property wrapper object.
          * @param[in] pds Pointer to the parent VIB::DataSet instance.
          */
         FieldProp(DataSet *pds) : ds(pds), fc(pds) {}

         /**
          * Allows array-style access to field properties.
          * @returns Pointer to an instance of FieldClass.
          * @note Reimplements TFields::operator []
          */
         FieldClass *operator [] (int i)
         {
            fc.index = i;
            fc.ds    = ds;
            return &fc;
         }
      };

      /**
       * Top-level Fields property wrapper class, equivalent to Borland TFields object.
       */
      class FieldsClass
      {
      protected:
         DataSet   *parent; //!< Parent DataSet instance
         FieldProp fp;      //!< FieldProp instance

      public:
         FieldsClass(DataSet *pParent);

         /**
          * Represents the number of fields in the Fields object.
          * @note Read-only property.
          * @note Reimplements TFields\::Count
          */
         Property<int> Count;
         /**
          * Lists the field references that are managed by the Fields object.
          * @note Read-only property.
          * @note Reimplements TFields\::Fields
          */
         ArrayProperty<FieldProp *, FieldClass *> Fields;
      };

      /**
       * Property implementation class for the SelectSQL property.
       */
      class SelectSQLClass
      {
      protected:
         DataSet *parent; //!< Pointer to parent VIB::DataSet instance.

      public:
         /**
          * Construct a SelectSQL wrapper object.
          * @param[in] pParent Pointer to parent VIB::DataSet instance.
          */
         SelectSQLClass(DataSet *pParent);

         /**
          * Clear the SelectSQL list of all queries.
          * @note Reimplements TStrings\::Clear
          */
         void Clear();
         /**
          * Add a query to the SelectSQL list.
          * @param[in] str SQL query code to add.
          * @note Reimplements TStrings\::Add
          */
         void Add(const std::string &str);

         /**
          * Lists the strings in the object as a single string with the
          * individual strings delimited by carriage returns and line feeds.
          * Use Text to get or set all the strings in the object at once.
          * @note Reimplements TStrings\::Text
          */
         Property<std::string> Text;
      };
      friend class SelectSQLClass;

   protected:
      // Property class instances
      FieldByNameClass fbnc; //!< Instance of FieldByName property wrapper.
      FieldsClass      fc;   //!< Instance of Fields property wrapper.
      SelectSQLClass   sql;  //!< Instance of SelectSQL property wrapper.

   public:
      /**
       * Finds a field based on its name. If the specified field does not exist,
       * an exception will be thrown.
       * @param[in] fieldname The name of a simple field, subfield of an object
       *   field qualified by the parent field's name, or name of an aggregated
       *   field.
       * @returns Pointer to the field object. An application can directly access
       *   specific properties and methods of the field returned by FieldByName.
       * @note Reimplements TDataSet\::FieldByName
       */
      FieldByNameClass *FieldByName(const std::string &fieldname);

      // Properties
      /**
       * Identifies the database component for which the dataset represents 
       * one or more tables.
       * @note Reimplements TIBCustomDataSet\::Database
       * @note API Difference - The returned object is a VIBDatabase C structure,
       *   not a TIBDatabase. The corresponding TIBDatabase is hidden by the VIBDatabase
       *   abstraction.
       */
      Property<VIBDatabase *>      Database;
      /**
       * Indicates the number of field components associated with the dataset.
       * For datasets with dynamically created fields, FieldCount may differ each time
       * a dataset is opened. FieldCount includes only the fields listed by the Fields
       * property. Any aggregated fields listed by the AggFields property are not 
       * included in the count.
       * @note Reimplements TDataSet\::FieldCount
       */
      Property<int>                FieldCount;
      /**
       * Lists all non-aggregate field components of the dataset. If fields are generated
       * dynamically at runtime, the order of field components in Fields corresponds
       * directly to the order of columns in the table or tables underlying a dataset.
       * @note Reimplements TDataSet\::Fields
       */
      Property<FieldsClass *>      Fields;
      /**
       * Provides the ability to directly access the SQL object encapsulating the SelectSQL
       * statement.
       * @note Reimplements TIBDataSet\::SelectSQL
       */
      Property<SelectSQLClass *>   SelectSQL;
      /**
       * Identifies the transaction under which the query executes.
       * @note Reimplements TIBCustomDataSet\::Transaction
       * @note API Difference - The returned object is a VIBTransaction C structure,
       *  not a TIBTransaction. The corresponding TIBTransaction is hidden by the
       *  VIBTransaction abstraction.
       */
      Property<VIBTransaction *>   Transaction;
      /**
       * Determines whether or not bidirectional cursors are enabled for a table.
       * @note Reimplements TIBDataSet\::UniDirectional
       */
      Property<bool>               UniDirectional;      
   };
}

#endif

// EOF

