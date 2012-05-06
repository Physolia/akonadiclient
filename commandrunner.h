/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Kevin Krammer <kevin.krammer@gmx.at>

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


#ifndef COMMANDRUNNER_H
#define COMMANDRUNNER_H

#include <QObject>

class AbstractCommand;

class KCmdLineArgs;

class QCoreApplication;

class CommandRunner : public QObject
{
  Q_OBJECT
    
  public:
    explicit CommandRunner( KCmdLineArgs *parsedArgs );
    ~CommandRunner();
    
    int exec();
    
  private:
    QCoreApplication *mApplication;
    AbstractCommand *mCommand;
    
  private Q_SLOTS:
    void onCommandFinished( int exitCode );
    void onCommandError( const QString &error );
};

#endif // COMMANDRUNNER_H
