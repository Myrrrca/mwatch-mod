## Linux Kernel Module

#### Overview
This Linux kernel module sets a watchpoint (hardware breakpoint) on a specified memory address. When this memory address is accessed, the module triggers its callbacks.

#### Functionality
1. The memory address to monitor is set via a module parameter and can be modified through the sysfs entry located at `/sys/module/mwatch/parameters/mwatch_addr`.
2. The module sets a hardware breakpoint (watchpoint) on the specified memory address.
3. Upon access to this memory address (read or write), the module invokes its callbacks (separate for read and write operations) and prints a backtrace.

#### File Descriptions
This module consists of two modules eventually: the source code for both modules and their corresponding Makefile located in `recipes-kernel/mwatch-mod/files/`.

However, this module's implementation is designed for building via Yocto for the qemux86 virtual machine. Consequently, this repository includes numerous additional files and directories, which will be addressed in the "Building and Running" section.

Comprehensive Yocto documentation is available on their official website:
[https://docs.yoctoproject.org/index.html](https://docs.yoctoproject.org/index.html)

#### Compatibility and Important Information
This module was developed for version 6.9.6-200.fc40.x86_64 of Linux on Fedora Workstation 40. To verify your kernel headers, use the `uname -r` command.

The process of building the Linux distribution image using Yocto can vary widely in duration, from minutes to over 24 hours, depending on your computer's capabilities. Ensure you have at least 90 gigabytes of free hard drive space before initiating the build.

#### Module Operation
**test_mwatch:**
This module should be loaded into the kernel first. It creates a static global variable and increments its value every 15 seconds. It also displays the memory address of this variable.

**mwatch:**
The mwatch module creates a sysfs entry at `/sys/module/mwatch/parameters/mwatch_addr`, initially set to 0. This entry represents the address where mwatch will set a hardware breakpoint (watchpoint). Once both modules are loaded, you can set the memory address of the static variable from test_mwatch to mwatch's sysfs entry using the `echo` command.

All module logs, including the virtual address of the variable from test_mwatch, are printed in the kernel space. You can monitor these logs using `dmesg` or `dmesg | tail` at any time.

#### Preparing for building 
To prepare for Yocto installation, ensure you have the necessary packages according to your distribution's package manager and the official Yocto documentation:
[https://docs.yoctoproject.org/ref-manual/system-requirements.html#required-packages-for-the-build-host](https://docs.yoctoproject.org/ref-manual/system-requirements.html#required-packages-for-the-build-host). 
For Fedora Linux, the required commands are:
```shell
sudo dnf install gawk make wget tar bzip2 gzip python3 unzip perl patch diffutils diffstat git cpp gcc gcc-c++ glibc-devel texinfo chrpath ccache perl-Data-Dumper perl-Text-ParseWords perl-Thread-Queue perl-bignum socat python3-pexpect findutils which file cpio python python3-pip xz python3-GitPython python3-jinja2 rpcgen perl-FindBin perl-File-Compare perl-File-Copy perl-locale zstd lz4 hostname glibc-langpack-en libacl
sudo dnf install git make python3-pip which inkscape texlive-fncychap
sudo pip3 install sphinx sphinx_rtd_theme pyyaml
```
Note that Yocto does not support all popular distributions; refer to the list of supported distributions here:
[https://docs.yoctoproject.org/ref-manual/system-requirements.html#supported-linux-distributions](https://docs.yoctoproject.org/ref-manual/system-requirements.html#supported-linux-distributions)

#### Getting Started
1. Clone the Yocto distribution image repository to your local machine:
```shell
git clone git://git.yoctoproject.org/poky
```
2. Create a local branch named `my-mickledore` in the `poky/` directory, synchronized with the `mickledore` release branch of the repository:
```shell
cd poky && git checkout -t origin/mickledore -b my-mickledore
```
3. Update your local repository just to make sure you synchronized:
```shell
git pull
```
4. Initialize the build environment by running the script in the `poky/` directory:
```shell
source oe-init-build-env
```
5. Clone the module's repository into the `poky/` directory:
```shell
cd .. && git clone https://github.com/Myrrrca/mwatch-mod
```
6. Create a new layer named `meta-mwatch-mod` in the `poky/` directory for inclusion in the build:
```shell
bitbake-layers create-layer meta-mwatch-mod
```
7. Add your layer to the build configuration:
```shell
bitbake-layers add-layer meta-mwatch-mod
```
8. Verify that your layer has been added:
```shell
bitbake-layers show-layers
```
The bottom of the output should confirm the inclusion of your layer `meta-mwatch-mod`.

9. Copy configuration files and recipes from `poky/mwatch-mod` to `poky/meta-mwatch-mod`, replacing files of the same name, and remove the `recipes-example` directory created automatically:
```shell
cp -r ./mwatch-mod/{conf,recipes-kernel}/* ./meta-mwatch-mod/ && rm -r ./meta-mwatch-mod/recipes-example
```
10. Verify the directory structure of `poky/meta-mwatch-mod` using the `tree` command:
```shell
tree meta-mwatch-mod/
```
The output structure should resemble:

```
meta-watchpoint-mod/
├── conf
│   └── layer.conf
├── COPYING.MIT
├── README
└── recipes-kernel
    └── mwatch-mod
        ├── files
        │   ├── Makefile
        │   ├── mwatch.c
        │   └── test_mwatch.c
        └── mwatch-mod_0.1.bb

5 directories, 7 files
```
11. Remove the locally cloned `mwatch-mod` repository:
```shell
rm -rf ./mwatch-mod
```
12. Configure the build settings in the ```poky/build/conf/local.conf``` file. Edit the file and append the line:
```
IMAGE_INSTALL:append = " mwatch-mod"
```
Ensure the line:

```
MACHINE ?= "qemux86"
```
is uncommented.

13. Initiate the build from the `poky/build` directory: (this can take a while depending on your PC)
```shell
bitbake core-image-minimal
```
14. Start the build. For convenience, open a new terminal (Ctrl+Alt+2). By default, you will be logged in as `root` without a password:
```shell
runqemu qemux86
```
15. Verify the presence of the modules in `/lib/modules/6.1.53-yocto-standard/extra`:
```shell
cd /lib/modules/6.1.53-yocto-standard/extra && ls
```
You should see `test_mwatch.ko` and `mwatch.ko` listed.

16. Load `test_mwatch.ko` module from the `/lib/modules/6.1.53-yocto-standard/extra` directory or specify the full path:
```shell
insmod test_mwatch.ko
dmesg | tail
```
17. Repeat the process for `mwatch.ko` module:
```shell
insmod mwatch.ko
dmesg
```
Note: The address variable is set to defaults 0x0 during module loading

18. Change the watchpoint address to where `test_mwatch` module updates the value every 15 seconds:
```shell
echo "0xffffffffd13dc284" > /sys/module/mwatch/parameters/mwatch_addr
dmesg | tail
```
You should receive a message confirming that the `mwatch` module has updated its watchpoint address and printed a backtrace upon access to that address.

19. Unload both modules:
```shell
rmmod mwatch && rmmod test_mwatch
```
20. Shut down the system:
```shell
shutdown -h now
```
or
```shell
poweroff
```

#### Alternative Approach
###### If you prefer not to use a virtual machine:
**Loading modules directly into your kernel without a virtual machine is strongly discouraged unless you are experienced.** Simply compile the modules using the provided Makefile:

```shell
sudo su
insmod test_mwatch.ko
insmod mwatch.ko
```
Always monitor kernel logs (`dmesg`) for module operation details.

After `mwatch` successfully prints a backtrace, unload both modules:

```shell
rmmod mwatch && rmmod test_mwatch
```
Ensure no instances of these modules are running:

```shell
lsmod | grep mwatch
```
The output should be empty.

#### Additional Information
Please report any bugs or issues on the project's GitHub Issues page: https://github.com/Myrrrca/mwatch-mod. Include detailed information about the issue for prompt resolution.
