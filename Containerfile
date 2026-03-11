# Containerfile for kdevcxx_with_ai build environment
# Based on debian:13 with all required development packages installed

#
# Run with:
# $ podman build -t kdevcxx_with_ai-build-env .
# $ podman run -it kdevcxx_with_ai-build-env
#

FROM debian:13

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Uncomment and set the hostname if you have a local apt-cache-ng server.
# RUN echo 'Acquire::http::Proxy "http://apt-cache.lan:3142/";' > /etc/apt/apt.conf.d/99proxy

# Install all required development packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    appstream \
    ninja-build \
    clang \
    clang-format \
    llvm \
    lld \
    cmake \
    extra-cmake-modules \
    curl ca-certificates \
    git \
    qtwebengine5-dev \
    libkf5texteditor-dev \
    libkf5parts-dev \
    libboost-ext-ut-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libssl-dev \
    kdevelop-dev \
    libcups2-dev \
    libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory to the project root
WORKDIR /app

# Copy the source code into the container
# Using .dockerignore to exclude build directories and other unnecessary files
COPY . /app

# Default command (can be overridden when running the container)
CMD ["/bin/sh", "-c", "cmake --workflow --preset='clang-release'"]
