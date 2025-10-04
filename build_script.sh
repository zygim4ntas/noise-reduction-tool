#!/bin/bash

# ProNoiseDAW Build Script
# Compiles the noise reduction tool with all dependencies

echo "🔨 Building ProNoiseDAW..."

# Check if source file exists
if [ ! -f "noise-reduction-tool.cpp" ]; then
    echo "❌ Error: noise-reduction-tool.cpp not found!"
    exit 1
fi

# Check for required ImGui files
REQUIRED_FILES=(
    "imgui.cpp"
    "imgui_draw.cpp"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "imgui_impl_sdl2.cpp"
    "imgui_impl_opengl3.cpp"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Error: $file not found!"
        echo "Please download ImGui and copy the required files."
        exit 1
    fi
done

# Compile
g++ -o ProNoiseDAW noise-reduction-tool.cpp \
    imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp \
    imgui_impl_sdl2.cpp imgui_impl_opengl3.cpp \
    -lportaudio -lrnnoise -lSDL2 -lGL -ldl -lpthread \
    -I. -I/usr/include/SDL2 \
    -std=c++17 -O2 -Wall

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "Run with: ./ProNoiseDAW"
else
    echo "❌ Build failed!"
    exit 1
fi
