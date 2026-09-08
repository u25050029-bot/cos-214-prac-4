# TaskForge

## Prerequisites

- Docker Engine 20.10 or newer (Docker Desktop on Windows or macOS)

Verify it is available:

```bash
docker --version
```

## 1. Build the Docker image

Run this from the project root, the directory containing the `Dockerfile`:

```bash
docker build -t taskforge .
```

This installs the toolchain, copies the source into `/taskforge` inside the image and
compiles the project. If compilation fails, the image build fails, so a successful
`docker build` is itself proof that the project compiles cleanly.

To force a completely fresh build with no cached layers:

```bash
docker build --no-cache -t taskforge .
```

## 2. Run the program

```bash
docker run --rm -it taskforge
```

To run it through the Makefile target instead:

```bash
docker run --rm -it taskforge make run
```

## 3. Debug with GDB

```bash
docker run --rm -it \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  taskforge gdb ./taskforge
```

Or via the Makefile target:

```bash
docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined taskforge make debug
```

The binary is compiled with `-g -O0`, so full symbol and line information is available
and no statements are optimised away.

A typical GDB session:

```
(gdb) break main
(gdb) run
(gdb) next
(gdb) step
(gdb) print variableName
(gdb) backtrace
(gdb) info locals
(gdb) continue
(gdb) quit
```

To break on a specific source location:

```
(gdb) break src/Trader.cpp:42
```

## 4. Check for memory errors with Valgrind

```bash
docker run --rm -it taskforge \
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./taskforge
```

Single line version:

```bash
docker run --rm -it taskforge valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./taskforge
```

Or via the Makefile target, which uses the same flags:

```bash
docker run --rm -it taskforge make memcheck
```

To write the report to a file inside a mounted directory on the host:

```bash
docker run --rm -it -v "$PWD":/taskforge taskforge \
  bash -c "make && valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./taskforge"
```

The report then appears as `valgrind.log` in the project directory on the host.

## 5. Interactive shell inside the container

For exploring the container or running several commands in one session:

```bash
docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined taskforge bash
```

Inside the shell:

```bash
make
./taskforge
gdb ./taskforge
valgrind --leak-check=full ./taskforge
```
