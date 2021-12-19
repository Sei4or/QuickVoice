#pragma once

#include <xaudio2.h>
#include <memory>
#include <vector>

namespace SoundInterface
{
	class SoundManager;

	class SourceVoiceManager
	{
	public:
		SourceVoiceManager(SoundManager& soundManager);
		std::shared_ptr<IXAudio2SourceVoice> getReadySourceVoice();
	private:
		SoundManager& soundManager;
		std::vector<std::shared_ptr<IXAudio2SourceVoice>> sourceVoices;
		std::shared_ptr<std::deque<short int>> readySourceVoiceIndices;
		WAVEFORMATEXTENSIBLE wfx = {
			{ 1, 1, 22050, 44100, 2, 16, 0 }
		};
	};
}