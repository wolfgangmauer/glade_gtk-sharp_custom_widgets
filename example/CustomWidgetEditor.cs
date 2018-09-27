using System;
using Gtk;

namespace design_namespace
{
	public class custom-widget-name + "Editor" : Box
	{
		// .ctor must look like this, parameter is the parent GtkBox
		public custom-widget-name + "Editor"(IntPtr rawParent)
			: base(Orientation.Vertical, 0)
		{
			try
			{
				// parent is a box !!!
				var parent = new Box(rawParent);
				parent.PackStart(this, false, false, 0);

				// add whatever your editor needed
				......
			}
			catch(Exception ex)
			{
				Console.WriteLine(ex.Message);
			}
		}
	}
}
