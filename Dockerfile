FROM ubuntu:22.04

LABEL Description="Build environment for Login Manager"
LABEL Version="0.1"
LABEL Maintainer="Tom Karlsson <tom.kg.karlsson(at)gmail.com>"

# Install necessary packages (excluding libsqlite3-dev)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install yaml-cpp
RUN wget https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.7.0.tar.gz && \
    tar -xzvf yaml-cpp-0.7.0.tar.gz && \
    cd yaml-cpp-yaml-cpp-0.7.0 && \
    cmake . && \
    make && \
    make install && \
    cd .. && \
    rm -rf yaml-cpp-0.7.0.tar.gz yaml-cpp-yaml-cpp-0.7.0

# Set the working directory
WORKDIR /app

# Copy the current directory contents into the container
COPY . /app

# Create build directory and run cmake
RUN mkdir build
WORKDIR /app/build
RUN cmake ..

# Build the project
RUN make

# Run tests
RUN ctest

# Specify the command to run on container start
# CMD ["./login_manager"]

