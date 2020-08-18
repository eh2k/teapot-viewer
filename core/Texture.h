// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "config.h"
#include "RefCounted.h"
#include "IDriver.h"
#include <string>

namespace eh
{
    class API_3D Texture: public RefCounted
    {
    private:
        Texture(const std::wstring& fileName);
        std::wstring m_file;
    public:
        virtual ~Texture();

        static Ptr<Texture> createFromFile(const std::wstring& sFileName);
        const std::wstring& getFile() const
        {
            return m_file;
        }
    public:
        Ptr<IResource> m_resource;
    };

}//end namespace
