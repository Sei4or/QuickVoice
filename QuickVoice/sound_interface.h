#pragma once

#include <xaudio2.h>
#include <memory>
#include <unordered_map>

#define I_GOT_IT 0
#define NEED_BOOST 1
#define WHAT_A_SAVE 2
#define NICE_SHOT 3
#define THANKS 4
#define E 5

namespace SoundInterface
{
	HRESULT loadSound(short int soundId);
	HRESULT playSound(short int soundId);
	void preloadSounds();
	void unloadSounds();
	void unload();
}