FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TERM=xterm-256color

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    make \
    gdb \
    valgrind \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /taskforge

COPY . .

RUN make clean && make

CMD ["./taskforge"]