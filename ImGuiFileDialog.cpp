// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
MIT License

Copyright (c) 2019-2020 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ImGuiFileDialog.h"

#ifdef __cplusplus

#include "imgui.h"

#include <float.h>
#include <string.h> // stricmp / strcasecmp
#include <sstream>
#include <iomanip>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#if defined (__EMSCRIPTEN__) // EMSCRIPTEN
#include <emscripten.h>
#endif // EMSCRIPTEN
#if defined(__WIN32__) || defined(_WIN32)
#ifndef WIN32
#define WIN32
#endif // WIN32
#define stat _stat
#define stricmp _stricmp
#include <cctype>
#include "dirent/dirent.h" // directly open the dirent file attached to this lib
#define PATH_SEP '\\'
#ifndef PATH_MAX
#define PATH_MAX 260
#endif // PATH_MAX
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__) || defined (__EMSCRIPTEN__)
#define UNIX
#define stricmp strcasecmp
#include <sys/types.h>
#include <dirent.h>
#define PATH_SEP '/'
#endif // defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <cstdlib>
#include <algorithm>
#include <iostream>

namespace IGFD
{
	// float comparisons
#ifndef IS_FLOAT_DIFFERENT
#define IS_FLOAT_DIFFERENT(a,b) (fabs((a) - (b)) > FLT_EPSILON)
#endif // IS_FLOAT_DIFFERENT
#ifndef IS_FLOAT_EQUAL
#define IS_FLOAT_EQUAL(a,b) (fabs((a) - (b)) < FLT_EPSILON)
#endif // IS_FLOAT_EQUAL

// width of filter combobox
#ifndef FILTER_COMBO_WIDTH
#define FILTER_COMBO_WIDTH 150.0f
#endif // FILTER_COMBO_WIDTH

// for lets you define your button widget
// if you have like me a special bi-color button
#ifndef IMGUI_PATH_BUTTON
#define IMGUI_PATH_BUTTON ImGui::Button
#endif // IMGUI_PATH_BUTTON
#ifndef IMGUI_BUTTON
#define IMGUI_BUTTON ImGui::Button
#endif // IMGUI_BUTTON

// locales
#ifndef createDirButtonString
#define createDirButtonString "+"
#endif // createDirButtonString
#ifndef okButtonString
#define okButtonString "OK"
#endif // okButtonString
#ifndef cancelButtonString
#define cancelButtonString "Cancel"
#endif // cancelButtonString
#ifndef resetButtonString
#define resetButtonString "R"
#endif // resetButtonString
#ifndef drivesButtonString
#define drivesButtonString "Drives"
#endif // drivesButtonString
#ifndef searchString
#define searchString "Search :"
#endif // searchString
#ifndef dirEntryString
#define dirEntryString "[Dir]"
#endif // dirEntryString
#ifndef linkEntryString
#define linkEntryString "[Link]"
#endif // linkEntryString
#ifndef fileEntryString
#define fileEntryString "[File]"
#endif // fileEntryString
#ifndef fileNameString
#define fileNameString "File Name :"
#endif // fileNameString
#ifndef dirNameString
#define dirNameString "Directory Path :"
#endif // dirNameString
#ifndef buttonResetSearchString
#define buttonResetSearchString "Reset search"
#endif // buttonResetSearchString
#ifndef buttonDriveString
#define buttonDriveString "Drives"
#endif // buttonDriveString
#ifndef buttonResetPathString
#define buttonResetPathString "Reset to current directory"
#endif // buttonResetPathString
#ifndef buttonCreateDirString
#define buttonCreateDirString "Create Directory"
#endif // buttonCreateDirString
#ifndef tableHeaderAscendingIcon
#define tableHeaderAscendingIcon "A|"
#endif // tableHeaderAscendingIcon
#ifndef tableHeaderDescendingIcon
#define tableHeaderDescendingIcon "D|"
#endif // tableHeaderDescendingIcon
#ifndef tableHeaderFileNameString
#define tableHeaderFileNameString "File name"
#endif // tableHeaderFileNameString
#ifndef tableHeaderFileTypeString
#define tableHeaderFileTypeString "Type"
#endif // tableHeaderFileTypeString
#ifndef tableHeaderFileSizeString
#define tableHeaderFileSizeString "Size"
#endif // tableHeaderFileSizeString
#ifndef tableHeaderFileDateString
#define tableHeaderFileDateString "Date"
#endif // tableHeaderFileDateString
#ifndef OverWriteDialogTitleString
#define OverWriteDialogTitleString "The file Already Exist !"
#endif // OverWriteDialogTitleString
#ifndef OverWriteDialogMessageString
#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
#endif // OverWriteDialogMessageString
#ifndef OverWriteDialogConfirmButtonString
#define OverWriteDialogConfirmButtonString "Confirm"
#endif // OverWriteDialogConfirmButtonString
#ifndef OverWriteDialogCancelButtonString
#define OverWriteDialogCancelButtonString "Cancel"
#endif // OverWriteDialogCancelButtonString
// see strftime functionin <ctime> for customize
#ifndef DateTimeFormat
#define DateTimeFormat "%Y/%m/%d %H:%M"
#endif // DateTimeFormat

#ifdef USE_THUMBNAILS
#ifndef DisplayMode_FilesList_ButtonString
#define DisplayMode_FilesList_ButtonString "FL"
#endif // DisplayMode_FilesList_ButtonString
#ifndef DisplayMode_FilesList_ButtonHelp
#define DisplayMode_FilesList_ButtonHelp "File List"
#endif // DisplayMode_FilesList_ButtonHelp
#ifndef DisplayMode_ThumbailsList_ButtonString
#define DisplayMode_ThumbailsList_ButtonString "TL"
#endif // DisplayMode_ThumbailsList_ButtonString
#ifndef DisplayMode_ThumbailsList_ButtonHelp
#define DisplayMode_ThumbailsList_ButtonHelp "Thumbnails List"
#endif // DisplayMode_ThumbailsList_ButtonHelp
#ifndef DisplayMode_ThumbailsSmall_ButtonString
#define DisplayMode_ThumbailsSmall_ButtonString "ST"
#endif // DisplayMode_ThumbailsSmall_ButtonString
#ifndef DisplayMode_ThumbailsSmall_ButtonHelp
#define DisplayMode_ThumbailsSmall_ButtonHelp "Small Thumbnails"
#endif // DisplayMode_ThumbailsSmall_ButtonHelp
#ifndef DisplayMode_ThumbailsBig_ButtonString
#define DisplayMode_ThumbailsBig_ButtonString "BT"
#endif // DisplayMode_ThumbailsBig_ButtonString
#ifndef DisplayMode_ThumbailsBig_ButtonHelp
#define DisplayMode_ThumbailsBig_ButtonHelp "Big Thumbnails"
#endif // DisplayMode_ThumbailsBig_ButtonHelp
#ifndef IMGUI_RADIO_BUTTON
inline bool inRadioButton(const char* vLabel, bool vToggled)
{
	bool pressed = false;

	if (vToggled)
	{
		ImVec4 bua = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		ImVec4 te = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		ImGui::PushStyleColor(ImGuiCol_Button, te);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, te);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, te);
		ImGui::PushStyleColor(ImGuiCol_Text, bua);
	}

	pressed = IMGUI_BUTTON(vLabel);

	if (vToggled)
	{
		ImGui::PopStyleColor(4); //-V112
	}

	return pressed;
}
#define IMGUI_RADIO_BUTTON inRadioButton
#endif // IMGUI_RADIO_BUTTON
#ifndef DisplayMode_ThumbailsList_ImageHeight 
#define DisplayMode_ThumbailsList_ImageHeight 50.0f
#endif // DisplayMode_ThumbailsList_ImageHeight
#endif  // USE_THUMBNAILS

#ifdef USE_BOOKMARK

#ifndef defaultBookmarkPaneWith
#define defaultBookmarkPaneWith 150.0f
#endif // defaultBookmarkPaneWith
#ifndef bookmarksButtonString
#define bookmarksButtonString "Bookmark"
#endif // bookmarksButtonString
#ifndef bookmarksButtonHelpString
#define bookmarksButtonHelpString "Bookmark"
#endif // bookmarksButtonHelpString
#ifndef addBookmarkButtonString
#define addBookmarkButtonString "+"
#endif // addBookmarkButtonString
#ifndef removeBookmarkButtonString
#define removeBookmarkButtonString "-"
#endif // removeBookmarkButtonString
#ifndef IMGUI_TOGGLE_BUTTON
	inline bool inToggleButton(const char* vLabel, bool* vToggled)
	{
		bool pressed = false;

		if (vToggled && *vToggled)
		{
			ImVec4 bua = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			//ImVec4 buh = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			//ImVec4 bu = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			ImVec4 te = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			ImGui::PushStyleColor(ImGuiCol_Button, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, te);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, te);
			ImGui::PushStyleColor(ImGuiCol_Text, bua);
		}

		pressed = IMGUI_BUTTON(vLabel);

		if (vToggled && *vToggled)
		{
			ImGui::PopStyleColor(4); //-V112
		}

		if (vToggled && pressed)
			*vToggled = !*vToggled;

		return pressed;
	}
#define IMGUI_TOGGLE_BUTTON inToggleButton
#endif // IMGUI_TOGGLE_BUTTON
#endif // USE_BOOKMARK

	// https://github.com/ocornut/imgui/issues/1720
	bool inSplitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 1.0f);
	}

	static std::string s_fs_root = std::string(1u, PATH_SEP);

	inline int inAlphaSort(const struct dirent** a, const struct dirent** b)
	{
		return strcoll((*a)->d_name, (*b)->d_name);
	}

#ifdef WIN32
	inline bool inWReplaceString(std::wstring& str, const std::wstring& oldStr, const std::wstring& newStr)
	{
		bool found = false;
		size_t pos = 0;
		while ((pos = str.find(oldStr, pos)) != std::wstring::npos)
		{
			found = true;
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
		return found;
	}

	inline std::vector<std::wstring> inWSplitStringToVector(const std::wstring& text, char delimiter, bool pushEmpty)
	{
		std::vector<std::wstring> arr;
		if (!text.empty())
		{
			std::wstring::size_type start = 0;
			std::wstring::size_type end = text.find(delimiter, start);
			while (end != std::wstring::npos)
			{
				std::wstring token = text.substr(start, end - start);
				if (!token.empty() || (token.empty() && pushEmpty)) //-V728
					arr.push_back(token);
				start = end + 1;
				end = text.find(delimiter, start);
			}
			std::wstring token = text.substr(start);
			if (!token.empty() || (token.empty() && pushEmpty)) //-V728
				arr.push_back(token);
		}
		return arr;
	}
#endif

	inline bool inReplaceString(std::string& str, const std::string& oldStr, const std::string& newStr)
	{
		bool found = false;
		size_t pos = 0;
		while ((pos = str.find(oldStr, pos)) != std::string::npos)
		{
			found = true;
			str.replace(pos, oldStr.length(), newStr);
			pos += newStr.length();
		}
		return found;
	}

	inline std::vector<std::string> inSplitStringToVector(const std::string& text, char delimiter, bool pushEmpty)
	{
		std::vector<std::string> arr;
		if (!text.empty())
		{
			std::string::size_type start = 0;
			std::string::size_type end = text.find(delimiter, start);
			while (end != std::string::npos)
			{
				std::string token = text.substr(start, end - start);
				if (!token.empty() || (token.empty() && pushEmpty)) //-V728
					arr.push_back(token);
				start = end + 1;
				end = text.find(delimiter, start);
			}
			std::string token = text.substr(start);
			if (!token.empty() || (token.empty() && pushEmpty)) //-V728
				arr.push_back(token);
		}
		return arr;
	}

	inline std::vector<std::string> inGetDrivesList()
	{
		std::vector<std::string> res;

#ifdef WIN32
		const DWORD mydrives = 2048;
		char lpBuffer[2048];
#define mini(a,b) (((a) < (b)) ? (a) : (b))
		const DWORD countChars = mini(GetLogicalDriveStringsA(mydrives, lpBuffer), 2047);
#undef mini
		if (countChars > 0)
		{
			std::string var = std::string(lpBuffer, (size_t)countChars);
			inReplaceString(var, "\\", "");
			res = inSplitStringToVector(var, '\0', false);
		}
#endif // WIN32

		return res;
	}

	inline bool inIsDirectoryExist(const std::string& name)
	{
		bool bExists = false;

		if (!name.empty())
		{
			DIR* pDir = nullptr;
			pDir = opendir(name.c_str());
			if (pDir != nullptr)
			{
				bExists = true;
				(void)closedir(pDir);
			}
		}

		return bExists;    // this is not a directory!
	}

#ifdef WIN32
	inline std::wstring inWGetString(const char* str)
	{
		std::wstring ret;
		size_t sz;
		if (!dirent_mbstowcs_s(&sz, nullptr, 0, str, 0))
		{
			ret.resize(sz);
			dirent_mbstowcs_s(nullptr, (wchar_t*)ret.data(), sz, str, sz - 1);
		}
		return ret;
	}
#endif

	inline bool CreateDirectoryIfNotExist(const std::string& name)
	{
		bool res = false;

		if (!name.empty())
		{
			if (!inIsDirectoryExist(name))
			{
#ifdef WIN32
				std::wstring wname = inWGetString(name.c_str());
				if (CreateDirectoryW(wname.c_str(), nullptr))
				{
					res = true;
				}
#elif defined(__EMSCRIPTEN__)
				std::string str = std::string("FS.mkdir('") + name + "');";
				emscripten_run_script(str.c_str());
				res = true;
#elif defined(UNIX)
				char buffer[PATH_MAX] = {};
				snprintf(buffer, PATH_MAX, "mkdir -p %s", name.c_str());
				const int dir_err = std::system(buffer);
				if (dir_err != -1)
				{
					res = true;
				}
#endif // defined(UNIX)
				if (!res) {
					std::cout << "Error creating directory " << name << std::endl;
				}
		}
	}

		return res;
	}

	struct PathStruct
	{
		std::string path;
		std::string name;
		std::string ext;

		bool isOk;

		PathStruct()
		{
			isOk = false;
		}
	};

	inline PathStruct inParsePathFileName(const std::string& vPathFileName)
	{
		PathStruct res;

		if (!vPathFileName.empty())
		{
			std::string pfn = vPathFileName;
			std::string separator(1u, PATH_SEP);
			inReplaceString(pfn, "\\", separator);
			inReplaceString(pfn, "/", separator);

			size_t lastSlash = pfn.find_last_of(separator);
			if (lastSlash != std::string::npos)
			{
				res.name = pfn.substr(lastSlash + 1);
				res.path = pfn.substr(0, lastSlash);
				res.isOk = true;
			}

			size_t lastPoint = pfn.find_last_of('.');
			if (lastPoint != std::string::npos)
			{
				if (!res.isOk)
				{
					res.name = pfn;
					res.isOk = true;
				}
				res.ext = pfn.substr(lastPoint + 1);
				inReplaceString(res.name, "." + res.ext, "");
			}
		}

		return res;
	}

	inline void inAppendToBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr)
	{
		std::string st = vStr;
		size_t len = vBufferLen - 1u;
		size_t slen = strlen(vBuffer);

		if (!st.empty() && st != "\n")
		{
			inReplaceString(st, "\n", "");
			inReplaceString(st, "\r", "");
		}
		vBuffer[slen] = '\0';
		std::string str = std::string(vBuffer);
		//if (!str.empty()) str += "\n";
		str += vStr;
		if (len > str.size()) len = str.size();
#ifdef MSVC
		strncpy_s(vBuffer, vBufferLen, str.c_str(), len);
#else // MSVC
		strncpy(vBuffer, str.c_str(), len);
#endif // MSVC
		vBuffer[len] = '\0';
	}

	inline void inResetBuffer(char* vBuffer)
	{
		vBuffer[0] = '\0';
	}

	inline void inSetBuffer(char* vBuffer, size_t vBufferLen, const std::string& vStr)
	{
		inResetBuffer(vBuffer);
		inAppendToBuffer(vBuffer, vBufferLen, vStr);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	IGFD::FileDialog::FileDialog()
	{
		puAnyWindowsHovered = false;
		prIsOk = false;
		prShowDialog = false;
		prShowDrives = false;
		prCreateDirectoryMode = false;
		dlg_optionsPane = nullptr;
		dlg_optionsPaneWidth = 250;
		dlg_filters = "";
		dlg_userDatas = 0;
#ifdef USE_THUMBNAILS
		prDisplayMode = DisplayModeEnum::DISPLAY_MODE_FILE_LIST;
#endif // USE_THUMBNAILS
#ifdef USE_BOOKMARK
		prBookmarkPaneShown = false;
		prBookmarkWidth = defaultBookmarkPaneWith;
#endif // USE_BOOKMARK
	}

	IGFD::FileDialog::~FileDialog() = default;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// CUSTOM SELECTABLE (Flashing Support) ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
#ifdef USE_EXPLORATION_BY_KEYS
	bool IGFD::FileDialog::prFlashableSelectable(const char* label, bool selected,
		ImGuiSelectableFlags flags, bool vFlashing, const ImVec2& size_arg)
	{
		using namespace ImGui;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		// Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
		ImGuiID id = window->GetID(label);
		ImVec2 label_size = CalcTextSize(label, NULL, true);
		ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
		ImVec2 pos = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;
		ItemSize(size, 0.0f);

		// Fill horizontal space
		// We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitely right-aligned sizes not visibly match other widgets.
		const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
		const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
		const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
		if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
			size.x = ImMax(label_size.x, max_x - min_x);

		// Text stays at the submission position, but bounding box may be extended on both sides
		const ImVec2 text_min = pos;
		const ImVec2 text_max(min_x + size.x, pos.y + size.y);

		// Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
		ImRect bb(min_x, pos.y, text_max.x, text_max.y);
		if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
		{
			const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
			const float spacing_y = style.ItemSpacing.y;
			const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
			const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
			bb.Min.x -= spacing_L;
			bb.Min.y -= spacing_U;
			bb.Max.x += (spacing_x - spacing_L);
			bb.Max.y += (spacing_y - spacing_U);
		}
		//if (g.IO.KeyCtrl) { GetForegroundprDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

		// Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
		const float backup_clip_rect_min_x = window->ClipRect.Min.x;
		const float backup_clip_rect_max_x = window->ClipRect.Max.x;
		if (span_all_columns)
		{
			window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
			window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
		}

		bool item_add;
		if (flags & ImGuiSelectableFlags_Disabled)
		{
			ImGuiItemFlags backup_item_flags = window->DC.ItemFlags;
			window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
			item_add = ItemAdd(bb, id);
			window->DC.ItemFlags = backup_item_flags;
		}
		else
		{
			item_add = ItemAdd(bb, id);
		}

		if (span_all_columns)
		{
			window->ClipRect.Min.x = backup_clip_rect_min_x;
			window->ClipRect.Max.x = backup_clip_rect_max_x;
		}

		if (!item_add)
			return false;

		// FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
		// which would be advantageous since most selectable are not selected.
		if (span_all_columns && window->DC.CurrentColumns)
			PushColumnsBackground();
		else if (span_all_columns && g.CurrentTable)
			TablePushBackgroundChannel();

		// We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
		ImGuiButtonFlags button_flags = 0;
		if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
		if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
		if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
		if (flags & ImGuiSelectableFlags_Disabled) { button_flags |= ImGuiButtonFlags_Disabled; }
		if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
		if (flags & ImGuiSelectableFlags_AllowItemOverlap) { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

		if (flags & ImGuiSelectableFlags_Disabled)
			selected = false;

		const bool was_selected = selected;
		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

		// Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
		if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
		{
			if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
			{
				g.NavDisableHighlight = true;
				SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent);
			}
		}
		if (pressed)
			MarkItemEdited(id);

		if (flags & ImGuiSelectableFlags_AllowItemOverlap)
			SetItemAllowOverlap();

		// In this branch, Selectable() cannot toggle the selection so this will never trigger.
		if (selected != was_selected) //-V547
			window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

		// Render
		if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld) || vFlashing)
			hovered = true;
		if (hovered || selected)
		{
			const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
			RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
			RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
		}

		if (span_all_columns && window->DC.CurrentColumns)
			PopColumnsBackground();
		else if (span_all_columns && g.CurrentTable)
			TablePopBackgroundChannel();

		if (flags & ImGuiSelectableFlags_Disabled) PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
		RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
		if (flags & ImGuiSelectableFlags_Disabled) PopStyleColor();

		// Automatically close popups
		if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
			CloseCurrentPopup();

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
		return pressed;
	}
#endif // USE_EXPLORATION_BY_KEYS

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// STANDARD DIALOG ////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// path and fileName can be specified
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,			
		const std::string& vTitle,			
		const char *vFilters,				
		const std::string& vPath,			
		const std::string& vFileName,
		const int& vCountSelectionMax,		
		UserDatas vUserDatas,				
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_title = vTitle;
		dlg_path = vPath;
		dlg_userDatas = vUserDatas;
		dlg_flags = vFlags;
		dlg_optionsPane = nullptr;
		dlg_optionsPaneWidth = 0.0f;
		dlg_countSelectionMax = vCountSelectionMax;
		dlg_modal = false;
		dlg_defaultExt.clear();

		prParseFilters(vFilters);
		prSetDefaultFileName(vFileName);
		prSetPath(prCurrentPath);

		prShowDialog = true;					// open dialog
#ifdef USE_BOOKMARK
		prBookmarkPaneShown = false;
#endif // USE_BOOKMARK
	}

	// path and filename are obtained from filePathName
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,	
		const std::string& vTitle,	
		const char *vFilters,			
		const std::string& vFilePathName,	
		const int& vCountSelectionMax,		
		UserDatas vUserDatas,					
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_title = vTitle;
		
		auto ps = inParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			dlg_path = ps.path;
			prSetDefaultFileName(vFilePathName);
			dlg_defaultExt = "." + ps.ext;
		}
		else
		{
			dlg_path = ".";
			prSetDefaultFileName("");
			dlg_defaultExt.clear();
		}

		dlg_optionsPane = nullptr;
		dlg_optionsPaneWidth = 0.0f;
		dlg_userDatas = vUserDatas;
		dlg_flags = vFlags;
		dlg_countSelectionMax = vCountSelectionMax; //-V101
		dlg_modal = false;

		prParseFilters(vFilters);
		prSetSelectedFilterWithExt(dlg_defaultExt);
		prSetPath(prCurrentPath);

		prShowDialog = true;
#ifdef USE_BOOKMARK
		prBookmarkPaneShown = false;
#endif // USE_BOOKMARK
	}
	
	// with pane
	// path and fileName can be specified
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vPath,
		const std::string& vFileName,
		const PaneFun& vSidePane,
		const float& vSidePaneWidth,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_title = vTitle;
		dlg_path = vPath;
		dlg_userDatas = vUserDatas;
		dlg_flags = vFlags;
		dlg_optionsPane = vSidePane;
		dlg_optionsPaneWidth = vSidePaneWidth;
		dlg_countSelectionMax = vCountSelectionMax;
		dlg_modal = false;
		dlg_defaultExt.clear();

		prParseFilters(vFilters);
		prSetDefaultFileName(vFileName);
		prSetPath(prCurrentPath);

		prShowDialog = true;					// open dialog
#ifdef USE_BOOKMARK
		prBookmarkPaneShown = false;
#endif // USE_BOOKMARK
	}

	// with pane
	// path and filename are obtained from filePathName
	void IGFD::FileDialog::OpenDialog(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vFilePathName,
		const PaneFun& vSidePane,
		const float& vSidePaneWidth,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		dlg_key = vKey;
		dlg_title = vTitle;

		auto ps = inParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			dlg_path = ps.path;
			prSetDefaultFileName(vFilePathName);
			dlg_defaultExt = "." + ps.ext;
		}
		else
		{
			dlg_path = ".";
			prSetDefaultFileName("");
			dlg_defaultExt.clear();
		}

		dlg_optionsPane = vSidePane;
		dlg_optionsPaneWidth = vSidePaneWidth;
		dlg_userDatas = vUserDatas;
		dlg_flags = vFlags;
		dlg_countSelectionMax = vCountSelectionMax; //-V101
		dlg_modal = false;

		prParseFilters(vFilters);
		prSetSelectedFilterWithExt(dlg_defaultExt);
		prSetPath(prCurrentPath);

		prShowDialog = true;
#ifdef USE_BOOKMARK
		prBookmarkPaneShown = false;
#endif // USE_BOOKMARK
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// MODAL DIALOG ///////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	void IGFD::FileDialog::OpenModal(
		const std::string& vKey, 
		const std::string& vTitle, 
		const char* vFilters,
		const std::string& vPath, 
		const std::string& vFileName,
		const int& vCountSelectionMax, 
		UserDatas vUserDatas, 
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		OpenDialog(
			vKey, vTitle, vFilters,
			vPath, vFileName,
			vCountSelectionMax, vUserDatas, vFlags);

		dlg_modal = true;
	}

	void IGFD::FileDialog::OpenModal(
		const std::string& vKey, 
		const std::string& vTitle, 
		const char *vFilters,
		const std::string& vFilePathName,
		const int& vCountSelectionMax, 
		UserDatas vUserDatas, 
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		OpenDialog(
			vKey, vTitle, vFilters,
			vFilePathName,
			vCountSelectionMax, vUserDatas, vFlags);

		dlg_modal = true;
	}

	// with pane
	// path and fileName can be specified
	void IGFD::FileDialog::OpenModal(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vPath,
		const std::string& vFileName,
		const PaneFun& vSidePane,
		const float& vSidePaneWidth,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		OpenDialog(
			vKey, vTitle, vFilters,
			vPath, vFileName, 
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, vFlags);

		dlg_modal = true;
	}

	// with pane
	// path and filename are obtained from filePathName
	void IGFD::FileDialog::OpenModal(
		const std::string& vKey,
		const std::string& vTitle,
		const char* vFilters,
		const std::string& vFilePathName,
		const PaneFun& vSidePane,
		const float& vSidePaneWidth,
		const int& vCountSelectionMax,
		UserDatas vUserDatas,
		ImGuiFileDialogFlags vFlags)
	{
		if (prShowDialog) // if already opened, quit
			return;

		OpenDialog(
			vKey, vTitle, vFilters,
			vFilePathName,
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, vFlags);

		dlg_modal = true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	///// MAIN FUNCTION //////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

	bool IGFD::FileDialog::Display(const std::string& vKey, ImGuiWindowFlags vFlags, ImVec2 vMinSize, ImVec2 vMaxSize)
	{
		if (prShowDialog && dlg_key == vKey)
		{
			bool res = false;

			// to be sure than only one dialog is displayed per frame
			ImGuiContext& g = *GImGui;
			if (g.FrameCount == prLastImGuiFrameCount) // one instance was displayed this frame before for this key +> quit
				return res;
			prLastImGuiFrameCount = g.FrameCount; // mark this instance as used this frame

			std::string name = dlg_title + "##" + dlg_key;
			if (prName != name)
			{
				prFileList.clear();
				prCurrentPath_Decomposition.clear();
			}

			prIsOk = false;	 // reset dialog result

			prResetEvents();

			ImGui::SetNextWindowSizeConstraints(vMinSize, vMaxSize);

			bool beg = false;
			if (dlg_modal &&
				!prOkResultToConfirm) // disable modal because the confirm dialog for overwrite is a new modal
			{
				ImGui::OpenPopup(name.c_str());
				beg = ImGui::BeginPopupModal(name.c_str(), (bool*)nullptr,
					vFlags | ImGuiWindowFlags_NoScrollbar);
			}
			else
			{
				beg = ImGui::Begin(name.c_str(), (bool*)nullptr, vFlags | ImGuiWindowFlags_NoScrollbar);
			}
			if (beg)
			{
				prName = name; //-V820
				puAnyWindowsHovered |= ImGui::IsWindowHovered();

				if (dlg_path.empty()) dlg_path = "."; // defaut path is '.'
				if (prSelectedFilter.empty() && // no filter selected
					!prFilters.empty()) // filter exist
					prSelectedFilter = *prFilters.begin(); // we take the first filter

				// init list of files
				if (prFileList.empty() && !prShowDrives)
				{
					inReplaceString(dlg_defaultFileName, dlg_path, ""); // local path
					if (!dlg_defaultFileName.empty())
					{
						prSetDefaultFileName(dlg_defaultFileName);
						prSetSelectedFilterWithExt(dlg_defaultExt);
					}
					else if (dlg_filters.empty()) // directory mode
						prSetDefaultFileName(".");
					prScanDir(dlg_path);
				}

				// draw dialog parts
				prDrawHeader(); // bookmark, directory, path
				prDrawContent(); // bookmark, files view, side pane 
				res = prDrawFooter(); // file field, filter combobox, ok/cancel buttons

				// for display in dialog center, the confirm to overwrite dlg
				prDialogCenterPos = ImGui::GetCurrentWindowRead()->ContentRegionRect.GetCenter();

				// when the confirm to overwrite dialog will appear we need to 
				// disable the modal mode of the main file dialog
				// see prOkResultToConfirm under
				if (dlg_modal &&
					!prOkResultToConfirm)
					ImGui::EndPopup();
			}

			// same things here regarding prOkResultToConfirm
			if (!dlg_modal || prOkResultToConfirm)
				ImGui::End();

			// confirm the result and show the confirm to overwrite dialog if needed
			return prConfirm_Or_OpenOverWriteFileDialog_IfNeeded(res, vFlags);
		}

		return false;
	}

	void IGFD::FileDialog::prResetEvents()
	{
		// reset events
		prDrivesClicked = false;
		prPathClicked = false;
		prCanWeContinue = true;
	}

	void IGFD::FileDialog::prDrawHeader()
	{
#ifdef USE_BOOKMARK
		prDrawBookMark();
		ImGui::SameLine();
#endif // USE_BOOKMARK
		prDrawDirectoryCreation();
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
		prDrawPathComposer();

#ifdef USE_THUMBNAILS
		prDrawDisplayModeToolBar();
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
#endif // USE_THUMBNAILS
		prDrawSearchBar();
	}

	void IGFD::FileDialog::prDrawContent()
	{
		ImVec2 size = ImGui::GetContentRegionAvail() - ImVec2(0.0f, prFooterHeight);

#ifdef USE_BOOKMARK
		if (prBookmarkPaneShown)
		{
			//size.x -= prBookmarkWidth;
			ImGui::PushID("##splitterbookmark");
			float otherWidth = size.x - prBookmarkWidth;
			inSplitter(true, 4.0f, &prBookmarkWidth, &otherWidth, 10.0f, 10.0f + dlg_optionsPaneWidth, size.y);
			ImGui::PopID();
			size.x -= otherWidth;
			prDrawBookmarkPane(size);
			ImGui::SameLine();
		}
#endif // USE_BOOKMARK

		size.x = ImGui::GetContentRegionAvail().x - dlg_optionsPaneWidth;

		if (dlg_optionsPane)
		{
			ImGui::PushID("##splittersidepane");
			inSplitter(true, 4.0f, &size.x, &dlg_optionsPaneWidth, 10.0f, 10.0f, size.y);
			ImGui::PopID();
		}
#ifdef USE_THUMBNAILS
		switch (prDisplayMode)
		{
		case DisplayModeEnum::DISPLAY_MODE_FILE_LIST:
#endif // USE_THUMBNAILS
			prDrawFileListView(size);
#ifdef USE_THUMBNAILS
			break;
		case DisplayModeEnum::DISPLAY_MODE_THUMBAILS_LIST:
			prDrawThumbnailsListView(size);
			break;
		case DisplayModeEnum::DISPLAY_MODE_SMALL_THUMBAILS:
			prDrawSmallThumbnailsView(size);
			break;
		case DisplayModeEnum::DISPLAY_MODE_BIG_THUMBAILS:
			prDrawBigThumbnailsView(size);
			break;
		}
#endif // USE_THUMBNAILS
		if (dlg_optionsPane)
		{
			prDrawSidePane(size.y);
		}
	}

	bool IGFD::FileDialog::prDrawFooter()
	{
		float posY = ImGui::GetCursorPos().y; // height of last bar calc

		if (!dlg_filters.empty())
			ImGui::Text(fileNameString);
		else // directory chooser
			ImGui::Text(dirNameString);

		ImGui::SameLine();

		// Input file fields
		float width = ImGui::GetContentRegionAvail().x;
		if (!dlg_filters.empty())
			width -= FILTER_COMBO_WIDTH;
		ImGui::PushItemWidth(width);
		ImGui::InputText("##FileName", puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
		ImGui::PopItemWidth();

		// combobox of filters
		if (!dlg_filters.empty())
		{
			ImGui::SameLine();

			bool needToApllyNewFilter = false;

			ImGui::PushItemWidth(FILTER_COMBO_WIDTH);
			if (ImGui::BeginCombo("##Filters", prSelectedFilter.filter.c_str(), ImGuiComboFlags_None))
			{
				intptr_t i = 0;
				for (auto filter : prFilters)
				{
					const bool item_selected = (filter.filter == prSelectedFilter.filter);
					ImGui::PushID((void*)(intptr_t)i++);
					if (ImGui::Selectable(filter.filter.c_str(), item_selected))
					{
						prSelectedFilter = filter;
						needToApllyNewFilter = true;
					}
					ImGui::PopID();
				}

				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			if (needToApllyNewFilter)
			{
				prSetPath(prCurrentPath);
			}
		}

		bool res = false;

		// OK Button
		if (prCanWeContinue && strlen(puFileNameBuffer))
		{
			if (IMGUI_BUTTON(okButtonString))
			{
				prIsOk = true;
				res = true;
			}

			ImGui::SameLine();
		}

		// Cancel Button
		if (IMGUI_BUTTON(cancelButtonString)
#ifdef USE_DIALOG_EXIT_WITH_KEY
			|| ImGui::IsKeyReleased(IGFD_EXIT_KEY)
#endif
			)
		{
			prIsOk = false;
			res = true;
		}

		prFooterHeight = ImGui::GetCursorPosY() - posY;

		return res;
	}
#ifdef USE_BOOKMARK
	void IGFD::FileDialog::prDrawBookMark()
	{
		IMGUI_TOGGLE_BUTTON(bookmarksButtonString, &prBookmarkPaneShown);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(bookmarksButtonHelpString);
	}
#endif // USE_BOOKMARK

	void IGFD::FileDialog::prDrawDirectoryCreation()
	{
		if (IMGUI_BUTTON(createDirButtonString))
		{
			if (!prCreateDirectoryMode)
			{
				prCreateDirectoryMode = true;
				inResetBuffer(puDirectoryNameBuffer);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonCreateDirString);

		if (prCreateDirectoryMode)
		{
			ImGui::SameLine();

			ImGui::PushItemWidth(100.0f);
			ImGui::InputText("##DirectoryFileName", puDirectoryNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
			ImGui::PopItemWidth();

			ImGui::SameLine();

			if (IMGUI_BUTTON(okButtonString))
			{
				std::string newDir = std::string(puDirectoryNameBuffer);
				if (prCreateDir(newDir))
				{
					prSetPath(prCurrentPath + PATH_SEP + newDir);
				}

				prCreateDirectoryMode = false;
			}

			ImGui::SameLine();

			if (IMGUI_BUTTON(cancelButtonString))
			{
				prCreateDirectoryMode = false;
			}
		}
	}

	void IGFD::FileDialog::prDrawPathComposer()
	{
		if (IMGUI_BUTTON(resetButtonString))
		{
			prSetPath(".");
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonResetPathString);

#ifdef WIN32
		ImGui::SameLine();

		if (IMGUI_BUTTON(drivesButtonString))
		{
			prDrivesClicked = true;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonDriveString);
#endif // WIN32

		ImGui::SameLine();

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

		// show current path
		if (!prCurrentPath_Decomposition.empty())
		{
			ImGui::SameLine();

			if (prInputPathActivated)
			{
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::InputText("##pathedition", puInputPathBuffer, MAX_PATH_BUFFER_SIZE);
				ImGui::PopItemWidth();
			}
			else
			{
				int _id = 0;
				for (auto itPathDecomp = prCurrentPath_Decomposition.begin();
					itPathDecomp != prCurrentPath_Decomposition.end(); ++itPathDecomp)
				{
					if (itPathDecomp != prCurrentPath_Decomposition.begin())
						ImGui::SameLine();
					ImGui::PushID(_id++);
					bool click = IMGUI_PATH_BUTTON((*itPathDecomp).c_str());
					ImGui::PopID();
					if (click)
					{
						prCurrentPath = prComposeNewPath(itPathDecomp);
						prPathClicked = true;
						break;
					}
					// activate input for path
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						inSetBuffer(puInputPathBuffer, MAX_PATH_BUFFER_SIZE, prComposeNewPath(itPathDecomp));
						prInputPathActivated = true;
						break;
					}
				}
			}
		}
	}

#ifdef USE_THUMBNAILS
	void IGFD::FileDialog::prDrawDisplayModeToolBar()
	{
		if (IMGUI_RADIO_BUTTON(DisplayMode_FilesList_ButtonString,
			prDisplayMode == DisplayModeEnum::DISPLAY_MODE_FILE_LIST))
			prDisplayMode = DisplayModeEnum::DISPLAY_MODE_FILE_LIST;
		if (ImGui::IsItemHovered())	ImGui::SetTooltip(DisplayMode_FilesList_ButtonHelp);
		ImGui::SameLine();
		if (IMGUI_RADIO_BUTTON(DisplayMode_ThumbailsList_ButtonString,
			prDisplayMode == DisplayModeEnum::DISPLAY_MODE_THUMBAILS_LIST))
			prDisplayMode = DisplayModeEnum::DISPLAY_MODE_THUMBAILS_LIST;
		if (ImGui::IsItemHovered())	ImGui::SetTooltip(DisplayMode_ThumbailsList_ButtonHelp);
		ImGui::SameLine();
		if (IMGUI_RADIO_BUTTON(DisplayMode_ThumbailsSmall_ButtonString,
			prDisplayMode == DisplayModeEnum::DISPLAY_MODE_SMALL_THUMBAILS))
			prDisplayMode = DisplayModeEnum::DISPLAY_MODE_SMALL_THUMBAILS;
		if (ImGui::IsItemHovered())	ImGui::SetTooltip(DisplayMode_ThumbailsSmall_ButtonHelp);
		ImGui::SameLine();
		if (IMGUI_RADIO_BUTTON(DisplayMode_ThumbailsBig_ButtonString,
			prDisplayMode == DisplayModeEnum::DISPLAY_MODE_BIG_THUMBAILS))
			prDisplayMode = DisplayModeEnum::DISPLAY_MODE_BIG_THUMBAILS;
		if (ImGui::IsItemHovered())	ImGui::SetTooltip(DisplayMode_ThumbailsBig_ButtonHelp);
		ImGui::SameLine();
	}
#endif // USE_THUMBNAILS

	void IGFD::FileDialog::prDrawSearchBar()
	{
		// search field
		if (IMGUI_BUTTON(resetButtonString "##BtnImGuiFileDialogSearchField"))
		{
			inResetBuffer(puSearchBuffer);
			searchTag.clear();
			prApplyFilteringOnFileList();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(buttonResetSearchString);
		ImGui::SameLine();
		ImGui::Text(searchString);
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		bool edited = ImGui::InputText("##InputImGuiFileDialogSearchField", puSearchBuffer, MAX_FILE_DIALOG_NAME_BUFFER);
		ImGui::PopItemWidth();
		if (edited)
		{
			searchTag = puSearchBuffer;
			prApplyFilteringOnFileList();
		}
	}

	void IGFD::FileDialog::prDrawFileListView(ImVec2 vSize)
	{
		ImGui::BeginChild("##FileDialog_FileList", vSize);

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY |
			ImGuiTableFlags_NoHostExtendY
#ifndef USE_CUSTOM_SORTING_ICON
			| ImGuiTableFlags_Sortable
#endif // USE_CUSTOM_SORTING_ICON
			;
		if (ImGui::BeginTable("##fileTable", 4, flags, vSize))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
			ImGui::TableSetupColumn(prHeaderFileName.c_str(), ImGuiTableColumnFlags_WidthStretch, -1, 0);
			ImGui::TableSetupColumn(prHeaderFileType.c_str(), ImGuiTableColumnFlags_WidthFixed, -1, 1);
			ImGui::TableSetupColumn(prHeaderFileSize.c_str(), ImGuiTableColumnFlags_WidthFixed, -1, 2);
			ImGui::TableSetupColumn(prHeaderFileDate.c_str(), ImGuiTableColumnFlags_WidthFixed, -1, 3);

#ifndef USE_CUSTOM_SORTING_ICON
			// Sort our data if sort specs have been changed!
			if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
			{
				if (sorts_specs->SpecsDirty && !prFileList.empty())
				{
					if (sorts_specs->Specs->ColumnUserID == 0)
						prSortFields(SortingFieldEnum::FIELD_FILENAME, true);
					else if (sorts_specs->Specs->ColumnUserID == 1)
						prSortFields(SortingFieldEnum::FIELD_TYPE, true);
					else if (sorts_specs->Specs->ColumnUserID == 2)
						prSortFields(SortingFieldEnum::FIELD_SIZE, true);
					else //if (sorts_specs->Specs->ColumnUserID == 3) => alwayd true for the moment, to uncomment if we add a fourth column
						prSortFields(SortingFieldEnum::FIELD_DATE, true);

					sorts_specs->SpecsDirty = false;
				}
			}

			ImGui::TableHeadersRow();
#else // USE_CUSTOM_SORTING_ICON
			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			for (int column = 0; column < 4; column++)
			{
				ImGui::TableSetColumnIndex(column);
				const char* column_name = ImGui::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
				ImGui::PushID(column);
				ImGui::TableHeader(column_name);
				ImGui::PopID();
				if (ImGui::IsItemClicked())
				{
					if (column == 0)
						prSortFields(SortingFieldEnum::FIELD_FILENAME, true);
					else if (column == 1)
						prSortFields(SortingFieldEnum::FIELD_TYPE, true);
					else if (column == 2)
						prSortFields(SortingFieldEnum::FIELD_SIZE, true);
					else //if (column == 3) => alwayd true for the moment, to uncomment if we add a fourth column
						prSortFields(SortingFieldEnum::FIELD_DATE, true);
				}
			}
#endif // USE_CUSTOM_SORTING_ICON
			if (!prFilteredFileList.empty())
			{
				prFileListClipper.Begin((int)prFilteredFileList.size(), ImGui::GetTextLineHeightWithSpacing());
				while (prFileListClipper.Step())
				{
					for (int i = prFileListClipper.DisplayStart; i < prFileListClipper.DisplayEnd; i++)
					{
						if (i < 0) continue;

						const FileInfoStruct& infos = prFilteredFileList[i];

						ImVec4 c;
						std::string icon;
						bool showColor = GetExtentionInfos(infos.ext, &c, &icon);
						if (showColor)
							ImGui::PushStyleColor(ImGuiCol_Text, c);

						std::string str = " " + infos.fileName;
						if (infos.type == 'd') str = dirEntryString + str;
						else if (infos.type == 'l') str = linkEntryString + str;
						else if (infos.type == 'f')
						{
							if (showColor && !icon.empty())
								str = icon + str;
							else
								str = fileEntryString + str;
						}
						bool selected = (prSelectedFileNames.find(infos.fileName) != prSelectedFileNames.end()); // found

						ImGui::TableNextRow();

						bool needToBreakTheloop = false;

						if (ImGui::TableNextColumn()) // file name
						{
							needToBreakTheloop = prSelectableItem(i, infos, selected, str.c_str());
						}
						if (ImGui::TableNextColumn()) // file type
						{
							ImGui::Text("%s", infos.ext.c_str());
						}
						if (ImGui::TableNextColumn()) // file size
						{
							if (infos.type != 'd')
							{
								ImGui::Text("%s ", infos.formatedFileSize.c_str());
							}
							else
							{
								ImGui::Text("");
							}
						}
						if (ImGui::TableNextColumn()) // file date + time
						{
							ImGui::Text("%s", infos.fileModifDate.c_str());
						}

						if (showColor)
							ImGui::PopStyleColor();

						if (needToBreakTheloop)
							break;
					}
				}
				prFileListClipper.End();
			}

			if (prInputPathActivated)
			{
				auto gio = ImGui::GetIO();
				if (ImGui::IsKeyReleased(gio.KeyMap[ImGuiKey_Enter]))
				{
					prSetPath(std::string(puInputPathBuffer));
					prInputPathActivated = false;
				}
				if (ImGui::IsKeyReleased(gio.KeyMap[ImGuiKey_Escape]))
				{
					prInputPathActivated = false;
				}
			}
#ifdef USE_EXPLORATION_BY_KEYS
			else
			{
				prLocateByInputKey();
				prExploreWithkeys();
			}
#endif // USE_EXPLORATION_BY_KEYS
			ImGui::EndTable();
		}
		// changement de repertoire
		if (prPathClicked)
		{
			prSetPath(prCurrentPath);
		}

		if (prDrivesClicked)
		{
			prGetDrives();
		}

		ImGui::EndChild();
	}

#ifdef USE_THUMBNAILS
	void IGFD::FileDialog::prDrawThumbnailsListView(ImVec2 vSize)
	{
		ImGui::BeginChild("##FileDialog_FileList", vSize);

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY |
			ImGuiTableFlags_NoHostExtendY
#ifndef USE_CUSTOM_SORTING_ICON
			| ImGuiTableFlags_Sortable
#endif // USE_CUSTOM_SORTING_ICON
			;
		if (ImGui::BeginTable("##fileTable", 5, flags, vSize))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
			ImGui::TableSetupColumn(prHeaderFileName.c_str(), ImGuiTableColumnFlags_WidthStretch, -1, 0);
			ImGui::TableSetupColumn(prHeaderFileType.c_str(), ImGuiTableColumnFlags_WidthFixed, -1, 1);
			ImGui::TableSetupColumn(prHeaderFileSize.c_str(), ImGuiTableColumnFlags_WidthFixed, -1, 2);
			ImGui::TableSetupColumn(prHeaderFileDate.c_str(), ImGuiTableColumnFlags_WidthFixed, -1, 3);
			ImGui::TableSetupColumn("Thumb", ImGuiTableColumnFlags_WidthFixed, -1, 4);

#ifndef USE_CUSTOM_SORTING_ICON
			// Sort our data if sort specs have been changed!
			if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
			{
				if (sorts_specs->SpecsDirty && !prFileList.empty())
				{
					if (sorts_specs->Specs->ColumnUserID == 0)
						prSortFields(SortingFieldEnum::FIELD_FILENAME, true);
					else if (sorts_specs->Specs->ColumnUserID == 1)
						prSortFields(SortingFieldEnum::FIELD_TYPE, true);
					else if (sorts_specs->Specs->ColumnUserID == 2)
						prSortFields(SortingFieldEnum::FIELD_SIZE, true);
					else //if (sorts_specs->Specs->ColumnUserID == 3) => alwayd true for the moment, to uncomment if we add a fourth column
						prSortFields(SortingFieldEnum::FIELD_DATE, true);

					sorts_specs->SpecsDirty = false;
				}
			}

			ImGui::TableHeadersRow();
#else // USE_CUSTOM_SORTING_ICON
			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			for (int column = 0; column < 4; column++)
			{
				ImGui::TableSetColumnIndex(column);
				const char* column_name = ImGui::TableGetColumnName(column); // Retrieve name passed to TableSetupColumn()
				ImGui::PushID(column);
				ImGui::TableHeader(column_name);
				ImGui::PopID();
				if (ImGui::IsItemClicked())
				{
					if (column == 0)
						prSortFields(SortingFieldEnum::FIELD_FILENAME, true);
					else if (column == 1)
						prSortFields(SortingFieldEnum::FIELD_TYPE, true);
					else if (column == 2)
						prSortFields(SortingFieldEnum::FIELD_SIZE, true);
					else //if (column == 3) => alwayd true for the moment, to uncomment if we add a fourth column
						prSortFields(SortingFieldEnum::FIELD_DATE, true);
				}
			}
#endif // USE_CUSTOM_SORTING_ICON
			if (!prFilteredFileList.empty())
			{
				prFileListClipper.Begin((int)prFilteredFileList.size(), ImMax(ImGui::GetTextLineHeightWithSpacing(), DisplayMode_ThumbailsList_ImageHeight));
				while (prFileListClipper.Step())
				{
					for (int i = prFileListClipper.DisplayStart; i < prFileListClipper.DisplayEnd; i++)
					{
						if (i < 0) continue;

						const FileInfoStruct& infos = prFilteredFileList[i];

						ImVec4 c;
						std::string icon;
						bool showColor = GetExtentionInfos(infos.ext, &c, &icon);
						if (showColor)
							ImGui::PushStyleColor(ImGuiCol_Text, c);

						std::string str = " " + infos.fileName;
						if (infos.type == 'd') str = dirEntryString + str;
						else if (infos.type == 'l') str = linkEntryString + str;
						else if (infos.type == 'f')
						{
							if (showColor && !icon.empty())
								str = icon + str;
							else
								str = fileEntryString + str;
						}
						bool selected = (prSelectedFileNames.find(infos.fileName) != prSelectedFileNames.end()); // found

						ImGui::TableNextRow();

						bool needToBreakTheloop = false;

						if (ImGui::TableNextColumn()) // file name
						{
							needToBreakTheloop = prSelectableItem(i, infos, selected, str.c_str());
						}
						if (ImGui::TableNextColumn()) // file type
						{
							ImGui::Text("%s", infos.ext.c_str());
						}
						if (ImGui::TableNextColumn()) // file size
						{
							if (infos.type != 'd')
							{
								ImGui::Text("%s ", infos.formatedFileSize.c_str());
							}
							else
							{
								ImGui::Text("");
							}
						}
						if (ImGui::TableNextColumn()) // file date + time
						{
							ImGui::Text("%s", infos.fileModifDate.c_str());
						}
						if (ImGui::TableNextColumn())
						{
							ImGui::Image(infos.texture, ImVec2(DisplayMode_ThumbailsList_ImageHeight, DisplayMode_ThumbailsList_ImageHeight));
						}

						if (showColor)
							ImGui::PopStyleColor();

						if (needToBreakTheloop)
							break;
					}
				}
				prFileListClipper.End();
			}

			if (prInputPathActivated)
			{
				auto gio = ImGui::GetIO();
				if (ImGui::IsKeyReleased(gio.KeyMap[ImGuiKey_Enter]))
				{
					prSetPath(std::string(puInputPathBuffer));
					prInputPathActivated = false;
				}
				if (ImGui::IsKeyReleased(gio.KeyMap[ImGuiKey_Escape]))
				{
					prInputPathActivated = false;
				}
			}
#ifdef USE_EXPLORATION_BY_KEYS
			else
			{
				prLocateByInputKey();
				prExploreWithkeys();
			}
#endif // USE_EXPLORATION_BY_KEYS
			ImGui::EndTable();
		}
		// changement de repertoire
		if (prPathClicked)
		{
			prSetPath(prCurrentPath);
		}

		if (prDrivesClicked)
		{
			prGetDrives();
		}

		ImGui::EndChild();
	}

	void IGFD::FileDialog::prDrawSmallThumbnailsView(ImVec2 vSize)
	{

	}

	void IGFD::FileDialog::prDrawBigThumbnailsView(ImVec2 vSize)
	{

	}
#endif // USE_THUMBNAILS

	bool IGFD::FileDialog::prSelectableItem(int vidx, const FileInfoStruct& vInfos, bool vSelected, const char* vFmt, ...)
	{
		static ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick | 
			ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_SpanAvailWidth;

		va_list args;
		va_start(args, vFmt);
		vsnprintf(puVariadicBuffer, MAX_FILE_DIALOG_NAME_BUFFER, vFmt, args);
		va_end(args);

		float h = 0.0f;
#ifdef USE_THUMBNAILS
		if (prDisplayMode == DISPLAY_MODE_THUMBAILS_LIST)
			h = DisplayMode_ThumbailsList_ImageHeight;
#endif // USE_THUMBNAILS
#ifdef USE_EXPLORATION_BY_KEYS
		bool flashed = prBeginFlashItem(vidx);
		bool res = prFlashableSelectable(puVariadicBuffer, vSelected, selectableFlags,
			flashed, ImVec2(-1.0f, h));
		if (flashed)
			prEndFlashItem();
#else // USE_EXPLORATION_BY_KEYS
		(void)vidx; // remove a warnings ofr unused var
		
		bool res = ImGui::Selectable(puVariadicBuffer, vSelected, selectableFlags, ImVec2(-1.0f, h));
#endif // USE_EXPLORATION_BY_KEYS
		if (res)
		{
			if (vInfos.type == 'd')
			{
				if (ImGui::IsMouseDoubleClicked(0)) // 0 -> left mouse button double click
				{
					prPathClicked = prSelectDirectory(vInfos); 
				}
				else if (dlg_filters.empty()) // directory chooser
				{
					prSelectFileName(vInfos);
				}

				return true; // needToBreakTheloop
			}
			else
			{
				prSelectFileName(vInfos);
			}
		}

		return false;
	}

	void IGFD::FileDialog::prDrawSidePane(float vHeight)
	{
		ImGui::SameLine();

		ImGui::BeginChild("##FileTypes", ImVec2(0, vHeight));

		dlg_optionsPane(prSelectedFilter.filter.c_str(), dlg_userDatas, &prCanWeContinue);

		ImGui::EndChild();
	}

	void IGFD::FileDialog::Close()
	{
		dlg_key.clear();
		prShowDialog = false;
	}

	bool IGFD::FileDialog::WasOpenedThisFrame(const std::string& vKey)
	{
		bool res = prShowDialog && dlg_key == vKey;
		if (res)
		{
			ImGuiContext& g = *GImGui;
			res &= prLastImGuiFrameCount == g.FrameCount; // return true if a dialog was displayed in this frame
		}
		return res;
	}

	bool IGFD::FileDialog::WasOpenedThisFrame()
	{
		bool res = prShowDialog;
		if (res)
		{
			ImGuiContext& g = *GImGui;
			res &= prLastImGuiFrameCount == g.FrameCount; // return true if a dialog was displayed in this frame
		}
		return res;
	}

	bool IGFD::FileDialog::IsOpened(const std::string& vKey)
	{
		return (prShowDialog && dlg_key == vKey);
	}

	bool IGFD::FileDialog::IsOpened()
	{
		return prShowDialog;
	}

	std::string IGFD::FileDialog::GetOpenedKey()
	{
		if (prShowDialog)
			return dlg_key;
		return "";
	}

	std::string IGFD::FileDialog::GetFilePathName()
	{
		std::string result = GetCurrentPath();

		std::string filename = GetCurrentFileName();
		if (!filename.empty())
		{
#ifdef UNIX
			if (s_fs_root != result)
#endif // UNIX
				result += PATH_SEP;

			result += filename;
		}

		return result;
	}

	std::string IGFD::FileDialog::GetCurrentPath()
	{
		std::string path = prCurrentPath;

		if (dlg_filters.empty()) // if directory mode
		{
			std::string selectedDirectory = puFileNameBuffer;
			if (!selectedDirectory.empty() && 
				selectedDirectory != ".")
				path += PATH_SEP + selectedDirectory;
		}

		return path;
	}

	std::string IGFD::FileDialog::GetCurrentFileName()
	{
		if (!dlg_filters.empty()) // if not directory mode
		{
			std::string result = puFileNameBuffer;

			// if not a collection we can replace the filter by the extention we want
			if (prSelectedFilter.collectionfilters.empty())
			{
				size_t lastPoint = result.find_last_of('.');
				if (lastPoint != std::string::npos)
				{
					result = result.substr(0, lastPoint);
				}

				result += prSelectedFilter.filter;
			}

			return result;
		}

		return ""; // directory mode
	}

	std::string IGFD::FileDialog::GetCurrentFilter()
	{
		return prSelectedFilter.filter;
	}

	UserDatas IGFD::FileDialog::GetUserDatas()
	{
		return dlg_userDatas;
	}

	bool IGFD::FileDialog::IsOk()
	{
		return prIsOk;
	}

	std::map<std::string, std::string> IGFD::FileDialog::GetSelection()
	{
		std::map<std::string, std::string> res;

		for (auto& it : prSelectedFileNames)
		{
			std::string result = GetCurrentPath();

#ifdef UNIX
			if (s_fs_root != result)
#endif // UNIX
				result += PATH_SEP;

			result += it;

			res[it] = result;
		}

		return res;
	}

	void IGFD::FileDialog::SetExtentionInfos(const std::string& vFilter, const FileExtentionInfosStruct& vInfos)
	{
		prFileExtentionInfos[vFilter] = vInfos;
	}

	void IGFD::FileDialog::SetExtentionInfos(const std::string& vFilter, const ImVec4& vColor, const std::string& vIcon)
	{
		prFileExtentionInfos[vFilter] = FileExtentionInfosStruct(vColor, vIcon);
	}

	bool IGFD::FileDialog::GetExtentionInfos(const std::string& vFilter, ImVec4* vOutColor, std::string* vOutIcon)
	{
		if (vOutColor)
		{
			if (prFileExtentionInfos.find(vFilter) != prFileExtentionInfos.end()) // found
			{
				*vOutColor = prFileExtentionInfos[vFilter].color;
				if (vOutIcon)
				{
					*vOutIcon = prFileExtentionInfos[vFilter].icon;
				}
				return true;
			}
		}
		return false;
	}

	void IGFD::FileDialog::ClearExtentionInfos()
	{
		prFileExtentionInfos.clear();
	}

	void IGFD::FileDialog::prSetDefaultFileName(const std::string& vFileName)
	{
		dlg_defaultFileName = vFileName;
		inSetBuffer(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, vFileName);
	}

	bool IGFD::FileDialog::prSelectDirectory(const FileInfoStruct& vInfos)
	{
		bool pathClick = false;

		if (vInfos.fileName == "..")
		{
			if (prCurrentPath_Decomposition.size() > 1)
			{
				prCurrentPath = prComposeNewPath(prCurrentPath_Decomposition.end() - 2);
				pathClick = true;
			}
		}
		else
		{
			std::string newPath;

			if (prShowDrives)
			{
				newPath = vInfos.fileName + PATH_SEP;
			}
			else
			{
#ifdef __linux__
				if (s_fs_root == prCurrentPath)
					newPath = prCurrentPath + vInfos.fileName;
				else
#endif // __minux__
					newPath = prCurrentPath + PATH_SEP + vInfos.fileName;
			}

			if (inIsDirectoryExist(newPath))
			{
				if (prShowDrives)
				{
					prCurrentPath = vInfos.fileName;
					s_fs_root = prCurrentPath;
				}
				else
				{
					prCurrentPath = newPath; //-V820
				}
				pathClick = true;
			}
		}

		return pathClick;
	}

	void IGFD::FileDialog::prSelectFileName(const FileInfoStruct& vInfos)
	{
		if (ImGui::GetIO().KeyCtrl)
		{
			if (dlg_countSelectionMax == 0) // infinite selection
			{
				if (prSelectedFileNames.find(vInfos.fileName) == prSelectedFileNames.end()) // not found +> add
				{
					prAddFileNameInSelection(vInfos.fileName, true);
				}
				else // found +> remove
				{
					prRemoveFileNameInSelection(vInfos.fileName);
				}
			}
			else // selection limited by size
			{
				if (prSelectedFileNames.size() < dlg_countSelectionMax)
				{
					if (prSelectedFileNames.find(vInfos.fileName) == prSelectedFileNames.end()) // not found +> add
					{
						prAddFileNameInSelection(vInfos.fileName, true);
					}
					else // found +> remove
					{
						prRemoveFileNameInSelection(vInfos.fileName);
					}
				}
			}
		}
		else if (ImGui::GetIO().KeyShift)
		{
			if (dlg_countSelectionMax != 1)
			{
				prSelectedFileNames.clear();
				// we will iterate filelist and get the last selection after the start selection
				bool startMultiSelection = false;
				std::string fileNameToSelect = vInfos.fileName;
				std::string savedLastSelectedFileName; // for invert selection mode
				for (auto& it : prFileList)
				{
					const FileInfoStruct& infos = it;

					bool canTake = true;
					if (!searchTag.empty() && infos.fileName.find(searchTag) == std::string::npos) canTake = false;
					if (canTake) // if not filtered, we will take files who are filtered by the dialog
					{
						if (infos.fileName == prLastSelectedFileName)
						{
							startMultiSelection = true;
							prAddFileNameInSelection(prLastSelectedFileName, false);
						}
						else if (startMultiSelection)
						{
							if (dlg_countSelectionMax == 0) // infinite selection
							{
								prAddFileNameInSelection(infos.fileName, false);
							}
							else // selection limited by size
							{
								if (prSelectedFileNames.size() < dlg_countSelectionMax)
								{
									prAddFileNameInSelection(infos.fileName, false);
								}
								else
								{
									startMultiSelection = false;
									if (!savedLastSelectedFileName.empty())
										prLastSelectedFileName = savedLastSelectedFileName;
									break;
								}
							}
						}

						if (infos.fileName == fileNameToSelect)
						{
							if (!startMultiSelection) // we are before the last Selected FileName, so we must inverse
							{
								savedLastSelectedFileName = prLastSelectedFileName;
								prLastSelectedFileName = fileNameToSelect;
								fileNameToSelect = savedLastSelectedFileName;
								startMultiSelection = true;
								prAddFileNameInSelection(prLastSelectedFileName, false);
							}
							else
							{
								startMultiSelection = false;
								if (!savedLastSelectedFileName.empty())
									prLastSelectedFileName = savedLastSelectedFileName;
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			prSelectedFileNames.clear();
			inResetBuffer(puFileNameBuffer);
			prAddFileNameInSelection(vInfos.fileName, true);
		}
	}

	void IGFD::FileDialog::prRemoveFileNameInSelection(const std::string& vFileName)
	{
		prSelectedFileNames.erase(vFileName);

		if (prSelectedFileNames.size() == 1)
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%s", vFileName.c_str());
		}
		else
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%zu files Selected", prSelectedFileNames.size());
		}
	}

	void IGFD::FileDialog::prAddFileNameInSelection(const std::string& vFileName, bool vSetLastSelectionFileName)
	{
		prSelectedFileNames.emplace(vFileName);

		if (prSelectedFileNames.size() == 1)
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%s", vFileName.c_str());
		}
		else
		{
			snprintf(puFileNameBuffer, MAX_FILE_DIALOG_NAME_BUFFER, "%zu files Selected", prSelectedFileNames.size());
		}

		if (vSetLastSelectionFileName)
			prLastSelectedFileName = vFileName;
	}

	void IGFD::FileDialog::prSetPath(const std::string& vPath)
	{
		prShowDrives = false;
		prCurrentPath = vPath;
		prFileList.clear();
		prCurrentPath_Decomposition.clear();
		if (dlg_filters.empty()) // directory mode
			prSetDefaultFileName(".");
		prScanDir(prCurrentPath);
	}

	static std::string sRound_n(double vvalue, int n)
	{
		std::stringstream tmp;
		tmp << std::setprecision(n) << std::fixed << vvalue;
		return tmp.str();
	}

	static void sFormatFileSize(size_t vByteSize, std::string* vFormat)
	{
		if (vFormat && vByteSize != 0)
		{
			static double lo = 1024.0;
			static double ko = 1024.0 * 1024.0;
			static double mo = 1024.0 * 1024.0 * 1024.0;

			double v = (double)vByteSize;

			if (v < lo)
				*vFormat = sRound_n(v, 0) + " o"; // octet
			else if (v < ko)
				*vFormat = sRound_n(v / lo, 2) + " Ko"; // ko
			else  if (v < mo)
				*vFormat = sRound_n(v / ko, 2) + " Mo"; // Mo 
			else
				*vFormat = sRound_n(v / mo, 2) + " Go"; // Go 
		}
	}

	void IGFD::FileDialog::prCompleteFileInfos(FileInfoStruct* vFileInfoStruct)
	{
		if (vFileInfoStruct && 
			vFileInfoStruct->fileName != "." && 
			vFileInfoStruct->fileName != "..")
		{
			// _stat struct :
			//dev_t     st_dev;     /* ID of device containing file */
			//ino_t     st_ino;     /* inode number */
			//mode_t    st_mode;    /* protection */
			//nlink_t   st_nlink;   /* number of hard links */
			//uid_t     st_uid;     /* user ID of owner */
			//gid_t     st_gid;     /* group ID of owner */
			//dev_t     st_rdev;    /* device ID (if special file) */
			//off_t     st_size;    /* total size, in bytes */
			//blksize_t st_blksize; /* blocksize for file system I/O */
			//blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
			//time_t    st_atime;   /* time of last access - not sure out of ntfs */
			//time_t    st_mtime;   /* time of last modification - not sure out of ntfs */
			//time_t    st_ctime;   /* time of last status change - not sure out of ntfs */

			std::string fpn;

			if (vFileInfoStruct->type == 'f') // file
				fpn = vFileInfoStruct->filePath + PATH_SEP + vFileInfoStruct->fileName;
			else if (vFileInfoStruct->type == 'l') // link
				fpn = vFileInfoStruct->filePath + PATH_SEP + vFileInfoStruct->fileName;
			else if (vFileInfoStruct->type == 'd') // directory
				fpn = vFileInfoStruct->filePath + PATH_SEP + vFileInfoStruct->fileName;

			struct stat statInfos;
			char timebuf[100];
			int result = stat(fpn.c_str(), &statInfos);
			if (!result)
			{
				if (vFileInfoStruct->type != 'd')
				{
					vFileInfoStruct->fileSize = (size_t)statInfos.st_size;
					sFormatFileSize(vFileInfoStruct->fileSize,
						&vFileInfoStruct->formatedFileSize);
				}

				size_t len = 0;
#ifdef MSVC
				struct tm _tm;
				errno_t err = localtime_s(&_tm, &statInfos.st_mtime);
				if (!err) len = strftime(timebuf, 99, DateTimeFormat, &_tm);
#else // MSVC
				struct tm* _tm = localtime(&statInfos.st_mtime);
				if (_tm) len = strftime(timebuf, 99, DateTimeFormat, _tm);
#endif // MSVC
				if (len)
				{
					vFileInfoStruct->fileModifDate = std::string(timebuf, len);
				}
			}
		}
	}

	void IGFD::FileDialog::prSortFields(SortingFieldEnum vSortingField, bool vCanChangeOrder)
	{
		if (vSortingField != SortingFieldEnum::FIELD_NONE)
		{
			prHeaderFileName = tableHeaderFileNameString;
			prHeaderFileType = tableHeaderFileTypeString;
			prHeaderFileSize = tableHeaderFileSizeString;
			prHeaderFileDate = tableHeaderFileDateString;
		}

		if (vSortingField == SortingFieldEnum::FIELD_FILENAME)
		{
			if (vCanChangeOrder && prSortingField == vSortingField)
				prSortingDirection[0] = !prSortingDirection[0];

			if (prSortingDirection[0])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileName = tableHeaderDescendingIcon + prHeaderFileName;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type == 'd'); // directory in first
						return (stricmp(a.fileName.c_str(), b.fileName.c_str()) < 0); // sort in insensitive case
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileName = tableHeaderAscendingIcon + prHeaderFileName;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type != 'd'); // directory in last
						return (stricmp(a.fileName.c_str(), b.fileName.c_str()) > 0); // sort in insensitive case
					});
			}
		}
		else if (vSortingField == SortingFieldEnum::FIELD_TYPE)
		{
			if (vCanChangeOrder && prSortingField == vSortingField)
				prSortingDirection[1] = !prSortingDirection[1];

			if (prSortingDirection[1])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileType = tableHeaderDescendingIcon + prHeaderFileType;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type == 'd'); // directory in first
						return (a.ext < b.ext); // else
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileType = tableHeaderAscendingIcon + prHeaderFileType;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type != 'd'); // directory in last
						return (a.ext > b.ext); // else
					});
			}
		}
		else if (vSortingField == SortingFieldEnum::FIELD_SIZE)
		{
			if (vCanChangeOrder && prSortingField == vSortingField)
				prSortingDirection[2] = !prSortingDirection[2];

			if (prSortingDirection[2])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileSize = tableHeaderDescendingIcon + prHeaderFileSize;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type == 'd'); // directory in first
						return (a.fileSize < b.fileSize); // else
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileSize = tableHeaderAscendingIcon + prHeaderFileSize;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type != 'd'); // directory in last
						return (a.fileSize > b.fileSize); // else
					});
			}
		}
		else if (vSortingField == SortingFieldEnum::FIELD_DATE)
		{
			if (vCanChangeOrder && prSortingField == vSortingField)
				prSortingDirection[3] = !prSortingDirection[3];

			if (prSortingDirection[3])
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileDate = tableHeaderDescendingIcon + prHeaderFileDate;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type == 'd'); // directory in first
						return (a.fileModifDate < b.fileModifDate); // else
					});
			}
			else
			{
#ifdef USE_CUSTOM_SORTING_ICON
				prHeaderFileDate = tableHeaderAscendingIcon + prHeaderFileDate;
#endif // USE_CUSTOM_SORTING_ICON
				std::sort(prFileList.begin(), prFileList.end(),
					[](const FileInfoStruct& a, const FileInfoStruct& b) -> bool
					{
						if (a.type != b.type) return (a.type != 'd'); // directory in last
						return (a.fileModifDate > b.fileModifDate); // else
					});
			}
		}

		if (vSortingField != SortingFieldEnum::FIELD_NONE)
		{
			prSortingField = vSortingField;
		}

		prApplyFilteringOnFileList();
	}

	void IGFD::FileDialog::prScanDir(const std::string& vPath)
	{
		struct dirent** files = nullptr;
		int          i = 0;
		int          n = 0;
		std::string		path = vPath;

		if (prCurrentPath_Decomposition.empty())
		{
			prSetCurrentDir(path);
		}

		if (!prCurrentPath_Decomposition.empty())
		{
#ifdef WIN32
			if (path == s_fs_root)
				path += PATH_SEP;
#endif // WIN32
			n = scandir(path.c_str(), &files, nullptr, inAlphaSort);

			prFileList.clear();

			if (n > 0)
			{
				for (i = 0; i < n; i++)
				{
					struct dirent* ent = files[i];

					FileInfoStruct infos;

					infos.filePath = path;
					infos.fileName = ent->d_name;
					infos.fileName_optimized = prOptimizeFilenameForSearchOperations(infos.fileName);

					if (infos.fileName != "." 
						|| dlg_filters.empty()) // in directory mode we must display the curent dir "."
					{
						switch (ent->d_type)
						{
						case DT_REG:
							infos.type = 'f'; break;
						case DT_DIR:
							infos.type = 'd'; break;
						case DT_LNK:
							infos.type = 'l'; break;
						}

						if (infos.type == 'f' ||
							infos.type == 'l') // link can have the same extention of a file
						{
							size_t lpt = infos.fileName.find_last_of('.');
							if (lpt != std::string::npos)
							{
								infos.ext = infos.fileName.substr(lpt);
							}

							if (!dlg_filters.empty())
							{
								// check if current file extention is covered by current filter
								// we do that here, for avoid doing that during filelist display
								// for better fps
								if (!prSelectedFilter.empty() && // selected filter exist
									(!prSelectedFilter.filterExist(infos.ext) && // filter not found
										prSelectedFilter.filter != ".*"))
								{
									continue;
								}
							}
						}

						prCompleteFileInfos(&infos);
						prFileList.push_back(infos);
					}
				}

				for (i = 0; i < n; i++)
				{
					free(files[i]);
				}

				free(files);
			}

			prSortFields(prSortingField);
		}
	}

	void IGFD::FileDialog::prSetCurrentDir(const std::string& vPath)
	{
		std::string path = vPath;
#ifdef WIN32
		if (s_fs_root == path)
			path += PATH_SEP;
#endif // WIN32
		char real_path[PATH_MAX];
		DIR* dir = opendir(path.c_str());
		if (dir == nullptr)
		{
			path = ".";
			dir = opendir(path.c_str());
		}

		if (dir != nullptr)
		{
#ifdef WIN32
			DWORD numchar = 0;
			//			numchar = GetFullPathNameA(path.c_str(), PATH_MAX, real_path, nullptr);
			std::wstring wpath = inWGetString(path.c_str());
			numchar = GetFullPathNameW(wpath.c_str(), 0, nullptr, nullptr);
			std::wstring fpath(numchar, 0);
			GetFullPathNameW(wpath.c_str(), numchar, (wchar_t*)fpath.data(), nullptr);
			int error = dirent_wcstombs_s(nullptr, real_path, PATH_MAX, fpath.c_str(), PATH_MAX - 1);
			if (error)numchar = 0;
			if (!numchar)
			{
				std::cout << "fail to obtain FullPathName " << path << std::endl;
			}
#elif defined(UNIX) // UNIX is LINUX or APPLE
			char* numchar = realpath(path.c_str(), real_path);
#endif // defined(UNIX)
			if (numchar != 0)
			{
				prCurrentPath = real_path;
				if (prCurrentPath[prCurrentPath.size() - 1] == PATH_SEP)
				{
					prCurrentPath = prCurrentPath.substr(0, prCurrentPath.size() - 1);
				}
				inSetBuffer(puInputPathBuffer, MAX_PATH_BUFFER_SIZE, prCurrentPath);
				prCurrentPath_Decomposition = inSplitStringToVector(prCurrentPath, PATH_SEP, false);
#if defined(UNIX) // UNIX is LINUX or APPLE
				prCurrentPath_Decomposition.insert(prCurrentPath_Decomposition.begin(), std::string(1u, PATH_SEP));
#endif // defined(UNIX)
				if (!prCurrentPath_Decomposition.empty())
				{
#ifdef WIN32
					s_fs_root = prCurrentPath_Decomposition[0];
#endif // WIN32
				}
			}

			closedir(dir);
		}
	}

	bool IGFD::FileDialog::prCreateDir(const std::string& vPath)
	{
		bool res = false;

		if (!vPath.empty())
		{
			std::string path = prCurrentPath + PATH_SEP + vPath;

			res = CreateDirectoryIfNotExist(path);
		}

		return res;
	}

	std::string IGFD::FileDialog::prComposeNewPath(std::vector<std::string>::iterator vIter)
	{
		std::string res;

		while (true)
		{
			if (!res.empty())
			{
#ifdef WIN32
				res = *vIter + PATH_SEP + res;
#elif defined(UNIX) // UNIX is LINUX or APPLE
				if (*vIter == s_fs_root)
				{
					res = *vIter + res;
				}
				else
				{
					res = *vIter + PATH_SEP + res;
				}
#endif // defined(UNIX)
			}
			else
			{
				res = *vIter;
			}

			if (vIter == prCurrentPath_Decomposition.begin())
			{
#if defined(UNIX) // UNIX is LINUX or APPLE
				if (res[0] != PATH_SEP)
					res = PATH_SEP + res;
#endif // defined(UNIX)
				break;
			}

			--vIter;
		}

		return res;
	}

	void IGFD::FileDialog::prGetDrives()
	{
		auto drives = inGetDrivesList();
		if (!drives.empty())
		{
			prCurrentPath.clear();
			prCurrentPath_Decomposition.clear();
			prFileList.clear();
			for (auto& drive : drives)
			{
				FileInfoStruct infos;
				infos.fileName = drive;
				infos.fileName_optimized = prOptimizeFilenameForSearchOperations(drive);
				infos.type = 'd';

				if (!infos.fileName.empty())
				{
					prFileList.push_back(infos);
				}
			}
			prShowDrives = true;
			prApplyFilteringOnFileList();
		}
	}

	void IGFD::FileDialog::prParseFilters(const char* vFilters)
	{
		prFilters.clear();

		if (vFilters)
			dlg_filters = vFilters;				// file mode
		else
			dlg_filters.clear();				// directtory mode

		if (!dlg_filters.empty())
		{
			// ".*,.cpp,.h,.hpp"
			// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md"

			bool currentFilterFound = false;

			size_t nan = std::string::npos;
			size_t p = 0, lp = 0;
			while ((p = dlg_filters.find_first_of("{,", p)) != nan)
			{
				FilterInfosStruct infos;

				if (dlg_filters[p] == '{') // {
				{
					infos.filter = dlg_filters.substr(lp, p - lp);
					p++;
					lp = dlg_filters.find('}', p);
					if (lp != nan)
					{
						std::string fs = dlg_filters.substr(p, lp - p);
						auto arr = inSplitStringToVector(fs, ',', false);
						for (auto a : arr)
						{
							infos.collectionfilters.emplace(a);
						}
					}
					p = lp + 1;
				}
				else // ,
				{
					infos.filter = dlg_filters.substr(lp, p - lp);
					p++;
				}

				if (!currentFilterFound && prSelectedFilter.filter == infos.filter)
				{
					currentFilterFound = true;
					prSelectedFilter = infos;
				}

				lp = p;
				if (!infos.empty())
					prFilters.emplace_back(infos);
			}

			std::string token = dlg_filters.substr(lp);
			if (!token.empty())
			{
				FilterInfosStruct infos;
				infos.filter = token;
				prFilters.emplace_back(infos);
			}

			if (!currentFilterFound)
				if (!prFilters.empty())
					prSelectedFilter = *prFilters.begin();
		}
	}

	void IGFD::FileDialog::prSetSelectedFilterWithExt(const std::string& vFilter)
	{
		if (!prFilters.empty())
		{
			if (!vFilter.empty())
			{
				// std::map<std::string, FilterInfosStruct>
				for (const auto& infos : prFilters)
				{
					if (vFilter == infos.filter)
					{
						prSelectedFilter = infos;
					}
					else
					{
						// maybe this ext is in an extention so we will 
						// explore the collections is they are existing
						for (const auto& filter : infos.collectionfilters)
						{
							if (vFilter == filter)
							{
								prSelectedFilter = infos;
							}
						}
					}
				}
			}

			if (prSelectedFilter.empty())
				prSelectedFilter = *prFilters.begin();
		}
	}

	std::string IGFD::FileDialog::prOptimizeFilenameForSearchOperations(std::string vFileName)
	{
		// convert to lower case
		for (char& c : vFileName)
			c = (char)std::tolower(c);
		return vFileName;
	}

	void IGFD::FileDialog::prApplyFilteringOnFileList()
	{
		prFilteredFileList.clear();

		for (auto& it : prFileList)
		{
			const FileInfoStruct& infos = it;

			bool show = true;

			// if search tag
			if (!searchTag.empty() &&
				infos.fileName_optimized.find(searchTag) == std::string::npos && // first try wihtout case and accents
				infos.fileName.find(searchTag) == std::string::npos) // second if searched with case and accents
			{
				show = false;
			}

			if (dlg_filters.empty() && infos.type != 'd') // directory mode
			{
				show = false;
			}

			if (show)
			{
				prFilteredFileList.push_back(infos);
			}
		}
	}

#ifdef USE_EXPLORATION_BY_KEYS

	//////////////////////////////////////////////////////////////////////////////
	//// LOCATE / EXPLORE WITH KEYS //////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	bool IGFD::FileDialog::prLocateItem_Loop(ImWchar vC)
	{
		bool found = false;

		for (size_t i = prLocateFileByInputChar_lastFileIdx; i < prFilteredFileList.size(); i++)
		{
			if (prFilteredFileList[i].fileName_optimized[0] == vC || // lower case search
				prFilteredFileList[i].fileName[0] == vC) // maybe upper case search
			{
				//float p = ((float)i) * ImGui::GetTextLineHeightWithSpacing();
				float p = (float)((double)i / (double)prFilteredFileList.size()) * ImGui::GetScrollMaxY();
				ImGui::SetScrollY(p);
				prLocateFileByInputChar_lastFound = true;
				prLocateFileByInputChar_lastFileIdx = i;
				prStartFlashItem(prLocateFileByInputChar_lastFileIdx);

				auto infos = &prFilteredFileList[prLocateFileByInputChar_lastFileIdx];

				if (infos->type == 'd')
				{
					if (dlg_filters.empty()) // directory chooser
					{
						prSelectFileName(*infos);
					}
				}
				else
				{
					prSelectFileName(*infos);
				}

				found = true;

				break;
			}
		}

		return found;
	}

	void IGFD::FileDialog::prLocateByInputKey()
	{
		ImGuiContext& g = *GImGui;
		if (!g.ActiveId && !prFilteredFileList.empty())
		{
			auto& queueChar = ImGui::GetIO().InputQueueCharacters;

			// point by char
			if (!queueChar.empty())
			{
				ImWchar c = queueChar.back();
				if (prLocateFileByInputChar_InputQueueCharactersSize != queueChar.size())
				{
					if (c == prLocateFileByInputChar_lastChar) // next file starting with same char until
					{
						if (prLocateFileByInputChar_lastFileIdx < prFilteredFileList.size() - 1)
							prLocateFileByInputChar_lastFileIdx++;
						else
							prLocateFileByInputChar_lastFileIdx = 0;
					}

					if (!prLocateItem_Loop(c))
					{
						// not found, loop again from 0 this time
						prLocateFileByInputChar_lastFileIdx = 0;
						prLocateItem_Loop(c);
					}

					prLocateFileByInputChar_lastChar = c;
				}
			}

			prLocateFileByInputChar_InputQueueCharactersSize = queueChar.size();
		}
	}

	void IGFD::FileDialog::prExploreWithkeys()
	{
		ImGuiContext& g = *GImGui;
		if (!g.ActiveId && !prFilteredFileList.empty())
		{
			// explore
			bool exploreByKey = false;
			bool enterInDirectory = false;
			bool exitDirectory = false;
			if (ImGui::IsKeyPressed(IGFD_KEY_UP))
			{
				exploreByKey = true;
				if (prLocateFileByInputChar_lastFileIdx > 0)
					prLocateFileByInputChar_lastFileIdx--;
			}
			else if (ImGui::IsKeyPressed(IGFD_KEY_DOWN))
			{
				exploreByKey = true;
				if (prLocateFileByInputChar_lastFileIdx < prFilteredFileList.size() - 1)
					prLocateFileByInputChar_lastFileIdx++;
			}
			else if (ImGui::IsKeyReleased(IGFD_KEY_ENTER) && ImGui::IsWindowHovered())
			{
				exploreByKey = true;
				enterInDirectory = true;
			}
			else if (ImGui::IsKeyReleased(IGFD_KEY_BACKSPACE) && ImGui::IsWindowHovered())
			{
				exploreByKey = true;
				exitDirectory = true;
			}

			if (exploreByKey)
			{
				//float totalHeight = prFilteredFileList.size() * ImGui::GetTextLineHeightWithSpacing();
				float p = (float)((double)prLocateFileByInputChar_lastFileIdx / (double)prFilteredFileList.size()) * ImGui::GetScrollMaxY();// seems not udpated in tables version outside tables
				//float p = ((float)locateFileByInputChar_lastFileIdx) * ImGui::GetTextLineHeightWithSpacing();
				ImGui::SetScrollY(p);
				prStartFlashItem(prLocateFileByInputChar_lastFileIdx);

				auto infos = &prFilteredFileList[prLocateFileByInputChar_lastFileIdx];

				if (infos->type == 'd')
				{
					if (!dlg_filters.empty() || enterInDirectory)
					{
						if (enterInDirectory)
						{
							if (prSelectDirectory(*infos))
							{
								// changement de repertoire
								prSetPath(prCurrentPath);
								if (prLocateFileByInputChar_lastFileIdx > prFilteredFileList.size() - 1)
								{
									prLocateFileByInputChar_lastFileIdx = 0;
								}
							}
						}
					}
					else // directory chooser
					{
						prSelectFileName(*infos);
					}
				}
				else
				{
					prSelectFileName(*infos);
				}

				if (exitDirectory)
				{
					FileInfoStruct nfo;
					nfo.fileName = "..";

					if (prSelectDirectory(nfo))
					{
						// changement de repertoire
						prSetPath(prCurrentPath);
						if (prLocateFileByInputChar_lastFileIdx > prFilteredFileList.size() - 1)
						{
							prLocateFileByInputChar_lastFileIdx = 0;
						}
					}
#ifdef WIN32
					else
					{
						if (prCurrentPath_Decomposition.size() == 1)
						{
							prGetDrives(); // display drives
						}
					}
#endif // WIN32
				}
			}
		}
	}

	void IGFD::FileDialog::prStartFlashItem(size_t vIdx)
	{
		prFlashAlpha = 1.0f;
		prFlashedItem = vIdx;
	}

	bool IGFD::FileDialog::prBeginFlashItem(size_t vIdx)
	{
		bool res = false;

		if (prFlashedItem == vIdx &&
			std::abs(prFlashAlpha - 0.0f) > 0.00001f)
		{
			prFlashAlpha -= prFlashAlphaAttenInSecs * ImGui::GetIO().DeltaTime;
			if (prFlashAlpha < 0.0f) prFlashAlpha = 0.0f;

			ImVec4 hov = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
			hov.w = prFlashAlpha;
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hov);
			res = true;
		}

		return res;
	}

	void IGFD::FileDialog::prEndFlashItem()
	{
		ImGui::PopStyleColor();
	}

	void IGFD::FileDialog::SetFlashingAttenuationInSeconds(float vAttenValue)
	{
		prFlashAlphaAttenInSecs = 1.0f / ImMax(vAttenValue, 0.01f);
	}
#endif // USE_EXPLORATION_BY_KEYS

#ifdef USE_BOOKMARK

	//////////////////////////////////////////////////////////////////////////////
	//// BOOKMARK FEATURE ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	void IGFD::FileDialog::prDrawBookmarkPane(ImVec2 vSize)
	{
		ImGui::BeginChild("##bookmarkpane", vSize);

		static int selectedBookmarkForEdition = -1;

		if (IMGUI_BUTTON(addBookmarkButtonString "##ImGuiFileDialogAddBookmark"))
		{
			if (!prCurrentPath_Decomposition.empty())
			{
				BookmarkStruct bookmark;
				bookmark.name = prCurrentPath_Decomposition.back();
				bookmark.path = prCurrentPath;
				prBookmarks.push_back(bookmark);
			}
		}
		if (selectedBookmarkForEdition >= 0 &&
			selectedBookmarkForEdition < (int)prBookmarks.size())
		{
			ImGui::SameLine();
			if (IMGUI_BUTTON(removeBookmarkButtonString "##ImGuiFileDialogAddBookmark"))
			{
				prBookmarks.erase(prBookmarks.begin() + selectedBookmarkForEdition);
				if (selectedBookmarkForEdition == (int)prBookmarks.size())
					selectedBookmarkForEdition--;
			}

			if (selectedBookmarkForEdition >= 0 &&
				selectedBookmarkForEdition < (int)prBookmarks.size())
			{
				ImGui::SameLine();

				ImGui::PushItemWidth(vSize.x - ImGui::GetCursorPosX());
				if (ImGui::InputText("##ImGuiFileDialogBookmarkEdit", puBookmarkEditBuffer, MAX_FILE_DIALOG_NAME_BUFFER))
				{
					prBookmarks[selectedBookmarkForEdition].name = std::string(puBookmarkEditBuffer);
				}
				ImGui::PopItemWidth();
			}
		}

		ImGui::Separator();

		if (!prBookmarks.empty())
		{
			prBookmarkClipper.Begin((int)prBookmarks.size(), ImGui::GetTextLineHeightWithSpacing());
			while (prBookmarkClipper.Step())
			{
				for (int i = prBookmarkClipper.DisplayStart; i < prBookmarkClipper.DisplayEnd; i++)
				{
					if (i < 0) continue;
					const BookmarkStruct& bookmark = prBookmarks[i];
					ImGui::PushID(i);
					if (ImGui::Selectable(bookmark.name.c_str(), selectedBookmarkForEdition == i,
						ImGuiSelectableFlags_AllowDoubleClick) |
						(selectedBookmarkForEdition == -1 &&
							bookmark.path == prCurrentPath)) // select if path is current
					{
						selectedBookmarkForEdition = i;
						inResetBuffer(puBookmarkEditBuffer);
						inAppendToBuffer(puBookmarkEditBuffer, MAX_FILE_DIALOG_NAME_BUFFER, bookmark.name);

						if (ImGui::IsMouseDoubleClicked(0)) // apply path
						{
							prSetPath(bookmark.path);
						}
					}
					ImGui::PopID();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("%s", bookmark.path.c_str());
				}
			}
			prBookmarkClipper.End();
		}

		ImGui::EndChild();
	}

	std::string IGFD::FileDialog::SerializeBookmarks()
	{
		std::string res;

		size_t idx = 0;
		for (auto& it : prBookmarks)
		{
			if (idx++ != 0)
				res += "##"; // ## because reserved by imgui, so an input text cant have ##
			res += it.name + "##" + it.path;
		}

		return res;
	}

	void IGFD::FileDialog::DeserializeBookmarks(const std::string& vBookmarks)
	{
		if (!vBookmarks.empty())
		{
			prBookmarks.clear();
			auto arr = inSplitStringToVector(vBookmarks, '#', false);
			for (size_t i = 0; i < arr.size(); i += 2)
			{
				BookmarkStruct bookmark;
				bookmark.name = arr[i];
				if (i + 1 < arr.size()) // for avoid crash if arr size is impair due to user mistake after edition
				{
					// if bad format we jump this bookmark
					bookmark.path = arr[i + 1];
					prBookmarks.push_back(bookmark);
				}
			}
		}
	}
#endif // USE_BOOKMARK

	//////////////////////////////////////////////////////////////////////////////
	//// OVERWRITE DIALOG ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	bool IGFD::FileDialog::prConfirm_Or_OpenOverWriteFileDialog_IfNeeded(bool vLastAction, ImGuiWindowFlags vFlags)
	{
		// if confirmation => return true for confirm the overwrite et quit the dialog
		// if cancel => return false && set IsOk to false for keep inside the dialog

		// if IsOk == false => return false for quit the dialog
		if (!prIsOk && vLastAction)
		{
			return true;
		}

		// if IsOk == true && no check of overwrite => return true for confirm the dialog
		if (prIsOk && vLastAction && !(dlg_flags & ImGuiFileDialogFlags_ConfirmOverwrite))
		{
			return true;
		}

		// if IsOk == true && check of overwrite => return false and show confirm to overwrite dialog
		if ((prOkResultToConfirm || (prIsOk && vLastAction)) && (dlg_flags & ImGuiFileDialogFlags_ConfirmOverwrite))
		{
			if (prIsOk) // catched only one time
			{
				if (!prIsFileExist(GetFilePathName())) // not existing => quit dialog
				{
					return true;
				}
				else // existing => confirm dialog to open
				{
					prIsOk = false;
					prOkResultToConfirm = true;
				}
			}

			std::string name = OverWriteDialogTitleString "##" + dlg_title + dlg_key + "OverWriteDialog";

			bool res = false;

			ImGui::OpenPopup(name.c_str());
			if (ImGui::BeginPopupModal(name.c_str(), (bool*)0,
				vFlags | ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				ImGui::SetWindowPos(prDialogCenterPos - ImGui::GetWindowSize() * 0.5f); // next frame needed for GetWindowSize to work

				ImGui::Text("%s", OverWriteDialogMessageString);

				if (IMGUI_BUTTON(OverWriteDialogConfirmButtonString))
				{
					prOkResultToConfirm = false;
					prIsOk = true;
					res = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (IMGUI_BUTTON(OverWriteDialogCancelButtonString))
				{
					prOkResultToConfirm = false;
					prIsOk = false;
					res = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			return res;
		}


		return false;
	}

	bool IGFD::FileDialog::prIsFileExist(const std::string& vFile)
	{
		std::ifstream docFile(vFile, std::ios::in);
		if (docFile.is_open())
		{
			docFile.close();
			return true;
		}
		return false;
	}
}

#endif // __cplusplus

/////////////////////////////////////////////////////////////////
///// C Interface ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Return an initialized IGFD_Selection_Pair
IMGUIFILEDIALOG_API IGFD_Selection_Pair IGFD_Selection_Pair_Get(void)
{
	IGFD_Selection_Pair res;
	res.fileName = 0;
	res.filePathName = 0;
	return res;
}

// destroy only the content of vSelection_Pair
IMGUIFILEDIALOG_API void IGFD_Selection_Pair_DestroyContent(IGFD_Selection_Pair* vSelection_Pair)
{
	if (vSelection_Pair)
	{
		if (vSelection_Pair->fileName)
			delete[] vSelection_Pair->fileName;
		if (vSelection_Pair->filePathName)
			delete[] vSelection_Pair->filePathName;
	}
}

// Return an initialized IGFD_Selection
IMGUIFILEDIALOG_API IGFD_Selection IGFD_Selection_Get(void)
{
	return { 0, 0U };
}

// destroy only the content of vSelection
IMGUIFILEDIALOG_API void IGFD_Selection_DestroyContent(IGFD_Selection* vSelection)
{
	if (vSelection)
	{
		if (vSelection->table)
		{
			for (size_t i = 0U; i < vSelection->count; i++)
			{
				IGFD_Selection_Pair_DestroyContent(&vSelection->table[i]);
			}
			delete[] vSelection->table;
		}
		vSelection->count = 0U;
	}
}

// create an instance of ImGuiFileDialog
IMGUIFILEDIALOG_API ImGuiFileDialog* IGFD_Create(void)
{
	return new ImGuiFileDialog();
}

// destroy the instance of ImGuiFileDialog
IMGUIFILEDIALOG_API void IGFD_Destroy(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		delete vContext;
		vContext = nullptr;
	}
}

// standard dialog
IMGUIFILEDIALOG_API void IGFD_OpenDialog(
	ImGuiFileDialog* vContext,
	const char* vKey, 
	const char* vTitle, 
	const char* vFilters, 
	const char* vPath,
	const char* vFileName, 
	const int vCountSelectionMax, 
	void* vUserDatas, 
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters, vPath, vFileName,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenDialog2(
	ImGuiFileDialog* vContext,
	const char* vKey, 
	const char* vTitle, 
	const char* vFilters, 
	const char* vFilePathName,
	const int vCountSelectionMax, 
	void* vUserDatas, 
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters, vFilePathName,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenPaneDialog(
	ImGuiFileDialog* vContext,
	const char* vKey, 
	const char* vTitle, 
	const char* vFilters, 
	const char* vPath,
	const char* vFileName, 
	IGFD_PaneFun vSidePane, 
	const float vSidePaneWidth,
	const int vCountSelectionMax, 
	void* vUserDatas, 
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters, 
			vPath, vFileName,
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenPaneDialog2(
	ImGuiFileDialog* vContext,
	const char* vKey, 
	const char* vTitle, 
	const char* vFilters, 
	const char* vFilePathName, 
	IGFD_PaneFun vSidePane, 
	const float vSidePaneWidth,
	const int vCountSelectionMax, 
	void* vUserDatas, 
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters, 
			vFilePathName,
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, flags);
	}
}

// modal dialog
IMGUIFILEDIALOG_API void IGFD_OpenModal(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vPath,
	const char* vFileName,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenModal(
			vKey, vTitle, vFilters, vPath, vFileName,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenModal2(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vFilePathName,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenModal(
			vKey, vTitle, vFilters, vFilePathName,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenPaneModal(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vPath,
	const char* vFileName,
	IGFD_PaneFun vSidePane,
	const float vSidePaneWidth,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenModal(
			vKey, vTitle, vFilters,
			vPath, vFileName,
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API void IGFD_OpenPaneModal2(
	ImGuiFileDialog* vContext,
	const char* vKey,
	const char* vTitle,
	const char* vFilters,
	const char* vFilePathName,
	IGFD_PaneFun vSidePane,
	const float vSidePaneWidth,
	const int vCountSelectionMax,
	void* vUserDatas,
	ImGuiFileDialogFlags flags)
{
	if (vContext)
	{
		vContext->OpenDialog(
			vKey, vTitle, vFilters,
			vFilePathName,
			vSidePane, vSidePaneWidth,
			vCountSelectionMax, vUserDatas, flags);
	}
}

IMGUIFILEDIALOG_API bool IGFD_DisplayDialog(ImGuiFileDialog* vContext, 
	const char* vKey, ImGuiWindowFlags vFlags, ImVec2 vMinSize, ImVec2 vMaxSize)
{
	if (vContext)
	{
		return vContext->Display(vKey, vFlags, vMinSize, vMaxSize);
	}

	return false;
}

IMGUIFILEDIALOG_API void IGFD_CloseDialog(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		vContext->Close();
	}
}

IMGUIFILEDIALOG_API bool IGFD_IsOk(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		return vContext->IsOk();
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_WasKeyOpenedThisFrame(ImGuiFileDialog* vContext, 
	const char* vKey)
{
	if (vContext)
	{
		vContext->WasOpenedThisFrame(vKey);
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_WasOpenedThisFrame(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		vContext->WasOpenedThisFrame();
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_IsKeyOpened(ImGuiFileDialog* vContext, 
	const char* vCurrentOpenedKey)
{
	if (vContext)
	{
		vContext->IsOpened(vCurrentOpenedKey);
	}

	return false;
}

IMGUIFILEDIALOG_API bool IGFD_IsOpened(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		vContext->IsOpened();
	}

	return false;
}

IMGUIFILEDIALOG_API IGFD_Selection IGFD_GetSelection(ImGuiFileDialog* vContext)
{
	IGFD_Selection res = IGFD_Selection_Get();

	if (vContext)
	{
		auto sel = vContext->GetSelection();
		if (!sel.empty())
		{
			res.count = sel.size();
			res.table = new IGFD_Selection_Pair[res.count];

			size_t idx = 0U;
			for (auto s : sel)
			{
				IGFD_Selection_Pair* pair = res.table + idx++;

				// fileName
				if (!s.first.empty())
				{
					size_t siz = s.first.size() + 1U;
					pair->fileName = new char[siz];
#ifndef MSVC
					strncpy(pair->fileName, s.first.c_str(), siz);
#else
					strncpy_s(pair->fileName, siz, s.first.c_str(), siz);
#endif
					pair->fileName[siz - 1U] = '\0';
				}

				// filePathName
				if (!s.second.empty())
				{
					size_t siz = s.first.size() + 1U;
					pair->filePathName = new char[siz];
#ifndef MSVC
					strncpy(pair->filePathName, s.first.c_str(), siz);
#else
					strncpy_s(pair->filePathName, siz, s.first.c_str(), siz);
#endif
					pair->filePathName[siz - 1U] = '\0';
				}
			}

			return res;
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetFilePathName(ImGuiFileDialog* vContext)
{
	char* res = 0;

	if (vContext)
	{
		auto s = vContext->GetFilePathName();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = new char[siz];
#ifndef MSVC
			strncpy(res, s.c_str(), siz);
#else
			strncpy_s(res, siz, s.c_str(), siz);
#endif
			res[siz - 1U] = '\0';
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetCurrentFileName(ImGuiFileDialog* vContext)
{
	char* res = 0;

	if (vContext)
	{
		auto s = vContext->GetCurrentFileName();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = new char[siz];
#ifndef MSVC
			strncpy(res, s.c_str(), siz);
#else
			strncpy_s(res, siz, s.c_str(), siz);
#endif
			res[siz - 1U] = '\0';
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetCurrentPath(ImGuiFileDialog* vContext)
{
	char* res = 0;

	if (vContext)
	{
		auto s = vContext->GetCurrentPath();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = new char[siz];
#ifndef MSVC
			strncpy(res, s.c_str(), siz);
#else
			strncpy_s(res, siz, s.c_str(), siz);
#endif
			res[siz - 1U] = '\0';
		}
	}

	return res;
}

IMGUIFILEDIALOG_API char* IGFD_GetCurrentFilter(ImGuiFileDialog* vContext)
{
	char* res = 0;

	if (vContext)
	{
		auto s = vContext->GetCurrentFilter();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = new char[siz];
#ifndef MSVC
			strncpy(res, s.c_str(), siz);
#else
			strncpy_s(res, siz, s.c_str(), siz);
#endif
			res[siz - 1U] = '\0';
		}
	}

	return res;
}

IMGUIFILEDIALOG_API void* IGFD_GetUserDatas(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		return vContext->GetUserDatas();
	}

	return nullptr;
}

IMGUIFILEDIALOG_API void IGFD_SetExtentionInfos(ImGuiFileDialog* vContext,
	const char* vFilter, ImVec4 vColor, const char* vIcon)
{
	if (vContext)
	{
		vContext->SetExtentionInfos(vFilter, vColor, vIcon);
	}
}

IMGUIFILEDIALOG_API void IGFD_SetExtentionInfos2(ImGuiFileDialog* vContext,
	const char* vFilter, float vR, float vG, float vB, float vA, const char* vIcon)
{
	if (vContext)
	{
		vContext->SetExtentionInfos(vFilter, ImVec4(vR, vG, vB, vA), vIcon);
	}
}

IMGUIFILEDIALOG_API bool IGFD_GetExtentionInfos(ImGuiFileDialog* vContext,
	const char* vFilter, ImVec4* vOutColor, char** vOutIcon)
{
	if (vContext)
	{
		std::string icon;
		bool res = vContext->GetExtentionInfos(vFilter, vOutColor, &icon);
		if (!icon.empty() && vOutIcon)
		{
			size_t siz = icon.size() + 1U;
			*vOutIcon = new char[siz];
#ifndef MSVC
			strncpy(*vOutIcon, icon.c_str(), siz);
#else
			strncpy_s(*vOutIcon, siz, icon.c_str(), siz);
#endif
			*vOutIcon[siz - 1U] = '\0';
		}
		return res;
	}

	return false;
}

IMGUIFILEDIALOG_API void IGFD_ClearExtentionInfos(ImGuiFileDialog* vContext)
{
	if (vContext)
	{
		vContext->ClearExtentionInfos();
	}
}

#ifdef USE_EXPLORATION_BY_KEYS
IMGUIFILEDIALOG_API void IGFD_SetFlashingAttenuationInSeconds(ImGuiFileDialog* vContext, float vAttenValue)
{
	if (vContext)
	{
		vContext->SetFlashingAttenuationInSeconds(vAttenValue);
	}
}
#endif

#ifdef USE_BOOKMARK
IMGUIFILEDIALOG_API char* IGFD_SerializeBookmarks(ImGuiFileDialog* vContext)
{
	char *res = 0;
	
	if (vContext)
	{
		auto s = vContext->SerializeBookmarks();
		if (!s.empty())
		{
			size_t siz = s.size() + 1U;
			res = new char[siz];
#ifndef MSVC
			strncpy(res, s.c_str(), siz);
#else
			strncpy_s(res, siz, s.c_str(), siz);
#endif
			res[siz - 1U] = '\0';
		}
	}

	return res;
}

IMGUIFILEDIALOG_API void IGFD_DeserializeBookmarks(ImGuiFileDialog* vContext, const char* vBookmarks)
{
	if (vContext)
	{
		vContext->DeserializeBookmarks(vBookmarks);
	}
}
#endif