FROM ubuntu:22.04
LABEL maintainer="Recalbox Team"

ENV TERM xterm
ENV ARCH ''
ENV RECALBOX_VERSION 'development'
ENV RECALBOX_CCACHE_ENABLED ''
ENV PACKAGE ''
ENV MAKE_JOBS=''

RUN apt-get update -y && \
    apt-get install -y tzdata && \
    ln -fs /usr/share/zoneinfo/Europe/Paris /etc/localtime && \
    dpkg-reconfigure --frontend noninteractive tzdata && \
    apt-get -y install \
        build-essential \
        git \
        libncurses5-dev \
        qtbase5-dev \
        qtchooser \
        qt5-qmake \
        qtbase5-dev-tools \
        mercurial \
        libdbus-glib-1-dev \
        texinfo \
        zip \
        openssh-client \
        libxml2-utils \
        libpng-dev \
        software-properties-common \
        wget \
        cpio \
        bc \
        locales \
        rsync \
        imagemagick \
        bison \
        flex \
        bsdmainutils \
        nano \
        vim \
        automake \
        autopoint \
        mtools \
        dosfstools \
        subversion \
        openjdk-8-jdk \
        libssl-dev \
        libelf-dev \
        graphviz \
        python3 \
        python3-matplotlib \
        python3-numpy \
        python3-six \
        re2c \
        libc6-dev-i386 \
        libtool \
        cabextract \
        dos2unix \
        p7zip-full \
        xsltproc \
        patch \
        ccache \
        && rm -rf /var/lib/apt/lists/*

RUN echo "en_US.UTF-8 UTF-8" > /etc/locale.gen && \
    locale-gen

RUN mkdir -p /work /share/dl /share/ccache
WORKDIR /work

CMD echo ">>> Recalbox Build" && \
    echo ">>> Version: ${RECALBOX_VERSION}" && \
    echo ">>> Architecture: ${ARCH}" && \
    echo ">>> Make Jobs: ${MAKE_JOBS:-auto}" && \
    echo "${RECALBOX_VERSION}" > board/recalbox/fsoverlay/recalbox/recalbox.version && \
    git config --global --add safe.directory /work && \
    git config --global --add safe.directory /work/buildroot && \
    ( cd buildroot && git reset --hard HEAD && git clean -dfx ) && \
    make recalbox-${ARCH}_defconfig && \
    JOBS_FLAG=$( [ -n "${MAKE_JOBS}" ] && echo "-j${MAKE_JOBS}" || echo "" ) && \
    BR2_CCACHE_FLAG=$( [ "${RECALBOX_CCACHE_ENABLED}" = "1" ] && echo "BR2_CCACHE=y BR2_CCACHE_DIR=/share/ccache BR2_CCACHE_INITIAL_SETUP=--max-size=500G BR2_CCACHE_USE_BASEDIR=y" || echo "" ) && \
    make BR2_DL_DIR="/share/dl" ${BR2_CCACHE_FLAG} ${JOBS_FLAG}
