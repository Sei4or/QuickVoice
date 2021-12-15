#include "SourceVoiceManager.h"

void SourceVoiceManager::initializeWfx(WAVEFORMATEXTENSIBLE* wfx)
{
	this->wfx = wfx;
}

std::shared_ptr<IXAudio2SourceVoice> SourceVoiceManager::getReadySourceVoice()
{
	HRESULT hr;

	if (readySourceVoiceIndicies.size == 0)
	{
		IXAudio2SourceVoice* pSourceVoice;
		if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)wfx)))
		{
			return hr;
		}

		std::shared_ptr<IXAudio2SourceVoice> sourceVoice = std::make_shared<IXAudio2SourceVoice>(pSourceVoice);
		this->sourceVoices.push_back(sourceVoice);
		return sourceVoice;
	}

	return this->sourceVoices.at(this->readySourceVoiceIndicies.at(0));
}