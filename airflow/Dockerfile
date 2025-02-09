#FROM ubuntu:latest AS build
#ARG BaseImage=ubuntu:latest
ARG BaseImage=apache/airflow:2.10.4
FROM $BaseImage AS build
ENV TZ=Europe/Minsk
ENV DEBIAN_FRONTEND=noninteractive 

#BASIC STEPS
USER root
RUN apt update -y
RUN apt install mc -y

#INSTALL CMAKE
WORKDIR /opt/
RUN apt install wget -y
RUN apt install build-essential clang-16 libboost-program-options1.81-dev libboost-json1.81-dev libgtest-dev -y
RUN apt install libbenchmark-dev -y
RUN wget https://github.com/Kitware/CMake/releases/download/v3.31.2/cmake-3.31.2-linux-x86_64.tar.gz && tar -xf cmake-3.31.2-linux-x86_64.tar.gz
ENV PATH="/opt/cmake-3.31.2-linux-x86_64/bin:$PATH"
ENV CMAKE_CXX_COMPILER="clang-16"

#BUILD APP
ADD . /app/src/
WORKDIR /app/build
RUN cmake -DCMAKE_CXX_COMPILER=clang++-16 -DCMAKE_BUILD_TYPE=Release ../src  && \
    cmake --build . -j
 
#FROM ubuntu:latest
FROM $BaseImage
USER root
RUN apt update -y
RUN apt install libboost-program-options1.81.0 libboost-json1.81.0 libboost-container1.81.0
USER airflow
COPY --from=build /app /app
ENV PATH="/app/build/bin:$PATH"
USER airflow

