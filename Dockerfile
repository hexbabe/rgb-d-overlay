FROM debian:bookworm

ENV HOME /root
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update

# definitely can lean this down; i don't know what i can remove and what i can't though
RUN apt-get update && \
    apt-get -y --no-install-recommends install \
    build-essential \
    bazel-bootstrap \
    ca-certificates \
    curl \
    doxygen \
    g++ \
    gdb \
    git \
    gnupg \
    gpg \
    less \
    libboost-all-dev \
    libc-ares-dev \
    libre2-dev \
    libssl-dev \
    ninja-build \
    pkg-config \
    software-properties-common \
    sudo \
    wget \
    zlib1g-dev \
    libprotobuf-dev \
    python3-pip \
    libgtk-3-bin \
    squashfs-tools \
    libglib2.0-bin \
    fakeroot \
    cmake \
    make \
    libgrpc-dev \
    libgrpc++-dev \
    protobuf-compiler-grpc \
    libjpeg62 \
    libjpeg-dev \
    && rm -rf /var/lib/apt/lists/*

# Download and install ImageMagick from source
RUN curl -L https://github.com/ImageMagick/ImageMagick/archive/refs/tags/7.1.0-4.tar.gz | tar xz && \
    cd ImageMagick-7.1.0-4 && \
    ./configure --with-jpeg=yes && \
    make -j$(nproc) && \
    make install && \
    ldconfig /usr/local/lib

# Add the public key for the llvm repository
RUN bash -c 'wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|apt-key add -'
RUN apt-add-repository -y 'deb http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-15 main'
RUN apt-add-repository -y 'deb http://deb.debian.org/debian bullseye-backports main'

RUN apt-get update

RUN apt-get -y --no-install-recommends install -t bullseye-backports \
    cmake

# Install viam-cpp-sdk
RUN mkdir -p ${HOME}/opt/src
ENV PINNED_COMMIT_HASH="802b70b234741d17345be77b4e52f39e4c40a252"
RUN cd ${HOME}/opt/src && \
    git clone https://github.com/viamrobotics/viam-cpp-sdk && \
    cd viam-cpp-sdk && \
    git checkout ${PINNED_COMMIT_HASH} && \
    mkdir build && \
    cd build && \
    cmake .. -G Ninja && \
    ninja all -j 3 && \
    ninja install -j 3

RUN cp -r ${HOME}/opt/src/viam-cpp-sdk/build/install/* /usr/local/

# Get appimage builder
# Credit to perrito666 for the ubuntu fix
RUN python3 -m pip install --break-system-packages git+https://github.com/hexbabe/appimage-builder.git

WORKDIR /app

COPY . /app

COPY packaging/. /app

CMD ["appimage-builder", "--recipe", "packaging/AppImageBuilder.yml"]
