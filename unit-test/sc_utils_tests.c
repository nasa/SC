
#include "cfe.h"
#include "sc_utils.h"
#include "sc_events.h"
#include "sc_msgids.h"
#include "sc_test_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* sc_utils_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

void SC_GetCurrentTime_Test(void)
{
    SC_AppData.CurrentTime = 0;

    /* Execute the function being tested */
    SC_GetCurrentTime();

    /* Verify results */
    UtAssert_True(SC_AppData.CurrentTime != 0, "SC_AppData.CurrentTime != 0");

} /* end SC_GetCurrentTime_Test */

void SC_GetAtsEntryTime_Test(void)
{
    SC_AbsTimeTag_t AbsTimeTag;

    SC_AtsEntryHeader_t Entry;
    Entry.TimeTag_MS = 0;
    Entry.TimeTag_LS = 10;

    /* Execute the function being tested */
    AbsTimeTag = SC_GetAtsEntryTime(&Entry);

    /* Verify results */
    UtAssert_True(AbsTimeTag == 10, "AbsTimeTag == 10");

} /* end SC_GetAtsEntryTime_Test */

void SC_ComputeAbsTime_Test(void)
{
    SC_AbsTimeTag_t AbsTimeTag;

    SC_AppData.CurrentTime = 0;

    /* Execute the function being tested */
    AbsTimeTag = SC_ComputeAbsTime(0);

    /* Verify results */

    /* The CFE_TIME_Add stub increments when status >= 0 */
    UtAssert_True(AbsTimeTag == 1, "AbsTimeTag == 1");

} /* end SC_ComputeAbsTime_Test */

void SC_CompareAbsTime_Test_True(void)
{
    bool Result;

    SC_AbsTimeTag_t AbsTimeTag1 = {0};
    SC_AbsTimeTag_t AbsTimeTag2 = {0};

    UT_SetDeferredRetcode(UT_KEY(CFE_TIME_Compare), 1, CFE_TIME_A_GT_B);

    /* Execute the function being tested */
    Result = SC_CompareAbsTime(AbsTimeTag1, AbsTimeTag2);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

} /* end SC_CompareAbsTime_Test_True */

void SC_CompareAbsTime_Test_False(void)
{
    bool Result;

    SC_AbsTimeTag_t AbsTimeTag1 = {0};
    SC_AbsTimeTag_t AbsTimeTag2 = {0};

    UT_SetDeferredRetcode(UT_KEY(CFE_TIME_Compare), 1, -1);

    /* Execute the function being tested */
    Result = SC_CompareAbsTime(AbsTimeTag1, AbsTimeTag2);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

} /* end SC_CompareAbsTime_Test_False */

void SC_VerifyCmdLength_Test_Nominal(void)
{
    SC_NoArgsCmd_t    CmdPacket;
    bool              Result;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    SC_InitTables();

    TestMsgId = SC_CMD_MID;
    FcnCode   = SC_NOOP_CC;
    MsgSize   = sizeof(SC_NoArgsCmd_t);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyCmdLength((CFE_MSG_Message_t *)(&CmdPacket), sizeof(SC_NoArgsCmd_t));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 0, "SC_OperData.HkPacket.CmdErrCtr == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyCmdLength_Test_Nominal */

void SC_VerifyCmdLength_Test_LenError(void)
{
    SC_NoArgsCmd_t    CmdPacket;
    bool              Result;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid msg length: ID = 0x%%08X, CC = %%d, Len = %%d, Expected = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    SC_InitTables();

    TestMsgId = SC_CMD_MID;
    FcnCode   = SC_NOOP_CC;
    MsgSize   = sizeof(SC_NoArgsCmd_t);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyCmdLength((CFE_MSG_Message_t *)(&CmdPacket), 999);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");
    UtAssert_True(SC_OperData.HkPacket.CmdErrCtr == 1, "SC_OperData.HkPacket.CmdErrCtr == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, SC_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyCmdLength_Test_LenError */

void SC_VerifyCmdLength_Test_LenErrorNotMID(void)
{
    SC_NoArgsCmd_t    CmdPacket;
    bool              Result;
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid msg length: ID = 0x%%08X, CC = %%d, Len = %%d, Expected = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    SC_InitTables();

    TestMsgId = SC_SEND_HK_MID;
    FcnCode   = 0;
    MsgSize   = sizeof(SC_NoArgsCmd_t);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(SC_AppendAtsCmd_t), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = SC_VerifyCmdLength((CFE_MSG_Message_t *)(&CmdPacket), 999);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    UtAssert_True(SC_OperData.HkPacket.CmdCtr == 0, "SC_OperData.HkPacket.CmdCtr == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, SC_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

} /* end SC_VerifyCmdLength_Test_LenErrorNotMID */

void SC_ToggleAtsIndex_Test(void)
{
    uint16 Result;

    SC_AtpControlBlock_t AtsCtrlBlck;

    SC_OperData.AtsCtrlBlckAddr = &AtsCtrlBlck;

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 1;

    Result = SC_ToggleAtsIndex();

    UtAssert_True(Result == 1, "Result == 1");

    SC_OperData.AtsCtrlBlckAddr->AtsNumber = 2;

    Result = SC_ToggleAtsIndex();

    UtAssert_True(Result == 0, "Result == 0");

} /* end SC_ToggleAtsIndex_Test */

void UtTest_Setup(void)
{
    UtTest_Add(SC_GetCurrentTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_GetCurrentTime_Test");
    UtTest_Add(SC_GetAtsEntryTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_GetAtsEntryTime_Test");
    UtTest_Add(SC_ComputeAbsTime_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ComputeAbsTime_Test");
    UtTest_Add(SC_CompareAbsTime_Test_True, SC_Test_Setup, SC_Test_TearDown, "SC_CompareAbsTime_Test_True");
    UtTest_Add(SC_CompareAbsTime_Test_False, SC_Test_Setup, SC_Test_TearDown, "SC_CompareAbsTime_Test_False");
    UtTest_Add(SC_VerifyCmdLength_Test_Nominal, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyCmdLength_Test_Nominal");
    UtTest_Add(SC_VerifyCmdLength_Test_LenError, SC_Test_Setup, SC_Test_TearDown, "SC_VerifyCmdLength_Test_LenError");
    UtTest_Add(SC_VerifyCmdLength_Test_LenErrorNotMID, SC_Test_Setup, SC_Test_TearDown,
               "SC_VerifyCmdLength_Test_LenErrorNotMID");
    UtTest_Add(SC_ToggleAtsIndex_Test, SC_Test_Setup, SC_Test_TearDown, "SC_ToggleAtsIndex_Test");

} /* end UtTest_Setup */

/************************/
/*  End of File Comment */
/************************/
