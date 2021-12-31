#include "pch.h"
#include "sound_interface.h"
#include "QuickVoice.h"

#include <xaudio2.h>
#include <sstream>

// Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

namespace SoundInterface
{
	SoundManager::SoundManager()
		: sourceVoiceManager(*this)
	{ }

	SoundManager::~SoundManager()
	{
		if (this->pXAudio2) this->pXAudio2->Release();
	}

	HRESULT SoundManager::initialize()
	{
		HRESULT hr = S_OK;

		// Create an interface to use the XAudio2 engine
		if (FAILED(hr = XAudio2Create(&this->pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		{
			throw hr;
		}

		// Create a Mastering Voice
		if (FAILED(hr = pXAudio2->CreateMasteringVoice(&this->pMasterVoice)))
		{
			throw hr;
		}
	}

	// Bjarne <3
	HRESULT verifyAndReadWaveData(HANDLE hFile, DWORD& dwDataSize, BYTE*& pDataBuffer)
	{
		DWORD bytesRead;

		// Verify RIFF Spec
		DWORD dwChunkID;
		if (FALSE == ReadFile(hFile, &dwChunkID, 4, &bytesRead, NULL))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		if (bytesRead != 4 || dwChunkID != fourccRIFF)
		{
			return E_INVALIDARG;
		}

		// Ignore Length
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 4, NULL, FILE_CURRENT))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		// Verify WAVE Format
		DWORD dwFormat;
		if (FALSE == ReadFile(hFile, &dwFormat, 4, &bytesRead, NULL))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		if (bytesRead != 4 || dwFormat != fourccWAVE)
		{
			return E_INVALIDARG;
		}

		// Verify FMT Header
		if (FALSE == ReadFile(hFile, &dwChunkID, 4, &bytesRead, NULL))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		if (bytesRead != 4 || dwChunkID != fourccFMT)
		{
			return E_INVALIDARG;
		}

		DWORD dwChunkSize;
		if (FALSE == ReadFile(hFile, &dwChunkSize, 4, &bytesRead, NULL))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		if (bytesRead != 4)
		{
			return E_INVALIDARG;
		}

		// Ignore FMT Info (easier for XAudio2)

		// Ignore any other subgroups until DATA header
		while (dwChunkID != fourccDATA)
		{
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkSize, NULL, FILE_CURRENT))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}

			if (FALSE == ReadFile(hFile, &dwChunkID, 4, &bytesRead, NULL))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
			if (bytesRead != 4)
			{
				return E_INVALIDARG;
			}

			if (FALSE == ReadFile(hFile, &dwChunkSize, 4, &bytesRead, NULL))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
			if (bytesRead != 4)
			{
				return E_INVALIDARG;
			}
		}

		// Write Data
		dwDataSize = dwChunkSize;
		BYTE* tPDataBuffer = new BYTE[dwChunkSize];
		if (FALSE == ReadFile(hFile, tPDataBuffer, dwChunkSize, &bytesRead, NULL))
		{
			delete[] tPDataBuffer;
			return HRESULT_FROM_WIN32(GetLastError());
		}
		if (bytesRead != dwChunkSize)
		{
			delete[] tPDataBuffer;
			return E_INVALIDARG;
		}
		pDataBuffer = tPDataBuffer;

		return S_OK;
	}

	HRESULT SoundManager::loadSound(short int soundId)
	{
		HRESULT hr = S_OK;

		XAUDIO2_BUFFER buffer = { 0 };

		// Open the file
		TCHAR* strFileName = (TCHAR*)(_globalGameWrapper->GetDataFolder() / "QuickVoice" / (std::to_string(soundId) + ".wav")).c_str();
		HANDLE hFile = CreateFile(
			strFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		// File open failure
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
		
		// Retrieve Wave File Data
		DWORD dwChunkSize;
		BYTE* pDataBuffer = nullptr;
		if (FAILED(hr = verifyAndReadWaveData(hFile, dwChunkSize, pDataBuffer)))
		{
			return hr;
		}

		buffer.AudioBytes = dwChunkSize; // Size of the buffer in bytes
		buffer.pAudioData = pDataBuffer; // Audio buffer data
		buffer.Flags = 0; // Possible data after buffers

		this->loadedSounds.insert(std::make_pair(soundId, buffer));

		return hr;
	}

	HRESULT SoundManager::playSound(short int soundId)
	{
		HRESULT hr = S_OK;

		if (this->loadedSounds.find(soundId) == this->loadedSounds.end())
		{
			if (FAILED(hr = loadSound(soundId)))
			{
				return hr;
			}
		}

		IXAudio2SourceVoice* pSourceVoice = sourceVoiceManager.getReadySourceVoice();

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
		for (std::unordered_map<short int, XAUDIO2_BUFFER>::iterator i = this->loadedSounds.begin(); i != this->loadedSounds.end(); i++)
		{
			delete[] i->second.pAudioData;
		}
	}

	void SoundManager::unload()
	{
		unloadSounds();
		this->sourceVoiceManager.unload();
	}
	
	HRESULT SoundManager::setVolume(float newVolume)
	{
		HRESULT hr = S_OK;
		FAILED(hr = this->pMasterVoice->SetVolume(newVolume));
		return hr;
	}
}