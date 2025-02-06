FROM ubuntu:latest
#ENV TZ=Europe/Minsk
ENV DEBIAN_FRONTEND=noninteractive 
RUN apt update -y
RUN apt install clang-16 libboost-program-options1.83-dev libboost-json1.83-dev -y
RUN apt install snapd -y
RUN systemctl unmask snapd.service
RUN snap install cmake --classic
RUN apt install mc -y
ADD src include scripts CMakeLists.txt /app/src/
WORKDIR /app/build
#RUN cmake --preset=release ../src  && \
#    cmake --build .    
