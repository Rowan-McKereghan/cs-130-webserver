<h3 align="center">ctrl-c-ctrl-v</h3>


<!-- GETTING STARTED -->
## Getting Started

We first set up the development environment needed to run our webserver.
Install a copy of Docker on your development machine.
Ensure you are connected to UCLA's network.

### Prerequisites

We first install the tools needed to create and run the required docker images.
Install
  ```sh
  git clone https://code.cs130.org/tools
  ```
Then in your development directory containing the tools folder run
```sh
   tools/env/start.sh -u ${USER}
```
where `$USER` is your UCLA usernmae.
This will build the development environment for Docker and give you a shell inside the container which will be named `${USER}_devel_env`.
The container contains
<ul>
<li> Boost C++ Libraries </li>
<li> Clang </li>
<li> Cmake </li>
<li> Docker </li>
<li> Google Cloud SDK </li>
<li> Google Test </li>
<li> Git </li>
<li> git-review </li>
<li> LLDB </li>
</ul>

To run an additional shell in the running development environment, simply run `start.sh` again from another terminal
```sh
    tools/env/start.sh -u ${USER}
```


### Installation

1. Clone the ctrl-c-ctrl-v repo.
   ```sh
   git clone "ssh://samarthu27@code.cs130.org:29418/ctrl-c-ctrl-v"
   ```
2. Configure and build the project. Running `make` may take a while.
   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```
<!-- USAGE EXAMPLES -->
## Usage

You can get up and running quickly using the config file provided in the repo.
In the root directory of the ctrl-c-ctrl-v repo run the following command.
```sh
   ./build/bin/webserver configs/dev_config
```
You can write your own config files.
A config file is specified as follows.
```
server {
   listem <port_number>;
   static_file_paths {
      <static_endpoints_1> <src_dir_1>;
      <static_endpoint_2> <src_dir_2>;
      ...
   }
   echo_paths {
      <echo_endpoint_1>;
      <echo_endpoint_2>;
   }
}
```
All contents in the config file must be wrapped in a `server` block.
Port numbers are specified by the `listen` keyword.
The static file endpoints are mapped to the source directories inside the `static_file_paths` block.
The echo endpoints are listed inside the `echo_paths` block.

<!-- Testing -->
## Testing
You can run the server tests by entering the build folder and running  `make test`.
