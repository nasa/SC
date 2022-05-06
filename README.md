core Flight System (cFS) Stored Command Application (SC) 
========================================================

Open Source Release Readme
==========================

SC Release Candidate 3.1.0

Date: 
September 9, 2021

Introduction
---------------

  The Stored Command application (SC) is a core Flight System (cFS) application 
  that is a plug in to the Core Flight Executive (cFE) component of the cFS.  
  
  The SC application allows a system to be autonomously commanded 24 hours a day 
  using sequences of commands that are loaded to SC. Each command has a time tag 
  associated with it, permitting the command to be released for distribution at 
  predetermined times. SC supports both Absolute Time tagged command Sequences 
  (ATSs) as well as multiple Relative Time tagged command Sequences (RTSs). The 
  purpose of ATS commands is to be able to specify commands to be executed at a 
  specific time.  The purpose of Relative Time Sequence commands is to be able 
  to specify commands to be executed at a specific time after (relative to) 
  an ATS.     

  The SC application is written in C and depends on the cFS Operating System 
  Abstraction Layer (OSAL) and cFE components. There is additional SC application 
  specific configuration information contained in the application user's guide.
    
  Developer's guide information can be generated using Doxygen:
  doxygen sc_doxygen_config.txt

  This software is licensed under the NASA Open Source Agreement. 
  http://ti.arc.nasa.gov/opensource/nosa
 

Software Included
--------------------

  Stored Command application (SC) 3.1.0
  
 
Software Required:
-------------------

 cFS Caelum

 Note: An integrated bundle including the cFE, OSAL, and PSP can 
 be obtained at https://github.com/nasa/cfs
 

About cFS
------------

  The cFS is a platform and project independent reusable software framework and
  set of reusable applications developed by NASA Goddard Space Flight Center.  
  This framework is used as the basis for the flight software for satellite data 
  systems and instruments, but can be used on other embedded systems.  More 
  information on the cFS can be found at http://cfs.gsfc.nasa.gov
 

EOF                       
