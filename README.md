# ctrl-c-ctrl-v

## Getting Started

We need set up the development environment in order to run our webserver. First, you must install [Docker](https://docs.docker.com/get-docker/) on your development machine.

### Prerequisites

Next, clone the following respository which contains scripts to create and run the required docker images.

```sh
git clone https://code.cs130.org/tools
```

Then, in your development directory containing the tools folder, run:

```sh
tools/env/start.sh -u ${USER}
```

where `$USER` is your UCLA username.

This will build the development environment for Docker and give you a shell inside the container which will be named `${USER}_devel_env`.

The container contains:

- Boost C++ Libraries
- Clang
- Cmake
- Docker
- Google Cloud SDK
- Google Test
- Git
- git-review
- LLDB

To run an additional shell in the running development environment, simply run `start.sh` again from another terminal
```sh
tools/env/start.sh -u ${USER}
```

For more information on setting up your development enviorment, see the [CS130 Documentation](https://www.cs130.org/guides/development/).

### Installation

1. Clone the ctrl-c-ctrl-v repo.
```sh
git clone "ssh://$USER@code.cs130.org:29418/ctrl-c-ctrl-v"
```

2. Configure and build the project. Running `make` may take a while.
```sh
mkdir build
cd build
cmake ..
make
```

## Usage

You can get up and running quickly using the config file provided in the repo.
In the root directory of the ctrl-c-ctrl-v repo run the following command.
```sh
   ./build/bin/webserver configs/dev_config
```
You can write your own config files.
A basic config file is specified as follows.
```
server {
    listen 80;
    location /static StaticHandler {
        root tests/static_test_files;
    }

    location /echo EchoHandler {

    }

    location /api CrudHandler {
        root /crud;
    }

    location /health HealthHandler {
        
    }

    location /integration_test TestHandler {
        
    }
	
	location /websocket_info WebsocketInfoHandler {
		
	}
}
```
- All contents in the config file must be wrapped in a `server` block.
- The config uses “#” for comments, which we found convenient because the Nginx parser already supports it.
- Each handler conforms to the “location-major typed” format
  - The keyword location, followed by a serving path, followed by the name of the handler, and a collection of arguments inside { ... }
  - This format emphasizes the paths, encouraging (but not forcing) their uniqueness
- Arguments for each handler appear as named elements within the { ... } block
   - The presence of explicit names avoids ambiguity, aids in readability (especially for humans, but also for machines) and allows for hard-coded default values.
- Each serving path stanza begins with the keyword `location`, that distinguishes from server-level arguments (such as `port`).
- The presence of quoting around strings (e.g. the serving path) is not required.
- Filesystem paths (such as `root` for the `StaticHandler`) are relative (implicitly anchored to the webserver binary location).
- Trailing slashes on URL serving paths are optional and should be ignored.
- Port numbers are specified by the `listen` or `port` keyword.
- Inside the static echo handler, 

## The main handlers
### Echo Handler
The echo handler is accessed at `/echo` and simply sends a response containing the request in its response body.
## Static Handler
The static handler serves static files from a specified location. These files can be accessed at the endpoint `/static/filepath`.
Users can also have access to a chatroom at `/static/chat.html`, a video calling platform at `/static/video.html` and a shared notebook at `/static/notepad.html`.
## Crud Handler
The Crud handler can be accessed at `/api`. The behavior of the API is best demonstrated using the following table.
| Action   | Example request                        | Visible behavior                                        | Internal Behavior                                                                                             |
|----------|----------------------------------------|---------------------------------------------------------|---------------------------------------------------------------------------------------------------------------|
| Create   | `POST /api/Shoes` (data in POST body)  | Returns the ID (1) of a newly created shoe.             | Find next available ID (1) and writes POST body to `/mnt/crud/Shoes/1`. Returns `{"id" : 1}` in the response. |
| Retrieve | `GET /api/Shoes/1`                     | Returns the data given in the previous step for shoe 1. | Reads `mnt/crud/Shoes/1` and returns to user.                                                                 |
| Update`  | `PUT /api/Shoes/1` (data in PUT body)  | Updates the data for shoe 1 with new data.              | Writes PUT body to `/mnt/crud/Shoes/1`.                                                                       |
| Delete   | `DELETE /api/Shoes/1`                  | Delete shoe 1 from the system.                          | Removes file `/mnt/crud/Shoes/1`.                                                                             |
| List     | `GET /api/Shoes`                       | Returns a JSON list of valid IDs for the given Entity.  | Lists filenames in `/mnt/crud/Shoes`. Returns the list of file names, e.g. `[1]`, in the response.

## Testing
You can run the server tests by entering the build folder and running  `make test`.

## Contributing

To commit code, please first install [clang-format](https://clang.llvm.org/docs/ClangFormat.html).

On Debian-based OSes (including the CS130 development enviorment), run `sudo apt install clang-format`.

Next, configure the pre-commit hook with: `git config core.hooksPath hooks`. This will automatically format any changed files before the commit.

### Directory Structure

- `src` contains all implementation code for the webserver itself with the corresponding definitions in `include`
- `docker` contains several dockerfiles and a a google cloudbuild config which sets up the docker container to build, test (including code coverage), and run the webserver both locally and on GCE
- `tests` contains all testing code and required files (e.g., config files for testing), organized into logical subfolders, except for `integration_test.sh`
- `configs` contains our default development and production config files
- `hooks` contains the bash scripts for running clang format hooks
- `logs`, which will be created once the server is run, contains the log files where `logger.cc` stores internal server data

### Code Structure

Our code is organized as follows. Where possible, we list implementations in the same order as they would be called as the webserver runs. We reference the implementation file (e.g., `src/*.cc`) but each has a corresponding header in `include/*.h`.

- `server_main.cc`: The entrypoint to our server. It takes in a single filepath as argument which should correspond to a config file. It initiates logging (`logger.cc`), calls the config parser (`config_parser.cc`) and, if sucessfull, starts the server (`server.cc`) by passing in a ServingConfig (`serving_config.cc`).
It initializes the state used by websocket operations.
- `logger.cc`: Sets up logging to console and disk with auto-rotation. Contains convinience methods for logging.
- `config_parser.cc`: Parses a config file at a given path and returns a `NginxConfig` representation of that config. This only validates that the format follows the Nginx configuration format, but does not validate whether the config contains correct parameters (e.g., a valid filepath).
- `serving_config.cc`: Accepts an `NginxConfig` and attempts to find and validate parameters needed to start the webserver (e.g., port and serving paths). 
- `server.cc`: Opens a TCP endpoint and asynchronously accepts requests by creating a new `Session` instance with a given socket and `ServingConfig`
- `session.cc`: Handles a single request and response from a client. Asynchronously reads in data from the socket, constructs the `boost::beast::http::request` and `boost::beast::http::response`, calls the `RequestDispatcher` (`request_dispatcher.cc`) to fill out the response, and subsequently writes the response to the socket.
- `request_dispatcher.cc`: Routes requests to an `I_RequestHandler` by using the `ServingConfig` to determine which implementation if any, to route to.
- `crud_filesystem_manager.cc`: Performs filesystem operations required to implement the crud handler. Provides functions to create directories and files, read to and write from files, delete files, and list the files present in a directory.
- `global_websocket_state.cc`: Manipulates the global websocket state and works in tandem with the Websocket handler to service websocket requests.
- `websocket_handler.cc`: Performs all the websocket communication. Handles the creation of new websocket creations, reading and writing data over websocket connections and handling any errors as a result of websocket communication.

We currently have 7 request handlers and corresponding request_handler_factories that implement `I_RequestHandler`, and `I_RequestHandlerFactory` respectively.
- `echo_handler`: echoes the request message exactly
- `static_handler`: serves static files on a specified path
- `health_handler`: always return `200 OK` to ensure our server is healthy 
- `not_found_handler`: deal with `404 NOT FOUND` HTTP responses
- `crud_handler_factory`: appropriately respond to crud requests
- `sleep_handler_Factory`: used for testing
- `websocket_info_handler_factory`: returns all the available chat channels 

We will use the simple example of `echo_handler` to illustrate how to add a new request handler in the future.

Once`request_dispatcher` determines based on the request that it should dispatch to the `echo_landler`, it uses the factory `EchoHandlerFactory` which returns a `EchoHandler` object. Although this case only requires the Request and Response to be passed to the factory, more complicated cases may require other arguments such as the `NginxConfig`.

The `EchoHandlerFactory` must implement `CreateHandler()` which constructs and returns an `EchoHandler`, although in this case the construction is trivial (seen in` echo_handler_factory.cc`). This constructed `EchoHandler` is short-lived, only used for a single request/response on a single-thread and is never shared. The dispatcher then calls the `HandleRequest()` of the `EchoHandler` which takes only a `boost::beast::http::request` object and the address of a `boost::beast::http::response` object.

Finally, the `EchoHandler` must fill out this response object. To implement a new request handler, developers must create a new header and implementation for the handler factory and handler itself following this pattern. Any response logic will be contained in the implementation of `I_RequestHandler`.

Similar request handlers `StaticHandler`, `HealthHandler`, and `NotFoundHandler` exist for our other currently supported options, serving static test files, a `200 OK` response, and a `404 NOT FOUND` response.

The `crud_handler_factory` handles the creation of CRUD handlers which act as a basic file management system which creates and delas with objects known as entities which contain JSON data. CRUD stands for Create, Read, Update and Delete, which are the four basic functions of the handler that have been implemented thus far. The `crud_handler_factory` operates in a similar to other handler factories such as static, it uses the path taken in through the config file to manage where the handler is created. It creates both a `CrudFileSystemManager` which deals with the actual movement and changes in the file organization in the database for CRUD operations, the factory also creates the `crud_handler` which manages the paths and commands to the file system manager depending on the operaton that is used.

The `crud_handler` has structures for each command and covers edge cases on what may occur depending on the state of the filesystem or a particualr entity at that point. It allows for the upload of JSON data which becomes associated with an ID that is returned, that ID can be used to retrieve it and GET the data, or to upload new JSON data to a particular ID, or for the deletion of the entity associated with that ID.

The `websocket_handler` manages all communications needed for the chatroom, video calling service and notepad service. It accepts new connections, handles the sending and recieiving of messages over the websocket connection and deals with any errors that occur when communicating over the websocket. It works in tandem with the `global_websocket_state` which manages and stores information about all running websocket connections and provides information about the available channels for users using the chatroom service. The `websocket_info_handler` simply returns the available channels which are stored in the `global_websocket_state`.

Note that `StaticHandler` accepts paths relative to the current working directory. The default developer config file `dev_config` we are using at the moment assumes that the working directory is the main `ctrl-c-ctrl-v` repository directory. If you wish to run the webserver from a different directory, please use a different config file that will create a working relative file path to your directory, instead of the default one.

For testing, `integration_test.sh` sets up a dummy client in a new terminal in order to test `boost::beast` functionality we cannot unit test, e.g. functions like `boost::beast::http::read` and other I/O methods, as well as some of the response and request formatting. Integration testing is particularly used for testing the CRUD operations as it allows the creation and deletion of files and entities with a higher ease and gives a better picture of operation health. The CRUD integration tests each deal with one of the operations and checking that the desired state has been achieved post the operation. We have static test files that test upload and download equality across server and client in `tests/IntegrationDiffs` and `tests/static_test_files`. We also recommend manually testing files to double check that the integration test works as intended.
The unit tests in test files that correspond to `.cc` files (for example, `request_dispatcher.cc` -> `request_dispatcher_test.cc`) do not directly test the response and request formatting because it is internally handled by `boost::beast` when reading and writing into buffers. Unit testing for CRUD operations exist in part for the `CrudFileSystemManager` and `crud_handler_factory` but they are largely focused on the actual function calls and effects in `crud_handler_test.cc`. The first portion of those deal with the soundness of the ID mechanics and saving, making sure they are accurate and scalable. After that the focus is split about equally into the four functions with each having a few common cases covered to ensure overall health of the operations.
