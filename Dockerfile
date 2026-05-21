FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    gdb \
    iproute2 \
    net-tools \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

# Create a build directory
RUN mkdir -p build

# Add a script or entrypoint if needed
CMD ["/bin/bash"]
