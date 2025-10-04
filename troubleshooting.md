# Troubleshooting Guide

Common issues and solutions for ProNoiseDAW on Linux.

## Table of Contents
- [Audio Issues](#audio-issues)
- [Compilation Problems](#compilation-problems)
- [GUI Issues](#gui-issues)
- [Performance Problems](#performance-problems)

---

## Audio Issues

### 🔴 Segmentation Fault After "Audio stream started successfully!"

**Symptoms:**
```
Using input device: 0
Using output device: 0
Audio stream started successfully!
Segmentation fault
```

**Root Cause:** Hardware ALSA devices (like `hw:0,1`) may not support the exact audio format ProNoiseDAW requires (48kHz, float32, mono).

**Solution 1: Let the app choose PipeWire/Pulse automatically**

The latest version automatically prefers `pipewire`, `pulse`, or `default` devices. Make sure you're using the updated code that includes this logic.

**Solution 2: Manually check which device is being selected**

Run the app and look at the device list:
```
Available audio devices: 7
Device 0: HDA Intel PCH: CS4206 Digital (hw:0,1) (In: 2, Out: 2)
Device 3: pipewire (In: 64, Out: 64)
Device 6: default (In: 64, Out: 64)
```

If it's using Device 0 (hardware), update your code to prefer Device 3 or 6.

**Solution 3: Force PipeWire device**

Modify the device selection code to always use `pipewire` or `default`:
```cpp
// Look for the specific device name
if (std::string(info->name).find("pipewire") != std::string::npos) {
    inputDevice = i;
    outputDevice = i;
    break;
}
```

---

### 🟡 ALSA/JACK Error Messages at Startup

**Symptoms:**
```
ALSA lib pcm_dmix.c:1000:(snd_pcm_dmix_open) unable to open slave
Cannot connect to server socket err = No such file or directory
jack server is not running or cannot be started
```

**Root Cause:** PortAudio probes all available audio backends (ALSA, JACK, OSS, PipeWire) on startup.

**Solution:** **These messages are NORMAL and can be safely ignored!** They indicate PortAudio is trying different audio systems. As long as you see "Audio stream started successfully!" afterward, everything is working.

**To reduce noise (optional):**
Set the PortAudio environment variable:
```bash
export PA_ALSA_PLUGHW=1
./ProNoiseDAW
```

---

### 🔴 "No valid input/output devices found!"

**Symptoms:**
```
Available audio devices: 0
No valid input/output devices found!
```

**Root Cause:** PipeWire isn't running or PortAudio can't detect devices.

**Solution 1: Start PipeWire**
```bash
# Check status
systemctl --user status pipewire pipewire-pulse

# If not running
systemctl --user start pipewire pipewire-pulse wireplumber

# Enable on boot
systemctl --user enable pipewire pipewire-pulse wireplumber
```

**Solution 2: Add user to audio group**
```bash
sudo usermod -a -G audio $USER
# Log out and back in for changes to take effect
```

**Solution 3: Install PipeWire utilities**
```bash
# Debian/Ubuntu
sudo apt install pipewire pipewire-pulse pipewire-alsa

# Arch
sudo pacman -S pipewire pipewire-pulse pipewire-alsa
```

**Solution 4: Check audio devices exist**
```bash
# With pactl (if available)
pactl list short sinks
pactl list short sources

# With PipeWire
pw-cli list-objects | grep node.name

# With ALSA
aplay -l    # Playback devices
arecord -l  # Recording devices
```

---

### 🟡 Audio is Choppy/Stuttering

**Symptoms:** Audio has glitches, pops, or interruptions.

**Root Cause:** Buffer too small, CPU too slow, or system under load.

**Solution 1: Increase buffer size**

Edit `noise-reduction-tool.cpp`:
```cpp
// Change this line
#define FRAMES_PER_BUFFER 480

// To a larger value
#define FRAMES_PER_BUFFER 960   // 20ms latency
// or
#define FRAMES_PER_BUFFER 1920  // 40ms latency
```

Then recompile: `./build.sh`

**Solution 2: Check CPU usage**
```bash
top
# Look for ProNoiseDAW process
# RNNoise is CPU-intensive, should use 20-40% on one core
```

**Solution 3: Reduce system load**
- Close unnecessary applications
- Check for background processes
- Ensure CPU isn't thermal throttling

---

### 🔴 No Audio Output (Silent)

**Symptoms:** App runs, but no sound comes through.

**Solution 1: Check system volume**
```bash
# With pactl
pactl list sinks | grep -A10 "Name:"

# Ensure volume isn't muted
pactl set-sink-mute @DEFAULT_SINK@ 0
pactl set-sink-volume @DEFAULT_SINK@ 65536  # 100%
```

**Solution 2: Verify audio routing**

ProNoiseDAW processes audio from input → output. You need:
1. A working microphone/input device
2. A working speaker/output device
3. Both must be detected by the app

**Solution 3: Test with speaker-test**
```bash
speaker-test -t sine -f 1000 -c 2
# You should hear a test tone
```

---

## Compilation Problems

### 🔴 "cannot find -lrnnoise"

**Symptoms:**
```
/usr/bin/ld: cannot find -lrnnoise: No such file or directory
```

**Solution 1: Install RNNoise**
```bash
git clone https://github.com/xiph/rnnoise.git
cd rnnoise
./autogen.sh
./configure
make
sudo make install
sudo ldconfig
```

**Solution 2: Check library path**
```bash
# Find where librnnoise.so is installed
sudo find / -name "librnnoise*"

# If it's in /usr/local/lib but not found
sudo ldconfig /usr/local/lib
```

**Solution 3: Add to LD_LIBRARY_PATH**
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
./build.sh
```

---

### 🔴 "cannot find -limgui"

**Symptoms:**
```
/usr/bin/ld: cannot find -limgui
```

**Solution:** ImGui is **NOT** a library - it's source files!

You need to:
1. Download ImGui source
2. Copy the .cpp and .h files to your project directory
3. Compile them together

See the README installation steps for details.

---

### 🔴 "cannot find -lportaudio"

**Solution:**
```bash
# Debian/Ubuntu
sudo apt install libportaudio2 portaudio19-dev

# Arch
sudo pacman -S portaudio

# Verify
pkg-config --libs portaudio-2.0
```

---

### 🔴 "SDL.h: No such file or directory"

**Solution:**
```bash
# Debian/Ubuntu
sudo apt install libsdl2-dev

# Arch
sudo pacman -S sdl2

# Verify
pkg-config --cflags sdl2
```

---

### 🟡 Warnings About "unused variable"

**Solution:** These are harmless. To suppress:
```bash
g++ ... -Wno-unused-variable
```

---

## GUI Issues

### 🔴 No Window Appears

**Symptoms:** App runs but no GUI shows up.

**Solution 1: Check OpenGL**
```bash
glxinfo | grep "OpenGL version"
# Should show 3.3 or higher
```

**Solution 2: Install Mesa drivers**
```bash
# Debian/Ubuntu
sudo apt install libgl1-mesa-glx libgl1-mesa-dev mesa-utils

# Arch
sudo pacman -S mesa mesa-utils
```

**Solution 3: Check display**
```bash
echo $DISPLAY
# Should output something like :0 or :1

# If empty, you're not in a graphical session
```

**Solution 4: Try software rendering**
```bash
LIBGL_ALWAYS_SOFTWARE=1 ./ProNoiseDAW
```

---

### 🟡 Window is Tiny or Huge

**Solution:** The window should auto-center at 900x600. If it's wrong, modify:
```cpp
SDL_Window* window = SDL_CreateWindow("ProNoiseDAW",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    900, 600,  // Change these values
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
```

---

### 🟡 Text is Blurry

**Solution:** Enable font scaling in ImGui (requires custom font loading):
```cpp
io.FontGlobalScale = 1.5f;  // Adjust as needed
```

---

## Performance Problems

### 🔴 High CPU Usage (>50%)

**Expected:** RNNoise is CPU-intensive. 20-40% usage on one core is normal.

**If >80%:**
1. Increase buffer size (reduces processing frequency)
2. Reduce visualization update rate
3. Check for memory leaks with `valgrind`

---

### 🟡 Memory Leak

**Symptoms:** Memory usage grows over time.

**Solution 1: Check with valgrind**
```bash
valgrind --leak-check=full ./ProNoiseDAW
```

**Solution 2: Known issues**
- Make sure RNNoise state is properly destroyed
- Check ImGui context cleanup

---

## Platform-Specific Issues

### Debian 13 (Trixie)

**Known Issue:** Some audio configurations cause segfaults with hardware devices.

**Solution:** Use PipeWire/Pulse devices (automatically selected in updated code).

---

### Wayland vs X11

**Issue:** Some display servers have different behavior.

**Solution:** Set environment variable:
```bash
# Force X11
SDL_VIDEODRIVER=x11 ./ProNoiseDAW

# Force Wayland
SDL_VIDEODRIVER=wayland ./ProNoiseDAW
```

---

## Still Having Issues?

1. **Enable verbose output** - Add debug prints in the code
2. **Check logs** - Look at system logs: `journalctl -f`
3. **Test components individually:**
   - PortAudio: `pacat` or `parec`
   - RNNoise: Create a simple test program
   - ImGui: Run an ImGui example
4. **Open a GitHub issue** with:
   - Linux distribution and version
   - Complete console output
   - Output of `uname -a`
   - Steps to reproduce

---

## Useful Debugging Commands

```bash
# Check audio system
systemctl --user status pipewire pipewire-pulse

# List audio devices
pactl list short sinks
pactl list short sources

# Check libraries
ldd ./ProNoiseDAW

# Monitor audio
pw-top

# Check OpenGL
glxinfo | head -n 20

# Test microphone
arecord -f cd -d 5 test.wav && aplay test.wav
```