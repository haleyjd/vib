/** @file vibinlines.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Internal Inline Utilities
 * @author James Haley
 *
 * @warning Internal only. Do not include in user programs!
 */

#ifndef VIBINLINES_H__
#define VIBINLINES_H__

#include <ibdatabase.hpp>
#include <ibcustomdataset.hpp>
#include <ibsql.hpp>

#include "vibdatabase.h"
#include "vibtransaction.h"
#include "vibsql.h"
#include "vibdataset.h"

/**
 * Retrieve a VIBDatabase's wrapped TIBDatabase instance.
 * @param vdb VIBDatabase wrapper instance
 * @return Wrapped TIBDatabase instance
 */
static inline TIBDatabase *TDBForVDB(VIBDatabase *vdb)
{
   return static_cast<TIBDatabase *>(vdb->opaque);
}

/**
 * Retrieve a VIBTransaction's wrapped TIBTransaction instance.
 * @param vtr VIBTransaction wrapper instance
 * @return Wrapped TIBTransaction instance
 */
static inline TIBTransaction *TTRForVTR(VIBTransaction *vtr)
{
   return static_cast<TIBTransaction *>(vtr->opaque);
}

/**
 * Retrieve a VIBSQL's wrapped TIBSQL instance.
 * @param vsql VIBSQL wrapper instance.
 * @return Wrapped TIBSQL instance.
 */
static inline TIBSQL *TSQLForVSQL(VIBSQL *vsql)
{
   return static_cast<TIBSQL *>(vsql->opaque);
}

/**
 * Retrieve a VIBDataSet's wrapped TIBDataSet instance.
 * @param vds VIBDataSet wrapper instance.
 * @return Wrapped TIBDataSet instance.
 */
static inline TIBDataSet *TDSForVDS(VIBDataSet *vds)
{
   return static_cast<TIBDataSet *>(vds->opaque);
}

#endif

// EOF


