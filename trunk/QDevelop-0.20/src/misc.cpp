#include "misc.h"

//
QVariant addressToVariant(void *it ) 
{
#if QT_POINTER_SIZE == 4
	return QVariant( reinterpret_cast<uint>(it)); 
#else
	return QVariant( reinterpret_cast<qulonglong>(it)); 
#endif
}
//
QTreeWidgetItem* variantToItem( QVariant variant )
{
#if QT_POINTER_SIZE == 4
		return (QTreeWidgetItem*)variant.toUInt();
#else
	return (QTreeWidgetItem*)variant.toULongLong();
#endif
}
//
QAction* variantToAction( QVariant variant )
{
#if QT_POINTER_SIZE == 4
	return (QAction*)variant.toUInt();
#else
	return (QAction*)variant.toULongLong();
#endif
}

