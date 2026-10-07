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

A demonstration bundle of the Core Flight System including the cFE, OSAL, and PSP can be obtained at https://github.com/nasa/cfs

For information about a mission ready cFS bundle, see: https://github.com/nasa/cFS#cfs-gov-mission-ready-version

## Known issues

See all [open issues](https://github.com/nasa/SC/issues) and closed to milestones later than this version.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>