/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "katesessionapplet.h"
#include <QStyleOptionGraphicsItem>
#include <QTreeView>
#include <QVBoxLayout>
#include <QGraphicsGridLayout>
#include <KStandardDirs>
#include <KIconLoader>
#include <KInputDialog>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <QGraphicsProxyWidget>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <KIcon>
#include <KToolInvocation>
#include <KDirWatch>
#include <QGraphicsLinearLayout>
#include <KGlobalSettings>
#include <KUrl>
#include <KStringHandler>
#include <QFile>


bool katesessions_compare_sessions(const QString &s1, const QString &s2) {
    return KStringHandler::naturalCompare(s1,s2)==-1;
}


KateSessionApplet::KateSessionApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_listView( 0 )
{
    KDirWatch *dirwatch = new KDirWatch( this );
    QStringList lst = KGlobal::dirs()->findDirs( "data", "kate/sessions/" );
    for ( int i = 0; i < lst.count(); i++ )
    {
        dirwatch->addDir( lst[i] );
    }
    connect( dirwatch, SIGNAL(dirty (const QString &) ), this, SLOT( slotUpdateSessionMenu() ) );
    setPopupIcon( "kate" );
}

KateSessionApplet::~KateSessionApplet()
{
    delete m_listView;
}

QWidget *KateSessionApplet::widget()
{
    if ( !m_listView )
    {
        m_listView= new QTreeView();
        m_listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
        m_listView->setRootIsDecorated(false);
        m_listView->setHeaderHidden(true);
        m_listView->setMouseTracking(true);

        m_kateModel = new QStandardItemModel(this);
        m_listView->setModel(m_kateModel);
        m_listView->setMouseTracking(true);

        initSessionFiles();

        connect(m_listView, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(slotOnItemClicked(const QModelIndex &)));
    }
    return m_listView;
}


void KateSessionApplet::slotUpdateSessionMenu()
{
   m_kateModel->clear();
   m_sessions.clear(); 
   initSessionFiles();
}

void KateSessionApplet::initSessionFiles()
{
    int index=0;
    QStandardItem *item = new QStandardItem();
    item->setData(i18n("Start Kate (no arguments)"), Qt::DisplayRole);
    item->setData( KIcon( "kate" ), Qt::DecorationRole );
    item->setData( index++, Index );
    m_kateModel->appendRow(item);

    item = new QStandardItem();
    item->setData( i18n("New Kate Session"), Qt::DisplayRole);
    item->setData( KIcon( "document-new" ), Qt::DecorationRole );
    item->setData( index++, Index );
    m_kateModel->appendRow(item);

    item = new QStandardItem();
    item->setData( i18n("New Anonymous Session"), Qt::DisplayRole);
    item->setData( index++, Index );
    item->setData( KIcon( "document-new" ), Qt::DecorationRole );
    m_kateModel->appendRow(item);

    const QStringList list = KGlobal::dirs()->findAllResources( "data", "kate/sessions/*.katesession", KStandardDirs::NoDuplicates );
    KUrl url;
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        url.setPath(*it);
        QString name=url.fileName();
        name = QUrl::fromPercentEncoding(QFile::encodeName(url.fileName()));
        name.chop(12);///.katesession==12
/*        KConfig _config( *it, KConfig::SimpleConfig );
        KConfigGroup config(&_config, "General" );
        QString name =  config.readEntry( "Name" );*/
        m_sessions.append( name );
    }
    qSort(m_sessions.begin(),m_sessions.end(),katesessions_compare_sessions);
    for(QStringList::ConstIterator it=m_sessions.constBegin();it!=m_sessions.constEnd();++it)
    {
        item = new QStandardItem();
        item->setData(*it, Qt::DisplayRole);
        item->setData( index++, Index );
        m_kateModel->appendRow( item);
    }
}

void KateSessionApplet::slotOnItemClicked(const QModelIndex &index)
{
    hidePopup();
    int id = index.data(Index).toInt();
    QStringList args;
    if ( id > 0 )
        args << "--start";

    // If a new session is requested we try to ask for a name.
    if ( id == 1 )
    {
        bool ok = false;
        QString name = KInputDialog::getText( i18n("Session Name"),
                                              i18n("Please enter a name for the new session"),
                                              QString(),
                                              &ok );
        if ( ! ok )
            return;

        if ( name.isEmpty() && KMessageBox::questionYesNo( 0,
                                                           i18n("An unnamed session will not be saved automatically. "
                                                                "Do you want to create such a session?"),
                                                           i18n("Create anonymous session?"),
                                                           KStandardGuiItem::yes(), KStandardGuiItem::cancel(),
                                                           "kate_session_button_create_anonymous" ) == KMessageBox::No )
            return;

        if ( m_sessions.contains( name ) &&
             KMessageBox::warningYesNo( 0,
                                        i18n("You already have a session named %1. Do you want to open that session?", name ),
                                        i18n("Session exists") ) == KMessageBox::No )
            return;
        args << name;
    }

    else if ( id == 2 )
        args << "";

    else if ( id > 2 )
        args << m_sessions[ id-3 ];

    KToolInvocation::kdeinitExec("kate", args);
}



#include "katesessionapplet.moc"
