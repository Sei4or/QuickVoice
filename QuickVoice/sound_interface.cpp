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

IXAudio2* pXAudio2 = nullptr;
IXAudio2MasteringVoice* pMasterVoice = nullptr;
WAVEFORMATEXTENSIBLE* wfx = nullptr;
std::unordered_map<short int, XAUDIO2_BUFFER> sounds;

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

HRESULT SoundInterface::loadSound(short int soundId)
{
	HRESULT hr = S_OK;

	if (pXAudio2 == nullptr)
	{
		// Create an interface to use the XAudio2 engine
		if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		{
			return hr;
		}
	}

	if (pMasterVoice == nullptr)
	{
		// Create a Mastering Voice
		if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
		{
			return hr;
		}
	}

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
	if (wfx == nullptr)
	{
		wfx = new WAVEFORMATEXTENSIBLE();
		ReadChunkData(hFile, wfx, dwChunkSize, dwChunkPosition);
	}

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	sounds.insert(std::make_pair(soundId, buffer));

	return hr;
}

HRESULT SoundInterface::playSound(short int soundId)
{
	if (sounds.find(soundId) == sounds.end())
	{
		loadSound(soundId);
	}

	HRESULT hr;

	IXAudio2SourceVoice* pSourceVoice;
	if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)wfx)))
	{
		return hr;
	}

	if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&sounds.at(soundId))))
	{
		return hr;
	}

	CVarWrapper volumeCvar = _globalCvarManager->getCvar("qv_volume");
	if (volumeCvar)
	{
		if (FAILED(hr = pSourceVoice->SetVolume(volumeCvar.getFloatValue())))
		{
			return hr;
		}
	}

	if (FAILED(hr = pSourceVoice->Start(0)))
	{
		return hr;
	}
}

void SoundInterface::preloadSounds()
{
	loadSound(WHAT_A_SAVE);
	loadSound(NICE_SHOT);
	loadSound(THANKS);
	loadSound(E);
}

void SoundInterface::unloadSounds()
{
	sounds.clear();
}

void SoundInterface::unload()
{
	unloadSounds();
	delete wfx;
	delete pMasterVoice;
	delete pXAudio2;
}