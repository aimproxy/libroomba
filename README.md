<p align="center">
<img src="https://media.giphy.com/media/mP9hvHDhy4E9i/giphy.gif" alt="roomba_gif">
</p>

<h1 align="center">LibRoomba</h1>
<p align="center">
<img alt="GitHub" src="https://img.shields.io/github/license/roombavacuum/libroomba">
<img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/roombavacuum/libroomba">
</p>
This is an implementation of the dorita980 lib, written in C for firmwares V2 and V3.

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
cd build
./discovery
```

### Get Robot Info
This function gives you the basic information about the robot,
such as firmaware version, hostname, the ip, and the blid.
```
cd build
./getRobotInfo
```

### Get Password
This method will only work correctly if you have triggered
wifi mode by holding the HOME button for several seconds
until the roomba beeps.
```
cd build
./getPassword
```
