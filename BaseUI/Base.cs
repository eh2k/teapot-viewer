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
using System;
using System.Drawing;
using System.Linq;

namespace TeapotViewer
{
    using Assimp;
    using System.Runtime.InteropServices;
    using wx;

    public class MainFrame : Frame
    {
        const string CURRENT_VERSION = @"1.0a";
        const string PROJECT_URL = @"https://github.com/eh2k/teapot-viewer";

        GLCanvas _canvas = null;

        private class HtmlCtrl : HtmlWindow
        {
            public HtmlCtrl(Window parent) : base(parent, wxID_ANY, Point.Empty, new Size(-1, -1))
            { }
            public override void OnLinkClicked(HtmlLinkInfo link)
            {
                System.Diagnostics.Process.Start(link.Href);
            }
        }

        public MainFrame(string title)
            : base(title)
        {
            this.Width = 640;
            this.Height = 480;
            this.Icon = new Icon("TeapotViewer.Base.ico", BitmapType.wxBITMAP_TYPE_BMP_RESOURCE);

            eh.SceneIO.RegisterPlugIn(new AssimpPlugIn());

            var menuBar = new MenuBar();
            Action updateCameraMenuItems = () => { };

            {
                var fileMenu = new Menu();
                fileMenu.AddMenuItem("&Open file\tCtrl-O", (a) =>
                {
                    //var importer = new AssimpContext();
                    //var formats = "All known Formats|" + string.Join(";", importer.GetSupportedImportFormats().Select(f => "*" + f)) + ";*.zip|";

                    //formats += "Zip Files (*.zip)|*.zip|";
                    //formats += "All Files (*.*)|*.*||";

                    var fd = new FileDialog(this,
                        "Open..",
                        "",
                        "",
                        eh.SceneIO.getFileWildcards(true));

                    if (fd.ShowModal() == wxID_OK)
                    {
                        using (var callBack = new ProgressCallback(this.StatusBar))
                            eh.SceneIO.read(_canvas._viewPort, fd.Path, callBack);

                        updateCameraMenuItems();

                        this.Title = fd.Path;
                    }
                });

                fileMenu.AddMenuItem("&Export..\tCtrl-S", (a) =>
                {
                    var fd = new FileDialog(this,
                        "Export..",
                        "",
                        "",
                        eh.SceneIO.getFileWildcards(false), wx.FileDialogStyle.wxSAVE);

                    if (fd.ShowModal() == wxID_OK)
                    {
                        using (var callBack = new ProgressCallback(this.StatusBar))
                            eh.SceneIO.write(_canvas._viewPort, fd.Path, callBack);

                        this.Title = fd.Path;
                    }
                });

                fileMenu.AppendSeparator();

                fileMenu.AddMenuItem("E&xit\tAlt-F4", (a) =>
                {
                    Close(true);
                });

                menuBar.Append(fileMenu, "&File");
            }

            {
                var viewMenu = new Menu();

                viewMenu.AddMenuCheckItem("&Wireframe\tW", (a) =>
                {
                    _canvas._viewPort.setModeFlag(eh.Mode.MODE_WIREFRAME, a.Checked);
                    _canvas.Refresh();
                }, false);

                viewMenu.AddMenuCheckItem("&Lighting\tL", (a) =>
                {
                    _canvas._viewPort.setModeFlag(eh.Mode.MODE_LIGHTING, a.Checked);
                    _canvas.Refresh();
                }, true);


                viewMenu.AddMenuCheckItem("&Shadow\tS", (a) =>
                {
                    _canvas._viewPort.setModeFlag(eh.Mode.MODE_SHADOW, a.Checked);
                    _canvas.Refresh();
                }, true);

                viewMenu.AddMenuCheckItem("&Background\tG", (a) =>
                {
                    _canvas._viewPort.setModeFlag(eh.Mode.MODE_BACKGROUND, a.Checked);
                    _canvas.Refresh();
                }, true);

                viewMenu.AppendSeparator();

                viewMenu.AddMenuCheckItem("&BoundingBoxes\tB", (a) =>
                {
                    _canvas._viewPort.setModeFlag(eh.Mode.MODE_DRAWPRIMBOUNDS, a.Checked);
                    _canvas.Refresh();
                }, false);

                viewMenu.AddMenuCheckItem("Sce&ne-AABB-Tree\tN", (a) =>
                {
                    _canvas._viewPort.setModeFlag(eh.Mode.MODE_DRAWAABBTREE, a.Checked);
                    _canvas.Refresh();
                }, false);

                viewMenu.AppendSeparator();

                viewMenu.AddMenuCheckItem("Fullscreen\tF11", (a) =>
                {
                    this.ShowFullScreen(a.Checked);
                }, false);

                menuBar.Append(viewMenu, "&View");
            }

            {
                var cameraMenu = new Menu();

                const int ID_CAMERA1 = 0xF003;

                MenuItem persp = null;
                MenuItem ortho = null;

                persp = cameraMenu.AddMenuCheckItem("&Perspective Projection\tP", (a) =>
                {
                    if (_canvas._viewPort.getModeFlag(eh.Mode.MODE_ORTHO) == a.Checked)
                    {
                        _canvas._viewPort.setModeFlag(eh.Mode.MODE_ORTHO, a.Checked == false);
                        _canvas.Refresh();
                        ortho.Checked = !a.Checked;
                    }

                }, true);

                ortho = cameraMenu.AddMenuCheckItem("&Orthogonal Projection\tO", (a) =>
                {
                    if (_canvas._viewPort.getModeFlag(eh.Mode.MODE_ORTHO) != a.Checked)
                    {
                        _canvas._viewPort.setModeFlag(eh.Mode.MODE_ORTHO, a.Checked);
                        _canvas.Refresh();

                        persp.Checked = !a.Checked;
                    }
                }, false);

                cameraMenu.AppendSeparator();

                cameraMenu.AddMenuItem(ID_CAMERA1, "&Default\t1", (a) =>
                {
                    _canvas._viewPort.setCamera(0);
                    _canvas.Refresh();
                });

                updateCameraMenuItems = () =>
                {
                    int id = ID_CAMERA1 + 1;
                    while (cameraMenu.Remove(id++) != null) { };

                    for (int i = 1; i < _canvas._viewPort.getCameraCount(); i++)
                    {
                        var tmp = i;
                        cameraMenu.AddMenuItem(ID_CAMERA1 + i, _canvas._viewPort.getCameraName(i) + "\t" + (1 + i), (a) =>
                        {
                            _canvas._viewPort.setCamera(tmp);
                            _canvas.Refresh();
                        });
                    }
                };

                menuBar.Append(cameraMenu, "&Camera");
            }

            {
                var helpMenu = new Menu();
                helpMenu.AddMenuItem("&About", (a) =>
                {
                    var size = new wxSize(570, 400);
                    var pos = new Point(this.Rect.Location.X + this.Rect.Size.Width / 2 - size.Width / 2, this.Rect.Location.Y + this.Rect.Size.Height / 2 - size.Height / 2);
                    var dlg = new wx.Dialog(this, wxID_ANY, "About", pos, size);

                    var h = new wx.BoxSizer(wx.Orientation.wxHORIZONTAL);

                    var hyperlink = new HtmlCtrl(dlg) { Width = 450, Height = 320 };
                    hyperlink.SetPage(string.Format("<body bgcolor='{5}'><h5>Teapot-Viewer {2}</h5><p>Copyright (C) 2010-2017 by E.Heidt</p> <p> <a href='{0}'>{1}</a> </p><hr/><pre>{3}</pre><pre>{4}</pre></body>",
                        PROJECT_URL, PROJECT_URL, CURRENT_VERSION, eh.SceneIO.getAboutString(), _canvas._viewPort.getDriverInfo(),
                        System.Drawing.ColorTranslator.ToHtml(Color.FromArgb(dlg.BackgroundColour.Red, dlg.BackgroundColour.Green, dlg.BackgroundColour.Blue))));

                    h.Add(hyperlink, 0, wx.Direction.wxALL, 10);

                    var btnOK = new wx.Button(dlg, wxID_OK, "OK");
                    h.Add(btnOK, 1, wx.Direction.wxALL, 10);


                    dlg.SetSizer(h);

                    dlg.ShowModal();

                });

                menuBar.Append(helpMenu, "&Help");
            }

            MenuBar = menuBar;

            // Set up a status bar
            var statusBar = CreateStatusBar();

            var sizer = new BoxSizer(Orientation.wxHORIZONTAL);
            var gauge = new Gauge(statusBar, -1, 100, new Point(-1, -1), new Size(-1, -1), Gauge.wxNO_BORDER | Gauge.wxGA_SMOOTH);
            sizer.Add(gauge, 1, Direction.wxALL | Stretch.wxEXPAND | Alignment.wxALIGN_RIGHT, 1);
            statusBar.SetSizer(sizer);
            statusBar.Layout();
            gauge.Hide();

            _canvas = new GLCanvas(this);
        }

        class ProgressCallback : eh.Callback
        {
            private readonly Gauge _gauge;
            public ProgressCallback(StatusBar statusBar)
            {
                _gauge = statusBar.Children.OfType<Gauge>().First();
                _gauge.Show();
            }
            public override void Dispose()
            {
                _gauge.Hide();
                base.Dispose();
            }
            public override void call(float value)
            {
                _gauge.Value = (int)(value * 100);
            }
        }
    }

    public static class Extention
    {
        static int cmd = 1;
        public static MenuItem AddMenuItem(this Menu menu, string text, Action<MenuItem> action)
        {
            return menu.AddMenuItem(cmd++, text, action);
        }

        public static MenuItem AddMenuItem(this Menu menu, int id, string text, Action<MenuItem> action)
        {
            var menuItem = menu.Append(id, text);
            menu.AddEvent(menuItem.ID, new EventListener((s, a) => action(menuItem)), menuItem);
            return menuItem;
        }

        public static MenuItem AddMenuCheckItem(this Menu menu, string text, Action<MenuItem> action, bool @checked)
        {
            return menu.AddMenuCheckItem(cmd++, text, action, @checked);
        }

        public static MenuItem AddMenuCheckItem(this Menu menu, int id, string text, Action<MenuItem> action, bool @checked)
        {
            var menuItem = menu.AppendCheckItem(id, text);
            menuItem.Checked = @checked;
            menu.AddEvent(menuItem.ID, new EventListener((s, a) => action(menuItem)), menuItem);
            return menuItem;
        }
    }

    public class GLCanvas : Window
    {
        internal readonly eh.IViewport _viewPort;

        [DllImport("wx-c.dll", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?GetHWND@wxWindow@@QBEPAXXZ")]
        public static extern IntPtr GetHWND(IntPtr thisA);

        private static IntPtr SafePtr(wx.Object obj)
        {
            var SafePtr = typeof(wx.Object).GetMethod("SafePtr", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static);
            return (IntPtr)SafePtr.Invoke(null, new object[] { obj });
        }

        public GLCanvas(Window parent)
            : base(parent)
        {
            IntPtr wxObject = SafePtr(this);

            var hwnd = GetHWND(wxObject);

            _viewPort = eh.eh.CreateViewport(hwnd);

            eh.SceneIO.read(_viewPort, @"D:\dev\teapot-viewer\bin\Debug\media\teapot.obj.zip");

            EVT_PAINT(new EventListener((s, a) => { _viewPort.drawScene(); a.Skip(); }));
            EVT_ERASE_BACKGROUND(new EventListener((s, a) => { _viewPort.drawScene(); }));
            EVT_SIZE(new EventListener((s, a) => { _viewPort.setDisplayRect(0, 0, this.Width, this.Height); }));

            EVT_MOUSE_EVENTS(new EventListener((s, a) => { OnMouseEvent((MouseEvent)a); }));

            this.AddEventListener(Event.wxEVT_MOUSEWHEEL, new EventListener((s, a) =>
            {
                OnMouseEvent(new MouseEvent(SafePtr(a)));
            }));

            //Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Base3DWnd::OnKeyEvent));
        }

        void OnMouseEvent(MouseEvent ev)
        {
            if (ev.LeftDown || ev.RightDown)
            {
                _viewPort.control().OnMouseDown(eh.IController.LBUTTON, ev.Position.X, ev.Position.Y);
                this.CaptureMouse();
                Refresh();
            }
            else if (ev.LeftUp || ev.RightUp)
            {
                if (this.HasCapture())
                {
                    _viewPort.control().OnMouseUp(eh.IController.LBUTTON, ev.Position.X, ev.Position.Y);
                    this.ReleaseMouse();
                    Refresh();
                }
            }
            else if (ev.WheelRotation != 0)

            {
                _viewPort.control().OnMouseWheel(0, (short)(ev.WheelRotation * ev.WheelDelta), ev.Position.X, ev.Position.Y);
                Refresh();
            }
            else if (ev.Dragging)

            {
                if (this.HasCapture())
                {
                    var flags = 0;

                    if (ev.LeftIsDown)
                        flags |= eh.IController.LBUTTON;

                    if (ev.RightIsDown)
                        flags |= eh.IController.RBUTTON;

                    _viewPort.control().OnMouseMove(flags, ev.Position.X, ev.Position.Y);

                    Refresh();

                    //if(m_pViewport->getModeFlag(Camera::MODE_DRAW_NOT_SIMPLE) == false)
                    //	SetTimer(0, 300,0);
                }
                else
                    _viewPort.control().OnMouseMove(0, ev.Position.X, ev.Position.Y);

                if (!_viewPort.isValid())
                    Refresh();

                ev.Skip();
            }

        }

        public override void Dispose()
        {
            _viewPort.Dispose();
            base.Dispose();
        }
    }

    public class Application : App
    {
        public override bool OnInit()
        {
            var frame = new MainFrame("TeapotViewer");
            frame.Show(true);
            return true;
        }

        [STAThread]
        static void Main()
        {
            var app = new Application();
            app.Run();
        }
    }
}
