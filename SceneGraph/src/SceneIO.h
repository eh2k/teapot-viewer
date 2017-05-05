/****************************************************************************
* Copyright (C) 2007-2010 by E.Heidt  http://teapot-viewer.sourceforge.net/ *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
****************************************************************************/


#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <memory>
#include "Scene.h"


namespace eh
{
	namespace boost
	{
		inline static bool iequals(const std::wstring str1, const std::wstring str2)
		{
			unsigned int sz = str1.size();
			if (str2.size() != sz)
				return false;

			for (unsigned int i = 0; i < sz; ++i)
				if (::tolower(str1[i]) != ::tolower(str2[i]))
					return false;
			return true;
		}

		inline const wchar_t* is_any_of(const wchar_t* str)
		{
			return str;
		}

		inline void split(std::vector<std::wstring>& tokens, const std::wstring &text, const wchar_t* sep)
		{
			std::size_t start = 0, end = 0;
			while ((end = text.find(sep, start)) != std::string::npos)
			{
				tokens.push_back(text.substr(start, end - start));
				start = end + 1;
			}

			tokens.push_back(text.substr(start));
		}

		namespace algorithm
		{
			inline void replace_all(std::wstring& str, const std::wstring& from, const std::wstring& to)
			{
				if (from.empty())
					return;

				size_t start_pos = 0;
				while ((start_pos = str.find(from, start_pos)) != std::string::npos)
				{
					str.replace(start_pos, from.length(), to);
					start_pos += to.length();
				}
			}

			inline void to_lower(std::wstring& str)
			{
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			}

			inline void to_upper(std::wstring& str)
			{
				std::transform(str.begin(), str.end(), str.begin(), ::toupper);
			}
		}

		inline void erase_all(std::wstring& str, const std::wstring& from)
		{
			if (from.empty())
				return;

			algorithm::replace_all(str, from, L"");
		}

		inline void erase_all(std::string& str, const std::string& from)
		{
			if (from.empty())
				return;

			std::wstring str2(str.begin(), str.end());
			algorithm::replace_all(str2, std::wstring(from.begin(), from.end()), L"");
			str = std::string(str2.begin(), str2.end());
		}

	}

	class API_3D SceneIO
	{
	public:
		typedef std::function<void(float)> progress_callback;
		typedef std::function<void(const std::wstring&)> status_callback;

		class API_3D File
		{
			File(const File&) = delete;
			std::wstring m_path;
		public:
			File(const std::wstring& file);
			File(const std::string& file);
			~File();

			const std::wstring  getExtension() const;
			const std::wstring  getName() const;
			const std::wstring& getPath() const
			{
				return m_path;
			}

			size_t getContent(std::unique_ptr<char>& data) const;
		};

		class IPlugIn
		{
			IPlugIn(const IPlugIn&) = delete;
		protected:
			IPlugIn()
			{}
		public:
			virtual ~IPlugIn() {};
			virtual std::wstring about() const = 0;
			virtual Uint file_type_count() const = 0;
			virtual std::wstring file_type(Uint i) const = 0;
			virtual std::wstring file_exts(Uint i) const = 0;
			virtual std::wstring rpath() const = 0;
			virtual bool canWrite(Uint i) const = 0;
			virtual bool canRead(Uint i) const = 0;

			virtual bool read(const std::wstring& aFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress) = 0;
			virtual bool write(const std::wstring& sFile, Ptr<Scene> pScene, SceneIO::progress_callback& progress) = 0;
		};

		SceneIO();
		~SceneIO();

		static void setSetStatusTextCallback(status_callback);
		static void setStatusText(const std::wstring& text);

		static Ptr<Texture> createTexture(const std::wstring& text);
		static Ptr<Texture> createTexture(const std::string& text);

		std::wstring getAboutString() const;
		std::wstring getFileWildcards(bool bLoading = true) const;

		bool read(const std::wstring& sFile, Ptr<Scene> pScene, progress_callback progress = NULL) const;
		bool write(const std::wstring& sFile, Ptr<Scene> pScene, progress_callback progress = NULL) const;
	private:
		bool execute(const std::wstring& file, Ptr<Scene> pScene, progress_callback progress, bool bLoading) const;

		struct Impl;
		Impl* m_pImpl;
	};

} //end namespace
