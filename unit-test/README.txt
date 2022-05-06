##############################################################################
## File: README.txt
##
## Purpose: CFS SC application unit test instructions, results, and coverage
##############################################################################

-------------------------
SC Unit Test Instructions
-------------------------
This unit test was run in a virtual machine running Ubuntu 18.04 and uses the
ut-assert stubs and default hooks for the cFE, OSAL and PSP.

To run the unit test enter the following commands at the command line prompt in
the top-level cFS directory (after ensuring that SC is listed as a target). Note
that in order to successfully compile the unit tests the "-Werror" compilation
flag must be disabled.

make distclean
make SIMULATION=native ENABLE_UNIT_TESTS=true prep
make
make test
make lcov

SC 3.1.0 Unit Test Results:

Tests Executed:    311
Assert Pass Count: 1844
Assert Fail Count: 0

==========================================================================
sc_app.c - Line Coverage:     100.0%
           Function Coverage: 100.0%
           Branch Coverage:    99.0%

==========================================================================
sc_atsrq.c - Line Coverage:     100.0%
             Function Coverage: 100.0%
             Branch Coverage:   100.0%

==========================================================================
sc_cmds.c - Line Coverage:     100.0%
            Function Coverage: 100.0%
            Branch Coverage:    98.0%

==========================================================================
sc_loads.c - Line Coverage:     100.0%
             Function Coverage: 100.0%
             Branch Coverage:   100.0%

==========================================================================
sc_rtsrq.c - Line Coverage:      98.9%
             Function Coverage: 100.0%
             Branch Coverage:   100.0%

==========================================================================
sc_state.c - Line Coverage:     100.0%
             Function Coverage: 100.0%
             Branch Coverage:    97.6%

==========================================================================
sc_utils.c - Line Coverage:     100.0%
             Function Coverage: 100.0%
             Branch Coverage:   100.0%

==========================================================================
