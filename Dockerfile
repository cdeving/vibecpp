FROM ubuntu:latest AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libcurl4-openssl-dev \
    libssl-dev \
    libpthread-stubs0-dev \
    git \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make release

FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    libcurl4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/bin/release/program ./vibecpp

RUN chmod +x ./vibecpp

CMD ["./vibecpp"]