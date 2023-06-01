### Build/test container ###
# Define builder stage
FROM ctrl-c-ctrl-v:base as builder

# Share work directory
COPY . /usr/src/projects/ctrl-c-ctrl-v
WORKDIR /usr/src/projects/ctrl-c-ctrl-v/build

# Build and test
RUN cmake ..
RUN make -j$(nproc)
RUN ctest --output-on_failure --verbose

### Deploy container ###
# Define deploy stage
FROM ubuntu:jammy as deploy

COPY --from=builder /usr/src/projects/ctrl-c-ctrl-v/build/bin/webserver .
COPY --from=builder /usr/src/projects/ctrl-c-ctrl-v/configs/prod_config .
COPY --from=builder /usr/src/projects/ctrl-c-ctrl-v/tests/static_test_files static_test_files

EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["prod_config"]
