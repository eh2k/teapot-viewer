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
using System.IO;
using System.Linq;

namespace TeapotViewer
{
    using System.Runtime.InteropServices;
    using wx;

    public class MainFrame : Frame
    {
        enum Cmd
        {
            FileOpen,
            About,
            Exit
        }

        // for FileOpen2
        GLCanvas _canvas = null;

        public MainFrame(string title)
            : base(title)
        {
            this.Width = 640;
            this.Height = 480;

            // Set the window icon
            Icon = new wx.Icon(Path.Combine(Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location), "base.ico"));

            var menuBar = new MenuBar();

            {
                var fileMenu = new Menu();
                fileMenu.Append((int)Cmd.FileOpen, "&Open file\tCtrl-O");
                EVT_MENU((int)Cmd.FileOpen, new EventListener(OnFileOpen));
                fileMenu.AppendSeparator();

                EVT_MENU(fileMenu.Append((int)Cmd.Exit, "E&xit\tAlt-X").ID, new EventListener((s, a) =>
                {
                    Close(true);
                }));

                menuBar.Append(fileMenu, "&File");
            }
            {
                var fileMenu = new Menu();
                EVT_MENU(fileMenu.Append((int)Cmd.About, "&About").ID, new EventListener((s, a) =>
                {
                    MessageDialog.ShowModal(_canvas._viewPort.getDriverInfo(), "Info", 0);
                }));

                menuBar.Append(fileMenu, "&Help");
            }

            MenuBar = menuBar;

            // Set up a status bar
            var statusBar = CreateStatusBar();

            var sizer = new BoxSizer(Orientation.wxHORIZONTAL);
            var gauge = new Gauge(statusBar, -1, 100, new Point(-1, -1), new Size(-1, -1), Gauge.wxNO_BORDER|Gauge.wxGA_SMOOTH);
            sizer.Add(gauge, 1, Direction.wxALL | Stretch.wxEXPAND | Alignment.wxALIGN_RIGHT, 1);
            statusBar.SetSizer(sizer);
            statusBar.Layout();
            gauge.Hide();

            // Set up the event table

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

        public void OnFileOpen(object sender, Event e)
        {
            FileDialog fd = new FileDialog(this,
                "Open 3D model",
                "",
                "",
                "All files (*.*)|*.*");

            //fd.Directory = Utils.GetHomeDir();

            if (fd.ShowModal() == wxID_OK)
            {
                using (var callBack = new ProgressCallback(this.StatusBar))
                    _canvas._viewPort.loadScene(fd.Path, callBack);

                this.Title = fd.Path;
            }
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

            _viewPort.loadScene(@"D:\dev\teapot-viewer\bin\Debug\media\teapot.obj.zip");

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
            frame.Iconized = false;
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
