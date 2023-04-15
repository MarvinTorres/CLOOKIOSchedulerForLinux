# C-LOOK IO Scheduler for Linux
This is my disk scheduler for Linux 3.6.6 that implements the [C-LOOK algorithm](https://www.geeksforgeeks.org/c-look-disk-scheduling-algorithm/). 

This was one of my works written for my Operating Systems course.

## How to Install
**Note: This is a modification of version 3.6.6. Other versions are not guaranteed to work.**
* [3.6 GitHub repo for help finding files (optional)](https://github.com/torvalds/linux/tree/v3.6)

### Prerequisites
* A modern Linux-based OS like [Ubuntu](https://ubuntu.com/). These steps will be executed in this OS.
* [VirtualBox](https://www.virtualbox.org/) to run said OS. 

Use the patch or patchless steps, either will work.

### Patch

#### Prerequisites
* [Download Git for Linux](https://git-scm.com/). Git Bash's patching feature is used here.

##### Steps
1. [Download Linux 3.6.6](https://launchpad.net/linux/3.6/3.6.6).
1. From the root folder, go to `block` and open Git Bash in this folder. In Git Bash, enter `patch --help` to make sure that the command is available.
1. Move `clook.patch` to `block`.
  * `clook.patch` can be found in this repo - in `upload`.
1. In Git Bash, apply `clook.patch` to `noop-iosched.o`
  * `noop-iosched.o` is already in the `block` folder.
  * `patch -u noop-iosched.o -i clook.patch`
1. Move files `Kconfig.iosched` and `Makefile` from this repo's `linux-3.6.6/block` to 3.6.6's `block`, and overwrite files when prompted.
1. In the root folder, run the makefile.

### Patchless

#### Prerequisites
* None.

##### Steps
1. [Download Linux 3.6.6](https://launchpad.net/linux/3.6/3.6.6).
1. Find the root folder but do not enter it. It should be called `linux-3.6.6`.
1. Move folder `linux-3.6.6` from this repo to the same directory as the root folder, and overwrite files when prompted.
1. In the root folder, run the makefile.

## Permissions

Feel free to fork this for personal usage only.
