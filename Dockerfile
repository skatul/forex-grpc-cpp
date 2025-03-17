FROM ubuntu:20.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libgrpc++-dev \
    libcurl4-openssl-dev \
    protobuf-compiler \
    curl

# Copy source code
COPY . /app
WORKDIR /app

# Build the application
RUN mkdir build && cd build && cmake .. && make

# Expose port
EXPOSE 50051

# Run the application
CMD ["/app/build/forex_service"]