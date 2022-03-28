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
	* Manage Snapper systemd units
* A front-end for Btrfs Maintenance
	* Manage systemd units
	* Easily manage configuration for defrag, balance and srub settings

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