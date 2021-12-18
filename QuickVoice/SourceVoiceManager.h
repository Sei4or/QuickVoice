#pragma once

#include <xaudio2.h>
#include <memory>
#include <vector>

class SourceVoiceManager
{
public:
	void initialize(std::shared_ptr<IXAudio2> pXAudio2);
	void updateWfx(WAVEFORMATEXTENSIBLE* wfx);
	std::shared_ptr<IXAudio2SourceVoice> getReadySourceVoice();
private:
	std::shared_ptr<IXAudio2> pXAudio2;
	std::shared_ptr<IXAudio2MasteringVoice> pMasterVoice;
	std::vector<std::shared_ptr<IXAudio2SourceVoice>> sourceVoices;
	std::shared_ptr<std::deque<short int>> readySourceVoiceIndices;
	WAVEFORMATEXTENSIBLE* wfx;
};