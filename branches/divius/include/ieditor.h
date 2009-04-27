#ifndef __IEDITOR_H__
#define __IEDITOR_H__

//! An abstract interface for accessing editors from plugins
class IEditor
{
	public:
		//! Close the tab with the editor
		virtual void close() = 0;
		//! Type of current file (reserved for future use)
		virtual QString fileType() = 0;
		virtual void copy() = 0;
		virtual void cut() = 0;
		virtual void paste() = 0;
		virtual void undo() = 0;
		virtual void redo() = 0;
		virtual void selectAll() = 0;
};

#endif // __IEDITOR_H__
