/****************************************************************************
* Copyright (C) 2007-2017 by E.Heidt  http://teapot-viewer.sourceforge.net/ *
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

using Assimp;
using System;
using System.Linq;

namespace TeapotViewer
{
    public class AssimpPlugIn : eh.IPlugIn
    {
        AssimpContext _context = new AssimpContext();
        eh.IGroupNode _root = null;

        public AssimpPlugIn()
        {
        }
        public override string about()
        {
            return _context.GetType().Assembly.GetName().Name + " " + _context.GetType().Assembly.GetName().Version + @" (https://github.com/assimp)";
        }
        public override bool canRead(int i)
        {
            return true;
        }
        public override bool canWrite(int i)
        {
            return false;
        }
        public override string file_exts(int i)
        {
            return "*" + _context.GetSupportedImportFormats()[i];
        }
        public override string file_type(int i)
        {
            return _context.GetSupportedImportFormats()[i].Replace(".", "").ToUpper() + " File";
        }
        public override int file_type_count()
        {
            return _context.GetSupportedImportFormats().Length;
        }
        public override bool readFile(string fileName, IntPtr handle, eh.Callback callback)
        {
            try
            {
                var importer = new AssimpContext();
                importer.SetConfig(new Assimp.Configs.NormalSmoothingAngleConfig(66.0f));
                importer.SetConfig(new Assimp.Configs.KeepSceneHierarchyConfig(true));

                var directory = System.IO.Path.GetDirectoryName(fileName);

                if (System.IO.File.Exists(fileName) == false && directory.EndsWith(".zip", StringComparison.OrdinalIgnoreCase))
                    fileName = directory;

                if (fileName.EndsWith(".zip", StringComparison.OrdinalIgnoreCase))
                {
                    using (var zip = System.IO.Compression.ZipFile.OpenRead(fileName))
                    {
                        var zipEntry = zip.Entries.First(e => importer.GetSupportedImportFormats().Any(ext => e.FullName.EndsWith(ext, StringComparison.OrdinalIgnoreCase)));
                        using (var stream = zipEntry.Open())
                        {
                            var model = importer.ImportFileFromStream(stream, System.IO.Path.GetExtension(zipEntry.Name));
                            _directory = fileName;
                            _root = eh.IGroupNode.FromHandle(handle);
                            var node = Translate(model, model.RootNode);
                        }
                    }
                }
                else
                {
                    var model = importer.ImportFile(fileName, PostProcessPreset.TargetRealTimeMaximumQuality);
                    _directory = System.IO.Path.GetDirectoryName(fileName);
                    _root = eh.IGroupNode.FromHandle(handle);
                    var node = Translate(model, model.RootNode);
                }
            }
            catch (Exception ex)
            {
                wx.MessageDialog.ShowModal(ex.Message, "Exception", 0);
                return false;
            }

            return true;
        }
        public override bool writeFile(string sFile, IntPtr scene, eh.Callback callback)
        {
            return false;
        }

        private string _directory;

        private eh.IGroupNode Translate(Scene scene, Node node)
        {
            var m = node.Transform;

            var t = new eh.Matrix(m.A1, m.B1, m.C1, m.D1,
                                   m.A2, m.B2, m.C2, m.D2,
                                   m.A3, m.B3, m.C3, m.D3,
                                   m.A4, m.B4, m.C4, m.D4);

            var group = eh.Scene.CreateGroupNode(t);

            if (scene.RootNode == node)
                group = _root;

            if (node.HasMeshes)
            {
                foreach (int index in node.MeshIndices)
                {
                    var mesh = scene.Meshes[index];

                    var material = scene.Materials[mesh.MaterialIndex];

                    bool hasColors = mesh.HasVertexColors(0);
                    bool hasTexCoords = mesh.HasTextureCoords(0);

                    var mat = eh.Scene.CreateMaterial();

                    if (material.HasColorDiffuse)
                        mat.SetDiffuseColor(material.ColorDiffuse.R, material.ColorDiffuse.G, material.ColorDiffuse.B, material.Opacity);

                    if (material.HasColorAmbient)
                        mat.SetAmbientColor(material.ColorAmbient.R, material.ColorAmbient.G, material.ColorAmbient.B, material.ColorAmbient.A);

                    if (material.HasColorSpecular)
                        mat.SetSpecularColor(material.ColorSpecular.R, material.ColorSpecular.G, material.ColorSpecular.B, material.ColorSpecular.A);

                    if (material.HasColorEmissive)
                        mat.SetEmissionColor(material.ColorEmissive.R, material.ColorEmissive.G, material.ColorEmissive.B, material.ColorEmissive.A);

                    if (material.HasTextureDiffuse)
                        mat.SetDiffuseTexture(System.IO.Path.Combine(_directory, material.TextureDiffuse.FilePath));

                    if (material.HasTextureReflection)
                        mat.SetReflectionTexture(System.IO.Path.Combine(_directory, material.TextureReflection.FilePath));

                    if (material.HasTextureNormal)
                        mat.SetBumpTexture(System.IO.Path.Combine(_directory, material.TextureNormal.FilePath));

                    var geo = eh.Scene.CreateGeometry();

                    foreach (Face face in mesh.Faces)
                    {
                        for (int i = 0; i < face.IndexCount; i++)
                        {
                            int indice = face.Indices[i];
                            var pos = mesh.Vertices[indice];
                            var normal = mesh.HasNormals ? mesh.Normals[indice] : new Vector3D();
                            normal.Normalize();
                            var texCoord = hasTexCoords ? mesh.TextureCoordinateChannels[0][indice] : new Vector3D();
                            var color = hasColors ? mesh.VertexColorChannels[0][indice] : new Color4D();

                            geo.AddVertex(
                                new eh.Vec3(pos.X, pos.Y, pos.Z),
                                new eh.Vec3(normal.X, normal.Y, normal.Z),
                                new eh.Vec3(texCoord.X, texCoord.Y, texCoord.Z));
                        }
                    }

                    var shape = eh.Scene.CreateShapeNode(mat, geo);
                    group.AddChildNode(shape);
                }
            }

            for (int i = 0; i < node.ChildCount; i++)
            {
                var child = Translate(scene, node.Children[i]);
                group.AddChildNode(child);
            }

            return group;
        }
    }
}
