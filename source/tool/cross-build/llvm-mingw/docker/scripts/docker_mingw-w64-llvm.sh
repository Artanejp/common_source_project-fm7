#!/bin/sh

#CIDFILE=`mktemp`
CONTAINER_NAME=`uuidgen`
CONTAINER_TAG='artanejp/mingw-w64-llvm10-ubuntu20.04:using'
CONTAINER_CMD='/bin/bash -i'
#CONTAINER_HOME="/Please_Modify_This/home/"
BEGIN_DATE=`date`

echo BEGIN ${CONTAINER_NAME} for ${CONTAINER_TAG} at ${BEGIN_DATE}
docker run --name=${CONTAINER_NAME} \
           -v ${CONTAINER_HOME}:/home/ \
	   -v /tmp/.X11-unix/:/tmp/.X11-unix/ \
	   -v /sys/fs/cgroup:/sys/fs/cgroup:ro \
	   --device /dev/snd \
	   -e DISPLAY=unix$DISPLAY \
	   -ti \
	   ${CONTAINER_TAG} \
	   ${CONTAINER_CMD} \
	   $@
	   
END_DATE=`date`
echo COMMIT ${CONTAINER_NAME} for ${CONTAINER_TAG} at ${END_DATE}
echo "Please input committing message:"
read COMMIT_MESSAGE
docker commit -m "${COMMIT_MESSAGE}" ${CONTAINER_NAME} ${CONTAINER_TAG}
echo END ${CONTAINER_NAME} for ${CONTAINER_TAG}
