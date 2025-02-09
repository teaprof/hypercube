FROM ubuntu:latest AS build
#ENV TZ=Europe/Minsk
ENV DEBIAN_FRONTEND=noninteractive 

#BASIC STEPS
RUN apt update -y
RUN apt install mc -y

#INSTALL CMAKE
WORKDIR /opt/
RUN apt install wget -y
RUN apt install build-essential clang-16 libboost-program-options1.83-dev libboost-json1.83-dev libgtest-dev -y
RUN apt install libbenchmark-dev -y
RUN wget https://github.com/Kitware/CMake/releases/download/v3.31.2/cmake-3.31.2-linux-x86_64.tar.gz && tar -xf cmake-3.31.2-linux-x86_64.tar.gz
ENV PATH="/opt/cmake-3.31.2-linux-x86_64/bin:$PATH"
ENV CMAKE_CXX_COMPILER="clang-16"

#BUILD APP
ADD . /app/src/
WORKDIR /app/build
RUN cmake -DCMAKE_CXX_COMPILER=clang++-16 -DCMAKE_BUILD_TYPE=Release ../src  && \
    cmake --build . -j
    
FROM ubuntu:latest
COPY --from=build /app /app
RUN apt update -y
RUN apt install libboost-program-options1.83.0 libboost-json1.83.0 libboost-container1.83.0 libbenchmark1.8.3 -y
ENV PATH="/app/build/bin:$PATH"

