#pragma once

#include <xaudio2.h>
#include <memory>
#include <vector>

class SourceVoiceManager
{
public:
	void initializeWfx(WAVEFORMATEXTENSIBLE* wfx);
	std::shared_ptr<IXAudio2SourceVoice> getReadySourceVoice();
private:
	std::vector<std::shared_ptr<IXAudio2SourceVoice>> sourceVoices;
	std::vector<short int> readySourceVoiceIndicies;
	WAVEFORMATEXTENSIBLE* wfx;
};