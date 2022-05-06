PROC scx_cpu1_sc_atsoffend
;*******************************************************************************
;  Test Name:  sc_atsoffend
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	This procedure creates an ATS table load image file containing a full
;	table of commands with the last command going over the end of the table
;	buffer.
;
;  Change History
;
;	Date		   Name		Description
;	01/30/09	Walt Moleski	Original Procedure.
;	03/30/10	Walt Moleski	Modified procedure to contain only
;					SC_JumpATS commands since they specify
;					a 4-byte time in the data portion
;	01/25/11	Walt Moleski	Modified to use variable names for the
;					SC app name and table name
;       10/24/16        Walt Moleski    Added hostCPU variable used to indicate
;                                       the proper CPU for the table load
;       11/12/2020      Walt Moleski    Updated to handle 4-byte ATS Cmd Num
;
;  Arguments
;	None.
;
;  Procedures Called
;	Name			Description
;
;**********************************************************************

local logging = %liv (log_procedure)
%liv (log_procedure) = FALSE

#include "sc_platform_cfg.h"

;**********************************************************************
; Define local variables
;**********************************************************************
LOCAL rawcmd, stream
LOCAL cmdCtr, errcnt
LOCAL atsPktId,atsAppId
local SCAppName = "SC"
local ATSATblName = SCAppName & "." & SC_ATS_TABLE_NAME & "1"
local hostCPU = "CPU3"

;; Set the pkt and app Ids for the appropriate CPU
atsPktId = "0FBB"
atsAppId = 4027

write ";***********************************************************************"
write ";  Create a table load containing a command that runs off the end of the"
write ";  ATS buffer. "
write ";***********************************************************************"
;; Using the RDL, populate the ATS data stream with the data you desire
;;
local j,atsCmdCtr,nextCmd[6],nextCRC[6],nextCmdNum

;; NOTE: prior to SC 2.1.0.0, the ATS Buffer was able to contain exactly
;;       SC_MAX_ATS_CMDS of the minimum size. With SC 2.1.0.0, the pad words
;;       were removed from the ATS Entry structure which reduced the minimum
;;	 entry size by 1 word. Only the JumpATS command is populated which is
;;	 specified in slot 1 of the command and CRC buffers below
;; Setup the command word and CRC word buffers
;; CPU1 is the default
nextCmd[1] = x'18A9'
nextCRC[1] = x'0982'
;;nextCmd[2] = x'1804'
;;nextCRC[2] = x'0022'
;;nextCmd[3] = x'1801'
;;nextCRC[3] = x'0126'
;;nextCmd[4] = x'1806'
;;nextCRC[4] = x'0020'
;;nextCmd[5] = x'1803'
;;nextCRC[5] = x'0025'
;;nextCmd[6] = x'1805'
;;nextCRC[6] = x'0023'

;; JumpATS Command = '0000 0005 000F 4B6D 1AA9 C000 0005 093A 000F 4DF8'
;; ATS Command entry size = 10 words
;; Loop for each word in the ATS Buffer
j = 1
atsCmdCtr = 1
nextCmdNum = 1
for i = 1 to SC_ATS_BUFF_SIZE - 10  do
  if (j = 2) then
    SCX_CPU1_SC_ATSDATA[i] = atsCmdCtr 
    atsCmdCtr = atsCmdCtr + 1
  elseif (j = 5) then
    SCX_CPU1_SC_ATSDATA[i] = nextCmd[1]
  elseif (j = 6) then
    SCX_CPU1_SC_ATSDATA[i] = x'C000'
  elseif (j = 7) then
    SCX_CPU1_SC_ATSDATA[i] = x'0005'
  elseif (j = 8) then
    SCX_CPU1_SC_ATSDATA[i] = nextCRC[1]
  else
    SCX_CPU1_SC_ATSDATA[i] = 0
  endif
  j = j + 1
  if (j = 11) then
    j = 1
  endif
enddo

;; Create the last command to be longer than the remaining words in the buffer
SCX_CPU1_SC_ATSDATA[i] = 0
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = atsCmdCtr
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = 0
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = 0
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = x'192B'
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = x'C000'
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = x'0027'
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = x'0365'
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = x'5343'
i = i + 1
SCX_CPU1_SC_ATSDATA[i] = x'2E41'

%liv (log_procedure) = logging

local endmnemonic = "SCX_CPU1_SC_ATSDATA[" & SC_ATS_BUFF_SIZE & "]"

;; Create the ATS Table Load file
s create_tbl_file_from_cvt (hostCPU,atsPktId,"ATS A Table Command off end Load", "cpu1_cmdoffend_ld",ATSATblName,"SCX_CPU1_SC_ATSDATA[1]",endmnemonic)
wait 5

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_sc_atsoffend"
write ";*********************************************************************"
ENDPROC
