#include "Texture.h"
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iostream>

#undef max

namespace eh
{

    typedef std::unordered_map<std::wstring, Texture*> TEXTURE_MAP;
    static  TEXTURE_MAP file_map;

    Texture::Texture(const std::wstring& sFileName):
            m_file(sFileName),
            m_resource(nullptr)
    {
    }

    Ptr<Texture> Texture::createFromFile(const std::wstring& sFileName)
    {
		Texture*& tex = file_map[sFileName];

        if (tex)
            return tex;
        else
            return tex = new Texture(sFileName);
    }

    Texture::~Texture()
    {
        for (TEXTURE_MAP::iterator it = file_map.begin();
                it != file_map.end(); ++it)
        {
            if (it->second == this)
            {
                file_map.erase(it->first);
                return;
            }
        }
    }

} //end namespace
