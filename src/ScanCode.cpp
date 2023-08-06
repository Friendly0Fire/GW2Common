#include "ScanCode.h"

ScanCode GetScanCode(KeyLParam lParam) {
    uint scanCode = lParam.scanCode;
    if (lParam.extendedFlag)
    {
        if (scanCode != 0x45)
            scanCode |= 0xE000;
    } else
    {
        if (scanCode == 0x45)
            scanCode = 0xE11D45;
        else if (scanCode == 0x54)
            scanCode = 0xE037;
    }

    return ScanCode(scanCode);
}

std::wstring GetScanCodeName(ScanCode scanCode) {
	if(IsMouse(scanCode))
	{
	    switch(scanCode)
	    {
		case ScanCode::LButton:
			return L"M1";
		case ScanCode::RButton:
			return L"M2";
		case ScanCode::MButton:
			return L"M3";
		case ScanCode::X1Button:
			return L"M4";
		case ScanCode::X2Button:
			return L"M5";
		default:
			return L"[Error]";
	    }
	}

	if (scanCode >= ScanCode::F13 && scanCode <= ScanCode::F23)
		return L"F" + std::to_wstring(uint(scanCode) - uint(ScanCode::F13) + 13);
	if (scanCode == ScanCode::F24)
		return L"F24";

	if (scanCode >= ScanCode::NumRow1 && scanCode <= ScanCode::NumRow9) {
		wchar_t c = wchar_t(scanCode) - 1 + 0x30;
		return std::wstring(1, c);
	}
	if (scanCode == ScanCode::NumRow0)
		return L"0";

	if (scanCode == ScanCode::MetaLeft)
		return L"LWIN";
	if (scanCode == ScanCode::MetaRight)
		return L"RWIN";

	if (IsUniversal(scanCode)) {
		switch (scanCode) {
		case ScanCode::Shift:
			return L"SHIFT";
		case ScanCode::Control:
			return L"CONTROL";
		case ScanCode::Alt:
			return L"ALT";
		case ScanCode::Meta:
			return L"META";
		}
	}

	wchar_t keyName[50];
	LPARAM lParam = 0;
	auto& lp = KeyLParam::Get(lParam);
	lp.scanCode = uint(scanCode);
	lp.extendedFlag = IsExtendedKey(scanCode) ? 1 : 0;
	if (GetKeyNameTextW(LONG(lParam), keyName, int(std::size(keyName))) != 0)
		return keyName;
	else {
		auto err = GetLastError();
		Log::i().Print(Severity::Warn, L"Could not get key name for scan code 0x{:x}, error 0x{:x}.", uint(scanCode), uint(err));
	}

	return L"[Error]";
}