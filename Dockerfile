FROM ubuntu:bionic
RUN apt update && apt install -y autoconf automake build-essential git libev-dev libpthread-stubs0-dev pkg-config
WORKDIR /build
ENV URL http://localhost
COPY . .
RUN ./scripts/bootstrap
RUN ./configure
RUN make all
ENTRYPOINT ./src/spoa-opentracing -r0 -u ${URL}
