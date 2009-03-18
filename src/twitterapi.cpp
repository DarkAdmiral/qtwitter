/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "twitterapi.h"

TwitterAPI::TwitterAPI( QObject *parent ) :
    QObject( parent ),
    publicTimelineSync( false ),
    directMessagesSync( false ),
    switchUser( false ),
    authDialogOpen( false ),
    xmlGet( NULL ),
    xmlPost( NULL ),
    statusesDone( false ),
    messagesDone( false )
{
}

TwitterAPI::~TwitterAPI() {}

void TwitterAPI::createConnections( XmlDownload *xmlDownload )
{
  connect( xmlDownload, SIGNAL(finished(XmlDownload::ContentRequested)), this, SLOT(setFlag(XmlDownload::ContentRequested)) );
  connect( xmlDownload, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  if ( xmlDownload->getRole() == XmlDownload::Destroy ) {
    connect( xmlDownload, SIGNAL(deleteEntry(int)), this, SIGNAL(deleteEntry(int)) );
  } else {
    connect( xmlDownload, SIGNAL(newEntry(Entry*)), this, SLOT(newEntry(Entry*)) );
    connect( xmlDownload, SIGNAL(newEntry(Entry*)), this, SLOT(downloadImage(Entry*)) );
  }
}

bool TwitterAPI::isPublicTimelineSync()
{
  return publicTimelineSync;
}

bool TwitterAPI::isDirectMessagesSync()
{
  return directMessagesSync;
}

bool TwitterAPI::setAuthData( const QString &user, const QString &password )
{
  switchUser = false;
  authData.setUser( user );
  authData.setPassword( password );
  if ( currentUser.isNull() ) {
    currentUser = user;
  } else if ( currentUser.compare( authData.user() ) ) {
    switchUser = true;
  }
  emit requestListRefresh( publicTimelineSync, switchUser );
  return switchUser;
}

bool TwitterAPI::setPublicTimelineSync( bool b )
{
  if ( publicTimelineSync != b ) {
    publicTimelineSync = b;
    return true;
  }
  return false;
}

bool TwitterAPI::setDirectMessagesSync( bool b )
{
  if ( directMessagesSync != b ) {
    directMessagesSync = b;
    if ( directMessagesSync == false ) {
      emit noDirectMessages();
    }
    return true;
  }
  return false;
}

bool TwitterAPI::get()
{
  if ( publicTimelineSync ) {
    xmlGet = new XmlDownload ( XmlDownload::RefreshStatuses, authData.user(), authData.password(), this );
    createConnections( xmlGet );
    xmlGet->getContent( "http://twitter.com/statuses/public_timeline.xml", XmlDownload::Statuses );
  } else {
    if ( authData.user().isEmpty() || authData.password().isEmpty() )
      return false;

    xmlGet = new XmlDownload ( XmlDownload::RefreshStatuses, authData.user(), authData.password(), this );
    createConnections( xmlGet );
    xmlGet->getContent( "http://twitter.com/statuses/friends_timeline.xml", XmlDownload::Statuses );
    if ( directMessagesSync ) {
      xmlGet->getContent( "http://twitter.com/direct_messages.xml", XmlDownload::DirectMessages );
    }
  }
  emit requestListRefresh( publicTimelineSync, switchUser );
  switchUser = false;
  return true;
}

bool TwitterAPI::post( const QByteArray &status, int inReplyTo )
{
  if ( authData.user().isEmpty() || authData.password().isEmpty() )
    return false;

  QByteArray request( "status=" );
  request.append( status );
  if ( inReplyTo != -1 ) {
    request.append( "&in_reply_to_status_id=" + QByteArray::number( inReplyTo ) );
  }
  request.append( "&source=qtwitter" );
  qDebug() << request;
  xmlPost = new XmlDownload( XmlDownload::Submit, authData.user(), authData.password(), this );
  createConnections( xmlPost );
  xmlPost->postContent( "http://twitter.com/statuses/update.xml", request, XmlDownload::Statuses );
  emit requestListRefresh( publicTimelineSync, switchUser );
  switchUser = false;
  return true;
}

bool TwitterAPI::destroyTweet( int id )
{
  if ( authData.user().isEmpty() || authData.password().isEmpty() )
    return false;

  qDebug() << "Tweet No." << id << "will be destroyed";
  xmlPost = new XmlDownload( XmlDownload::Destroy, authData.user(), authData.password(), this );
  createConnections( xmlPost );
  xmlPost->postContent( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ), QByteArray(), XmlDownload::Statuses );
  emit requestListRefresh( publicTimelineSync, switchUser );
  switchUser = false;
  return true;
}

const QAuthenticator& TwitterAPI::getAuthData() const
{
  return authData;
}

void TwitterAPI::setFlag( XmlDownload::ContentRequested flag )
{
  switch ( flag ) {
    case XmlDownload::DirectMessages:
      messagesDone = true;
      break;
    case XmlDownload::Statuses:
    default:
      statusesDone = true;
  }
  emit done();
  if ( publicTimelineSync ) {
    emit switchToPublic();
  }
  if ( statusesDone && ( publicTimelineSync || (!directMessagesSync ? true : messagesDone) || (xmlPost && !publicTimelineSync)  ) ) {
    emit timelineUpdated();
    emit authDataSet( authData );
    destroyXmlConnection();
    currentUser = authData.user();
    statusesDone = false;
    messagesDone = false;
  }
}

void TwitterAPI::newEntry( Entry *entry )
{
  if ( entry->login() == authData.user() ) {
    entry->setOwn( true );
  }
  emit addEntry( entry );
}

void TwitterAPI::destroyXmlConnection()
{
  if ( xmlPost ) {
    qDebug() << "destroying xmlPost";
    xmlPost->deleteLater();
    xmlPost = NULL;
  }
  if ( xmlGet ) {
    qDebug() << "destroying xmlGet";
    xmlGet->deleteLater();
    xmlGet = NULL;
  }
}
