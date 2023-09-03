# Btrfs Assistant

## Overview
Btrfs Assistant is a GUI management tool to make managing a Btrfs filesystem easier.  The primary features it offers are:
* An easy to read overview of Btrfs metadata
* A simple view of subvolumes with or without Snapper/Timeshift snapshots
* Run and monitor scrub and balance operations
* A pushbutton method for removing subvolumes
* A management front-end for Snapper with enhanced restore functionality
	* View, create and delete snapshots
	* Restore snapshots in a variety of situations
	  * When the filesystem is mounted in a different distro
	  * When booted off a snapshot
	  * From a live ISO
	* View, create, edit, remove Snapper configurations
	* Browse snapshots and restore individual files
	* Browse diffs of a single file across snapshot versions
	* Manage Snapper systemd units
* A front-end for Btrfs Maintenance
	* Manage systemd units
	* Easily manage configuration for defrag, balance and scrub settings

### Screenshots
![image](/uploads/21da59577c3e8a101347cf0d59569c09/image.png)

![image](/uploads/41aa431b6a0de85bc70b84d90da392ea/image.png)

![image](/uploads/65b6004c3257d66154828259a0fed47d/image.png)

![image](/uploads/d255a9d9839ba8633b8e911858f4b48f/image.png)

![image](/uploads/429be74e9fb92088697944d23a1def1d/image.png)

![image](/uploads/ea3940775576a3a0ef7f205b8f2fd77a/image.png)

## Installing

#### Arch
Btrfs Assistant can be installed from the AUR as `btrfs-assistant`

#### Debian
There are unofficial Debian packages [here](https://software.opensuse.org/download/package?package=btrfs-assistant&project=home:iDesmI:more) coutesy of @idesmi or you can follow the instructions for Ubuntu to build it yourself.

#### Ubuntu
1. Install the prerequisites: `sudo apt install git cmake qtbase5-dev qttools5-dev fonts-noto libqt5svg5 libqt5core5a g++ libbtrfs-dev libbtrfsutil-dev`
1. Download the tar.gz from the latest version [here](https://gitlab.com/btrfs-assistant/btrfs-assistant/-/tags)
1. Untar the archive and cd into the directory
1. `cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE='Release'`
1. `make -C build`
1. `sudo make -C build install`
1. Optionally install Snapper - `sudo apt install snapper`

#### Fedora
Btrfs Assistant is available in the Fedora repos as `btrfs-assistant`

## Contributing
Contributions are welcome!

Please see [CONTRIBUTING.md](docs/CONTRIBUTING.md) for more details.


### Development Requirements
* Qt5 / Qt Design UI
* C++17
* Cmake >= 3.5
* Root user privileges
* Btrfs filesystem
