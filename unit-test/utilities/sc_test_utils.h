
/*
 * Includes
 */

#include "sc_app.h"
#include "utstubs.h"

extern SC_AppData_t  SC_AppData;
extern SC_OperData_t SC_OperData;

/*
 * Global context structures
 */

typedef struct
{
    uint16      EventID;
    uint16      EventType;
    const char *Spec;
} __attribute__((packed)) CFE_EVS_SendEvent_context_t;

typedef struct
{
    const char *Spec;
} __attribute__((packed)) CFE_ES_WriteToSysLog_context_t;

/*
 * Function Definitions
 */

int32 UT_Utils_stub_reporter_hook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context);
void SC_Test_Setup(void);
void SC_Test_TearDown(void);

/************************/
/*  End of File Comment */
/************************/
