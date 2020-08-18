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

#include "SceneIO.h"

#include "plugin.h"
eh::SceneIO::IPlugIn* XXX(IImportPlugIn* p);

#include <iostream>
#include <fstream>
#include <string.h>

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

size_t TryReadFromZip(const char* path, void** data);

namespace eh
{
    extern "C" SceneIO::IPlugIn *XcreateOBJPlugIn(); //OBJ

	using namespace std;

	static filesystem::path s_path;

	static void s_set_path(const filesystem::path& _path)
	{
		s_path = _path;
		s_path.remove_filename();
	}

	static std::wstring s_abs_path(const filesystem::path& file)
	{
		if (!file.is_absolute())
			return (s_path / file).wstring();
		else
			return file.wstring();
	}

	///////////////////////////////////////////////////////////////////////////

	SceneIO::File::File(const std::wstring& file) :
		m_path(file)
	{
		m_path = s_abs_path(m_path);
	}

	SceneIO::File::File(const std::string& file) :
		m_path(file.begin(), file.end())
	{
		m_path = s_abs_path(m_path);
	}

	SceneIO::File::~File()
	{
	}

	const std::wstring SceneIO::File::getExtension() const
	{
		return filesystem::path(m_path).extension().wstring();
	}

	const std::wstring SceneIO::File::getName() const
	{
		return filesystem::path(m_path).filename().wstring();
	}

	size_t SceneIO::File::getContent(std::unique_ptr<char>& data) const
	{
		if (filesystem::exists(m_path))
		{
			std::ifstream file(m_path.string().c_str(), std::ios_base::binary);

			if (!file.is_open())
				return 0;

			file.seekg(0, std::ios::end);
			size_t size = (size_t)file.tellg();

			data.reset(new char[size]);

			file.seekg(0, std::ios::beg);
			file.read(data.get(), size);
			file.close();

			return size;
		}
		else
		{
            void* pdata = nullptr;
            auto size = TryReadFromZip(m_path.string().c_str(), &pdata);
            data.reset((char*)pdata);
            return size;
		}

		return 0;
	}

	Ptr<Texture> SceneIO::createTexture(const std::string& text)
	{
		return createTexture(std::wstring(text.begin(), text.end()));
	}
	Ptr<Texture> SceneIO::createTexture(const std::wstring& text)
	{
		return Texture::createFromFile(s_abs_path(text));
	}

	struct SceneIO::Impl
	{
		std::vector< std::shared_ptr<IPlugIn> > m_plugins;
		std::map< std::wstring, IPlugIn* > m_ext_plugin_map;
	};

	static SceneIO::status_callback s_printStatus = NULL;

	void SceneIO::setSetStatusTextCallback(status_callback cb)
	{
		s_printStatus = cb;
	}

	void SceneIO::setStatusText(const std::wstring& text)
	{
		if (s_printStatus)
			s_printStatus(text);
	}

	SceneIO::~SceneIO()
	{
		delete m_pImpl;
	}
	SceneIO::SceneIO() :m_pImpl(new Impl())
	{
        std::shared_ptr<SceneIO::IPlugIn> pPlugIn(XcreateOBJPlugIn()); 
        RegisterPlugIn(pPlugIn);

		filesystem::path path = filesystem::current_path();

		for (filesystem::directory_iterator it(path), end; it != end; ++it)
		{
			if (filesystem::is_directory(it->status()))
				continue;

			filesystem::path file = *it;

			if (!boost::iequals(file.extension().wstring(), L".dll") && !boost::iequals(file.extension().wstring(), L".so"))
				continue;

			setStatusText(std::wstring(L"Loading PlugIn: ") + file.wstring() + L"...");

#if defined(_MSC_VER)

			std::wstring rpath = file.wstring();
			boost::algorithm::replace_all(rpath, L"/", L"\\");
			boost::algorithm::replace_all(rpath, L".dll", L"");
			if (filesystem::is_directory(rpath))
				SetDllDirectoryW(rpath.c_str());

			std::wstring dll = file.wstring();
			boost::algorithm::replace_all(dll, L"/", L"\\");

            std::wcout << dll << L" loading!!!" << std::endl;

			HMODULE hModule = LoadLibraryW(dll.c_str());
            if (hModule == NULL)
            {
                std::wcout << dll << L" loading failed!" << std::endl;
				continue;
            }
#else
			std::string fileA(file.string().begin(), file.string().end());
			void* hModule = dlopen(fileA.c_str(), RTLD_GLOBAL);
			if (!hModule)
            {
				std::cerr << fileA << dlerror() << std::endl;
                continue;
            }

#endif
			typedef IImportPlugIn* (*createPlugInFunc)();



#if defined(_MSC_VER)
			if (createPlugInFunc createPlugIn = (createPlugInFunc)GetProcAddress(hModule, "XcreatePlugIn"))
#else
			if (createPlugInFunc createPlugIn = (createPlugInFunc)dlsym(hModule, "XcreatePlugIn"))
#endif
			{
                auto pp = XXX(createPlugIn());

                std::wcout << file.string().c_str() << L" loaded:" <<  pp->about() << std::endl;

				RegisterPlugIn(std::shared_ptr<IPlugIn>(pp));
			}
			else
#if defined(_MSC_VER)
				;//std::cerr <<  "GetProcAdress failed" << std::endl;
#else
				;//std::cerr <<  dlerror() << std::endl;
#endif
			/*
			#if defined(_MSC_VER)
			FreeLibrary(hModule);
			#else
			dlclose(hModule);
			#endif
			*/
		}

		setStatusText(L"");
	}

    void SceneIO::RegisterPlugIn(IImportPlugIn* plugIn)
    {                
        auto pp = XXX(plugIn);
        RegisterPlugIn(std::shared_ptr<IPlugIn>(pp));
    }

	void SceneIO::RegisterPlugIn(std::shared_ptr<SceneIO::IPlugIn> pPlugIn)
	{
		m_pImpl->m_plugins.push_back(pPlugIn);

		for (Uint i = 0; i < pPlugIn->file_type_count(); i++)
		{
			std::wstring fileexts = pPlugIn->file_exts(i);
			std::vector< std::wstring > exts;
			boost::split(exts, fileexts, boost::is_any_of(L";"));

			if (exts.size() == 0)
				exts.push_back(fileexts);

			for (size_t j = 0; j < exts.size(); j++)
			{
				auto ext = exts[j];
				boost::erase_all(ext, L"*");
				boost::algorithm::to_lower(ext);

				if (m_pImpl->m_ext_plugin_map.find(ext) == m_pImpl->m_ext_plugin_map.end())
				{
					m_pImpl->m_ext_plugin_map[ext] = pPlugIn.get();
				}
				else
				{
					continue;
				}
			}
		}
	}

	std::wstring SceneIO::getAboutString() const
	{
		std::wstring about;
		about += L"-------------------------------------------\n";
		about += L"SceneIO PlugIns:\n";
		for (size_t i = 0; i < m_pImpl->m_plugins.size(); i++)
		{
			about += L"\t" + m_pImpl->m_plugins[i]->about() + L"\n";
		}
		return about;
	}

	std::wstring SceneIO::getFileWildcards(bool bLoading /*= true*/) const
	{
		std::wstring strFilter;
		if (bLoading)
		{
			strFilter += L"All known Formats:";

			for (size_t i = 0; i < m_pImpl->m_plugins.size(); i++)
			{
				for (size_t j = 0; j < m_pImpl->m_plugins[i]->file_type_count(); j++)
					if (m_pImpl->m_plugins[i]->canRead(j))
					{
						std::wstring exts = m_pImpl->m_plugins[i]->file_exts(j);
                        boost::algorithm::replace_all(exts, L";", L",");
                        boost::algorithm::replace_all(exts, L"*.", L"");
						boost::algorithm::to_upper(exts); strFilter += exts + L",";
						boost::algorithm::to_lower(exts); strFilter += exts + L",";
					}
			}
			strFilter += L"*.zip;";
		}

		for (size_t i = 0; i < m_pImpl->m_plugins.size(); i++)
		{
			for (size_t j = 0; j < m_pImpl->m_plugins[i]->file_type_count(); j++)
				if ((bLoading && m_pImpl->m_plugins[i]->canRead(j)) ||
					(!bLoading && m_pImpl->m_plugins[i]->canWrite(j)))
				{
					std::wstring exts = m_pImpl->m_plugins[i]->file_exts(j);
                    boost::algorithm::replace_all(exts, L";", L",");
					boost::algorithm::to_lower(exts);

					std::vector< std::wstring > vexts;
					boost::split(vexts, exts, boost::is_any_of(L";"));

					if (vexts.size() == 0)
						vexts.push_back(exts);

					bool bContinue = false;
					for (size_t k = 0; k < vexts.size(); k++)
					{
						boost::erase_all(vexts[k], L"*");

						if (m_pImpl->m_ext_plugin_map.find(vexts[k])->second != m_pImpl->m_plugins[i].get())
							bContinue = true;
					}

					if (bContinue)
						continue;

					strFilter += m_pImpl->m_plugins[i]->file_type(j) + L" (" + exts + L"):";

                    boost::algorithm::replace_all(exts, L"*.", L"");
					strFilter += exts + L",";
					boost::algorithm::to_lower(exts); strFilter += exts + L";";
				}
		}

		if (bLoading)
		{
			strFilter += L"Zip Files:*.zip";
			//strFilter += L"All Files:;";
		}
		else
			strFilter += L"Zip Files:*.zip";

		return strFilter;

	}

	bool SceneIO::execute(const std::wstring& file, Ptr<Scene> pScene, progress_callback progress, bool bLoading) const
	{
		bool ret = false;

		progress(0.f);

		try
		{
			filesystem::path sFile = filesystem::absolute(file);

			std::wstring ext = sFile.extension().wstring();
			boost::algorithm::to_lower(ext);

			if (m_pImpl->m_ext_plugin_map.find(ext) != m_pImpl->m_ext_plugin_map.end())
			{
				IPlugIn* plugin = m_pImpl->m_ext_plugin_map.find(ext)->second;

				if (bLoading)
				{
					s_set_path(sFile);

					if (plugin->read(sFile.wstring(), pScene, progress) == false)
						throw - 1;
					else
						ret = true;
				}
				else
				{
					if (plugin->write(sFile.wstring(), pScene, progress) == false)
						throw - 1;
					else
						ret = true;
				}
			}
		}
		catch (int e)
		{
			std::cerr << "error in " << __FUNCTION__ << " " << e << std::endl;
		}
		catch (...)
		{
			std::cerr << "Unknown Exception in " << __FUNCTION__ << std::endl;
		}

		progress(1.f);

		return ret;
	};

	static void dummy_callback(float f) {}

	bool SceneIO::read(const std::wstring& sFile, Ptr<Scene> pScene, progress_callback progress) const
	{
		if (progress == NULL)
			progress = dummy_callback;

		return execute(sFile, pScene, progress, true);
	}

	bool SceneIO::write(const std::wstring& sFile, Ptr<Scene> pScene, progress_callback progress) const
	{
		if (progress == NULL)
			progress = dummy_callback;

		return execute(sFile, pScene, progress, false);
	}

	SceneIO& SceneIO::getInstance()
	{
		static SceneIO io;
		return io;
	}

	//#include <boost/thread.hpp>
	//bool FormatManager::loadAsThread(const std::wstring& sFile, Ptr<Scene> pScene) const
	//{
	//	boost::thread( boost::bind(&FormatManager::load, this, sFile, pScene ) );
	//	return false;
	//}

} //end namespace
