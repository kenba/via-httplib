# Copyright (c) 2018 Ken Barker

FROM alpine:3.8 AS build

LABEL maintainer="ken.barker@via-technology.aero"

# Load build packages
RUN apk --update add --no-cache \
  build-base \
  cmake \
  boost boost-dev \
  openssl openssl-dev

# The cmake boost variables
ENV BOOST_INCLUDEDIR /usr/include
ENV BOOST_LIBRARYDIR /usr/lib

WORKDIR /opt

COPY . via-httplib

# Build the via-httplib library
RUN cd via-httplib \
 && mkdir build \
 && cd build \
 && cmake \
    -DBOOST_INCLUDEDIR=${BOOST_INCLUDEDIR} \
    -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR} \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    .. \
 && make \
 && make install \
 && rm -fr *
 
# Build a via-httplib server 
RUN cd /opt/via-httplib/build \
 && cmake \
    -DBOOST_INCLUDEDIR=${BOOST_INCLUDEDIR} \
    -DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR} \
    -DCMAKE_BUILD_TYPE=Release \
    ../examples/server \
 && make

# Create a clean alpine image and copy the server to it
FROM alpine:3.8

RUN apk --update add --no-cache \
  openssh openssl-dev \
  boost
  
COPY --from=build /opt/via-httplib/build/HttpServer /bin/HttpServer

# Start the server on entry
ENTRYPOINT ["/bin/HttpServer"]
