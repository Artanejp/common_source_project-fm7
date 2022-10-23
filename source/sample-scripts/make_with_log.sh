#!/bin/bash

CUR_DIR="$PWD"
START_TIME="`date --rfc-3339=seconds`"

LOG_NAME="000_make_log.${START_TIME}.log"

_TIME=time 
_MAKE=make
_TEE=tee
_NICE=nice
NICEVAL=19

printf "Begin ${START_TIME} \n" | ${_TEE} "${LOG_NAME}"
printf "\n" | ${_TEE} -a "${LOG_NAME}"

printf "MAKE ARGS=${@} \n\n" | ${_TEE} -a "${LOG_NAME}"
printf "\n" | ${_TEE} -a "${LOG_NAME}"

${_TIME} ${_NICE} -n ${NICEVAL} ${_MAKE} $@ 2>&1 | ${_TEE} -a "${LOG_NAME}"
printf "\n" | ${_TEE} -a "${LOG_NAME}"
printf "End `date --rfc-3339=seconds` .\n" | ${_TEE} -a "${LOG_NAME}"
printf "\n" | ${_TEE} -a "${LOG_NAME}"
