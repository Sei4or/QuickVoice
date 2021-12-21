#include "pch.h"
#include "sound_interface.h"
#include "QuickVoice.h"

#include <xaudio2.h>

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

namespace SoundInterface
{
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;

		while (hr == S_OK)
		{
			DWORD dwRead;
			if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			switch (dwChunkType)
			{
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					return HRESULT_FROM_WIN32(GetLastError());
			}

			dwOffset += sizeof(DWORD) * 2;

			if (dwChunkType == fourcc)
			{
				dwChunkSize = dwChunkDataSize;
				dwChunkDataPosition = dwOffset;
				return S_OK;
			}

			dwOffset += dwChunkDataSize;

			if (bytesRead >= dwRIFFDataSize) return S_FALSE;

		}

		return S_OK;

	}

	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());
		DWORD dwRead;
		if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	SoundManager::SoundManager()
		: sourceVoiceManager(*this)
	{ }

	HRESULT SoundManager::initialize()
	{
		HRESULT hr = S_OK;

		// Create an interface to use the XAudio2 engine
		IXAudio2* rawPXAudio2;
		if (FAILED(hr = XAudio2Create(&rawPXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		{
			throw hr;
		}

		this->pXAudio2 = std::unique_ptr<IXAudio2>(rawPXAudio2);

		// Create a Mastering Voice
		IXAudio2MasteringVoice* rawPMasterVoice;
		if (FAILED(hr = pXAudio2->CreateMasteringVoice(&rawPMasterVoice)))
		{
			throw hr;
		}

		this->pMasterVoice = std::unique_ptr<IXAudio2MasteringVoice>(rawPMasterVoice);
	}

	HRESULT SoundManager::loadSound(short int soundId)
	{
		HRESULT hr = S_OK;

		XAUDIO2_BUFFER buffer = { 0 };

		TCHAR* strFileName = (TCHAR*)(_globalGameWrapper->GetDataFolder() / "QuickVoice" / (std::to_string(soundId) + ".wav")).c_str();
		// Open the file
		HANDLE hFile = CreateFile(
			strFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			HRESULT_FROM_WIN32(GetLastError());
		}

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		{
			HRESULT_FROM_WIN32(GetLastError());
		}

		DWORD dwChunkSize;
		DWORD dwChunkPosition;
		//check the file type, should be fourccWAVE or 'XWMA'
		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (filetype != fourccWAVE)
		{
			S_FALSE;
		}

		FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);

		//fill out the audio data buffer with the contents of the fourccDATA chunk
		FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		BYTE* pDataBuffer = new BYTE[dwChunkSize];
		ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

		buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
		buffer.pAudioData = pDataBuffer;  //buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		this->loadedSounds.insert(std::make_pair(soundId, buffer));

		return hr;
	}

	HRESULT SoundManager::playSound(short int soundId)
	{
		if (this->loadedSounds.find(soundId) == this->loadedSounds.end())
		{
			loadSound(soundId);
		}

		HRESULT hr = S_OK;
		std::shared_ptr<IXAudio2SourceVoice> pSourceVoice = sourceVoiceManager.getReadySourceVoice();

		if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&this->loadedSounds.at(soundId))))
		{
			return hr;
		}

		FAILED(hr = pSourceVoice->Start(0));
		return hr;
	}

	void SoundManager::preloadSounds()
	{
		for (std::unordered_map<std::string, short int>::iterator i = quickChatIds.begin(); i != quickChatIds.end(); i++)
		{
			loadSound(i->second);
		}
	}

	void SoundManager::unloadSounds()
	{
		this->loadedSounds.clear();
	}

	void SoundManager::unload()
	{
		unloadSounds();
		this->sourceVoiceManager.unload();
		this->pMasterVoice.release();
		this->pXAudio2->Release();
		this->pXAudio2.release();
	}
	
	HRESULT SoundManager::setVolume(float newVolume)
	{
		HRESULT hr = S_OK;
		FAILED(hr = this->pMasterVoice->SetVolume(newVolume));
		return hr;
	}
}