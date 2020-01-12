<p align="center">
<img src="https://media.giphy.com/media/mP9hvHDhy4E9i/giphy.gif" alt="roomba_gif">
</p>

<h1 align="center">LibRoomba</h1>
<p align="center">
<img alt="GitHub" src="https://img.shields.io/github/license/roombavacuum/libroomba">
<img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/roombavacuum/libroomba">
</p>

The intention of this project is making an implementation of dorita980 in C.

This project has been implemented in C for firmwares V2 and V3.
With this library you can send commands to your wifi enabled Roomba through the iRobot directly from your LAN.

I recommend blocking the internet access to your robot to avoid the OTA firmware updates, new OTA can cause libroomba to stop working.

If you enjoy libroomba and it works nice for you, please give me a star.

## How to contribute
If you wanna contribute to the project, please make a fork, write your code and then submit a PR.

## Features
- [x] Get username/password easily from Roomba
- [x] Auto discovery roomba IP
- [x] Control your robot with the local API
- [x] Firmware 2.x.x/3.x.x compatible

## Commands

| Commands               | 2.x.x Local | 3.x.x Local   |
|------------------------|-------------|---------------|
| Clean                  | yes         | yes           |
| Start                  | yes         | yes           |
| Stop                   | yes         | yes           |
| Pause                  | yes         | yes           |
| Dock                   | yes         | yes           |
| Resume                 | yes         | yes           |
| Discovery Robot IP     | yes         | yes           |
| Get BLID and Password  | yes         | yes           |

## Dependencies
- wolfSSL with oldTLS support which contains AES128-SHA256 cipher suite
- wolfMQTT with TLS support
- Json-C

## Building
```sh
git clone https://github.com/roombavacuum/libroomba.git && cd home
meson build && cd build
ninja
```

## How to Use it

### Discovery
Discover your Roomba through the network.
This function sends a probe to the broadcast until find a roomba!
```sh
cd build/examples
./discovery
```

### Get Robot Info
This function gives you the basic information about the robot,
such as firmaware version, hostname, the ip, and the blid.
```
cd build/examples
./getRobotInfo
```

### Get Password
This method will only work correctly if you have triggered
wifi mode by holding the HOME button for several seconds
until the roomba beeps.
```
cd build/examples
./getPassword
```

### Exec a command
Pass the desired command in argv[1]
```
./startRoomba "clean"
```
