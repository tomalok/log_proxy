# log_proxy

[//]: # (automatically generated from https://github.com/metwork-framework/resources/blob/master/cookiecutter/_%7B%7Bcookiecutter.repo%7D%7D/README.md)

**Status (master branch)**




[![Drone CI](http://metwork-framework.org:8000/api/badges/metwork-framework/log_proxy/status.svg)](http://metwork-framework.org:8000/metwork-framework/log_proxy)
[![Maintenance](https://github.com/metwork-framework/resources/blob/master/badges/maintained.svg)]()


[//]: # (TABLE_OF_CONTENTS_PLACEHOLDER)

## What is it?

**log_proxy** is a tiny C utility for log rotation for apps that write their logs to stdout.

This is very useful, specially for [12-factor apps that write their logs to stdout](https://12factor.net/logs).

It can be used to avoid loosing some logs if you use `logrotate` with `copytruncate` feature or to prevent a log file from filling your hard disk.

## Features

- [x] usable as a pipe (`myapp myapp_arg1 myapp_arg2 |log_proxy /log/myapp.log`)
- [x] configurable log rotation suffix with `stftime` placeholders (for example: `.%Y%m%d%H%M%S`)
- [x] can limit the number of rotated files (and delete oldest)
- [x] can rotate files depending on their size (in bytes)
- [x] can rotate files depending on their age (in seconds)
- [x] does not need a specific log directory for a given app (you can have one directory with plenty of different log files from different apps)
- [x] several instances of the same app can log to the same file without issue (example: `myapp arg1 |log_proxy --use-locks /log/myapp.log` and `myapp arg2 |log_proxy --use-locks /log/myapp.log` can run at the same time)
- [ ] configurable action (a command to execute) to run after each log rotation
- [ ] rock solid (it's perfectly stable in our use case but we are waiting for other success stories to check this feature)
- [ ] option to add a timestamp before each log line
- [x] really fast
- [x] do not eat a lot of memory
- [x] configurable with CLI options as well with env variables
- [x] usable as a wrapper to capture stdout and stderr (`log_proxy_wrapper --stdout=/log/myapp.stdout --stderr=/log/myapp.stderr -- myapp myapp_arg1 myapp_arg2`)
- [x] usable as a wrapper to capture stdout and stderr in the same file (`log_proxy_wrapper --stdout=/log/myapp.log --stderr=STDOUT -- myapp myapp_arg1 myapp_arg2`)
- [x] very few dependencies (only `glib2` is required)
- [x] very easy to build (event on old distributions like `CentOS 6`)

## How to install?

We provide Linux 64 bits binaries in [releases section](https://github.com/metwork-framework/log_proxy/releases). There is virtually no requirement (you just need a Linux 64 bits distribution more recent than CentOS 6 (2011!)).

Of course, you can also build the tool by yourself (see at the end of this document).

To install the binary distribution:

```
# As root user (or with sudo)
# (example with 0.1.0 release, please fix the number to the release you want to install)
export RELEASE=v0.1.0
cd /opt
wget -O log_proxy-linux64-${RELEASE}.tar.gz "https://github.com/metwork-framework/log_proxy/releases/download/${RELEASE}/log_proxy-linux64-${RELEASE}.tar.gz"
zcat log_proxy-linux64-${RELEASE}.tar.gz |tar xvf -
ln -s log_proxy-linux64-${RELEASE} log_proxy
rm -f log_proxy-linux64-${RELEASE}.tar.gz
```

You can now use it with absolute path `/opt/log_proxy/bin/log_proxy` and `opt/log_proxy/bin/log_proxy_wrapper`.

For a more convenient way, configure your `PATH` to prepend the `/opt/log_proxy/bin` path. For example (maybe you would have to adapt this to your specific Linux distribution):

```
cat >/etc/profile.d/log_proxy.sh <<EOF
# Add log_proxy directory to PATH
export PATH="/opt/log_proxy/bin:${PATH}"
EOF
```

After a restart of your terminal, you should use `log_proxy` directly by its name.

## Why this tool?

### Why not using `logrotate` with `copytruncate` feature?

If you use `myapp myapp_arg1 myapp_arg2 >/log/myapp.log 2>&1`for example and if you can't stop easily your app (because it's a critical thing), you can configure `logrotate` with `copytruncate` feature to do the log rotation of `/log/myapp.log` but:

- you may loose a few lines during log rotation (1)
- the rotation is mainly time-based, so you can fill your storage if your app suddently start to be very very verbose

(1), see https://unix.stackexchange.com/questions/475524/how-copytruncate-actually-works

> Please note also that copyrotate has an inherent race condition, in that it's possible that the writer will append a line to the logfile just after logrotate finished the copy and before it has issued the truncate operation. That race condition would cause it to lose those lines of log forever. That's why rotating logs using copytruncate is usually not recommended, unless it's the only possible way to do it.

### Why developing another tool?

After reading: https://superuser.com/questions/291368/log-rotation-of-stdout and http://zpz.github.io/blog/log-rotation-of-stdout/, we reviewed plenty of existing tools (`multilog`, `rotatelogs`, `piper`...).

But none of them was ok with our needed features:

- configurable log rotation on size **AND** age
- no dedicated log directory for an app
- (and) several instances of the same app can log to the same log file without issue

The [piper tool](https://github.com/gongled/piper) was the more close but does not support the last feature (several instances to the same log file).

## Usage

### As a filter

```console
your_app your_app_arg1 your_app_arg2 2>&1 |log_proxy --rotation-size=1000000 my_log_file_max_1MB.log
```

Full help:

```console
$ ./log_proxy --help
Usage:
  log_proxy [OPTION?] LOGNAME  - log proxy

Help Options:
  -h, --help                Show help options

Application Options:
  -s, --rotation-size       maximum size (in bytes) for a log file before rotation (0 => no maximum, default: content of environment variable LOGPROXY_ROTATION_SIZE or 104857600 (100MB))
  -t, --rotation-time       maximum lifetime (in seconds) for a log file before rotation (0 => no maximum, default: content of environment variable LOGPROXY_ROTATION_TIME or 86400 (24H))
  -S, --rotation-suffix     strftime based suffix to append to rotated log files (default: content of environment variable LOGPROXY_ROTATION_SUFFIX or .%Y%m%d%H%M%S)
  -n, --rotated-files       maximum number of rotated files to keep including main one (0 => no cleaning, default: content of environment variable LOGPROXY_ROTATED_FILES or 5)
  -m, --use-locks           use locks to append to main log file (useful if several process writes to the same file)
  -f, --fifo                if set, read lines on this fifo instead of stdin
  -r, --rm-fifo-at-exit     if set, drop fifo at then end of the program (you have to use --fifo option of course)

Optional environment variables to override defaults :
    LOGPROXY_ROTATION_SIZE
    LOGPROXY_ROTATION_TIME
    LOGPROXY_ROTATION_SUFFIX
    LOGPROXY_ROTATED_FILES

Example for rotation-size option :
- If log_proxy is run with the option --rotation-size on command line, rotation-size will take the provided value
- If the option --rotation-size is not provided on command line :
  - If the environment variable LOGPROXY_ROTATION_SIZE is set, rotation-size will take this value
  - If the environment variable LOGPROXY_ROTATION_SIZE is not set, rotation-size will take the default value 104857600
```

## As a wrapper

```console
log_proxy_wrapper --rotation-size=1000000 --stdout=my_log_file_max_1MB.log --stderr=STDOUT -- your_app your_app_arg1 your_app_arg2
```

Full help:

```console
$ ./log_proxy_wrapper --help
Usage:
  log_proxy_wrapper [OPTION?] -- COMMAND [COMMAND_ARG1] [COMMAND_ARG2] [...] - log proxy

Help Options:
  -h, --help                Show help options

Application Options:
  -s, --rotation-size       maximum size (in bytes) for a log file before rotation (0 => no maximum, default: content of environment variable LOGPROXY_ROTATION_SIZE or 104857600 (100MB))
  -t, --rotation-time       maximum lifetime (in seconds) for a log file before rotation (0 => no maximum, default: content of environment variable LOGPROXY_ROTATION_TIME or 86400 (24H))
  -S, --rotation-suffix     strftime based suffix to append to rotated log files (default: content of environment variable LOGPROXY_ROTATION_SUFFIX or .%Y%m%d%H%M%S
  -n, --rotated-files       maximum number of rotated files to keep including main one (0 => no cleaning, default: content of environment variable LOGPROXY_ROTATED_FILES or 5)
  -m, --use-locks           use locks to append to main log file (useful if several process writes to the same file)
  -O, --stdout              stdout file path (NULL string (default) can be used to redirect to /dev/null)
  -E, --stderr              stderr file path (STDOUT string (default) can be used to redirect to the same file than stdout)
```

## How to build?

### Requirements

A Linux/Unix distribution with standard development tools (`git`, `gcc`, `make`, `pkg-config`) and `glib2` library with `devel` support (provided for example in CentOS 6 in the `glib2-devel` standard package).

### Build and install

```console
git clone https://github.com/metwork-framework/log_proxy # or download/unpack a zip with the github interface
cd log_proxy
make
```

Then as `root` user or prefixed with `sudo`:

```console
make PREFIX=/usr/local install
```











## Contributing guide

See [CONTRIBUTING.md](CONTRIBUTING.md) file.



## Code of Conduct

See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) file.



## Sponsors

*(If you are officially paid to work on MetWork Framework, please contact us to add your company logo here!)*

[![logo](https://raw.githubusercontent.com/metwork-framework/resources/master/sponsors/meteofrance-small.jpeg)](http://www.meteofrance.com)
