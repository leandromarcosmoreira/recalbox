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
        curl \
        && rm -rf /var/lib/apt/lists/*

RUN curl -sL https://github.com/BLAKE3-team/BLAKE3/releases/download/1.5.4/b3sum-bin-1.5.4-x86_64-unknown-linux-gnu.zip -o /tmp/b3.zip && \
    unzip -o /tmp/b3.zip -d /usr/local/bin && \
    chmod +x /usr/local/bin/b3sum && \
    rm /tmp/b3.zip

RUN echo "en_US.UTF-8 UTF-8" > /etc/locale.gen && \
    locale-gen

RUN mkdir -p /work /share/dl /share/ccache
WORKDIR /work

ENTRYPOINT ["/bin/bash", "-c"]

CMD ["echo 'Build container ready'"]
