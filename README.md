# core Flight System (cFS) Stored Command Application (SC) 

## Introduction

The Stored Command application (SC) is a core Flight System (cFS) application 
that is a plug in to the Core Flight Executive (cFE) component of the cFS.

The SC application allows a system to be autonomously commanded
using sequences of commands that are loaded into SC. Each command has a time tag
or wakeup count associated with it, permitting the command to be released for
distribution at predetermined times or wakeup counts. SC supports both
Absolute Time tagged command Sequences (ATSs) and multiple Relative Time tagged
command Sequences (RTSs). The purpose of ATS commands is to be able to specify
commands to be executed at a specific time. The purpose of Relative Time
Sequence commands is to be able to specify commands to be executed at a
relative wakeup count.

The SC application is written in C and depends on the cFS Operating System 
Abstraction Layer (OSAL) and cFE components. There is additional SC application 
specific configuration information contained in the application user's guide.

User's guide information can be generated using Doxygen (from top mission directory):
```
  make prep
  make -C build/docs/sc-usersguide sc-usersguide
```

## Software Required

cFS Framework (cFE, OSAL, PSP)

An integrated bundle including the cFE, OSAL, and PSP can
be obtained at https://github.com/nasa/cfs

## About cFS

The cFS is a platform and project independent reusable software framework and
set of reusable applications developed by NASA Goddard Space Flight Center.
This framework is used as the basis for the flight software for satellite data
systems and instruments, but can be used on other embedded systems.  More
information on the cFS can be found at http://cfs.gsfc.nasa.gov
