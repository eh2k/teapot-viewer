<b>Teapot-Viewer 1.0b</b>

The Teapot-Viewer is a fast and extendable 3D-Model Viewer for Linux and Windows.

The GUI is written in GoLang and is based on [ImGui](https://github.com/ocornut/imgui). The core/scenegraph is written in C++ using common design pattens (graph,visitor). For rendering there s an graphics API independent renderer interface. 

<!--
Currently there is a basic Direct3D 9 and a OpenGL 1.4 renderer implementation.

* The Direct3D 9 renderer runs a simple shader, supporting bumpmapping, reflectiontexture and alphatextures.
* The OpenGL renderer uses the fixed pipeline, the textures are loaded with devIL (http://openil.sourceforge.net).
* -->

The SceneIO Module, provides an internal OBJ reader/writer. Additionally there is a simple plugin interface. It is possible to load Models from zip files.

Currently there are several importer plugins, all base on 3rd-party libraries:

* Assimp (.dae and more) (https://github.com/assimp/assimp/blob/master/Readme.md)
* lib3DS (.3ds) (http://code.google.com/p/lib3ds/)
* OpenCascade (.stp, .iges, .brep) (http://www.opencascade.org/)
* SketchUpSDK (.skp) (http://code.google.com/apis/sketchup/)
* Coin3d (.iv, .wrl) (http://www.coin3d.org)
* libG3D (.q3d and more) (http://automagically.de/g3dviewer/) '>
* <strike>FCollada (.dae) (http://www.feelingsoftware.com/)</strike>

Screenshots:

<img src='https://github.com/eh2k/teapot-viewer/raw/master/doc/screenshots/teapot.obj.png' width='320'> 
<img src='https://github.com/eh2k/teapot-viewer/raw/master/doc/screenshots/F40.dae.png' width='320'> 
<img src='https://github.com/eh2k/teapot-viewer/raw/master/doc/screenshots/tiefite.3ds.png' width='320'> 
<img src='https://github.com/eh2k/teapot-viewer/raw/master/doc/screenshots/CUI-DEFAULT.wrl.png' width='320'>
