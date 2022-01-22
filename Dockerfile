FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get install -y git build-essential make cmake bison flex libboost-all-dev

ADD . /app

WORKDIR /app

RUN rm -rf build
RUN mkdir build && cmake -B build -S sources

WORKDIR /app/build

RUN make

