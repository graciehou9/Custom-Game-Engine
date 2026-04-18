FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-ttf-dev \
    liblua5.4-dev

WORKDIR /app
COPY . .

RUN mkdir build && cd build && cmake .. && cmake --build .