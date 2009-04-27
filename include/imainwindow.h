#ifndef __IMAINWINDOW_H__
#define __IMAINWINDOW_H__

class IEditor;

//! An abstract interface for accessing main window from plugins
class IMainWindow
{
	public:
		//! Count of opened files
		virtual uint filesCount() = 0;
		//! Return an editor for tab
		virtual IEditor * editor(uint index) = 0;
};

#endif // __IMAINWINDOW_H__
