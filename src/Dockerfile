FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    gcc-12 g++-12 \
    build-essential \
    wget \
    libtool \
    autoconf \
    automake \
    software-properties-common \
    libnetcdf-dev libhdf5-dev zlib1g-dev libxml2-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp

# Install OpenMPI
ENV OPENMPI_VERSION=4.1.5
RUN wget https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-${OPENMPI_VERSION}.tar.gz && \
    tar -xzf openmpi-${OPENMPI_VERSION}.tar.gz && \
    cd openmpi-${OPENMPI_VERSION} && \
    ./configure && make all && make install && ldconfig

# Install FFTW with OpenMP support
ENV FFTW_VERSION=3.3.10
RUN cd /tmp && \
    wget http://www.fftw.org/fftw-${FFTW_VERSION}.tar.gz && \
    tar -xzvf fftw-${FFTW_VERSION}.tar.gz && \
    cd fftw-${FFTW_VERSION} && \
    ./configure --enable-openmp && make && make install && \
    cd /tmp && rm -rf fftw-${FFTW_VERSION}*

# Install HDF5 (required for NetCDF-C)
ENV HDF5_VERSION=1.14.3
RUN cd /tmp && \
    wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.14/hdf5-${HDF5_VERSION}/src/hdf5-${HDF5_VERSION}.tar.gz && \
    tar -xzf hdf5-${HDF5_VERSION}.tar.gz && \
    cd hdf5-${HDF5_VERSION} && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && make install && \
    cd /tmp && rm -rf hdf5*

# Install NetCDF-C
ENV NETCDF_C_VERSION=4.9.2
RUN cd /tmp && \
    wget https://github.com/Unidata/netcdf-c/archive/refs/tags/v${NETCDF_C_VERSION}.tar.gz && \
    tar xzvf v${NETCDF_C_VERSION}.tar.gz && \
    cd netcdf-c-${NETCDF_C_VERSION} && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && make install && \
    cd /tmp && rm -rf netcdf-c*

ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Install NetCDF-C++ 4.3.1
ENV NETCDF_CPP_VERSION=4.3.1
RUN cd /tmp && \
    wget https://downloads.unidata.ucar.edu/netcdf-cxx/${NETCDF_CPP_VERSION}/netcdf-cxx4-${NETCDF_CPP_VERSION}.tar.gz && \
    tar xzvf netcdf-cxx4-${NETCDF_CPP_VERSION}.tar.gz && \
    cd netcdf-cxx4-${NETCDF_CPP_VERSION} && \
    ./configure --prefix=/usr/local && \
    make -j$(nproc) && make install && \
    cd /tmp && rm -rf netcdf-cxx4*

RUN apt-get purge -y \
    wget \
    libtool \
    autoconf \
    automake \
    software-properties-common \
    libnetcdf-dev libhdf5-dev zlib1g-dev libxml2-dev \
    && apt-get clean

WORKDIR /project
