using System;
using Gtk;

namespace runtime_namespace
{
	public static class GTypeRegister // .config -> gtyperegister_static_class=
	{
        	internal static void Register() // .config -> gtyperegister_static_function=
        	{
			ExtTextBox.Register();
			ExtComboBox.Register();
			......
        	}
    	}

	public class ExtTextBox : ....... 
    	{
		public ExtTextBox(IntPtr raw) 
			: base(raw)
        	{	
		}

		public ExtTextBox() 
			: this(IntPtr.Zero)
        	{	
		}

		// remember, in gtk-sharp the gtypes are look like this 
		// __gtksharp_X_runtime_namespace_ExtTextBox
		public static void Register()
        	{
			Debug.WriteLine($"Registering GType {RegisterGType(typeof(ExtTextBox))}");
        	}
    	}

	public class ExtComboBox : ....... 
    	{
		public ExtComboBox(IntPtr raw) 
			: base(raw)
        	{	
		}

		public ExtComboBox() 
			: this(IntPtr.Zero)
        	{	
		}

		// remember, in gtk-sharp the gtypes are look like this 
		// __gtksharp_X_runtime_namespace_ExtTextBox
		public static void Register()
        	{
			Debug.WriteLine($"Registering GType {RegisterGType(typeof(ExtComboBox))}");
        	}
    	}
}
