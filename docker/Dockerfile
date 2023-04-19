### Build/test container ###
# Define builder stage
FROM ctrl-c-ctrl-v:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:jammy as deploy

# COPY --from=builder /usr/src/project/build/bin/example_server .
COPY --from=builder /usr/src/project/build/bin/webserver .
COPY --from=builder /usr/src/project/tests/GetPortNumber/success_pn2 .

EXPOSE 800

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["success_pn2"]
