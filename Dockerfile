FROM gitpod/workspace-full:2022-06-20-19-54-55

# Setup directories
ENV PROJECT_SOURCE_DIR /workspace/project
ENV PROJECT_BUILD_DIR /workspace/build
ENV PROJECT_DEPENDENCY_DIR /workspace/dependency
ENV SOFTWARE_INSTALL_DIR /workspace/software/install
ENV SOFTWARE_SRC_DIR /workspace/software/src

RUN mkdir -p ${PROJECT_SOURCE_DIR} ${PROJECT_BUILD_DIR} ${PROJECT_DEPENDENCY_DIR} ${SOFTWARE_INSTALL_DIR} ${SOFTWARE_SRC_DIR}

# Install dependencies
RUN sudo apt-get update -q --fix-missing && \
    sudo apt-get install -yq gcc-8 g++-8

RUN DEBIAN_FRONTEND="noninteractive" sudo apt-get install -y --no-install-recommends \
    autoconf \
    automake \
    ca-certificates \
    curl \
    environment-modules \
    git \
    build-essential \
    python \
    python-dev \
    python3-dev \
    vim \
    sudo \
    unzip \
    cmake \
    lcov \
    zlib1g-dev \
    libsdl2-dev \
    gfortran

# Install spack
ENV SPACK_DIR=$SOFTWARE_SRC_DIR/spack
ENV MOCHI_DIR=$SOFTWARE_SRC_DIR/mochi

RUN git clone https://github.com/spack/spack ${SPACK_DIR}
RUN cd ${SPACK_DIR} && git checkout tags/v0.18.0 -b release-v0.18.0
RUN git clone https://github.com/mochi-hpc/mochi-spack-packages.git ${MOCHI_DIR}

# Setup spack
ENV spack=${SPACK_DIR}/bin/spack
RUN . ${SPACK_DIR}/share/spack/setup-env.sh

RUN $spack repo add ${MOCHI_DIR}

RUN $spack compiler find
RUN $spack compiler list
RUN $spack compiler rm clang@15.0.0
RUN $spack compiler rm gcc@9.4.0

RUN echo "export PATH=${SPACK_DIR}/bin:$PATH" >> /home/$USER/.bashrc
RUN echo ". ${SPACK_DIR}/share/spack/setup-env.sh" >> /home/$USER/.bashrc

# Clone Project
RUN git clone --recurse-submodules https://github.com/hariharan-devarajan/tailorfs.git ${PROJECT_SOURCE_DIR}

# install dependencies
RUN cp ${PROJECT_SOURCE_DIR}/dependency/spack.yaml ${PROJECT_DEPENDENCY_DIR}
RUN cd ${PROJECT_DEPENDENCY_DIR} && spack env create -d . spack.yaml && spack env activate -p . && spack install

ENV PATH=${SPACK_DIR}/bin:${SOFTWARE_INSTALL_DIR}/bin:${PROJECT_BUILD_DIR}/bin:${PROJECT_DEPENDENCY_DIR}/.spack-env/view/bin:$PATH

# These commands copy your files into the specified directory in the image
# and set that as the working location
WORKDIR ${PROJECT_BUILD_DIR}

LABEL Name=tailorfs Version=0.0.1
