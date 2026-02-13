FROM ubuntu:24.04

ARG RB_VERSION=''

ENV RUN DEBIAN_FRONTEND=noninteractive
RUN ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
RUN apt-get update && apt-get install -y libsdl2-dev libsdl2-mixer-dev libfreeimage-dev libfreetype6-dev libcurl4-openssl-dev libasound2-dev libgl1-mesa-dev build-essential cmake gettext libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libswresample-dev libswscale-dev libpostproc-dev libzip-dev liblzma-dev libssl-dev libssl3 rapidjson-dev libproc2-dev

WORKDIR /src

ADD . /src

# This test cause unexpected & crash only in docker run. Remove when fixed.
RUN mv ./tests/tests/test_messaging.cpp ./tests/tests/test_messaging.ignore

RUN cmake -DPISTACHE_BUILD_TESTS=false -DPISTACHE_BUILD_EXAMPLES=false .

RUN (test -x "RB_VERSION" && echo ok || true ) && make -j$(nproc)

RUN ./emulationstation_test --gtest_output="xml:report.xml"

