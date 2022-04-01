# Btrfs Assistant

## Overview
Btrfs Assistant is a GUI management tool to make managing a Btrfs filesystem easier.  The primary features it offers are:
* An easy to read overview of Btrfs metadata
* A simple view of subvolumes with or without Snapper/Timeshift snapshots
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
