# Copyright (c) 2018-2019 Ken Barker

FROM alpine:3.10 AS build

LABEL maintainer="ken.barker@via-technology.aero"

# Load build packages
RUN apk --update add --no-cache \
  build-base \
  cmake \
  boost boost-dev \
  openssl openssl-dev

WORKDIR /opt

COPY . via-httplib

WORKDIR /opt/via-httplib

# Build the via-httplib library
RUN mkdir build \
 && cd build \
 && cmake \
    -DVIA_HTTPLIB_UNIT_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    .. \
 && make \
 && make install \
 && rm -fr *

WORKDIR /opt/via-httplib/build

# Build an via-httplib server 
RUN cmake \
    -DCMAKE_BUILD_TYPE=Release \
    ../examples/server \
 && make

# Create a clean alpine image and copy the server to it
FROM alpine:3.10

RUN apk --update add --no-cache \
  openssl \
  boost-system \
  boost-date_time \
  boost-regex
  
COPY --from=build /opt/via-httplib/build/HttpServer /bin/HttpServer

# Start the server on entry
ENTRYPOINT ["/bin/HttpServer"]
