PROC scx_cpu1_sc_appoffend
;*******************************************************************************
;  Test Name:  sc_appoffend
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	This procedure creates an ATS Append table load image file containing a
;	full table of commands with the last command going over the end of the 
;	table buffer.
;
;  Change History
;
;	Date		   Name		Description
;	01/28/11	Walt Moleski	Original Procedure.
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
local ATSAppendATblName = SCAppName & "." & SC_APPEND_TABLE_NAME
local hostCPU = "CPU3"

;; Set the pkt and app Ids for the appropriate CPU
atsPktId = "0FBB"
atsAppId = 4027

write ";***********************************************************************"
write ";  Create a table load containing as many valid commands that will fit. "
write ";***********************************************************************"
;; Using the RDL, populate the ATS data stream with the data you desire
;;
local i,j,atsCmdCtr,nextCmd,nextCRC

;; Setup the command word and CRC word buffers
nextCmd = x'18A9'
nextCRC = x'0982'

;; JumpATS Command example = '0000 0005 000F 4B6D 1AA9 C000 0005 093A 000F 4DF8'
;; ATS Command entry size = 10 words
;; Loop for each word in the ATS Buffer
j = 1
atsCmdCtr = 1
for i = 1 to SC_APPEND_BUFF_SIZE - 10  do
  if (j = 2) then
    SCX_CPU1_SC_ATSAPPENDDATA[i] = atsCmdCtr 
    atsCmdCtr = atsCmdCtr + 1
  elseif (j = 5) then
    SCX_CPU1_SC_ATSAPPENDDATA[i] = nextCmd
  elseif (j = 6) then
    SCX_CPU1_SC_ATSAPPENDDATA[i] = x'C000'
  elseif (j = 7) then
    SCX_CPU1_SC_ATSAPPENDDATA[i] = x'0005'
  elseif (j = 8) then
    SCX_CPU1_SC_ATSAPPENDDATA[i] = nextCRC
  else
    SCX_CPU1_SC_ATSAPPENDDATA[i] = 0
  endif
  j = j + 1
  if (j = 11) then
    j = 1
  endif
enddo

;; Create the last command to be longer than the remaining words in the buffer
SCX_CPU1_SC_ATSAPPENDDATA[i] = 0
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = atsCmdCtr
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = 0
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = 0
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = x'192B'
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = x'C000'
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = x'0029'
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = x'0365'
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = x'5343'
i = i + 1
SCX_CPU1_SC_ATSAPPENDDATA[i] = x'2E41'

%liv (log_procedure) = logging

local endmnemonic = "SCX_CPU1_SC_ATSAPPENDDATA[" & SC_APPEND_BUFF_SIZE & "]"

;; Create the ATS Table Load file
s create_tbl_file_from_cvt (hostCPU,atsPktId,"Append Table Cmd off end Load","cpu1_appoffend_ld",ATSAppendATblName,"SCX_CPU1_SC_ATSAPPENDDATA[1]",endmnemonic)
wait 5

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_sc_appoffend"
write ";*********************************************************************"
ENDPROC
