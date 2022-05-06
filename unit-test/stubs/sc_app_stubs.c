
/*
 * Includes
 */

#include "sc_app.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

SC_AppData_t  SC_AppData;
SC_OperData_t SC_OperData;

void SC_AppMain(void)
{
    UT_DEFAULT_IMPL(SC_AppMain);
}

int32 SC_AppInit(void)
{
    return UT_DEFAULT_IMPL(SC_AppMain);
}

int32 SC_InitTables(void)
{
    return UT_DEFAULT_IMPL(SC_InitTables);
}

int32 SC_RegisterAllTables(void)
{
    return UT_DEFAULT_IMPL(SC_RegisterAllTables);
}

int32 SC_GetDumpTablePointers(void)
{
    return UT_DEFAULT_IMPL(SC_GetDumpTablePointers);
}

int32 SC_GetLoadTablePointers(void)
{
    return UT_DEFAULT_IMPL(SC_GetLoadTablePointers);
}

void SC_LoadDefaultTables(void)
{
    UT_DEFAULT_IMPL(SC_LoadDefaultTables);
}

void SC_RegisterManageCmds(void)
{
    UT_DEFAULT_IMPL(SC_RegisterManageCmds);
}
