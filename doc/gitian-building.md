Gitian building
================

*Setup instructions for a Gitian build of Bytz Core using a Debian VM or physical system.*

Gitian is the deterministic build process that is used to build the Bytz
Core executables. It provides a way to be reasonably sure that the
executables are really built from the source on GitHub. It also makes sure that
the same, tested dependencies are used and statically built into the executable.

Multiple developers build the source code by following a specific descriptor
("recipe"), cryptographically sign the result, and upload the resulting signature.
These results are compared and only if they match, the build is accepted and uploaded
to bytz.gg.

More independent Gitian builders are needed, which is why this guide exists.
It is preferred you follow these steps yourself instead of using someone else's
VM image to avoid 'contaminating' the build.

Table of Contents
------------------

- [Create a new VirtualBox VM](#create-a-new-virtualbox-vm)
- [Connecting to the VM](#connecting-to-the-vm)
- [Setting up Debian for Gitian building](#setting-up-debian-for-gitian-building)
- [Installing Gitian](#installing-gitian)
- [Setting up the Gitian image](#setting-up-the-gitian-image)
- [Getting and building the inputs](#getting-and-building-the-inputs)
- [Building Bytz Core](#building-bytz-core)
- [Building an alternative repository](#building-an-alternative-repository)
- [Signing externally](#signing-externally)
- [Uploading signatures](#uploading-signatures)

Preparing the Gitian builder host
---------------------------------

The first step is to prepare the host environment that will be used to perform the Gitian builds.
This guide explains how to set up the environment, and how to start the builds.

Debian Linux was chosen as the host distribution because it has a lightweight install (in contrast to Ubuntu) and is readily available.
Any kind of virtualization can be used, for example:
- [VirtualBox](https://www.virtualbox.org/) (covered by this guide)
- [KVM](http://www.linux-kvm.org/page/Main_Page)
- [LXC](https://linuxcontainers.org/), see also [Gitian host docker container](https://github.com/gdm85/tenku/tree/master/docker/gitian-bitcoin-host/README.md).

You can also install Gitian on actual hardware instead of using virtualization.

Create a new VirtualBox VM
---------------------------

* ### In the VirtualBox GUI click "New" and choose the following parameters in the wizard:

![](vbox-create-debian/VBox1.png)

* ### Type: Linux, Debian (64-bit)
* ### Memory Size: at least 4096MB, anything less and the build might not complete.
* ### Hard Disk: Create a virtual hard disk now
* ### Click Create

![](vbox-create-debian/VBox2.png)

* ### Hard Disk file type: Use the default, VDI (VirtualBox Disk Image)
* ### Storage on physical hard disk: Dynamically Allocated
* ### File location and size: at least 40GB; as low as 20GB *may* be possible, but better to err on the safe side
* ### Click `Create`

![](vbox-create-debian/VBox3.png)

## After creating the VM, we need to configure it.

* ### Click the `Settings` button, then go to `System` tab and `Processor` sub-tab. Increase the number of processors to the number of cores on your machine if you want builds to be faster.

![](vbox-create-debian/VBox4.png)

* ### Go to the `Network` tab. Adapter 1 should be attached to `NAT`.
* ### Click `Advanced`, then `Port Forwarding`. We want to set up a port through which we can reach the VM to get files in and out.

![](vbox-create-debian/VBox5.png)

* ### Create a new rule by clicking the plus icon.

![](vbox-create-debian/VBox6.png)

- Set up the new rule the following way:
  - Name: `SSH`
  - Protocol: `TCP`
  - Leave Host IP empty
  - Host Port: `22222`
  - Leave Guest IP empty
  - Guest Port: `22`

* ### Click `Ok` twice to save.

Download the most recent Debian ISO from [https://www.debian.org/download](https://www.debian.org/download) (Version 11 at the time of writing) (a more recent minor version should also work, see also [Debian Network installation](https://www.debian.org/CD/netinst/)).
This DVD image can be [validated](https://www.debian.org/CD/verify) using a SHA256 hashing tool, for example on
Unixy OSes by entering the following in a terminal:

    echo "ae6d563d2444665316901fe7091059ac34b8f67ba30f9159f7cef7d2fdc5bf8a  debian-11.0.0-amd64-netinst.iso" | sha256sum -c
    # (must return OK)

Replace `sha256sum` with `shasum` on OSX.

* ### Start the VM. On the first launch you will be asked for a CD or DVD image. Choose the downloaded ISO by selecting "Add" when given the list of media.

![](vbox-create-debian/VBox7.png)

* ### Select the Debian 11 ISO file that you downloaded above

![](vbox-create-debian/VBox8.png)

* ### Click Start and your virtual machine will start using the Debian installer ISO

![](vbox-create-debian/VBox9.png)

Installing Debian
------------------

This section will explain how to install Debian on the newly created VM.

* ### Select Install from the boot menu. We do not need to install the graphical environment

![setup1](setup-debian/setup1.png)

* ### Select your preferred language to install (English shown)

![setup2](setup-debian/setup2.png)

* ### Select your location (United States shown)

![setup3](setup-debian/setup3.png)

* ### Select the keyboard to use (American English shown)

![setup4](setup-debian/setup4.png)

* ### Wait for the installer components to load

![setup5](setup-debian/setup5.png)

* ### Give the system a hostname

![setup6](setup-debian/setup6.png)

* ### Give the system a domain name

![setup7](setup-debian/setup7.png)

* ### Set the root password

![setup8](setup-debian/setup8.png)

* ### Confirm the root password

![setup9](setup-debian/setup9.png)

* ### Create a new user which will be used for gitian building

![setup10](setup-debian/setup10.png)

* ### Set the new user's username

![setup11](setup-debian/setup11.png)

* ### Create a password for the new user

![setup12](setup-debian/setup12.png)

* ### Confirm the password for the new user

![setup13](setup-debian/setup13.png)

* ### Select your timezone

![setup14](setup-debian/setup14.png)

* ### Partition the disk (Guided - use entire disk recommended)

![setup15](setup-debian/setup15.png)

* ### Select the new disk  to partition

![setup16](setup-debian/setup16vb.png)

* ### Select "All files in one partition"

![setup17](setup-debian/setup17vb.png)

* ### Select "Finish partitioning and write changes to disk"

![setup18](setup-debian/setup18vb.png)

* ### Select "Yes" to write the changes to the disk

![setup19](setup-debian/setup19.png)

* ### Wait for the base system to be installed

![setup20](setup-debian/setup20.png)

* ### Select "No" when asked for extra installation media

![setup21](setup-debian/setup21.png)

* ### Select your location to find a package repository close to you (United States shown)

![setup22](setup-debian/setup22.png)

* ### Select a package mirror from the list

![setup23](setup-debian/setup23.png)

* ### Configure a proxy if needed

![setup24](setup-debian/setup24.png)

* ### Wait for the package manager to configure itself

![setup25](setup-debian/setup25.png)

* ### Make your choice whether to participate in the package usage survey

![setup26](setup-debian/setup26.png)

* ### Make sure to select ssh server here so you can log in to your system and deselect Debian Desktop Environment and GNOME

![setup27](setup-debian/setup27.png)

* ### Wait for the software to install

![setup28](setup-debian/setup28.png)

* ### Select "Yes" here to set up the boot loader

![setup29](setup-debian/setup29.png)

* ### Select "/dev/sda" to install the boot loader on

![setup30](setup-debian/setup30.png)

* ### Wait for the installation to complete

![setup31](setup-debian/setup31.png)

* ### Installation is now complete

![setup32](setup-debian/setup32.png)

* ### Choose continue to boot into your Debian Operating System



After Installation
-------------------
The next step in the guide involves logging in as root via SSH.
SSH login for root users is disabled by default, so we'll enable that now.

Login to the VM using username `root` and the root password you chose earlier.
You'll be presented with a screen similar to this.

![](gitian-building/debian_root_login.png)

Type:

```
sed -i 's/^PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config
```
and press enter. Then,
```
systemctl ssh restart
```
and enter to restart SSH. Logout by typing 'logout' and pressing 'enter'.

Connecting to the VM
----------------------

After the VM has booted you can connect to it using SSH, and files can be copied from and to the VM using a SFTP utility.
Connect to `localhost`, port `22222` (or the port configured when installing the VM).
On Windows you can use [putty](http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html) and [WinSCP](http://winscp.net/eng/index.php).

For example, to connect as `root` from a Linux command prompt use

    $ ssh root@localhost -p 22222
    The authenticity of host '[localhost]:22222 ([127.0.0.1]:22222)' can't be established.
    RSA key fingerprint is ae:f5:c8:9f:17:c6:c7:1b:c2:1b:12:31:1d:bb:d0:c7.
    Are you sure you want to continue connecting (yes/no)? yes
    Warning: Permanently added '[localhost]:22222' (RSA) to the list of known hosts.
    root@localhost's password: (enter root password configured during install)

    The programs included with the Debian GNU/Linux system are free software;
    the exact distribution terms for each program are described in the
    individual files in /usr/share/doc/*/copyright.

    Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
    permitted by applicable law.
    root@debian:~#

Replace `root` with `gitianuser` to log in as user.

Setting up Debian for Gitian building
--------------------------------------

In this section we will be setting up the Debian installation for Gitian building.
We assume that a user `gitianuser` was previously added.

First we need to set up dependencies. Type/paste the following in the terminal:

```bash
su -
<enter root password>
apt-get -y install sudo git
echo 'gitianuser  ALL=(ALL:ALL) NOPASSWD:ALL' >> /etc/sudoers
logout
```

Then set up LXC and the rest with the following, which is a complex jumble of settings and workarounds:

```bash
sudo -s
# the version of lxc-start in Debian needs to run as root, so make sure
# that the build script can execute it without providing a password
echo "%sudo ALL=NOPASSWD: /usr/bin/lxc-start" > /etc/sudoers.d/gitian-lxc
echo "%sudo ALL=NOPASSWD: /usr/bin/lxc-execute" >> /etc/sudoers.d/gitian-lxc
# make sure that USE_LXC is always set when logging in as gitianuser,
# and configure LXC BRIDGE interface
echo 'export USE_LXC=1' >> /home/gitianuser/.profile
echo 'export LXC_BRIDGE="br0"' >> /home/gitianuser/.profile
sudo reboot
```

At the end Debian is rebooted to make sure that the changes take effect. The steps in this
section only need to be performed once.

**Note**: When sudo asks for a password, enter the password for the user `gitianuser` not for `root`.

Installing Gitian
------------------

Re-login as the user `gitianuser` that was created during installation.
The rest of the steps in this guide will be performed as that user.

**Note**: If sudo asks for a password, enter the password for the user `gitianuser` not for `root`.

Clone the git repositories for BYTZ and Gitian.

```bash
git clone https://github.com/devrandom/gitian-builder.git
git clone https://github.com/bytzcurrency/bytz
git clone https://github.com/bytzcurrency/gitian.sigs.git
git clone https://github.com/bytzcurrency/bytz-detached-sigs.git
mkdir -p gitian-builder/inputs
cd gitian-builder/inputs
wget https://<url and path to MAC OSX SDK>/MacOSX10.11.sdk.tar.xz
cd -
```

Setting up the Gitian image
-------------------------

Gitian needs a virtual image of the operating system to build in.
Currently this is Ubuntu Bionic x86_64.
This image will be copied and used every time that a build is started to
make sure that the build is deterministic.
Creating the image will take a while, but only has to be done once.

Execute the following as user `gitianuser`:

```bash
bytz/contrib/gitian-builder.py -S
```

There will be a lot of warnings printed during the build of the image. These can be ignored.

**Note**: If sudo asks for a password, enter the password for the user `gitianuser` not for `root`.

Getting and building the inputs
--------------------------------
At this point you have two options, you can either use the automated script (found in [https://github.com/bytzcurrency/bytz/blob/develop/contrib/gitian-build.py](https://github.com/bytzcurrency/bytz/blob/develop/contrib/gitian-build.py), only works in Debian/Ubuntu) or you could manually do everything by following this guide.
If you are using the automated script, then run it with the `-S switch` `contrib/gitian-build.py -S`. Afterwards, run it with the `--build` command (example: `contrib/gitian-build.py -b <signer> 3.1.0`). Otherwise ignore this.

Follow the instructions in [release-process.md](release-process#fetch-and-create-inputs-first-time-or-when-dependency-versions-change)
in the bytz repository under 'Fetch and create inputs' to install sources which require
manual intervention. Also optionally follow the next step: 'Seed the Gitian sources cache
and offline git repositories' which will fetch the remaining files required for building
offline.

## Building Bytz Core

To build Bytz Core (for Linux, OS X and Windows) just follow the steps under 'perform
Gitian builds' in [release-process.md](release-process#setup-and-perform-gitian-builds).

This may take some time as it will build all the dependencies needed for each descriptor.
These dependencies will be cached after a successful build to avoid rebuilding them when possible.

At any time you can check the package installation and build progress with

```bash
tail -f gbuilder/var/install.log
tail -f gbuilder/var/build.log
```

Output from `gbuild` will look something like

    Initialized empty Git repository in /home/gitianuser/gitian-builder/inputs/bytz/.git/
    remote: Counting objects: 57959, done.
    remote: Total 57959 (delta 0), reused 0 (delta 0), pack-reused 57958
    Receiving objects: 100% (57959/57959), 53.76 MiB | 484.00 KiB/s, done.
    Resolving deltas: 100% (41590/41590), done.
    From https://github.com/bytzcurrency/bytz
    ... (new tags, new branch etc)
    --- Building for bionic amd64 ---
    Stopping target if it is up
    Making a new image copy
    stdin: is not a tty
    Starting target
    Checking if target is up
    Preparing build environment
    Updating apt-get repository (log in var/install.log)
    Installing additional packages (log in var/install.log)
    Grabbing package manifest
    stdin: is not a tty
    Creating build script (var/build-script)
    lxc-start: Connection refused - inotify event with no name (mask 32768)
    Running build script (log in var/build.log)

## Building an alternative repository

If you want to do a test build of a pull on GitHub it can be useful to point
the Gitian builder at an alternative repository, using the same descriptors
and inputs.

For example:
```bash
URL=https://github.com/bytzcurrency/bytz/bitcoin.git
COMMIT=2014_03_windows_unicode_path
./bin/gbuild --commit bytz=${COMMIT} --url bytz=${URL} ../bitcoin/contrib/gitian-descriptors/gitian-linux.yml
./bin/gbuild --commit bytz=${COMMIT} --url bytz=${URL} ../bitcoin/contrib/gitian-descriptors/gitian-win.yml
./bin/gbuild --commit bytz=${COMMIT} --url bytz=${URL} ../bitcoin/contrib/gitian-descriptors/gitian-osx.yml
```
Using gitian-build.py
```bash
bytz/contrib/gitian-build.py -u <URL> -b {-c} <signer> <commit|tag> -j<num cpus> -m<memory to use> -o<architectures to build>
```
## Building fully offline

For building fully offline including attaching signatures to unsigned builds, the detached-sigs repository
and the bitcoin git repository with the desired tag must both be available locally, and then gbuild must be
told where to find them. It also requires an apt-cacher-ng which is fully-populated but set to offline mode, or
manually disabling gitian-builder's use of apt-get to update the VM build environment.

To configure apt-cacher-ng as an offline cacher, you will need to first populate its cache with the relevant
files. You must additionally patch target-bin/bootstrap-fixup to set its apt sources to something other than
plain archive.ubuntu.com: us.archive.ubuntu.com works.

So, if you use LXC:

```bash
export PATH="$PATH":/path/to/gitian-builder/libexec
export USE_LXC=1
export LXC_BRIDGE="lxcbr0"

cd /path/to/gitian-builder
./libexec/make-clean-vm --suite bionic --arch amd64

LXC_ARCH=amd64 LXC_SUITE=bionic on-target -u root apt-get update
LXC_ARCH=amd64 LXC_SUITE=bionic on-target -u root \
  -e DEBIAN_FRONTEND=noninteractive apt-get --no-install-recommends -y install \
  $( sed -ne '/^packages:/,/[^-] .*/ {/^- .*/{s/"//g;s/- //;p}}' ../bytz/contrib/gitian-descriptors/*|sort|uniq )
LXC_ARCH=amd64 LXC_SUITE=bionic on-target -u root apt-get -q -y purge grub
LXC_ARCH=amd64 LXC_SUITE=bionic on-target -u root -e DEBIAN_FRONTEND=noninteractive apt-get -y dist-upgrade
```

And then set offline mode for apt-cacher-ng:

```
/etc/apt-cacher-ng/acng.conf
[...]
Offlinemode: 1
[...]

service apt-cacher-ng restart
```

Then when building, override the remote URLs that gbuild would otherwise pull from the Gitian descriptors::
```bash

cd /some/root/path/
git clone https://github.com/bytzcurrency/bytz-detached-sigs.git

BTCPATH=/some/root/path/bitcoin
SIGPATH=/some/root/path/bitcoin-detached-sigs

./bin/gbuild --url bytz=${BTCPATH},signature=${SIGPATH} ../bitcoin/contrib/gitian-descriptors/gitian-win-signer.yml
```

## Signing externally

If you want to do the PGP signing on another device, that's also possible; just define `SIGNER` as mentioned
and follow the steps in the build process as normal.

    gpg: skipped "bytzkey": secret key not available

When you execute `gsign` you will get an error from GPG, which can be ignored. Copy the resulting `.assert` files
in `gitian.sigs` to your signing machine and do

```bash
    gpg --detach-sign ${VERSION}-linux/${SIGNER}/bitcoin-linux-build.assert
    gpg --detach-sign ${VERSION}-win/${SIGNER}/bitcoin-win-build.assert
    gpg --detach-sign ${VERSION}-osx-unsigned/${SIGNER}/bitcoin-osx-build.assert
```

This will create the `.sig` files that can be committed together with the `.assert` files to assert your
Gitian build.

## Uploading signatures

After building and signing you can push your signatures (both the `.assert` and `.assert.sig` files) to the
[bytzcurrency/gitian.sigs](https://github.com/bytzcurrency/gitian.sigs/) repository, or if that's not possible create a pull
request.
