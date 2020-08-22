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

#include <IDriver.h>
#include <Geometry.h>
#include <Material.h>
#include <SceneIO.h>

using namespace eh;

#include <iostream>
#include <sstream>

#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#	include <windows.h>
#	define glGetProcAddress wglGetProcAddress
#else
#	include <GL/glx.h>
#	define glGetProcAddress(x) glXGetProcAddress((GLubyte*)x)
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#if defined(USE_DEVIL)

#define ILUT_USE_OPENGL
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#endif
static_assert(sizeof(Matrix) == sizeof(GLfloat)*16, "");
static_assert(sizeof(RGBA) == sizeof(GLfloat)*4, "");
static_assert(sizeof(Rect) == sizeof(GLfloat)*4, "");

static PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;
static PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;
static PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;
static PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;

static Uint s_vertices = 0;
static Uint s_textures = 0;
static Uint s_vbos = 0;

Uint NewOpenGLTexture(void* ptr, size_t size);

class OpenGLVBO: public IResource
{
public:
    static Ptr<OpenGLVBO> create( Ptr<IVertexBuffer> pBuff )
    {
        OpenGLVBO* ret = NULL;

        if (glGenBuffersARB && glBindBufferARB && glBufferDataARB)
        {
            GLuint id = 0;
            glGenBuffersARB(1, &id);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, pBuff->getBufferSize(), pBuff->getBuffer(), GL_STATIC_DRAW_ARB);

            ret = new OpenGLVBO(id, pBuff->getStride());
        }

        return ret;
    }

    void bind()
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vboid);

        if (m_stride >= sizeof(Vec3))
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, m_stride, 0);
        }

        if (m_stride >= sizeof(Vec3)*2)
        {
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, m_stride, (GLvoid*)sizeof(Vec3));
        }

        if (m_stride > sizeof(Vec3)*2)
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, m_stride, (GLvoid*)(sizeof(Vec3)*2));
        }
    }

    void unbind()
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

private:
    OpenGLVBO(GLuint vboid, GLuint stride):m_vboid(vboid),m_stride(stride)
    {
        s_vbos++;
    }
public:
    ~OpenGLVBO()
    {
        glDeleteBuffersARB(1, &m_vboid);
        s_vbos--;
    }
private:
    GLuint m_vboid;
    GLuint m_stride;
};

class OpenGLTexture: public IResource
{
public:
    static Ptr<OpenGLTexture> create( const std::wstring& sFile )
    {
        SceneIO::File aFile(sFile);
        SceneIO::setStatusText( std::wstring(L"Laoding ") + aFile.getName() + L"...");

        std::unique_ptr<char> data;
        size_t size = aFile.getContent(data);

        if ( size == 0 )
        {
            //std::wcerr << L"OpenGLTexture::creae(" << sFile.c_str() << ") aFile.getContent(data.get()) == 0" << std::endl;
            return nullptr;
        }

        std::wcout << L"creating Texture " << sFile.c_str() << L"...";

#if defined(USE_DEVIL)
        ILuint	Id;
        ilGenImages(1, &Id);
        ilBindImage(Id);

        if (!ilLoadL(IL_TYPE_UNKNOWN, data.get(), size))
        {
            std::wcerr << L"ilLoadL failed: " << sFile.c_str() << std::endl;
            return nullptr;
        }

        GLuint texId = ilutGLBindTexImage();	// ilutGLLoadImage( (wchar_t*)file.c_str() );
#else
        GLuint texId = NewOpenGLTexture(data.get(), size);
#endif
        std::wcout << L" -> TextureId: " << texId << std::endl;

        if ( texId == 0 )
        {
            std::wcerr << L"OpenGLTexture::create failed: " << sFile.c_str() << std::endl;
            return new OpenGLTexture( -1 );
        }
        else
            return new OpenGLTexture( texId );
    }

    void bindTexture( int stage )
    {
        if (stage != 0)
            return; // Todo: stage > 0 not renderen with ImGui ???

        if (m_texId > 0)
        {
            //glEnable(GL_TEXTURE_2D);
        }
        else
        {
            //glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }

        //        static PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)glGetProcAddress("glClientActiveTexture");
        static PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)glGetProcAddress("glActiveTextureARB");

//        if (glClientActiveTexture)
//            glClientActiveTexture( GL_TEXTURE0_ARB + stage);

        if (glActiveTextureARB)
            glActiveTextureARB( GL_TEXTURE0_ARB + stage);

        if (stage == 1)     //Reflection
        {
            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );

            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

            GLfloat factor=0.15f;
            GLfloat constColor[4] = {factor, factor, factor, factor};

            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
            glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
            //glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
            //glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
            //glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
            //glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
            //glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
            //glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_ALPHA);
        }
        else    //default
        {
            glDisable( GL_TEXTURE_GEN_S );
            glDisable( GL_TEXTURE_GEN_T );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }

        glBindTexture(GL_TEXTURE_2D, m_texId);

    }
    static void unbindTexture(int stage)
    {
        OpenGLTexture(0).bindTexture(stage);
    }

private:
    OpenGLTexture(Uint res):m_texId(res)
    {
        s_textures++;
    }
public:
    virtual ~OpenGLTexture()
    {
        s_textures--;

        if (m_texId <= 0)
            return;	//nichts zu tun hier...

        if ( glIsTexture(m_texId) )
        {
            glDeleteTextures(1, &m_texId);
        }
    }
private:
    Uint m_texId;
};


class OpenGLDriver: public IDriver
{
    Matrix m_world;
    Matrix m_view;
    Matrix m_shadow;
    bool m_bDrawShadow;
public:

    OpenGLDriver(int* hWnd)
    {
        verifyNoErrors();
        //////////////////////////////////////////////////////////////////////////
        // Set clear Z-Buffer value
        glClearDepth(1.f);
        glClearColor(1, 1, 1, 1);	//Hintergrundfarbe Weiß

        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glShadeModel(GL_SMOOTH);

        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
        glDisable(GL_COLOR_MATERIAL);

        RGBA ambient(0.0f,0.0f,0.0f,1.0f);
        RGBA diffuse(1.2f,1.2f,1.2f,1.0f);
        RGBA specular(1.0f,1.0f,1.0f,1.0f);

        setAmbientLight(RGBA(0,0,0,0));
        setDirectionalLight(0, Vec3(0,0,1000), Vec3(0,0,-1), Vec3(1,1,1), ambient, diffuse, specular);
        setDirectionalLight(1, Vec3(0,0,-1000), Vec3(0,0,1), Vec3(1,1,1), ambient, diffuse, specular);

#if defined(USE_DEVIL)
        ilInit();
        iluInit();
        ilutInit();
        ilutRenderer(ILUT_OPENGL);
#endif

        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) glGetProcAddress("glGenBuffersARB");
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC) glGetProcAddress("glBindBufferARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC) glGetProcAddress("glBufferDataARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) glGetProcAddress("glDeleteBuffersARB");

        glEnable(GL_TEXTURE_2D);
    }

    virtual ~OpenGLDriver()
    {
#if defined(USE_DEVIL)
        ilShutDown();
#endif
    }

    void setAmbientLight(const RGBA& color)
    {
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, reinterpret_cast<const GLfloat*>(&color));
    }

    void setDirectionalLight(int num,
                             const Vec3& pos,
                             const Vec3& dir,
                             const Vec3& autention,
                             const RGBA& ambient,
                             const RGBA& diffuse,
                             const RGBA& specular)
    {
//		const Float fMaxLightDistance =  sqrt(FLT_MAX);
//		const Float fFallOff = 1.f;
//		const Float fPhi = (Float)PI;

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        GLenum lidx = GL_LIGHT0 + num;
        GLfloat data[4];

        // set direction
        data[0] = -dir.x;
        data[1] = -dir.y;
        data[2] = -dir.z;
        data[3] = 0.0f; // 0.0f for directional light
        glLightfv(lidx, GL_POSITION, data);

        glLightf(lidx, GL_SPOT_EXPONENT, 0.0f);
        glLightf(lidx, GL_SPOT_CUTOFF, 180.0f);

        // set diffuse color
        data[0] = diffuse.r;
        data[1] = diffuse.g;
        data[2] = diffuse.b;
        data[3] = diffuse.a;
        glLightfv(lidx, GL_DIFFUSE, data);

        // set specular color
        data[0] = specular.r;
        data[1] = specular.g;
        data[2] = specular.b;
        data[3] = specular.a;
        glLightfv(lidx, GL_SPECULAR, data);

        // set ambient color
        data[0] = ambient.r;
        data[1] = ambient.g;
        data[2] = ambient.b;
        data[3] = ambient.a;
        glLightfv(lidx, GL_AMBIENT, data);

        // 1.0f / (constant + linear * d + quadratic*(d*d);

        // set attenuation
        glLightf(lidx, GL_CONSTANT_ATTENUATION, autention.x);
        glLightf(lidx, GL_LINEAR_ATTENUATION, autention.y);
        glLightf(lidx, GL_QUADRATIC_ATTENUATION, autention.z);

        glEnable(lidx);
    }

    virtual bool beginScene(bool bDrawBG )
    {
        s_vertices = 0;

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );

        this->setTexture(NULL, 0);
        this->setTexture(NULL, 1);
        this->setTexture(NULL, 2);
        this->setTexture(NULL, 3);

        if (bDrawBG)
        {
            this->enableWireframe(false);
            this->enableLighting(false);
            this->enableZWriting(false);
            this->setProjectionMatrix( Matrix::Ortho(0, 1.f, -1.f, 1.f, -1.f, +1.f) );
            this->setViewMatrix( Matrix::Identity() );
            this->setWorldMatrix( Matrix::Identity() );

            glNormal3f(0,0,1);

            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.9f,0.9f,0.9f);
            glVertex3f(0,0,0);
            glColor3f(0.9f,0.9f,0.9f);
            glVertex3f(1,0,0);
            glColor3f(0.9f,0.9f,1.f);
            glVertex3f(1,1,0);
            glColor3f(0.9f,0.9f,1.f);
            glVertex3f(0,1,0);
            glEnd();

            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.3f,0.3f,0.3f);
            glVertex3f(0,-1,0);
            glColor3f(0.3f,0.3f,0.3f);
            glVertex3f(1,-1,0);
            glColor3f(0.9f,0.9f,0.9f);
            glVertex3f(1,0,0);
            glColor3f(0.9f,0.9f,0.9f);
            glVertex3f(0,0,0);
            glEnd();
        }
        this->enableZWriting(true);

        return true;
    }
    virtual bool endScene( bool bShow )
    {
        glFlush();

        verifyNoErrors(__FUNCTION__);

        std::basic_ostringstream<wchar_t> str;

        str << L" Vertices: " << s_vertices;
        str << L" VBOs: " << s_vbos;
        str << L" Textures: " << s_textures;

        SceneIO::setStatusText(str.str().c_str());
        return true;
    }

    virtual void enableDepthTest(bool enable)
    {
        if (enable)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    virtual void draw2DText(const char* text, int x, int y)
    {
        glDisable(GL_DEPTH_TEST);

        setMaterial(NULL);

        glLoadIdentity();
        glRasterPos3i(x,y, 0);

        for (int i = 0; text[i] != 0; i++)
        {
//			Uint displist = this->createChar( "Arial", 0, text[i]);
//			glCallList(displist);
//			glDeleteLists(displist, 1);
        }

        glEnable(GL_DEPTH_TEST);
    }

    virtual void enableWireframe(bool bEnable)
    {
        if (bEnable)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    virtual std::string getDriverInformation() const
    {
        std::stringstream str;

        str << "-------------------------------------------" << std::endl;
        str << "OpenGL Driver:" << std::endl;
        str << "\t"   << (const char*)glGetString(GL_VENDOR) << std::endl;
        str << "\t" << (const char*)glGetString(GL_RENDERER) << std::endl;
        str << "\t"  << (const char*)glGetString(GL_VERSION) << std::endl;
        str << "-------------------------------------------" << std::endl;
        str << "TextureLoader:" << std::endl;
#if defined(USE_DEVIL)
        str << "\tDevIL " << IL_VERSION << std::endl;
#endif
        return str.str();
    }

    virtual void setViewport(int x, int y, int dx, int dy)
    {
        glViewport(x,y,dx,dy);
    }

    virtual const Rect& getViewport() const
    {
        static Rect rect;
        glGetFloatv(GL_VIEWPORT, reinterpret_cast<GLfloat*>(&rect));
        return rect;
    }

    virtual void setProjectionMatrix(const Matrix& mat)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&mat[0]);
        glMatrixMode(GL_MODELVIEW);

        verifyNoErrors(__FUNCTION__);
    }

    virtual void setWorldMatrix(const Matrix& mat)
    {
        m_world = mat;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&(m_world*m_view)[0]);
    }

    virtual void setViewMatrix(const Matrix& mat)
    {
        m_view = mat;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&(m_world*m_view)[0]);
    }

    virtual void setShadowMatrix(const Matrix& m)
    {
        m_shadow = m;
    }

    virtual void enableShadow(bool bEnable)
    {
        m_bDrawShadow = bEnable;
    }

    virtual void enableLighting(bool bEnable)
    {
        if ( bEnable)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);
    }

    virtual void enableBlending(bool bEnable)
    {
        if (bEnable)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
            glLineWidth(1);
        }
        else
        {
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);
        }

        verifyNoErrors(__FUNCTION__);
    }

    virtual void enableZWriting(bool bEnable)
    {
        glDepthMask(bEnable);
    }
    virtual void enableCulling(bool bEnable)
    {
        if (bEnable)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else
            glDisable(GL_CULL_FACE);
    }

    virtual void cullFace(bool bEnable)
    {
        if (bEnable)
            glCullFace(GL_FRONT);
        else
            glCullFace(GL_BACK);
    }


    virtual void setDepthOffset(Uint n, Float f)
    {
        glDepthRange(5*f-(f*n), 1 - (f*n));
    }

    virtual void setMaterial(const Material* pMaterial)
    {
        RGBA rgba(0,0,0,1);

        if (pMaterial)
            rgba = pMaterial->getDiffuse();

        GLboolean bLighting = true;
        glGetBooleanv( GL_LIGHTING, &bLighting );
        if ( bLighting )
            glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, reinterpret_cast<const GLfloat*>(&rgba) );
        else
            glColor4fv(reinterpret_cast<const GLfloat*>(&rgba));

        setTexture( pMaterial->getTexture().get(), 0);
        setTexture( pMaterial->getReflTexture().get(), 1);
        setTexture( pMaterial->getBumpTexture().get(), 2);
        setTexture( pMaterial->getOpacTexture().get(), 3);
    }

    virtual void setTexture(Texture* pTexture, int mode)
    {
        if (pTexture)
        {
            Ptr<OpenGLTexture> t = dynamic_cast<OpenGLTexture*>(pTexture->m_resource.get());
            if (t == nullptr)
            {
                pTexture->m_resource = t = OpenGLTexture::create( pTexture->getFile() );
            }

            if (t)
                t->bindTexture(mode);
        }
        else
            OpenGLTexture::unbindTexture(mode);
    }

    virtual bool drawPrimitive(Geometry& node)
    {
        GLenum mode = GL_TRIANGLES;
        switch (node.getType())
        {
        case Geometry::POINTS:
            mode = GL_POINTS;
            break;
        case Geometry::LINES:
            mode = GL_LINES;
            break;
        case Geometry::LINE_STRIP:
            mode = GL_LINE_STRIP;
            break;
        case Geometry::TRIANGLES:
            mode = GL_TRIANGLES;
            break;
        case Geometry::TRIANGLE_STRIP:
            mode = GL_TRIANGLE_STRIP;
            break;
        case Geometry::TRIANGLE_FAN:
            mode = GL_TRIANGLE_FAN;
            break;
        default:
            mode = GL_TRIANGLES;
        }

        Ptr<OpenGLVBO> pVB = node.getVertexBuffer()->m_resource;

        if (pVB == nullptr)
            pVB = node.getVertexBuffer()->m_resource = OpenGLVBO::create(node.getVertexBuffer());

        if (pVB != nullptr)
        {
            pVB->bind();

            if (node.getIndices().size()>0)
            {
                glDrawElements(mode, (GLsizei)node.getIndices().size(), GL_UNSIGNED_INT, &node.getIndices()[0]);
                s_vertices += node.getIndices().size();
            }
            else
            {
                glDrawArrays(mode, 0, node.getVertexBuffer()->getVertexCount() );
                s_vertices += node.getVertexBuffer()->getVertexCount();
            }


            if (m_bDrawShadow)
            {
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadMatrixf(&(m_world*m_shadow*m_view)[0]);

                glDisableClientState(GL_NORMAL_ARRAY);
                glNormal3f(0,0,0);  //black color

                if (node.getIndices().size()>0)
                {
                    glDrawElements(mode, (GLsizei)node.getIndices().size(), GL_UNSIGNED_INT, &node.getIndices()[0]);
                    s_vertices += node.getIndices().size();
                }
                else
                {
                    glDrawArrays(mode, 0, node.getVertexBuffer()->getVertexCount() );
                    s_vertices += node.getVertexBuffer()->getVertexCount();
                }


                glPopMatrix();
            }

            pVB->unbind();
        }
        else
        {
            glBegin(mode);

            for (Uint i = 0; i < node.getIndices().size(); i++ )
            {
                glNormal3fv( &node.getVertexBuffer()->getNormal( node.getIndices()[i] ).x );
                glTexCoord2fv( &node.getVertexBuffer()->getTexCoord( node.getIndices()[i] ).x );
                glVertex3fv( &node.getVertexBuffer()->getCoord( node.getIndices()[i] ).x );
            }

            glEnd();
        }
        return true;
    }

    bool verifyNoErrors(const char* call_function = "")
    {
        GLenum errCode = glGetError();
        if (errCode != GL_NO_ERROR)
        {
            std::cerr << "\n" << call_function  << ": glErorr: " << errCode << std::endl;
            return false;
        }

        return true;
    }
};

extern "C"
#if defined(_MSC_VER)
    __declspec(dllexport)
#endif
    IDriver* CreateOpenGL1Driver(int* pWindow)
{

    OpenGLDriver* pDriver = new OpenGLDriver(pWindow);
    if (pDriver->verifyNoErrors() == false)
    {
        delete pDriver;
        return NULL;
    }
    return pDriver;

}
