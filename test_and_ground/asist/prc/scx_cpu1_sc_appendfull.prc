PROC scx_cpu1_sc_appendfull
;*******************************************************************************
;  Test Name:  sc_appendfull
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	This procedure creates an ATS Append table load image file containing a
;	full table of valid commands. 
;
;  Change History
;
;	Date		   Name		Description
;	01/31/11	Walt Moleski	Original Procedure.
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
local i,j,atsCmdCtr,nextCmd,nextCRC,wordsLeft=0

;; Setup the command word and CRC word buffers
;; CPU1 is the default
nextCmd = x'18A9'
nextCRC = x'0982'

;; JumpATS Command example = '0000 0005 0000 0000 1xA9 C000 0005 09yy 0000 0000'
;; ATS Command entry size = 10 words
;; Loop for each word in the Append Buffer
j = 1
atsCmdCtr = 1
for i = 1 to SC_APPEND_BUFF_SIZE do
  if (j = 2) then
    wordsLeft = SC_APPEND_BUFF_SIZE - i
    if (wordsLeft > 10) then
      SCX_CPU1_SC_ATSAPPENDDATA[i] = atsCmdCtr 
      atsCmdCtr = atsCmdCtr + 1
    else
      break
    endif
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

%liv (log_procedure) = logging

write "==> i after cmds loop = ", i

;; Loop for the remaining words in the buffer
for j = i to SC_APPEND_BUFF_SIZE do
  SCX_CPU1_SC_ATSAPPENDDATA[i] = 0
enddo

local endmnemonic = "SCX_CPU1_SC_ATSAPPENDDATA[" & SC_APPEND_BUFF_SIZE & "]"

;; Create the ATS Table Load file
s create_tbl_file_from_cvt (hostCPU,atsPktId,"Append Table Max Cmd Load","cpu1_appfull_ld",ATSAppendATblName,"SCX_CPU1_SC_ATSAPPENDDATA[1]",endmnemonic)
wait 5

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_sc_appendfull"
write ";*********************************************************************"
ENDPROC
