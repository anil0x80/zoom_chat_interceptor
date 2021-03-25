#include "helper.h"
#include <Windows.h>
#include <Psapi.h>

uintptr_t helper::find_pattern(const char* module_name, std::vector<unsigned char> pattern, uintptr_t start)
{
	auto module_start = GetModuleHandleA(module_name);
	MODULEINFO module_info{};
	GetModuleInformation(GetCurrentProcess(), module_start, &module_info, sizeof(MODULEINFO));
	if (!start)
	{
		start = (uintptr_t)module_start;
	}
	auto* end = (unsigned char*)((uintptr_t)module_start + module_info.SizeOfImage);

	for (auto* p = (unsigned char*)start; p < end; p++)
	{
		if (p + pattern.size() > end)
			break;

		auto matched_pattern_index = 0u;
		auto* match_current = p;
		while (true)
		{
			if (matched_pattern_index == pattern.size())
				return reinterpret_cast<uintptr_t>(p);
			if (*match_current == pattern[matched_pattern_index] || pattern[matched_pattern_index] == 0xCC)
			{
				match_current++;
				matched_pattern_index++;
			}
			else
				break;
		}
	}

	return 0;
}

uintptr_t helper::find_pattern(const char* module_name, const char* pattern, uintptr_t start)
{
	return find_pattern(module_name, split_bytes(pattern), start);
}

std::vector<unsigned char> helper::split_bytes(std::string string)
{
	std::vector<unsigned char> bytes;
	size_t delimiter_pos;
	while ((delimiter_pos = string.find(' ')) != std::string::npos)
	{
		std::string token = string.substr(0, delimiter_pos);
		if (token == "?")
			bytes.push_back(0xCC);
		else
			bytes.push_back((unsigned char)stoi(token, nullptr, 16));
		string.erase(0, delimiter_pos + 1);
	}
	if (string == "?")
		bytes.push_back(0xCC);
	else
		bytes.push_back((unsigned char)stoi(string, nullptr, 16));
	return bytes;
}

void helper::unmute_windows()
{
	// Open the mixer device
	HMIXER hmx;
	mixerOpen(&hmx, 0, 0, 0, 0);

	// Get the line info for the wave in destination line
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(mxl);
	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

	// Find a mute control, if any, of the line out
	LPMIXERCONTROL pmxctrl = (LPMIXERCONTROL)malloc(sizeof MIXERCONTROL);
	MIXERLINECONTROLS mxlctrl = { sizeof mxlctrl, mxl.dwLineID,MIXERCONTROL_CONTROLTYPE_MUTE, 1, sizeof MIXERCONTROL,
pmxctrl };
	if (!mixerGetLineControls((HMIXEROBJ)hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE))
	{
		// Found!
		DWORD cChannels = mxl.cChannels;
		if (MIXERCONTROL_CONTROLF_UNIFORM & pmxctrl->fdwControl)
			cChannels = 1;

		LPMIXERCONTROLDETAILS_BOOLEAN pbool = (LPMIXERCONTROLDETAILS_BOOLEAN)malloc(cChannels * sizeof
			MIXERCONTROLDETAILS_BOOLEAN);

		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct = sizeof(mxcd);
		mxcd.dwControlID = pmxctrl->dwControlID;
		mxcd.cChannels = cChannels;
		mxcd.hwndOwner = (HWND)0;
		mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mxcd.paDetails = (LPVOID)pbool;
		mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
		// Mute/Unmute the line (for both channels) 
		pbool[0].fValue = pbool[cChannels - 1].fValue = (pbool[0].fValue ? 0 : 1);
		mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

		free(pbool);
	}
	free(pmxctrl);
	mixerClose(hmx);
}
