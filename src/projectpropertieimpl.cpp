/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jl.biord@free.fr>
* Program URL   : http://qdevelop.org
*
*/
#include "projectpropertieimpl.h"
#include "projectmanager.h"
#include "misc.h"
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDebug>
//
ProjectPropertieImpl::ProjectPropertieImpl(ProjectManager * parent, QTreeWidget *tree, QTreeWidgetItem *itProject)
        : QDialog(0), m_projectManager(parent), m_itProject(itProject), m_treeFiles(tree)
{
    setupUi(this);
    m_copyItProject = 0;
    m_copyTreeFiles = new QTreeWidget(this);
    m_copyTreeFiles->setColumnCount(1);
    m_copyTreeFiles->setHeaderLabels(QStringList(""));
    m_copyTreeFiles->setHidden( true );
    m_projectName = itProject->text(0);
    srcDirectory->setText( m_projectManager->srcDirectory( itProject ) );
    uiDirectory->setText( m_projectManager->uiDirectory( itProject ) );
    new QTreeWidgetItem(m_copyTreeFiles);
    copyTreeWidget(m_treeFiles->topLevelItem(0), m_copyTreeFiles->topLevelItem(0));
    while ( m_projectManager->toKey( m_copyItProject->data(0, Qt::UserRole) ) != "PROJECT" )
        m_copyItProject = m_copyItProject->parent();
    setWindowTitle( tr("Properties of %1").arg(m_projectName) );
    populateComboScope();
    resize(10,10);
}
//
void ProjectPropertieImpl::clearFields()
{
    QList<QObject*> enfants;
    enfants = groupeDebug->children();
    QListIterator<QObject*> iterator(enfants);
    while ( iterator.hasNext() )
    {
        QObject *objet = iterator.next();
        QString classe = objet->metaObject()->className();
        if ( classe == "QCheckBox" )
            ((QCheckBox *)objet)->setChecked(false);
        else if ( classe == "QRadioButton" )
            ((QRadioButton *)objet)->setChecked(false);
    }
    //
    enfants = groupeWarn->children();
    iterator = QListIterator<QObject*>(enfants);
    while ( iterator.hasNext() )
    {
        QObject *objet = iterator.next();
        QString classe = objet->metaObject()->className();
        if ( classe == "QCheckBox" )
            ((QCheckBox *)objet)->setChecked(false);
        else if ( classe == "QRadioButton" )
            ((QRadioButton *)objet)->setChecked(false);
    }
    //
    enfants = groupeBibliotheques->children();
    iterator = QListIterator<QObject*>(enfants);
    while ( iterator.hasNext() )
    {
        QObject *objet = iterator.next();
        QString classe = objet->metaObject()->className();
        if ( classe == "QCheckBox" )
            ((QCheckBox *)objet)->setChecked(false);
        else if ( classe == "QRadioButton" )
            ((QRadioButton *)objet)->setChecked(false);
    }
    //
    enfants = groupeAvancees->children();
    iterator = QListIterator<QObject*>(enfants);
    while ( iterator.hasNext() )
    {
        QObject *objet = iterator.next();
        QString classe = objet->metaObject()->className();
        if ( classe == "QCheckBox" )
            ((QCheckBox *)objet)->setChecked(false);
        else if ( classe == "QRadioButton" )
            ((QRadioButton *)objet)->setChecked(false);
    }
    supplement->clear();
}
//
void ProjectPropertieImpl::connections()
{
    connect(subdirs, SIGNAL(toggled(bool)), this, SLOT(slotSubdirs(bool)) );
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotAccept()) );
    connect(comboScope, SIGNAL(activated(int)), this, SLOT(slotComboScope(int)) );
    connect(variablesList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(slotCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)) );
    connect(valuesList, SIGNAL(doubleClicked ( const QModelIndex &)), this, SLOT(slotModifyValue()) );
    //
    connect(addVariable, SIGNAL(clicked()), this, SLOT(slotAddVariable()) );
    connect(deleteVariable, SIGNAL(clicked()), this, SLOT(slotDeleteVariable()) );
    connect(addValue, SIGNAL(clicked()), this, SLOT(slotAddValue()) );
    connect(deleteValue, SIGNAL(clicked()), this, SLOT(slotDeleteValue()) );
    connect(modifyValue, SIGNAL(clicked()), this, SLOT(slotModifyValue()) );
    //
    connect(debug, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(release, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(debug_and_release, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(build_all, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(warn_on, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(warn_off, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(app_bundle, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(assistant, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(console, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(designer, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(dll, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(exceptions, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(lib_bundle, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(no_lflags_merge, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(opengl, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(plugin, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(ppc, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(qaxcontainer, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(qaxserver, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(qt, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(qtestlib, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(resources, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(rtti, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(staticlib, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(stl, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(Thread, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(uic3, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(uitools, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(windows, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(x11, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(x86, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(core, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(network, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(sql, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(xml, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(gui, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(libopengl, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(svg, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(qt3support, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(app, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(lib, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(subdirs, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    connect(webkit, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    //
    connect(srcDirectoryButton, SIGNAL(clicked()), this, SLOT(slotSrcDirectory()) );
    connect(uiDirectoryButton, SIGNAL(clicked()), this, SLOT(slotUiDirectory()) );
}
//
void ProjectPropertieImpl::unconnections()
{
    disconnect(subdirs, SIGNAL(toggled(bool)), this, SLOT(slotSubdirs(bool)) );
    disconnect(okButton, SIGNAL(clicked()), this, SLOT(slotAccept()) );
    disconnect(comboScope, SIGNAL(activated(int)), this, SLOT(slotComboScope(int)) );
    disconnect(variablesList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(slotCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)) );
    //
    disconnect(addVariable, SIGNAL(clicked()), this, SLOT(slotAddVariable()) );
    disconnect(deleteVariable, SIGNAL(clicked()), this, SLOT(slotDeleteVariable()) );
    disconnect(addValue, SIGNAL(clicked()), this, SLOT(slotAddValue()) );
    disconnect(deleteValue, SIGNAL(clicked()), this, SLOT(slotDeleteValue()) );
    disconnect(modifyValue, SIGNAL(clicked()), this, SLOT(slotModifyValue()) );
    //
    disconnect(debug, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(release, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(debug_and_release, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(build_all, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(warn_on, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(warn_off, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(app_bundle, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(assistant, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(console, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(designer, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(dll, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(exceptions, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(lib_bundle, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(no_lflags_merge, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(opengl, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(plugin, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(ppc, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(qaxcontainer, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(qaxserver, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(qt, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(qtestlib, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(resources, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(rtti, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(staticlib, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(stl, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(Thread, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(uic3, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(uitools, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(windows, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(x11, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(x86, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(core, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(network, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(sql, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(xml, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(gui, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(libopengl, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(svg, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(qt3support, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(app, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(lib, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(subdirs, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
    disconnect(webkit, SIGNAL(toggled(bool)), this, SLOT(slotCheck(bool)) );
}
//
void ProjectPropertieImpl::slotCheck(bool activer)
{
    QVariant variant = comboScope->itemData( comboScope->currentIndex() );
    //QTreeWidgetItem *itCombo = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
    QTreeWidgetItem *itCombo = (QTreeWidgetItem*)variantToItem(variant);
    QObject *objet = sender();
    QString classe = objet->metaObject()->className();
    QString nomVariable;
    QTreeWidgetItem *it;
    nomVariable = objet->objectName().simplified();
    nomVariable = nomVariable.toLower(); // some (actually, one) of the checkboxes have upper characters to avoid name conflicts

    if ( QString(":app:lib:subdirs:").contains( ":"+nomVariable+":" ) )
    {
        QTreeWidgetItem *itTemplate = subItTemplate( itCombo );
        if ( !itTemplate )
        {
            itTemplate = new QTreeWidgetItem( itCombo );
            itTemplate->setText(0, "TEMPLATE");
            itTemplate->setData(0, Qt::UserRole, m_projectManager->toItem("TEMPLATE", "="));
            m_copyTreeFiles->setItemHidden(itTemplate, true);
        }
        it = itTemplate;
    }
    else if ( QString(":core:gui:network:libopengl:sql:svg:xml:qt3support:webkit:").contains( ":"+nomVariable+":" ) )
    {
        if (nomVariable == "libopengl") nomVariable = "opengl"; // There's both CONFIG += opengl and QT += opengl - the latter widget is called libopengl so the name must be manually replaced here
        QTreeWidgetItem *itQT = subItQT( itCombo );
        if ( !itQT )
        {
            itQT = new QTreeWidgetItem( itCombo );
            itQT->setText(0, "QT");
            itQT->setData(0, Qt::UserRole, m_projectManager->toItem("QT", "+="));
            m_copyTreeFiles->setItemHidden(itQT, true);
        }
        it = itQT;
    }
    else
    {
        QTreeWidgetItem *itConfig = subItConfig( itCombo );
        if ( !itConfig )
        {
            itConfig = new QTreeWidgetItem( itCombo );
            itConfig->setText(0, "CONFIG");
            itConfig->setData(0, Qt::UserRole, m_projectManager->toItem("CONFIG", "+="));
            m_copyTreeFiles->setItemHidden(itConfig, true);
        }
        it = itConfig;
    }
    if ( activer )
    {
        QTreeWidgetItem *nouvelItem = new QTreeWidgetItem( it );
        nouvelItem->setText(0, nomVariable);
        nouvelItem->setData(0, Qt::UserRole, m_projectManager->toItem("DATA"));
    }
    else
    {
        for (int i=0; i<it->childCount(); i++)
        {
            if ( it->child( i )->text( 0 ) == nomVariable )
            {
                QTreeWidgetItem *parent = it->child( i )->parent();
                delete it->child( i );
                while ( !parent->childCount()
                        && m_projectManager->toKey( parent->data(0, Qt::UserRole) ) != "PROJECT"
                        && m_projectManager->toKey( parent->data(0, Qt::UserRole) ) != "SCOPE"
                      )
                {
                    it = parent->parent();
                    delete parent;
                    parent = it;
                }
            }
        }
    }
}
//
QTreeWidgetItem *ProjectPropertieImpl::subItTemplate(QTreeWidgetItem *it)
{
    for (int i=0; i<it->childCount(); i++)
    {
        QTreeWidgetItem *item = it->child( i );
        QString cle = m_projectManager->toKey( item->data(0, Qt::UserRole) );
        if ( cle == "TEMPLATE" )
            return item;
    }
    return 0;
}
//
QTreeWidgetItem *ProjectPropertieImpl::subItQT(QTreeWidgetItem *it)
{
    for (int i=0; i<it->childCount(); i++)
    {
        QTreeWidgetItem *item = it->child( i );
        QString cle = m_projectManager->toKey( item->data(0, Qt::UserRole) );
        if ( cle == "QT" )
            return item;
    }
    return 0;
}
//
QTreeWidgetItem *ProjectPropertieImpl::subItConfig(QTreeWidgetItem *it)
{
    for (int i=0; i<it->childCount(); i++)
    {
        QTreeWidgetItem *item = it->child( i );
        QString cle = m_projectManager->toKey( item->data(0, Qt::UserRole) );
        if ( cle == "CONFIG" )
            return item;
    }
    return 0;
}
//
void ProjectPropertieImpl::copyTreeWidget(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    if ( source == m_itProject )
        m_copyItProject = dest;
    dest->setText(0, source->text(0));
    dest->setData(0, Qt::UserRole, source->data(0, Qt::UserRole ) );
    dest->setToolTip(0, source->toolTip(0) );
    dest->setIcon(0, source->icon(0));
    dest->treeWidget()->setItemHidden(dest, source->treeWidget()->isItemHidden(source));
    dest->treeWidget()->setItemExpanded(dest, source->treeWidget()->isItemExpanded(source));

    for (int i=0; i<source->childCount(); i++)
    {
        copyTreeWidget(source->child(i), new QTreeWidgetItem( dest ) );
    }
}
//
void ProjectPropertieImpl::slotAccept()
{
    QVariant variant = comboScope->itemData( comboScope->currentIndex() );
    //QTreeWidgetItem *itCombo = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
    QTreeWidgetItem *itCombo = (QTreeWidgetItem*)variantToItem(variant);
    QTreeWidgetItem *itConfig = subItConfig( itCombo );
    if ( !itConfig && supplement->text().simplified().split(" ", QString::SkipEmptyParts).count() )
    {
        itConfig = new QTreeWidgetItem( m_copyItProject );
        itConfig->setText(0, "CONFIG");
        itConfig->setData(0, Qt::UserRole, m_projectManager->toItem("CONFIG", "+=") );
        m_copyTreeFiles->setItemHidden(itConfig, true);
    }
    foreach(QString nomVariable, supplement->text().simplified().split(" ", QString::SkipEmptyParts))
    {
        QTreeWidgetItem *nouvelItem = new QTreeWidgetItem( itConfig );
        nouvelItem->setText(0, nomVariable);
        nouvelItem->setData(0, Qt::UserRole, m_projectManager->toItem("DATA"));
    }
    m_treeFiles->clear();
    new QTreeWidgetItem(m_treeFiles);
    copyTreeWidget(m_copyTreeFiles->topLevelItem(0), m_treeFiles->topLevelItem(0));
    m_treeFiles->setItemExpanded(m_treeFiles->topLevelItem(0), m_copyTreeFiles->isItemExpanded(m_copyTreeFiles->topLevelItem(0)));
    //m_projectManager->setSrcDirectory(m_projectManager->itemProject(m_projectName), srcDirectory->text() );
    //m_projectManager->setUiDirectory(m_projectManager->itemProject(m_projectName), uiDirectory->text() );
    accept();
}
//
void ProjectPropertieImpl::slotCurrentItemChanged ( QListWidgetItem * current, QListWidgetItem * /*previous*/ )
{
    valuesList->clear();
    if ( !current )
        return;
    QVariant variant = current->data( Qt::UserRole );
    QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
    for (int nb=0; nb < item->childCount(); nb++)
    {
        valuesList->addItem( item->child( nb )->text(0) );
        valuesList->item( valuesList->count()-1 )->setData(Qt::UserRole, addressToVariant(item->child( nb ) ) );
    }
}
//
void ProjectPropertieImpl::slotAddVariable()
{
    QString nouvelleVariable;
    //QString text = QInputDialog::getText(this, "QDevelop",
    //tr("New variable:"), QLineEdit::Normal,
    //"", &ok);
    QString text;

    QDialog *newVariable = new QDialog;
    Ui::NewVariable ui;
    ui.setupUi(newVariable);
    newVariable->setWindowTitle("QDevelop");
    QComboBox *c = ui.comboVariables;
    makeComboVariables( c );
    bool ok = newVariable->exec();
    if (ok)
    {
        if ( ui.groupUserVariable->isChecked() && !ui.userVariable->text().isEmpty() )
            nouvelleVariable = ui.userVariable->text();
        else if ( !ui.groupUserVariable->isChecked() )
            nouvelleVariable = ui.comboVariables->itemData(ui.comboVariables->currentIndex(), Qt::UserRole).toString();
        else
        {
            delete newVariable;
            return;
        }
    }
    else
    {
        delete newVariable;
        return;
    }
    QComboBox *comboOperator = ui.comboOperator;
    QVariant variant = comboScope->itemData( comboScope->currentIndex() );
    //QTreeWidgetItem *item = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
    QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
    QTreeWidgetItem *nouvelItem = new QTreeWidgetItem( item );
    m_copyTreeFiles->setItemHidden(nouvelItem, true);
    nouvelItem->setText(0, nouvelleVariable);
    nouvelItem->setData(0, Qt::UserRole, m_projectManager->toItem(nouvelleVariable, comboOperator->currentText()));
    slotComboScope( comboScope->currentIndex() );
}
//
void ProjectPropertieImpl::slotDeleteVariable()
{
    QListWidgetItem *itemCourant;
    if ( !(itemCourant = variablesList->currentItem()) )
        return;
    QVariant variant = itemCourant->data( Qt::UserRole );
    //QTreeWidgetItem *itemTree = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
    QTreeWidgetItem *itemTree = (QTreeWidgetItem*)variantToItem(variant);
    delete itemTree;
    slotComboScope( comboScope->currentIndex() );
}
//
void ProjectPropertieImpl::slotAddValue()
{
    QListWidgetItem *itemCourant;
    if ( !(itemCourant = variablesList->currentItem()) )
        return;
    QString nouvelleValeur;
    bool ok;
    int corriger = -1;
    QString text;
    do
    {
        text = QInputDialog::getText(this, "QDevelop",
                                     tr("New Value:"), QLineEdit::Normal,
                                     text, &ok);
        if ( text.contains(" ") && !text.contains("\""))
        {
            corriger = QMessageBox::warning(0, "QDevelop",
                                            tr("The spaces aren't correctly parsed by qmake")
                                            +",\n"
                                            +tr("delete spaces or put quotes."),
                                            tr("Modify"), tr("Cancel") );
            if ( corriger == 1 )
                ok = false;
        }
    }
    while ( ok && corriger==0 );
    if (ok && !text.isEmpty())
        nouvelleValeur = text;
    else
        return;
    QVariant variant = itemCourant->data( Qt::UserRole );
    QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
    QTreeWidgetItem *nouvelItem = new QTreeWidgetItem( item );
    m_copyTreeFiles->setItemHidden(nouvelItem, true);
    nouvelItem->setText(0, nouvelleValeur);
    nouvelItem->setData(0, Qt::UserRole, m_projectManager->toItem("DATA"));
    slotCurrentItemChanged(variablesList->currentItem(), 0);
}
//
void ProjectPropertieImpl::slotDeleteValue()
{
    QListWidgetItem *itemCourant;
    if ( !(itemCourant = valuesList->currentItem()) )
        return;
    QVariant variant = itemCourant->data( Qt::UserRole );
    //QTreeWidgetItem *itemTree = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
    QTreeWidgetItem *itemTree = (QTreeWidgetItem*)variantToItem(variant);
    delete itemTree;
    slotCurrentItemChanged(variablesList->currentItem(), 0);

}
//
void ProjectPropertieImpl::slotModifyValue()
{
    QListWidgetItem *itemCourant;
    if ( !(itemCourant = valuesList->currentItem()) )
        return;
    QVariant variant = itemCourant->data( Qt::UserRole );
    QString nouvelleValeur;
    //QTreeWidgetItem *itemTree = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
    QTreeWidgetItem *itemTree = (QTreeWidgetItem*)variantToItem(variant);
    bool ok;
    int corriger = -1;
    QString text = itemCourant->text();
    do
    {
        text = QInputDialog::getText(this, "QDevelop",
                                     tr("New Value:"), QLineEdit::Normal,
                                     text, &ok);
        if ( text.contains(" ") && !text.contains("\""))
        {
            corriger = QMessageBox::warning(0, "QDevelop",
                                            tr("The spaces aren't correctly parsed by qmake")
                                            +",\n"
                                            +tr("delete spaces or put quotes."),
                                            tr("Modify"), tr("Cancel") );
            if ( corriger == 1 )
                ok = false;
        }
    }
    while ( ok && corriger==0 );
    if (ok && !text.isEmpty())
        nouvelleValeur = text;
    else
        return;
    itemTree->setText(0, nouvelleValeur);
    slotCurrentItemChanged(variablesList->currentItem(), 0);
}
//

void ProjectPropertieImpl::slotComboScope(int index)
{
    unconnections();
    variablesList->clear();
    valuesList->clear();
    clearFields();
    QVariant variant = comboScope->itemData( index );
    //QTreeWidgetItem *item = reinterpret_cast<QTreeWidgetItem*>(variant.toUInt());
    QTreeWidgetItem *item = (QTreeWidgetItem*)variantToItem(variant);
    if ( m_projectManager->toKey( item->data(0, Qt::UserRole) ) != "PROJECT" )
        projectTemplate->setDisabled( true );
    else
        projectTemplate->setDisabled( false );
    parse( item );
    QList<QTreeWidgetItem *> listeScope;
    for (int nbScope=0; nbScope < item->childCount(); nbScope++)
    {

        QTreeWidgetItem *it = item->child( nbScope );
        QString cle = m_projectManager->toKey( it->data(0, Qt::UserRole) );
        QString donnee = it->text( 0 );
        if ( !QString("CONFIG|FORMS|HEADERS|QT|RESOURCES|SOURCES|TRANSLATIONS|TEMPLATE|SCOPE|SUBDIRS|absoluteNameProjectFile|projectDirectory|subProjectName|qmake|srcDirectory|uiDirectory").contains( cle ) )
        {
            variablesList->addItem( cle );
            //variablesList->item( variablesList->count()-1 )->setData(Qt::UserRole, QVariant(reinterpret_cast<uint>(it) ) );
            variablesList->item( variablesList->count()-1 )->setData(Qt::UserRole, addressToVariant(it) );
        }
    }
    if ( variablesList->count() )
        slotCurrentItemChanged( variablesList->item( 0 ), 0);
    connections();
    if ( m_projectManager->toKey( item->data(0, Qt::UserRole) ) != "PROJECT" )
        projectTemplate->setDisabled( true );
    else
        projectTemplate->setDisabled( false );
}
//
void ProjectPropertieImpl::parse(QTreeWidgetItem *it)
{
    for (int i=0; i<it->childCount(); i++)
    {
        QTreeWidgetItem *item = it->child( i );
        QString cle = m_projectManager->toKey( item->data(0, Qt::UserRole) );
        if ( cle == "TEMPLATE" )
            parseTemplate(item);
        else if ( cle == "QT" )
            parseQT(item);
        else if ( cle == "CONFIG" )
            parseConfig(item);
        //else if( cle == "SCOPE" )
        //analyseScope(it);
    }
}
//
void ProjectPropertieImpl::parseConfig(QTreeWidgetItem *it)
{
    for (int i=0; i<it->childCount(); i++)
    {
        QTreeWidgetItem *itemConfig = it->child( i );
        QString donnee = itemConfig->text(0);
        if ( donnee == "debug" )	debug->setChecked(true);
        else if ( donnee == "release" )	release->setChecked(true);
        else if ( donnee == "debug_and_release" )	debug_and_release->setChecked(true);
        else if ( donnee == "build_all" )	build_all->setChecked(true);
        else if ( donnee == "warn_on" )	warn_on->setChecked(true);
        else if ( donnee == "warn_off" )	warn_off->setChecked(true);
        else if ( donnee == "app_bundle" ) app_bundle->setChecked(true);
        else if ( donnee == "assistant" )	assistant->setChecked(true);
        else if ( donnee == "console" )	console->setChecked(true);
        else if ( donnee == "designer" )	designer->setChecked(true);
        else if ( donnee == "dll" )	dll->setChecked(true);
        else if ( donnee == "exceptions" )	exceptions->setChecked(true);
        else if ( donnee == "lib_bundle" )	lib_bundle->setChecked(true);
        else if ( donnee == "no_lflags_merge" )	no_lflags_merge->setChecked(true);
        else if ( donnee == "opengl" )	opengl->setChecked(true);
        else if ( donnee == "plugin" ) plugin->setChecked(true);
        else if ( donnee == "ppc" )	ppc->setChecked(true);
        else if ( donnee == "qaxcontainer" )	qaxcontainer->setChecked(true);
        else if ( donnee == "qaxserver" )qaxserver->setChecked(true);
        else if ( donnee == "qt" )	qt->setChecked(true);
        else if ( donnee == "qtestlib" )	qtestlib->setChecked(true);
        else if ( donnee == "resources" )resources->setChecked(true);
        else if ( donnee == "rtti" )	rtti->setChecked(true);
        else if ( donnee == "staticlib" )	staticlib->setChecked(true);
        else if ( donnee == "stl" )	stl->setChecked(true);
        else if ( donnee == "thread" )	Thread->setChecked(true);
        else if ( donnee == "uic3" )	uic3->setChecked(true);
        else if ( donnee == "uitools" )	uitools->setChecked(true);
        else if ( donnee == "windows" )	windows->setChecked(true);
        else if ( donnee == "x11" )	x11->setChecked(true);
        else if ( donnee == "x86" )	x86->setChecked(true);
        else
        {
            supplement->setText( supplement->text() + " "+ donnee );
            delete itemConfig;
        }
    }
    supplement->setText( supplement->text().simplified() );
}
//
void ProjectPropertieImpl::parseQT(QTreeWidgetItem *it)
{
    for (int i=0; i<it->childCount(); i++)
    {
        QTreeWidgetItem *itemConfig = it->child( i );
        QString donnee = itemConfig->text(0);
        if ( donnee == "core" )	core->setChecked(true);
        if ( donnee == "network" )	network->setChecked(true);
        if ( donnee == "sql" )	sql->setChecked(true);
        if ( donnee == "xml" )	xml->setChecked(true);
        if ( donnee == "gui" )	gui->setChecked(true);
        if ( donnee == "opengl" )	libopengl->setChecked(true);
        if ( donnee == "svg" )	svg->setChecked(true);
        if ( donnee == "qt3support" )	qt3support->setChecked(true);
        if ( donnee == "webkit" )	webkit->setChecked(true);
    }
}
//
void ProjectPropertieImpl::parseTemplate(QTreeWidgetItem *it)
{
    for (int i=0; i<it->childCount(); i++)
    {
        QTreeWidgetItem *itemTemplate = it->child( i );
        QString donnee = itemTemplate->text(0);
        if ( donnee == "app" )
        {
            app->setChecked( true );
            subdirs->setDisabled( true );
        }
        else if ( donnee == "lib" )
        {
            lib->setChecked( true );
            subdirs->setDisabled( true );
        }
        else if ( donnee == "subdirs" )
        {
            subdirs->setChecked( true );
            lib->setDisabled (true );
            app->setDisabled (true );
            slotSubdirs( true );
        }
    }
}
//
void ProjectPropertieImpl::populateComboScope()
{
    //comboScope->addItem(m_copyItProject->text(0) , QVariant(reinterpret_cast<uint>(m_copyItProject)));
    comboScope->addItem(m_copyItProject->text(0) , addressToVariant(m_copyItProject));
    QString projectName = m_projectManager->projectFilename( m_copyItProject );
    QList<QTreeWidgetItem *> listeScope;
    m_projectManager->childsList(m_copyItProject, "SCOPE", listeScope);
    for (int nbScope=0; nbScope < listeScope.count(); nbScope++)
    {
        QString scopeName;
        QTreeWidgetItem *tmp = listeScope.at(nbScope);
        int nbEspace = 0;
        while ( tmp )
        {
            QString cleTmp = m_projectManager->toKey( tmp->data(0,Qt::UserRole) );
            QString indent;
            for (int i=0; i<nbEspace; i++)
                indent += "  ";
            if ( cleTmp == "SCOPE" || cleTmp == "PROJECT" )
                scopeName = indent + tmp->text(0) + ":" + scopeName.simplified();
            if ( cleTmp == "PROJECT" )
                break;
            tmp = tmp->parent();
            nbEspace++;
        }
        if ( m_projectManager->projectFilename( listeScope.at(nbScope) ) == projectName )
            comboScope->addItem( scopeName, addressToVariant(listeScope.at(nbScope)));
        //comboScope->addItem( scopeName, QVariant(reinterpret_cast<uint>(listeScope.at(nbScope))));
    }
    if ( comboScope->count() == 1 )
        comboScope->setDisabled( true );
    if ( comboScope->count() )
    {
        comboScope->setCurrentIndex( 0 );
        slotComboScope( 0 );
    }
}
//
void ProjectPropertieImpl::slotSubdirs(bool coche)
{
    tabConfig->setEnabled(!coche);
}
//
//
void ProjectPropertieImpl::makeComboVariables( QComboBox *combo )
{
    combo->addItem("DEFINES: qmake adds the values of this variable as compiler C preprocessor macros (-D option).", QVariant( QString("DEFINES") ) );
    combo->addItem("DEF_FILE: This is only used on Windows when using the app template.", QVariant( QString("DEF_FILE") ) );
    combo->addItem("DEPENDPATH: This variable contains the list of all directories to look in to resolve dependencies. This will be used when crawling through included files. ", QVariant( QString("DEPENDPATH") ) );
    combo->addItem("DESTDIR: Specifies where to put the target file.", QVariant( QString("DESTDIR") ) );
    combo->addItem("DLLDESTDIR: Specifies where to copy the target dll. ", QVariant( QString("DLLDESTDIR") ) );
    combo->addItem("DISTFILES: This variable contains a list of files to be included in the dist target.", QVariant( QString("DISTFILES") ) );
    combo->addItem("INCLUDEPATH: This variable specifies the #include directories which should be searched when compiling the project.", QVariant( QString("INCLUDEPATH") ) );
    combo->addItem("INSTALLS: This variable contains a list of resources that will be installed when make install is executed.", QVariant( QString("INSTALLS") ) );
    combo->addItem("LEXSOURCES: This variable contains a list of lex source files.", QVariant( QString("LEXSOURCES") ) );
    combo->addItem("LIBS: This variable contains a list of libraries to be linked into the project.", QVariant( QString("LIBS") ) );
    combo->addItem("MAKEFILE: This variable specifies the name of the Makefile which qmake should use when outputting the dependency information for building a project.", QVariant( QString("MAKEFILE") ) );
    combo->addItem("MOC_DIR: This variable specifies the directory where all intermediate moc files should be placed.", QVariant( QString("MOC_DIR") ) );
    combo->addItem("OBJECTS_DIR: This variable specifies the directory where all intermediate objects should be placed.", QVariant( QString("OBJECTS_DIR") ) );
    combo->addItem("QTPLUGIN: This variable contains a list of names of static plugins that are to be compiled with an application so that they are available as built-in resources. ", QVariant( QString("QTPLUGIN") ) );
    combo->addItem("RCC_DIR: This variable specifies the directory where all intermediate resource files should be placed.", QVariant( QString("RCC_DIR") ) );
    combo->addItem("TARGET: This specifies the name of the target file.", QVariant( QString("TARGET") ) );
    combo->addItem("UI_DIR: This variable specifies the directory where all intermediate files from uic should be placed.", QVariant( QString("UI_DIR") ) );
    combo->addItem("UI_HEADERS_DIR: This variable specifies the directory where all declaration files (as generated by uic) should be placed.", QVariant( QString("UI_HEADERS_DIR") ) );
    combo->addItem("UI_SOURCES_DIR: This variable specifies the directory where all implementation files (as generated by uic) should be placed.", QVariant( QString("UI_SOURCES_DIR") ) );
    combo->addItem("VERSION: This variable contains the version number of the library if the lib TEMPLATE is specified.", QVariant( QString("VERSION") ) );
    combo->addItem("VER_MAJ: This variable contains the major version number of the library, if the lib template is specified. ", QVariant( QString("VER_MAJ") ) );
    combo->addItem("VER_MIN: This variable contains the minor version number of the library, if the lib template is specified. ", QVariant( QString("VER_MIN") ) );
    combo->addItem("VER_PAT: This variable tells qmake where to search for files it cannot open.", QVariant( QString("VER_PAT") ) );
}
//
void ProjectPropertieImpl::slotSrcDirectory()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the sources location"),
                    srcDirectory->text(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        return;
    }
    srcDirectory->setText( s );
}
//
void ProjectPropertieImpl::slotUiDirectory()
{
    QString s = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose the dialogs location"),
                    uiDirectory->text(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    if ( s.isEmpty() )
    {
        return;
    }
    uiDirectory->setText( s );
}
//
