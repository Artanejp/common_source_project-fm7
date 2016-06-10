#!/bin/bash

INSTALL="/usr/bin/install -p"
SUDO=/usr/bin/sudo
LDCONFIG=/sbin/ldconfig

CSP_ARCH="x86_64-linux-gnu"
MULTIARCH="Yes"
CSP_PREFIX=/usr/local
CSP_GUILIB="libCSPgui.so.1.3.3 libCSPosd.so.1.2.2 libCSPemu_utils.so.1.0.1 libCSPavio.1.2.1"

for i in "$@"; do
  case "$1" in 
     -h | --help )
        echo "Common Source Code Project for Qt/*nix"
        echo "Architecture:" ${CSP_ARCH}
        echo "Installer usage:"
        echo "./installer_unix.sh [options]"
        echo "  options:"
        echo "-p path : Set install prefix ("${CSP_PREFIX}")"
        echo "-a arch : Set architecture   ("${CSP_ARCH}")"
        echo "-m [Yes|No] : Set multiarch  ("${MULTIARCH}")"
        exit 0
        ;;
     -m | --multiarch )
        MULTIARCH=$2
	shift
	shift
	;;
     -p | --prefix )
        CSP_PREFIX=$2
	shift
	shift
	;;
     -a | --arch )
        CSP_ARCH=$2
	shift
	shift
	;;
   esac
done

cd ${CSP_ARCH}
for i in emu* ; do
    echo "Install :" ${i} to ${CSP_PREFIX}"/bin"
    ${SUDO} ${INSTALL} ./${i} ${CSP_PREFIX}/bin
done

if [ $MULTIARCH = "Yes" ] ; then
       LIB_PATH=${CSP_PREFIX}/lib/${CSP_ARCH}
else
       LIB_PATH=${CSP_PREFIX}/lib
fi

for j in ${CSP_GUILIB} ; do
   if [ -e lib/${j} ] ; then
     echo "Install :" ${j} to ${LIB_PATH}
      ${SUDO} ${INSTALL} ./lib/${j} ${LIB_PATH}
   fi
done
${SUDO} ${LDCONFIG}

cd ..
exit 0