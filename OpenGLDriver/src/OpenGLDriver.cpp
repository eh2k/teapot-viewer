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

using namespace eh;

#include <boost/static_assert.hpp>

#include <iostream>
#include <sstream>

#if defined(_MSC_VER)
#	include <windows.h>
#	define glGetProcAddress wglGetProcAddress
#else
#	include <GL/glx.h>
#	define glGetProcAddress(x) glXGetProcAddress((GLubyte*)x)
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#define ILUT_USE_OPENGL

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

BOOST_STATIC_ASSERT(sizeof(Matrix) == sizeof(GLfloat)*16);
BOOST_STATIC_ASSERT(sizeof(RGBA) == sizeof(GLfloat)*4);
BOOST_STATIC_ASSERT(sizeof(Rect) == sizeof(GLfloat)*4);

class OpenGLTexture: public IResource
{
public:
    typedef IResource::pointer<OpenGLTexture> ptr;

    static OpenGLTexture::ptr create( const std::wstring& file )
    {
        GLuint texId = 0;

        std::wcerr << L"loading: " << file.c_str() << std::endl;
#if defined(_MSC_VER)
        texId = ilutGLLoadImage( (wchar_t*)file.c_str() );
#else
        texId = ilutGLLoadImage( (char*) std::string(file.begin(), file.end()).c_str() );
#endif
        if ( texId == 0 )
        {
            std::wcerr << L"OpenGLTexture::create failed: " << file.c_str() << std::endl;
            return NULL;
        }
        else
            return new OpenGLTexture( texId );
    }

    virtual ~OpenGLTexture()
    {
        if (m_texId == 0)
            return;	//nichts zu tun hier...

        if ( glIsTexture(m_texId) )
        {
            glDeleteTextures(1, &m_texId);
        }
    }

    void bindTexture( int stage )
    {
//        static PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)glGetProcAddress("glClientActiveTexture");
        static PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)glGetProcAddress("glActiveTextureARB");

//        if (glClientActiveTexture)
//            glClientActiveTexture( GL_TEXTURE0_ARB + stage);

        if (glActiveTextureARB)
            glActiveTextureARB( GL_TEXTURE0_ARB + stage);

        if (m_texId)
            glEnable(GL_TEXTURE_2D);
        else
        {
            glDisable(GL_TEXTURE_2D);
            return;
        }

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

            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }

        glBindTexture(GL_TEXTURE_2D, m_texId);

    }
    static void unbindTexture(int stage)
    {
        OpenGLTexture(0).bindTexture(stage);
    }

protected:
    OpenGLTexture(Uint res):m_texId(res)
    {
    }
private:
    Uint m_texId;
};


class OpenGLDriver: public IDriver
{
    Matrix m_world;
    Matrix m_view;

public:

    typedef IDriver::pointer<OpenGLDriver> ptr;

    OpenGLDriver(int* hWnd)
    {
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

        glEnable(GL_TEXTURE_2D);

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

        ilInit();
        ilutInit();
        ilutRenderer(ILUT_OPENGL);
    }

    virtual ~OpenGLDriver()
    {
        ilShutDown();
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
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);
        }
    }

    virtual std::string getDriverInformation() const
    {
        std::stringstream str;

        str << "OpenGL Driver:" << std::endl;
        str << "-------------------------------------------" << std::endl;
        str << ""   << (const char*)glGetString(GL_VENDOR) << std::endl;
        str << "" << (const char*)glGetString(GL_RENDERER) << std::endl;
        str << ""  << (const char*)glGetString(GL_VERSION) << std::endl;

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

    }

    virtual void enableShadow(bool bEnable)
    {

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

            verifyNoErrors(__FUNCTION__);
        }
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
            OpenGLTexture::ptr t = dynamic_cast<OpenGLTexture*>(pTexture->m_resource.get());
            if (t == NULL)
            {
                t = OpenGLTexture::create( pTexture->getFile() );
                pTexture->m_resource = t;
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

#if 0
        glBegin(mode);

        for (Uint i = 0; i < node.getIndices().size(); i++ )
        {
            glNormal3fv( &node.getVertexBuffer()->getNormal( node.getIndices()[i] ).x );
            glVertex3fv( &node.getVertexBuffer()->getCoord( node.getIndices()[i] ).x );
            glTexCoord2fv( &node.getVertexBuffer()->getTexCoord( node.getIndices()[i] ).x );
        }

        glEnd();

        return true;
#endif

        void const* vertices = NULL;
        void const* normals = NULL;
        void const* texcoords = NULL;
        void const* colors = NULL;

        GLint stride = node.getVertexBuffer()->getStride();

        if (node.getVertexBuffer()->getStride() == sizeof(Vec3))
        {
            vertices = &node.getVertexBuffer()->getCoord(0);
        }
        else if (node.getVertexBuffer()->getStride() == sizeof(Vec3)*2)
        {
            vertices = &node.getVertexBuffer()->getCoord(0);
            normals = &node.getVertexBuffer()->getNormal(0);
        }
        else
        {
            vertices = &node.getVertexBuffer()->getCoord(0);
            normals = &node.getVertexBuffer()->getNormal(0);
            texcoords = &node.getVertexBuffer()->getTexCoord(0);
        }

        if (vertices)	glEnableClientState (GL_VERTEX_ARRAY);
        if (normals)	glEnableClientState (GL_NORMAL_ARRAY);
        if (texcoords)	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        if (colors)	glEnableClientState (GL_COLOR_ARRAY);

        if (vertices)	glVertexPointer(3, GL_FLOAT, stride, vertices);
        if (normals)	glNormalPointer(GL_FLOAT, stride, normals);
        if (texcoords)	glTexCoordPointer(2, GL_FLOAT, stride, texcoords);
        if (colors)	glColorPointer(4, GL_UNSIGNED_BYTE, stride, colors);

        if (node.getIndices().size()>0)
            glDrawElements(mode, (GLsizei)node.getIndices().size(), GL_UNSIGNED_INT, &node.getIndices()[0]);
        else
            glDrawArrays(mode, 0, node.getVertexBuffer()->getVertexCount() );

        if (vertices)	glDisableClientState (GL_VERTEX_ARRAY);
        if (normals)	glDisableClientState (GL_NORMAL_ARRAY);
        if (texcoords)	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
        if (colors)	glDisableClientState (GL_COLOR_ARRAY);

        return true;
    }

    static bool verifyNoErrors(const char* call_function = "")
    {
        GLenum errCode = glGetError();
        if (errCode != GL_NO_ERROR)
        {
            std::cerr << "\nglErorr: " << errCode;
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
    return new OpenGLDriver(pWindow);
}
