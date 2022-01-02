#include "pch.h"
#include "SourceVoiceManager.h"

#include "sound_interface.h"

#include <sstream>

namespace SoundInterface
{
	class SourceVoiceCallback : public IXAudio2VoiceCallback
	{
	public:
		SourceVoiceCallback(short int index, std::shared_ptr<std::deque<short int>> readySourceVoiceIndices) : index(index), readySourceVoiceIndices(readySourceVoiceIndices)
		{ }

		void OnStreamEnd()
		{
			this->readySourceVoiceIndices->push_back(this->index);
		}

		// Inherited via IXAudio2VoiceCallback
		void OnVoiceProcessingPassStart(UINT32 BytesRequired) override
		{ }
		void OnVoiceProcessingPassEnd(void) override
		{ }
		void OnBufferStart(void* pBufferContext) override
		{ }
		void OnBufferEnd(void* pBufferContext) override
		{ }
		void OnLoopEnd(void* pBufferContext) override
		{ }
		void OnVoiceError(void* pBufferContext, HRESULT Error) override
		{ }
	private:
		short int index;
		std::shared_ptr<std::deque<short int>> readySourceVoiceIndices;
	};

	SourceVoiceManager::SourceVoiceManager(SoundManager& soundManager)
		: soundManager(soundManager), readySourceVoiceIndices(std::make_shared<std::deque<short int>>())
	{ }

	IXAudio2SourceVoice* SourceVoiceManager::getReadySourceVoice()
	{
		HRESULT hr = S_OK;

		if (this->readySourceVoiceIndices->size() == 0)
		{
			IXAudio2SourceVoice* pSourceVoice;
			SourceVoiceCallback* sourceVoiceCallback = new SourceVoiceCallback(this->sourceVoices.size(), this->readySourceVoiceIndices);
			if (FAILED(hr = this->soundManager.pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&this->wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, sourceVoiceCallback)))
			{
				delete sourceVoiceCallback;
				throw hr;
			}

			this->sourceVoiceCallbacks.push_back(sourceVoiceCallback);
			this->sourceVoices.push_back(pSourceVoice);
			return pSourceVoice;
		}

		short int sourceVoiceIndex = this->readySourceVoiceIndices->at(0);
		this->readySourceVoiceIndices->pop_front();
		return this->sourceVoices.at(sourceVoiceIndex);
	}

	void SourceVoiceManager::unload()
	{
		this->sourceVoices.clear();
		for (std::vector<SourceVoiceCallback*>::iterator i = this->sourceVoiceCallbacks.begin(); i != this->sourceVoiceCallbacks.end(); i++)
		{
			delete *i;
		}			
	}
}