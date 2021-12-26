#pragma once

#include <xaudio2.h>
#include <memory>
#include <vector>

namespace SoundInterface
{
	class SoundManager;
	class SourceVoiceCallback;

	class SourceVoiceManager
	{
	public:
		SourceVoiceManager(SoundManager& soundManager);
		IXAudio2SourceVoice* getReadySourceVoice();
		void unload();
	private:
		SoundManager& soundManager;
		std::vector<IXAudio2SourceVoice*> sourceVoices;
		std::shared_ptr<std::deque<short int>> readySourceVoiceIndices;
		std::vector<SourceVoiceCallback*> sourceVoiceCallbacks;
		WAVEFORMATEXTENSIBLE wfx = {
			{ 1, 1, 22050, 44100, 2, 16, 0 }
		};
	};
}