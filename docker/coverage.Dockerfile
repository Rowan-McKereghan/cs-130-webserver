### Build/test container ###
# Define builder stage
FROM ctrl-c-ctrl-v:base as builder

# Share work directory
COPY . /usr/src/projects/ctrl-c-ctrl-v
WORKDIR /usr/src/projects/ctrl-c-ctrl-v/build_coverage

# Build and test
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage -j$(nproc)