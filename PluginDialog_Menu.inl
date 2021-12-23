// ����� � ��� ��� �������, ����������� � ����.

const UINT VK_ALT = 18;

BOOL IsKey(WPARAM code, char k1, char k2) { return (code == k1 || code == k2); }

void CPluginDialog::OnKeyDown(WPARAM code, LPARAM cnt) {
	BOOL bShift = GetAsyncKeyState(VK_SHIFT);
	BOOL bCtrl  = GetAsyncKeyState(VK_CONTROL);
	if (code == VK_ADD || code == VK_DECIMAL) {
		if (bCtrl)									OnMenuCommand(IDM_NEXT_DATA_TYPE,	   (code == VK_ADD));
		else if (GetAsyncKeyState(VK_SPACE))		OnMenuCommand(IDM_NEXT_COLOR_ENCODING, (code == VK_ADD));
		else if (bShift)							OnMenuCommand(IDM_NEXT_VIEWMODE, (code == VK_ADD));
	}
	else if (bCtrl) {
		if (bShift && IsKey(code, 'I', 'i'))		OnMenuCommand(IDM_RENDER2IMAGE);
		else if (bShift && IsKey(code, 'O', 'o'))	OnMenuCommand(IDM_RUN_OPENGL);
		else if (bShift && IsKey(code, 'D', 'd'))	OnMenuCommand(IDM_RUN_DIRECTX);
		
		else if (IsKey(code, 'S', 's'))				OnMenuCommand(IDM_SAVE);
		else if (IsKey(code, 'I', 'i'))				OnMenuCommand(IDM_BYTE_ORDER);
		else if (IsKey(code, 'A', 'a'))				OnMenuCommand(IDM_AUTO_IMG_SIZE);
		else if (IsKey(code, 'B', 'b'))				OnMenuCommand(IDM_SET_BKG_COLOR);
		else if (IsKey(code, 'M', 'm'))				OnMenuCommand(IDM_SET_YUV_MATRIX);
		else if (IsKey(code, 'L', 'l'))				OnMenuCommand(IDM_SHOW_ALWAYS);
		else if (IsKey(code, 'R', 'r'))				OnMenuCommand(IDM_SHOW_AS_RGB);
	}
}

void SwitchCheck(HMENU hMenu, UINT uID, UINT uFirstID, UINT uLastID) {
	for (UINT i = uFirstID; i <= uLastID; i++)
		CheckMenuItem(hMenu, i, MF_BYCOMMAND | (uID == i ? MF_CHECKED : MF_UNCHECKED));
}

BOOL InvertCheck(HMENU hMenu, UINT uID) {
	BOOL bChecked = GetMenuState(hMenu, uID, MF_BYCOMMAND) & MF_CHECKED;
	CheckMenuItem(hMenu, uID, MF_BYCOMMAND | (bChecked ? MF_UNCHECKED : MF_CHECKED));
	return !bChecked;
}

// ���� ������ IDM_COLOR_SUBSAMPLING_FLOAT ������ ����������� "������������� RGB" � "������������� �� ���������"
// ���� ������ ��� ������ COLOR, �� ������ �� �������� IDM_BYTE_ORDER (+������ �� ����� ������� ������)

void CPluginDialog::OnMenuCommand(UINT uID, BOOL arg) {
	if (uID >= IDM_BPP_DEFAULT && uID <= IDM_BPP_RGB444) {
		SwitchCheck(m_hMenu, uID, IDM_BPP_DEFAULT, IDM_BPP_RGB444);
		if (uID == IDM_BPP_DEFAULT)		InvertCheck(m_hMenu, IDM_BPP_RGB888);
		else if (uID > IDM_BPP_FLOATS)	InvertCheck(m_hMenu, IDM_BPP_DEFAULT);
		m_bi.clr.bpp = static_cast<BI::BPPFormat>(uID - IDM_BPP_DEFAULT);
		if (uID == IDM_BPP_FLOATS) {
			SwitchCheck(m_hMenu, uID, IDM_SHOW_DEFAULT, IDM_SHOW_AS_REAL);
#ifdef BYTEINTERPRETER_BYTE2STRING
			m_bi.clr.viewmode = BI::ColorViewMode::REAL;
#endif // END BYTEINTERPRETER_BYTE2STRING
		}
		UpdateOutput();
	}
	else if (uID == IDM_RESET_OFFSET) {
		InvertCheck(m_hMenu, uID);
		m_bResetOffset = !m_bResetOffset;
	}
	else if (uID == IDM_NEXT_DATA_TYPE) {}										// -
	else if (uID == IDM_NEXT_COLOR_ENCODING) {}									// -
	else if (uID == IDM_NEXT_VIEWMODE) {}										// -
	else if (uID == IDM_RENDER2IMAGE) {
		BOOL bChecked = InvertCheck(m_hMenu, uID);
		if (bChecked) {
			m_bi.flags.bShowAsRGB = TRUE;
			CheckMenuItem(m_hMenu, IDM_SHOW_AS_RGB, MF_BYCOMMAND | MF_CHECKED);
		}
		ShowImage(bChecked);
	}
	else if (uID == IDM_RUN_OPENGL) {											// -
		BOOL bChecked = InvertCheck(m_hMenu, uID);
	}
	else if (uID == IDM_RUN_DIRECTX) {											// -
		BOOL bChecked = InvertCheck(m_hMenu, uID);
	}
	else if (uID == IDM_BYTE_ORDER) {
		m_bi.flags.bInvert = InvertCheck(m_hMenu, uID);
		UpdateOutput(FALSE);
	}
	else if (uID == IDM_USE_YCOCG_R) {
		m_bi.flags.bYCoCgR = InvertCheck(m_hMenu, uID);
		UpdateOutput(FALSE);
	}
	else if (uID == IDM_USE_FULLRANGE_YCBCR) {
		m_bi.flags.bFullRangeYCbCr = InvertCheck(m_hMenu, uID);
		UpdateOutput(FALSE);
	}
	else if (uID == IDM_SHOW_ALWAYS) {											// -
		BOOL bChecked = InvertCheck(m_hMenu, uID);
	}
	else if (uID == IDM_AUTO_IMG_SIZE) {										// -
		BOOL bChecked = InvertCheck(m_hMenu, uID);
	}
	else if (uID == IDM_SET_YUV_MATRIX || uID == IDM_SET_BKG_COLOR)
		m_settings.Show(static_cast<SettingMode>(uID-IDM_SET_YUV_MATRIX));
	else if (uID == IDM_SAVE) {}												// -
	else if (uID == IDM_USE_ARGB) {
		m_bi.flags.bARGB = InvertCheck(m_hMenu, uID);
		UpdateOutput();
	}
	else if (uID == IDM_OBSERVER_2 || uID == IDM_OBSERVER_10) {					// -
		SwitchCheck(m_hMenu, uID, IDM_OBSERVER_2, IDM_OBSERVER_10);
	}
	else if (uID >= IDM_ILLUMINANT_A && uID <= IDM_ILLUMINANT_F12) {			// -
		SwitchCheck(m_hMenu, uID, IDM_ILLUMINANT_A, IDM_ILLUMINANT_F12);
	}
	else if (uID >= IDM_USE_YUV444 && uID <= IDM_USE_NV21) {					// -
		SwitchCheck(m_hMenu, uID, IDM_USE_YUV444, IDM_USE_NV21);
	}
	else if (uID >= IDM_SHOW_DEFAULT && uID <= IDM_SHOW_AS_REAL) {
		SwitchCheck(m_hMenu, uID, IDM_SHOW_DEFAULT, IDM_SHOW_AS_REAL);
#ifdef BYTEINTERPRETER_BYTE2STRING
		m_bi.clr.viewmode = static_cast<BI::ColorViewMode>(uID-IDM_SHOW_DEFAULT);
#endif // END BYTEINTERPRETER_BYTE2STRING
		UpdateOutput();
	}
	else if (uID == IDM_SHOW_AS_RGB) {
		m_bi.flags.bShowAsRGB = InvertCheck(m_hMenu, uID);
		UpdateOutput(0);
	}
}
