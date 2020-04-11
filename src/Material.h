// Copyright (c) 2007,2010, Eduard Heidt

#pragma once

#include "RefCounted.h"
#include "Texture.h"

namespace eh
{

    class RGBA
    {
    public:
        Float r, g, b, a;

        RGBA(Float _r, Float _g, Float _b, Float _a = 1):
                r(_r), g(_g), b(_b), a(_a)
        {}

        RGBA(const RGBA& c):
                r(c.r), g(c.g), b(c.b), a(c.a)
        {}

        inline bool operator<(const RGBA& other) const
        {
            return (r + g + b + a) < (other.r + other.g + other.b + other.a);
        }
        inline bool operator>(const RGBA& other) const
        {
            return other < *this;
        }

        inline bool operator==(const RGBA &other) const
        {
            return  fequal(r,other.r) &&
                    fequal(g,other.g) &&
                    fequal(b,other.b) &&
                    fequal(a,other.a);
        }
        inline bool operator!=(const RGBA &other) const
        {
            return  ((*this) == other) == false;
        }

        inline RGBA operator*(Float f) const
        {
            return RGBA(r*f, g*f, b*f, a*f);
        }

        inline RGBA operator+(const RGBA& other) const
        {
            return RGBA((r+other.r)/2, (g+other.g)/2, (b+other.b)/2, (a+other.a)/2);
        }

        inline void setBlack()
        {
            r = g = b = a = 0;
        }
        inline bool isBlack() const
        {
            return  fequal(r,0) &&
                    fequal(g,0) &&
                    fequal(b,0);
        }

        operator const Float*() const
        {
            return &r;
        }
    };

    class API_3D Material: public RefCounted
    {
    private:
        RGBA m_diffuse;
        RGBA m_ambient;
        RGBA m_specular;
        RGBA m_emission;
        Float m_power;
        Ptr<Texture> m_texture;
        Ptr<Texture> m_texrefl;
        Ptr<Texture> m_texbump;
        Ptr<Texture> m_texopac;
    public:

        static Ptr<Material> create(const RGBA& diffuse, bool bReplacable = true)
        {
            return new Material(diffuse, RGBA(0,0,0), RGBA(0,0,0), RGBA(0,0,0), 0.f, bReplacable);
        }
        static Ptr<Material> create()
        {
            return new Material(RGBA(0,0,0), RGBA(0,0,0), RGBA(0,0,0), RGBA(0,0,0), 0, true);
        }

        static Ptr<Material> Red(bool bReplacable = true)
        {
            return Material::create(RGBA(1,0,0), bReplacable);
        }
        static Ptr<Material> Green(bool bReplacable = true)
        {
            return Material::create(RGBA(0,1,0), bReplacable);
        }
        static Ptr<Material> Blue(bool bReplacable = true)
        {
            return Material::create(RGBA(0,0,1), bReplacable);
        }
        static Ptr<Material> White(bool bReplacable = true)
        {
            return Material::create(RGBA(1,1,1), bReplacable);
        }
        static Ptr<Material> Black(bool bReplacable = true)
        {
            return Material::create(RGBA(0,0,0), bReplacable);
        }
        static Ptr<Material> Shadow(bool bReplacable = true)
        {
            return Material::create(RGBA(0.9f,0.9f,0.9f), bReplacable);
        }

        const RGBA& getDiffuse() const
        {
            return m_diffuse;
        }
        const RGBA& getAmbient() const
        {
            return m_ambient;
        }
        const RGBA& getSpecular() const
        {
            return m_specular;
        }
        const RGBA& getEmmision() const
        {
            return m_emission;
        }

        void setDiffuse(const RGBA& diffuse)
        {
            m_diffuse = diffuse;
        }
        void setAmbient(const RGBA& ambient)
        {
            m_ambient = ambient;
        }
        void setSpecular(const RGBA& specular)
        {
            m_specular = specular;
        }
        void setSpecularFactor(Float f)
        {
            m_power = f;
        }
        void setEmission(const RGBA& emission)
        {
            m_emission = emission;
        }

        Ptr<Texture> getTexture() const
        {
            return m_texture;
        }
        Ptr<Texture> getReflTexture() const
        {
            return m_texrefl;
        }
        Ptr<Texture> getBumpTexture() const
        {
            return m_texbump;
        }
        Ptr<Texture> getOpacTexture() const
        {
            return m_texopac;
        }

        void setTexture( Ptr<Texture> pTexture )
        {
            m_texture = pTexture;
        }
        void setReflTexture( Ptr<Texture> pTexture )
        {
            m_texrefl = pTexture;
        }
        void setBumpTexture( Ptr<Texture> pTexture )
        {
            m_texbump = pTexture;
        }
        void setOpacTexture( Ptr<Texture> pTexture )
        {
            m_texopac = pTexture;
        }

        bool	isReplacable() const
        {
            return m_bReplacable;
        }
        virtual ~Material(){}
    private:
        bool m_bReplacable;

        Material(const RGBA& diffuse,
                 const RGBA& ambient,
                 const RGBA& specular,
                 const RGBA& emission,
                 Float power,
                 bool bReplacable)
                :
                m_diffuse(diffuse),
                m_ambient(ambient),
                m_specular(specular),
                m_emission(emission),
                m_power(power),
                m_bReplacable(bReplacable)
        {}
    };

} //end namespace
