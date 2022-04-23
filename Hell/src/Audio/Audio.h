#pragma once
#include "soloud/soloud.h"
#include "soloud/soloud_wav.h"
#include <unordered_map>

struct AudioFile {
	const char* name;
	SoLoud::Wav* audio;
};


class Audio
{
public: // functions
	static void Init();
	static void LoadAudio(const char* name);
	static SoLoud::handle PlayAudio(const char* name, float volume = 1.0f);
	static SoLoud::handle LoopAudio(const char* name, float volume = 1.0f);


	static void Terminate();

	static void PlayGlockFlesh();

public: // variables
	static SoLoud::Soloud gSoloud;
	static std::unordered_map<std::string, SoLoud::Wav*> s_loadedAudio;
};
