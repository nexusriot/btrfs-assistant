# Btrfs Assistant

## Overview
Btrfs Assistant is a GUI management tool to make managing a Btrfs filesystem easier.  The primary features it offers are:
* An easy to read overview of Btrfs metadata
* A simple view of subvolumes with or without Snapper/Timeshift snapshots
* Run and monitor scrub and balance operations
* A pushbutton method for removing subvolumes
* A management front-end for Snapper
	* View, create and delete snapshots
	* Restore snapshots
	* View, create, edit, remove Snapper configurations
	* Browse snapshots and restore individual files
	* Browse diffs of a single file across snapshot versions
	* Manage Snapper systemd units
* A front-end for Btrfs Maintenance
	* Manage systemd units
	* Easily manage configuration for defrag, balance and srub settings

### Screenshots
![image](/uploads/7a89a3a3ff6776ae5d3f5a24a32985bb/image.png)

![image](/uploads/03d8541ddca3f0375c1aace1a6450ec6/image.png)

![image](/uploads/229b5792fea354baff5899baaadfb601/image.png)

![image](/uploads/429be74e9fb92088697944d23a1def1d/image.png)

![image](/uploads/ea3940775576a3a0ef7f205b8f2fd77a/image.png)

## Installing

#### Arch
Btrfs Assistant can be installed from the AUR as `btrfs-assistant`

#### Debian/Ubuntu
1. Install the prerequisites: `sudo apt install git cmake qtbase5-dev qttools5-dev fonts-noto libqt5svg5 libqt5core5a g++`
1. Download the tar.gz from the latest version [here](https://gitlab.com/btrfs-assistant/btrfs-assistant/-/tags)
1. Untar the archive and cd into the directory
1. `cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE='Release'`
1. `make -C build`
1. `sudo make -C build install`
1. `sudo cp btrfs-assistant.conf /etc/.`
1. `sudo cp org.btrfs-assistant.pkexec.policy /usr/share/polkit-1/actions/.`
1. Optionally install Snapper - `sudo apt install snapper`

## Contributing
Contributions are welcome!

If you want add or change significant functionality it is better to discuss it before beginning work either in an issue or a WIP merge request.

Please use `clang-format` to ensure any contributions enforce the style used throughout Btrfs Assistant


### Development Requirements
* Qt5 / Qt Design UI
* C++20
* Cmake >= 3.5
* Root user privileges
* Btrfs filesystem
