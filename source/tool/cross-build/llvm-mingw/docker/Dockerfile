FROM debian:bullseye
#FROM ubuntu:16.04

ARG FORCE_THREADS=4
ARG TOOLCHAIN_PREFIX=/opt/llvm-mingw-14
ARG LLVM_VER=release/14.x
ARG DEBIAN_FRONTEND=noninteractive

ENV TOOLCHAIN_TARGET_OSES="mingw32 mings32uwp"

ENV ROOT_WORK_DIR=/build

RUN apt-get update -qq 

RUN apt-get install -qqy --no-install-recommends \
    git wget bzip2 file unzip libtool pkg-config cmake build-essential \
    automake yasm gettext autopoint vim \
	python \
	python3 \
	python3-distutils \
	ninja-build subversion \
    cmake cmake-curses-gui cmake-extras \
	mingw-w64-tools binutils-mingw-w64 \
    pkg-config sudo openssh-client openssh-server p7zip-full pixz \
    lftp ncftp nano jed locales mc less lv \
    wget yasm nasm \
    libz-mingw-w64 libz-mingw-w64-dev \
    ccache aptitude vim emacs \
    ca-certificates && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*

RUN git config --global user.name "LLVM MinGW" && \
    git config --global user.email root@localhost

WORKDIR $ROOT_WORK_DIR
WORKDIR llvm-build
ENV BUILD_ROOT_DIR=${ROOT_WORK_DIR}/llvm-build
ENV SCRIPT_ROOT_DIR=${ROOT_WORK_DIR}/scripts

COPY bootstrap.sh ./

ARG TOOLCHAIN_ARCHS="i686 x86_64 armv7 aarch64"
# Build everything that uses the llvm monorepo. We need to build the mingw runtime before the compiler-rt/libunwind/libcxxabi/libcxx runtimes.

COPY build-llvm.sh strip-llvm.sh ${SCRIPT_ROOT_DIR}/
COPY build-llvm.sh strip-llvm.sh ${BUILD_ROOT_DIR}/build/

COPY wrappers/*.sh wrappers/*.c wrappers/*.h ${SCRIPT_ROOT_DIR}/wrappers/
COPY wrappers/*.sh wrappers/*.c wrappers/*.h ${BUILD_ROOT_DIR}/build/wrappers/

COPY test/* ${SCRIPT_ROOT_DIR}/test/
COPY test/* ${BUILD_ROOT_DIR}/build/test/

RUN ${SCRIPT_ROOT_DIR}/build-llvm.sh --llvm-version $LLVM_VER \
                            --build-threads $FORCE_THREADS \
                            $TOOLCHAIN_PREFIX
		    
#RUN ../scripts/strip-llvm.sh $TOOLCHAIN_PREFIX
WORKDIR ${BUILD_ROOT_DIR}/build
COPY install-wrappers.sh ${SCRIPT_ROOT_DIR}/
COPY install-wrappers.sh ${BUILD_ROOT_DIR}/build/

RUN ${SCRIPT_ROOT_DIR}/install-wrappers.sh \
                            $TOOLCHAIN_PREFIX

ENV PATH=${TOOLCHAIN_PREFIX}/bin:$PATH
		     
WORKDIR ${BUILD_ROOT_DIR}/build
COPY build-mingw-w64.sh ${SCRIPT_ROOT_DIR}/
COPY build-mingw-w64.sh ${BUILD_ROOT_DIR}/build/

COPY build-mingw-w64-libraries.sh ${SCRIPT_ROOT_DIR}/
COPY build-mingw-w64-libraries.sh ${BUILD_ROOT_DIR}/build/

RUN ${SCRIPT_ROOT_DIR}/build-mingw-w64.sh \
                     --build-threads $FORCE_THREADS \
                     $TOOLCHAIN_PREFIX
		     

WORKDIR ${BUILD_ROOT_DIR}
COPY build-compiler-rt.sh ${SCRIPT_ROOT_DIR}/
COPY build-compiler-rt.sh ${BUILD_ROOT_DIR}/build/

RUN ${SCRIPT_ROOT_DIR}/build-compiler-rt.sh \
                     --build-threads $FORCE_THREADS \
                     $TOOLCHAIN_PREFIX

WORKDIR ${BUILD_ROOT_DIR}/build

RUN ${SCRIPT_ROOT_DIR}/build-mingw-w64-libraries.sh \
                     --build-threads $FORCE_THREADS \
                     $TOOLCHAIN_PREFIX
					 
WORKDIR ${BUILD_ROOT_DIR}
COPY build-libcxx.sh ${SCRIPT_ROOT_DIR}/
COPY build-libcxx.sh ${BUILD_ROOT_DIR}/build/

RUN ${SCRIPT_ROOT_DIR}/build-libcxx.sh \
                     --build-threads $FORCE_THREADS \
                     $TOOLCHAIN_PREFIX

WORKDIR ${BUILD_ROOT_DIR}
COPY build-compiler-rt.sh ${SCRIPT_ROOT_DIR}/
COPY build-compiler-rt.sh ${BUILD_ROOT_DIR}/build/

RUN ${SCRIPT_ROOT_DIR}/build-compiler-rt.sh \
                    --build-threads $FORCE_THREADS \
                    $TOOLCHAIN_PREFIX


WORKDIR ${BUILD_ROOT_DIR}/build
COPY build-libssp.sh ${BUILD_ROOT_DIR}/build/
COPY build-libssp.sh ${SCRIPT_ROOT_DIR}

RUN ${SCRIPT_ROOT_DIR}/build-libssp.sh \
                     --build-threads $FORCE_THREADS \
                     $TOOLCHAIN_PREFIX
		     
#RUN  rm -rf ${ROOT_WORK_DIR}/

