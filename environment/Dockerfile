FROM ubuntu:24.04 as base 
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update 
# Install dependencies
RUN apt-get install -y --no-install-recommends git python3 \
    python3.12-venv python3.12-dev openmpi-bin \
    openmpi-common libopenmpi-dev \
    gcc g++ gfortran cmake build-essential glibc-source \
    pkgconf autoconf automake libtool libtool-bin make \
    openssh-client openssh-server nmap iproute2 \
    gdb clangd clang-format vim
RUN mkdir -p -m 0600 ~/.ssh && \
    ssh-keyscan -H github.com >> ~/.ssh/known_hosts

# Setup SSH
EXPOSE 22
ENV USERNAME=root
RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN passwd -d ${USERNAME}
RUN sed -i'' -e's/^#PermitRootLogin prohibit-password$/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && sed -i'' -e's/^#PasswordAuthentication yes$/PasswordAuthentication yes/' /etc/ssh/sshd_config \
    && sed -i'' -e's/^#PermitEmptyPasswords no$/PermitEmptyPasswords yes/' /etc/ssh/sshd_config \
    && sed -i'' -e's/^UsePAM yes/UsePAM no/' /etc/ssh/sshd_config
EXPOSE 22

# Master
FROM base AS master
# Setup spack

ENV SHARED=/mount/shared
RUN mkdir -p $SHARED
RUN chown ${USERNAME} ${SHARED}
RUN chmod +rw ${SHARED} -R
USER ${USERNAME}
ENV SOFTWARE=${SHARED}/software
ENV SPACK_DIR=${SOFTWARE}/spack
ENV PROJECT_DIR ${SHARED}/code
RUN mkdir -p ${SOFTWARE}
RUN mkdir -p ${SPACK_DIR}
RUN git clone https://github.com/spack/spack.git ${SPACK_DIR}
RUN sh ${SPACK_DIR}/share/spack/setup-env.sh
ENV PATH="${PATH}:${SPACK_DIR}/bin"
# RUN spack install --sh libuv@1.46.0 ucx@1.15.0
ENTRYPOINT service ssh restart && bash

# worker
FROM base AS worker
ENTRYPOINT service ssh restart && bash