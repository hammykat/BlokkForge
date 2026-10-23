//
// Created by berke on 10/17/2026.
//

#include "Sound.hpp"

#include <algorithm>
#include <unordered_map>
#include <ranges>
#include <filesystem>
#include <cctype>

#include "SDL3/SDL_audio.h"
#include "SDL3/SDL_stdinc.h"

namespace {
    namespace fs = std::filesystem;

    ALCdevice* device = nullptr;
    ALCcontext* context = nullptr;

    std::unordered_map<std::string, ALuint> sources;
    std::unordered_map<std::string, ALuint> buffers;

    bool CheckALErrors(const char* message) {
        const ALenum error = alGetError();

        if (error != AL_NO_ERROR) {
            // error("{}, OpenAL error code: {}", message, error);

            return false;
        }

        return true;
    }

    // IMPORTANT, USES SDL
    ALenum GetOpenALFormat(const SDL_AudioSpec& spec) {
        if (spec.channels == 1) {
            if (spec.format == SDL_AUDIO_U8) return AL_FORMAT_MONO8;
            if (spec.format == SDL_AUDIO_S16LE) return AL_FORMAT_MONO16;
        }
        else if (spec.channels == 2) {
            if (spec.format == SDL_AUDIO_U8) return AL_FORMAT_STEREO8;
            if (spec.format == SDL_AUDIO_S16LE) return AL_FORMAT_STEREO16;
        }

        return 0;
    }

    // Add more in the future when more file types are supported
    bool IsSupportedSoundExtension(std::string extension) {
        std::ranges::transform(
            extension,
            extension.begin(),
            [](const unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return extension == ".wav";
    }

    // Stripping means "Sounds/Doors/open" and
    // "Sounds/Doors/open.wav" both find the sound.
    std::string NormalizeSoundFileName(const std::string& fileName) {
        if (fileName.empty()) return {};
        return fs::path(fileName).lexically_normal().replace_extension().generic_string();
    }
}

namespace Sound {
    bool Init() {
        if (device != nullptr) {
            // spdlog::warn("Sound::Init called twice");

            return false; // or true if you think its not an error
        }
        // Find a device, let OpenAL pick the best one
        device = alcOpenDevice(nullptr);

        if (device == nullptr) {
            // LOG ERROR, DEVICE NOT FOUND
            return false;
        }

        context = alcCreateContext(device, nullptr);
        if (context == nullptr) {
            // LOG ERROR, CONTEXT NOT FOUND

            alcCloseDevice(device);
            device = nullptr;

            return false;
        }

        if (!alcMakeContextCurrent(context)) {
            // LOG ERROR, COULDNT BIND CONTEXT

            alcDestroyContext(context);
            alcCloseDevice(device);

            context = nullptr;
            device = nullptr;

            return false;
        }

        // SUCCESFULY INITIALIZED

        buffers.clear();
        sources.clear();

        return true;
    }

    void Destroy() {
        for (ALuint source : sources | std::views::values) {
            alSourceStop(source);
            alDeleteSources(1, &source);
        }

        sources.clear();

        for (auto& [fileName, buffer] : buffers) {
            if (buffer == 0) continue;
            alDeleteBuffers(1, &buffer);
            buffer = 0;
        }

        buffers.clear();

        if (context != nullptr) {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
            context = nullptr;
        }

        if (device != nullptr) {
            alcCloseDevice(device);
            device = nullptr;
        }

        // spdlog::info("OpenAL destroyed");
    }

    bool CreateSource(const std::string& name) {
        if (sources.contains(name)) return true;

        ALuint source = 0;
        alGenSources(1, &source);

        if (!CheckALErrors("alGenSources failed")) {
            // spdlog::error("Failed to create OpenAL source '{}'", name);

            return false;
        }

        // todo BLOKKTODO set different default if needed
        alSourcef(source, AL_GAIN, 1.0f);
        alSourcef(source, AL_PITCH, 1.0f);
        alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        alSourcei(source, AL_LOOPING, AL_FALSE);

        if (!CheckALErrors("Failed to configure OpenAL source")) {
            alDeleteSources(1, &source);
            return false;
        }

        sources[name] = source;

        // spdlog::info("Created OpenAL source '{}'", name);

        return true;
    }

    // IMPORTANT, USES SDL
    bool LoadWAVToOpenALBuffer(const char* path, ALuint& buffer) {
        SDL_AudioSpec wavSpec{};
        Uint8* wavData = nullptr;
        Uint32 wavLength = 0;

        if (!SDL_LoadWAV(path, &wavSpec, &wavData, &wavLength)) {
            // spdlog::error("SDL_LoadWAV failed for {}: {}", path, SDL_GetError());
            return false;
        }

        const ALenum format = GetOpenALFormat(wavSpec);

        if (format == 0) {
            // spdlog::error(
            //     "Unsupported WAV format. Channels: {}, Format: {}. "
            //     "Use 8-bit or 16-bit PCM mono/stereo WAV",
            //     wavSpec.channels,
            //     static_cast<int>(wavSpec.format)
            // );

            SDL_free(wavData);
            return false;
        }

        alGenBuffers(1, &buffer);
        if (!CheckALErrors("alGenBuffers failed")) {
            SDL_free(wavData);
            return false;
        }

        alBufferData(buffer, format, wavData, static_cast<ALsizei>(wavLength), wavSpec.freq);

        SDL_free(wavData);

        if (!CheckALErrors("alBufferData failed")) {
            alDeleteBuffers(1, &buffer);
            buffer = 0;
            return false;
        }

        // spdlog::info("Loaded WAV into OpenAL buffer: {}", path);
        return true;
    }

    bool GenerateSoundFromFile(const std::string& filePath) {
        fs::path file(filePath);
        const std::string key = NormalizeSoundFileName(filePath);

        if (buffers.contains(key)) {
            // Ignore duplicate file names

            return true; // Or false if duplicates are an issue
        }

        if (buffers.contains(file.string())) {
            // IGNORE DUPLICATE FILENAMES

            return true; // or false if duplicates arent an issue
        }

        // Check if the file exists and is a regular file
        if (!(fs::exists(file) && fs::is_regular_file(file))) {
            //LOG ERROR, INVALID FILE

            return false;
        }

        if (!IsSupportedSoundExtension(file.extension().string())) {
            // LOG ERROR, UNSUPPORTED FORMAT

            return false;
        }

        ALuint buffer = 0;

        if (!LoadWAVToOpenALBuffer(file.string().c_str(), buffer)) {
            // spdlog::error("Failed to load sound '{}'", fileName);
            return false;
        }
        buffers.emplace(key, buffer);
        // spdlog::info("Loaded sound '{}'", fileName);

        // spdlog::info("Generated {} sound buffer(s), {} reused unchanged", buffers.size(), reusedCount);

        return true;
    }

    void DestroySource(const std::string& sourceName) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::warn("Tried to destroy missing source: '{}'", sourceName);

            return;
        }

        const ALuint source = iterator->second;

        alSourceStop(source);
        alDeleteSources(1, &source);

        sources.erase(iterator);

        // spdlog::info("Destroyed OpenAL source '{}'", sourceName);
    }

    bool IsSourcePlaying(const std::string& sourceName) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) return false;

        ALint state = 0;
        alGetSourcei(iterator->second, AL_SOURCE_STATE, &state);

        CheckALErrors("Failed to get source state");

        return state == AL_PLAYING;
    }

    void PlaySoundOnSource(const std::string& sourceName, const std::string& soundFileName) {
        const auto sourceIterator = sources.find(sourceName);

        if (sourceIterator == sources.end()) {
           //  spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        const std::string normalizedFileName = NormalizeSoundFileName(soundFileName);
        const auto bufferIterator = buffers.find(normalizedFileName);

        if (bufferIterator == buffers.end()) {
           // spdlog::error("Sound not found: '{}'", soundFileName);
            return;
        }

        const ALuint buffer = bufferIterator->second;

        if (buffer == 0) {
            // spdlog::error("Sound '{}' has no valid OpenAL buffer", soundFileName);
            return;
        }

        const ALuint source = sourceIterator->second;

        alSourceStop(source);
        alSourcei(source, AL_BUFFER, static_cast<ALint>(buffer));
        alSourcePlay(source);

        CheckALErrors("Failed to play sound on source");
        // spdlog::info("Played sound '{}' on source '{}'", soundFileName, sourceName);
    }

    void PlaySoundOnSourceIfNotPlaying(const std::string& sourceName, const std::string& soundFileName) {
        if (IsSourcePlaying(sourceName)) return;
        PlaySoundOnSource(sourceName, soundFileName);
    }

    void StopSource(const std::string& sourceName) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
          //  spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourceStop(iterator->second);
        CheckALErrors("Failed to stop source");
    }

    void PauseSource(const std::string& sourceName) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcePause(iterator->second);
        CheckALErrors("Failed to pause source");
    }

    bool ResumeSource(const std::string& sourceName) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
           // spdlog::error("Source not found: '{}'", sourceName);
            return false;
        }

        ALint state = 0;
        alGetSourcei(iterator->second, AL_SOURCE_STATE, &state);

        // alSourcePlay on a stopped source would restart it, which is Play's job.
        if (state != AL_PAUSED) return false;

        alSourcePlay(iterator->second);
        return CheckALErrors("Failed to resume source");
    }

    void BeginSession() {
        // Picks up sounds added or edited since the last session.
        // todo BLOKKTODO loop through all sound files and Call GenerateSound(), since I don't know what kind of Asset folder structure
        // the engine will use, I couldn't implement it myself
        // GenerateSound();

        // These values should be overwritten by the level start anyways but put different values if needed
        SetListenerPosition({0.0f, 0.0f, 0.0f});
        SetListenerVelocity({0.0f, 0.0f, 0.0f});
        SetListenerOrientation({0.0f, 0.0f, 1.0f});
    }

    void EndSession() {
        for (const ALuint source : sources | std::views::values) {
            alSourceStop(source);
            alDeleteSources(1, &source);
        }

        sources.clear();
    }

    // ====================================
    // Setters
    // ====================================

    void SetSourcePitch(const std::string& sourceName,const float pitch) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second, AL_PITCH, pitch);
        CheckALErrors("Failed to set source pitch");
    }

    void SetSourceGain(const std::string& sourceName, const float gain) {
        const auto iterator =sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second,AL_GAIN, gain);
        CheckALErrors("Failed to set source gain");
    }

    void SetSourcePosition(const std::string& sourceName, const Vector3 &pos) {
        const auto iterator =sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSource3f(iterator->second, AL_POSITION, pos.x, pos.y, pos.z);
        CheckALErrors("Failed to set source position");
    }

    void SetSourceDirection(const std::string& sourceName, const Vector3 &direction) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSource3f(iterator->second, AL_DIRECTION, direction.x, direction.y, direction.z);
        CheckALErrors("Failed to set source direction");
    }

    void SetSourceLooping(const std::string& sourceName, const bool looping) {
        const auto iterator =sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcei(iterator->second, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
        CheckALErrors("Failed to set source looping");
    }

    void SetSourceReferenceDistance(const std::string& sourceName, const float distance) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second, AL_REFERENCE_DISTANCE, distance);
        CheckALErrors("Failed to set AL_REFERENCE_DISTANCE");
    }

    void SetSourceMaxDistance(const std::string& sourceName, const float maxDistance) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
           // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second, AL_MAX_DISTANCE, maxDistance);
        CheckALErrors("Failed to set AL_MAX_DISTANCE");
    }

    void SetSourceRollOffFactor(const std::string& sourceName, const float rollOffFactor) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second, AL_ROLLOFF_FACTOR, rollOffFactor);
        CheckALErrors("Failed to set AL_ROLLOFF_FACTOR");
    }

    void SetSourceInnerConeAngle(const std::string& sourceName,const float innerConeAngle) {
        const auto iterator =sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second, AL_CONE_INNER_ANGLE, std::clamp(innerConeAngle, 0.0f, 360.0f));
        CheckALErrors("Failed to set AL_CONE_INNER_ANGLE");
    }

    void SetSourceOuterConeAngle(const std::string& sourceName, const float outerConeAngle) {
        const auto iterator =sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second, AL_CONE_OUTER_ANGLE, std::clamp(outerConeAngle, 0.0f, 360.0f));
        CheckALErrors("Failed to set AL_CONE_OUTER_ANGLE");
    }

    void SetSourceOuterGain(const std::string& sourceName, const float outerGain) {
        const auto iterator = sources.find(sourceName);

        if (iterator == sources.end()) {
            // spdlog::error("Source not found: '{}'", sourceName);
            return;
        }

        alSourcef(iterator->second, AL_CONE_OUTER_GAIN, std::clamp(outerGain, 0.0f, 1.0f));
        CheckALErrors("Failed to set AL_CONE_OUTER_GAIN");
    }

    void SetListenerPosition(const Vector3& pos) {
        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
        CheckALErrors("Failed to set listener position");
    }

    void SetListenerVelocity(const Vector3 &velocity) {
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
        CheckALErrors("Failed to set listener velocity");
    }

    void houSetListenerOrientation(const Vector3& forward) {
        //todo BLOKKTODO right now this hardcodes "up" as forward, change it if the engine uses a different convention
        const ALfloat orientation[] = {
            forward.x,
            forward.y,
            forward.z,

            0.0f,
            1.0f,
            0.0f
        };

        alListenerfv(AL_ORIENTATION, orientation);

        CheckALErrors("Failed to set listener orientation");
    }

    void SetListenerGain(const float gain) {
        alListenerf(AL_GAIN, gain);
        CheckALErrors("Failed to set listener gain");
    }

    void SetListenerDistanceModel(const ALenum model) {
        alDistanceModel(model);
        CheckALErrors("Failed to set distance model");
    }

    void SetListenerDopplerFactor(const float factor) {
        alDopplerFactor(factor);
        CheckALErrors("Failed to set Doppler factor");
    }

    void SetListenerSpeedOfSound(const float speed) {
        alSpeedOfSound(speed);
        CheckALErrors("Failed to set speed of sound");
    }
}
