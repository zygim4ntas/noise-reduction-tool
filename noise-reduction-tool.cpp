// neon_noise_reduction.cpp - IMPROVED VERSION
#include <iostream>
#include <vector>
#include <atomic>
#include <string>
#include <cmath>
#include <portaudio.h>
#include "rnnoise.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 480

// Atomic float for thread-safe slider
std::atomic<float> reductionStrength(1.0f);
std::atomic<float> inputLevel(0.0f);
std::atomic<float> outputLevel(0.0f);

// Audio level history for visualization
constexpr int HISTORY_SIZE = 100;
float inputHistory[HISTORY_SIZE] = {0};
float outputHistory[HISTORY_SIZE] = {0};
int historyIndex = 0;

// RNNoise wrapper
struct RNNoiseProcessor {
    DenoiseState* st;
    RNNoiseProcessor() { st = rnnoise_create(nullptr); }
    ~RNNoiseProcessor() { 
        if (st) rnnoise_destroy(st); 
    }

    void process(float* frame, int frame_size, float strength) {
        if (!st) return; // Safety check
        std::vector<float> tmp(frame, frame + frame_size);
        rnnoise_process_frame(st, frame, tmp.data());
        for (int i = 0; i < frame_size; ++i)
            frame[i] = tmp[i] * strength + frame[i] * (1.0f - strength);
    }
};

// PortAudio callback
static int paCallback(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void* userData) {
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;
    RNNoiseProcessor* proc = (RNNoiseProcessor*)userData;

    if (!inputBuffer || !outputBuffer || !proc) {
        if (outputBuffer) {
            for (unsigned long i = 0; i < framesPerBuffer; ++i)
                out[i] = 0.0f;
        }
        return paContinue;
    }

    float strength = reductionStrength.load();
    
    // Calculate input RMS level
    float inRMS = 0.0f;
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        inRMS += in[i] * in[i];
    }
    inRMS = sqrtf(inRMS / framesPerBuffer);
    inputLevel.store(inRMS);
    
    // RNNoise expects exactly 480 samples (10ms at 48kHz)
    if (framesPerBuffer == FRAMES_PER_BUFFER) {
        // Process the entire frame at once
        std::vector<float> tmp(in, in + framesPerBuffer);
        rnnoise_process_frame(proc->st, out, tmp.data());
        
        // Apply strength mixing
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            out[i] = out[i] * strength + in[i] * (1.0f - strength);
        }
    } else {
        // Fallback: just pass through if frame size doesn't match
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            out[i] = in[i];
        }
    }
    
    // Calculate output RMS level
    float outRMS = 0.0f;
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        outRMS += out[i] * out[i];
    }
    outRMS = sqrtf(outRMS / framesPerBuffer);
    outputLevel.store(outRMS);

    return paContinue;
}

// Modern Glassmorphic Style
void ApplyModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Rounded corners everywhere
    style.WindowRounding = 16.f;
    style.ChildRounding = 12.f;
    style.FrameRounding = 8.f;
    style.PopupRounding = 8.f;
    style.ScrollbarRounding = 12.f;
    style.GrabRounding = 8.f;
    style.TabRounding = 8.f;
    
    // Spacing and padding
    style.WindowPadding = ImVec2(20, 20);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(12, 10);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 12.0f;
    
    // Borders
    style.WindowBorderSize = 0.f;
    style.ChildBorderSize = 0.f;
    style.FrameBorderSize = 0.f;
    
    ImVec4* colors = style.Colors;
    
    // Dark glassmorphic background
    colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.05f, 0.92f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.5f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.02f, 0.02f, 0.05f, 0.95f);
    
    // Title bar - invisible for modern look
    colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Frames and inputs - glassmorphic
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.22f, 0.4f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.30f, 0.45f, 0.5f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.40f, 0.60f, 0.6f);
    
    // Buttons - vibrant gradient feel
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.50f, 0.6f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.40f, 0.70f, 0.8f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.50f, 0.90f, 1.0f);
    
    // Sliders - cyan/blue gradient
    colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.70f, 1.00f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.85f, 1.00f, 1.0f);
    
    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.40f, 0.5f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.40f, 0.60f, 0.7f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.45f, 0.70f, 0.8f);
    
    // Text
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
    
    // Checkmarks and borders
    colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.85f, 1.00f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.35f, 0.50f, 0.3f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.0f);
}

int main(int argc, char* argv[]) {
    // SDL Init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL Init error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("ProNoiseDAW",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        std::cerr << "Window creation error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "OpenGL context error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    ApplyModernStyle();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    RNNoiseProcessor processor;

    // PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio init error: " << Pa_GetErrorText(err) << std::endl;
        // Continue without audio - show GUI with error message
    }

    PaStream* stream = nullptr;
    bool audioEnabled = false;

    if (err == paNoError) {
        // Enumerate and display all devices
        int numDevices = Pa_GetDeviceCount();
        std::cout << "Available audio devices: " << numDevices << std::endl;
        
        int inputDevice = -1;
        int outputDevice = -1;
        
        // First pass: look for PipeWire/PulseAudio/default devices (most reliable)
        for (int i = 0; i < numDevices; ++i) {
            const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
            if (info) {
                std::cout << "Device " << i << ": " << info->name 
                         << " (In: " << info->maxInputChannels 
                         << ", Out: " << info->maxOutputChannels << ")" << std::endl;
                
                std::string name = info->name;
                bool isPreferred = (name.find("pipewire") != std::string::npos ||
                                   name.find("pulse") != std::string::npos ||
                                   name.find("default") != std::string::npos);
                
                if (isPreferred && inputDevice == -1 && info->maxInputChannels > 0)
                    inputDevice = i;
                if (isPreferred && outputDevice == -1 && info->maxOutputChannels > 0)
                    outputDevice = i;
            }
        }
        
        // Second pass: fallback to any device if preferred not found
        if (inputDevice == -1 || outputDevice == -1) {
            for (int i = 0; i < numDevices; ++i) {
                const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
                if (info) {
                    if (inputDevice == -1 && info->maxInputChannels > 0)
                        inputDevice = i;
                    if (outputDevice == -1 && info->maxOutputChannels > 0)
                        outputDevice = i;
                }
            }
        }

        if (inputDevice != -1 && outputDevice != -1) {
            std::cout << "Using input device: " << inputDevice << std::endl;
            std::cout << "Using output device: " << outputDevice << std::endl;

            PaStreamParameters inputParams, outputParams;

            inputParams.device = inputDevice;
            inputParams.channelCount = 1;
            inputParams.sampleFormat = paFloat32;
            inputParams.suggestedLatency = Pa_GetDeviceInfo(inputDevice)->defaultLowInputLatency;
            inputParams.hostApiSpecificStreamInfo = nullptr;

            outputParams.device = outputDevice;
            outputParams.channelCount = 1;
            outputParams.sampleFormat = paFloat32;
            outputParams.suggestedLatency = Pa_GetDeviceInfo(outputDevice)->defaultLowOutputLatency;
            outputParams.hostApiSpecificStreamInfo = nullptr;

            err = Pa_OpenStream(&stream, &inputParams, &outputParams,
                                SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff,
                                paCallback, &processor);
            
            if (err == paNoError) {
                err = Pa_StartStream(stream);
                if (err == paNoError) {
                    audioEnabled = true;
                    std::cout << "Audio stream started successfully!" << std::endl;
                } else {
                    std::cerr << "PortAudio start stream error: " << Pa_GetErrorText(err) << std::endl;
                }
            } else {
                std::cerr << "PortAudio open stream error: " << Pa_GetErrorText(err) << std::endl;
            }
        } else {
            std::cerr << "No valid input/output devices found!" << std::endl;
        }
    }

    bool running = true;
    int windowWidth = 900, windowHeight = 600;

    std::cout << "Entering main loop..." << std::endl;

    // Main loop
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) 
                running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Modern centered window
        ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.5f, windowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_Always);
        
        ImGui::Begin("##MainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        
        // Header with icon-style text
        ImGui::PushFont(io.Fonts->Fonts[0]);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("ProNoiseDAW").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "ProNoiseDAW");
        ImGui::PopFont();
        
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("AI-Powered Noise Reduction").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.8f, 1.0f), "AI-Powered Noise Reduction");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Status indicator with colored dot
        ImGui::BeginChild("##StatusCard", ImVec2(0, 80), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::Spacing();
        
        if (audioEnabled) {
            // Animated green dot
            float pulse = 0.7f + 0.3f * sinf(ImGui::GetTime() * 3.0f);
            ImVec2 dotPos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(dotPos.x + 15, dotPos.y + 10), 
                6.0f, 
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 1.0f, 0.4f, pulse))
            );
            ImGui::SetCursorPosX(35);
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Active");
            ImGui::SetCursorPosX(35);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 1.0f), "Real-time processing enabled");
        } else {
            ImVec2 dotPos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(dotPos.x + 15, dotPos.y + 10), 
                6.0f, 
                ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.3f, 0.3f, 1.0f))
            );
            ImGui::SetCursorPosX(35);
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Inactive");
            ImGui::SetCursorPosX(35);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.7f, 1.0f), "Audio initialization failed");
        }
        
        ImGui::Spacing();
        ImGui::EndChild();
        
        ImGui::Spacing();
        
        if (audioEnabled) {
            // Reduction strength control
            ImGui::BeginChild("##ControlCard", ImVec2(0, 120), true, ImGuiWindowFlags_NoScrollbar);
            ImGui::Spacing();
            
            ImGui::Text("Noise Reduction");
            ImGui::Spacing();
            
            float tmpStrength = reductionStrength.load();
            ImGui::SetNextItemWidth(-1);
            
            if (ImGui::SliderFloat("##strength", &tmpStrength, 0.0f, 1.0f, "")) {
                reductionStrength.store(tmpStrength);
            }
            
            // Percentage display
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("100%").x) * 0.5f);
            char percentage[32];
            snprintf(percentage, sizeof(percentage), "%.0f%%", tmpStrength * 100.0f);
            ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "%s", percentage);
            
            ImGui::Spacing();
            ImGui::EndChild();
            
            ImGui::Spacing();
            
            // Audio visualization
            ImGui::BeginChild("##VisualizerCard", ImVec2(0, 280), true, ImGuiWindowFlags_NoScrollbar);
            ImGui::Spacing();
            
            ImGui::Text("Audio Levels");
            ImGui::Spacing();
            
            // Update history
            float inLevel = inputLevel.load();
            float outLevel = outputLevel.load();
            inputHistory[historyIndex] = inLevel * 10.0f; // Scale for visibility
            outputHistory[historyIndex] = outLevel * 10.0f;
            historyIndex = (historyIndex + 1) % HISTORY_SIZE;
            
            // Input waveform
            ImGui::Text("Input");
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.12f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::PlotLines("##InputWave", inputHistory, HISTORY_SIZE, historyIndex, 
                            nullptr, 0.0f, 1.0f, ImVec2(-1, 60));
            ImGui::PopStyleColor(2);
            
            ImGui::Spacing();
            
            // Output waveform
            ImGui::Text("Output (Processed)");
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.12f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.2f, 1.0f, 0.4f, 1.0f));
            ImGui::PlotLines("##OutputWave", outputHistory, HISTORY_SIZE, historyIndex, 
                            nullptr, 0.0f, 1.0f, ImVec2(-1, 60));
            ImGui::PopStyleColor(2);
            
            ImGui::Spacing();
            
            // Level meters as progress bars
            ImGui::Text("Live Meters");
            ImGui::Spacing();
            
            ImGui::Text("IN ");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.4f, 0.4f, 0.8f));
            ImGui::ProgressBar(inLevel * 5.0f, ImVec2(-1, 20), "");
            ImGui::PopStyleColor();
            
            ImGui::Text("OUT");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 1.0f, 0.4f, 0.8f));
            ImGui::ProgressBar(outLevel * 5.0f, ImVec2(-1, 20), "");
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::EndChild();
        }
        
        ImGui::Spacing();
        
        // Quit button - full width, modern style
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.22f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.35f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.30f, 0.40f, 1.0f));
        
        if (ImGui::Button("Exit", ImVec2(-1, 40))) {
            running = false;
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, windowWidth, windowHeight);
        
        // Gradient background
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }
    Pa_Terminate();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}