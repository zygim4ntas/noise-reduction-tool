## How to Contribute

### Reporting Bugs

If you find a bug, please open an issue with:
- **Description:** Clear summary of the problem
- **Environment:** Linux distro, version, audio system (PipeWire/Pulse)
- **Console Output:** Full error messages
- **Steps to Reproduce:** Detailed steps to trigger the bug
- **Expected vs Actual:** What should happen vs what actually happens

### Suggesting Features

Feature requests are welcome! Please include:
- **Use Case:** Why this feature would be useful
- **Description:** How it should work
- **Mockups:** Visual examples if applicable

### Pull Requests

1. **Fork the repository**
2. **Create a feature branch:** `git checkout -b feature/amazing-feature`
3. **Make your changes**
4. **Test thoroughly:** Ensure no regressions
5. **Commit:** Use clear, descriptive commit messages
6. **Push:** `git push origin feature/amazing-feature`
7. **Open a Pull Request**

## Development Setup

```bash
# Clone your fork
git clone https://github.com/yourusername/ProNoiseDAW.git
cd ProNoiseDAW

# Install dependencies
sudo apt install build-essential libportaudio2 portaudio19-dev \
    libsdl2-dev libgl1-mesa-dev pipewire pipewire-pulse

# Build RNNoise
git clone https://github.com/xiph/rnnoise.git
cd rnnoise && ./autogen.sh && ./configure && make && sudo make install
cd ..

# Download ImGui
wget https://github.com/ocornut/imgui/archive/refs/tags/v1.90.0.tar.gz
tar -xzf v1.90.0.tar.gz
cp imgui-1.90.0/*.{cpp,h} .
cp imgui-1.90.0/backends/imgui_impl_{sdl2,opengl3}.* .

# Build
./build.sh

# Test
./ProNoiseDAW
```

## Code Style

- **C++ Standard:** C++17
- **Indentation:** 4 spaces (no tabs)
- **Naming:**
  - Functions: `camelCase`
  - Variables: `camelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Classes: `PascalCase`
- **Comments:** Use `//` for single-line, `/* */` for multi-line
- **Braces:** Opening brace on same line

Example:
```cpp
void processAudio(float* buffer, int size) {
    for (int i = 0; i < size; ++i) {
        buffer[i] *= 0.5f;  // Reduce volume
    }
}
```

## Testing

Before submitting:
- [ ] Code compiles without warnings
- [ ] Application launches successfully
- [ ] Audio processing works correctly
- [ ] No memory leaks (`valgrind --leak-check=full ./ProNoiseDAW`)
- [ ] GUI renders properly
- [ ] No crashes or segfaults

## Areas for Contribution

### Easy (Good First Issues)
- [ ] Add keyboard shortcuts (Ctrl+Q to quit, etc.)
- [ ] Improve error messages
- [ ] Add tooltips to UI elements
- [ ] Color scheme customization

### Medium
- [ ] Spectral analyzer visualization
- [ ] Save/load preset configurations
- [ ] Multiple noise profiles
- [ ] Recording to WAV file
- [ ] Dark/light theme toggle

### Advanced
- [ ] VST3 plugin version
- [ ] Windows/macOS port
- [ ] Multi-channel support
- [ ] Real-time frequency analysis
- [ ] GPU acceleration for processing

## Questions?

Feel free to open a discussion or issue if you have any questions!

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
