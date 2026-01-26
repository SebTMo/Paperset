# Paperset
A Minimal GTK Wallpapermanager written in C (WIP) for GNOME systems
---
<img src="https://github.com/SebTMo/Paperset/blob/main/screenshot.png" alt="-" style="width: 20vw;" />

[![GitHub Repository Größe](https://img.shields.io/github/repo-size/SebTMo/Paperset)](https://github.com/SebTMo/Paperset)
[![GitHub letzer Commit](https://img.shields.io/github/last-commit/SebTMo/Paperset)](https://github.com/SebTMo/Paperset/commits/main)
[![Hauptsprache](https://img.shields.io/github/languages/top/SebTMo/Paperset)](https://github.com/SebTMo/Paperset)
[![Lizenz: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

##  About Paperset

**Paperset** started as a sideproject for a clean and minimal Wallpapermanager written in C for GNOME utilizing GTK. It's very much a Work in progress and more of a weekendproject that hopefully becomes something propper in the future.

---

## Features

-  Low resource usage
- completely offline
- Auto Scan of library
- Animated changing of Wallpaper
- (coming in the future) Widget function for quick changing of your Wallpaper

---

## Diclaimer
**I am not responsible for any damage to anyones system. This is an experimental app and support is limited. Try it with your own percautions.**

##  Installation
**Fedora** haven't tested on other distros yet.
For now i'm still focusing on the app itself so no installer is available.

### Dependencies
* **meson** to build the app
* **git** to clone this repo
* **GNOME** its a GTK application

### Install
Open your terminal and run:

```bash
git clone https://github.com/SebTMo/Paperset.git
cd Paperset
meson setup build
sudo ninja -C build
````
**The app is now installed**

### Compiling

Compile the files to an executable for one time use:

```bash
cd src/
gcc main.c -o paperset $(pkg-config --cflags --libs gtk4 libadwaita-1)
```

-----

## Usage

Running the executable:

```bash
./paperset
```

## Unistall

Return to the parent directory:

```bash
cd ~/Paperset/
sudo ninja -C build uninstall
cd ..
rm -r Paperset/
```

##  Contact me

**SebTech**

  * **GitHub:** [https://github.com/SebTMo](https://www.google.com/search?q=https://github.com/CROXY04)
  * **E-Mail:** [migrating]

-----
