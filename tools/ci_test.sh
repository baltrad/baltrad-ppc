#!/bin/sh
############################################################
# Description: Script that should be executed from a continuous
# integration runner. It is necessary to point out the proper
# paths to bRopo since this test sequence should be run whenever
# bRopo is changed.
#
# Author(s):   Anders Henja
#
# Copyright:   Swedish Meteorological and Hydrological Institute, 2011
#
# History:  2011-08-20 Created by Anders Henja
############################################################
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

"$SCRIPTPATH/run_python_script.sh" "${SCRIPTPATH}/../test/pytest/PpcXmlTestSuite.py" "${SCRIPTPATH}/../test/pytest"
exit $?
