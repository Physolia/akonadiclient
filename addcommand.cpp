/*
    Copyright (C) 2012  Kevin Krammer <krammer@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "addcommand.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>

#include <akonadi/private/collectionpathresolver_p.h>

#include <KCmdLineOptions>
#include <KMimeType>
#include <KUrl>

#include <QFile>

using namespace Akonadi;

AddCommand::AddCommand( QObject *parent ): AbstractCommand( parent )
{
  mShortHelp = ki18nc( "@info:shell", "Add items to a specified collection" ).toString();
}

AddCommand::~AddCommand()
{

}

void AddCommand::start()
{
  if ( mCollection.isValid() ) {
    fetchBase();
    return;
  }
  
  CollectionPathResolver *resolver = new CollectionPathResolver( mPath, this );
  connect( resolver, SIGNAL(result(KJob*)), this, SLOT(onPathResolved(KJob*)) );
  resolver->start();
}

void AddCommand::setupCommandOptions( KCmdLineOptions &options )
{
  options.add( "+add", ki18nc( "@info:shell", "The name of the command" ) );
  options.add( "+collection", ki18nc( "@info:shell", "The collection to add to, either as a path or akonadi URL" ) );
  options.add( "+files", ki18nc( "@info:shell", "The files to add to the collection." ) );
}

int AddCommand::initCommand( KCmdLineArgs *parsedArgs )
{
  if ( parsedArgs->count() < 2 ) {
    emit error( ki18nc( "@info:shell",
                         "Missing collection argument. See <application>%1</application> help add'" ).subs( KCmdLineArgs::appName()
                       ).toString()
              );
    return InvalidUsage;
  }
  
  if ( parsedArgs->count() < 3 ) {
    emit error( ki18nc( "@info:shell",
                         "Missing file argument. See <application>%1</application> help add'" ).subs( KCmdLineArgs::appName()
                       ).toString()
              );
    return InvalidUsage;
  }
  
  // check if we have an Akonadi URL. if not check if we have a path
  const QString collectionArg = parsedArgs->arg( 1 );
  const QUrl url = parsedArgs->url( 1 );
  if ( url.isValid() && url.scheme() == QLatin1String( "akonadi" ) ) {
    mCollection = Collection::fromUrl( url );
  } else {
    if ( collectionArg.startsWith( CollectionPathResolver::pathDelimiter() ) ) {
      mPath = collectionArg;
    }
  }
    
  if ( !mCollection.isValid() && mPath.isEmpty() ) {
    emit error( ki18nc( "@info:shell",
                         "Invalid collection argument '%1. See <application>%2</application> help add'" ).subs( collectionArg )
                                                                                                          .subs( KCmdLineArgs::appName()
                       ).toString()
              );

    return InvalidUsage;
  }
  
  for ( int i = 2; i < parsedArgs->count(); ++i ) {
    mFiles << parsedArgs->arg( i );
  }
  
  return NoError;
}

void AddCommand::fetchBase()
{
  if ( mCollection == Collection::root() ) {
    processNextFile();
    return;
  }
  
  CollectionFetchJob *job = new CollectionFetchJob( mCollection, CollectionFetchJob::Base, this );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(onBaseFetched(KJob*)) );
}

void AddCommand::processNextFile()
{
  if ( mFiles.isEmpty() ) {
    emit finished( NoError );
    return;
  }
  
  const QString fileName = mFiles.first();
  mFiles.pop_front();
  
  QFile file( fileName );
  if ( !file.exists() ) {
    emit error( i18nc( "@info:shell", "File <filename>%1</filename> does not exist" ).arg( fileName ) );
    QMetaObject::invokeMethod( this, "processNextFile", Qt::QueuedConnection );
    return;
  }
  
  if ( !file.open( QIODevice::ReadOnly ) ) {
    emit error( i18nc( "@info:shell", "File <filename>%1</filename> cannot be read" ).arg( fileName ) );
    QMetaObject::invokeMethod( this, "processNextFile", Qt::QueuedConnection );
    return;
  }
  
  const KMimeType::Ptr mimeType = KMimeType::findByNameAndContent( fileName, &file );
  if ( !mimeType->isValid() ) {
    emit error( i18nc( "@info:shell", "Cannot determine MIME type of file <filename>%1</filename>" ).arg( fileName ) );
    QMetaObject::invokeMethod( this, "processNextFile", Qt::QueuedConnection );
    return;
  }
  
  kDebug() << "file=" << fileName << "mime=" << mimeType->name();

  Item item;
  item.setMimeType( mimeType->name() );
  
  file.reset();
  item.setPayloadFromData( file.readAll() );
  
  ItemCreateJob *job = new ItemCreateJob( item, mCollection );
  job->setProperty( "fileName", fileName );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(onItemCreated(KJob*)) );
}

void AddCommand::onPathResolved( KJob *job )
{
  if ( job->error() != 0 ) {
    emit error( job->errorString() );
    emit finished( -1 ); // TODO correct error code
    return;
  }

  CollectionPathResolver *resolver = qobject_cast<CollectionPathResolver*>( job );
  Q_ASSERT( resolver != 0 );
  
  mCollection = Collection( resolver->collection() );
  fetchBase();
}

void AddCommand::onBaseFetched(KJob* job)
{
  if ( job->error() != 0 ) {
    emit error( job->errorString() );
    emit finished( -1 ); // TODO correct error code
    return;
  }
  
  CollectionFetchJob *fetchJob = qobject_cast<CollectionFetchJob*>( job );
  Q_ASSERT( fetchJob != 0 );
  
  const Collection::List collections = fetchJob->collections();
  if ( collections.isEmpty() ) {
    emit error( job->errorString() );
    emit finished( -1 ); // TODO correct error code
    return;
  }

  mCollection = fetchJob->collections().first();
  
  processNextFile();
}

void AddCommand::onItemCreated( KJob *job )
{
  const QString fileName = job->property( "fileName" ).toString();
  
  if ( job->error() != 0 ) {
    const QString msg = i18nc( "@info:shell", "Failed to add <filename>%1</filename>: %2" ).arg( fileName ). arg( job->errorString() );
    emit error( msg );
  } else {
    kDebug() << "Successfully added file" << fileName; 
  }
  
  processNextFile();
}
