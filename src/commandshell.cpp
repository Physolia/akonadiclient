/*
 * Copyright (C) 2014  Bhaskar Kandiyal <bkandiyal@gmail.com>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "commandshell.h"

#include "abstractcommand.h"
#include "commandfactory.h"
#include "errorreporter.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <QTextStream>
#include <QCoreApplication>
#include <QVector>
#include <QDebug>
#include <QTextCodec>
#include <QVarLengthArray>

bool CommandShell::sIsActive = false;

CommandShell::CommandShell(const KAboutData &aboutData)
    : mCommand(0),
      mAboutData(aboutData)
{
    mTextStream = new QTextStream(stdin);
    mTextStream->setCodec(QTextCodec::codecForLocale());

    ErrorReporter::setAppName(aboutData.appName());
    sIsActive = true;
}

CommandShell::~CommandShell()
{
    delete mTextStream;
}

void CommandShell::start()
{
    if (!enterCommandLoop())
    {
        sIsActive = false;
        QCoreApplication::quit();
    }
}

bool CommandShell::enterCommandLoop()
{
    QString input = mTextStream->readLine();
    if (mTextStream->atEnd()) return (false);

    QStringList list;
    list.insert(0, mAboutData.appName());

    QString arg;
    bool quotestarted = false;

    // Split the options string by space and quotation marks
    for (int i = 0; i < input.length(); i++) {
        if (input.at(i) == ' ') {
            if (quotestarted) {
                arg.append(input.at(i));
            } else if (arg.length() > 0) {
                list.append(arg);
                arg.clear();
            }
        } else if (input.at(i) == '\"') {
            if (quotestarted) {
                list.append(arg);
                arg.clear();
                quotestarted = false;
            } else {
                quotestarted = true;
            }
        } else {
            arg.append(input.at(i));
        }
    }

    if (arg.length() > 0) {
        list.append(arg);
    }

    QVarLengthArray<char*> tempArgs;
    tempArgs.reserve(list.size());

    for (int i = 0; i < list.size(); i++) {
        std::string str = list.at(i).toStdString();
        char *temp = new char[str.length()+1];
        str.copy(temp, str.length());
        temp[str.length()] = '\0';
        tempArgs.append(temp);
    }

    char **args = tempArgs.data();

    KCmdLineArgs *parsedArgs = getParsedArgs(list.length(), args);
    if (parsedArgs->arg(0)=="quit" || parsedArgs->arg(0)=="exit") return (false);

    CommandFactory factory(parsedArgs);

    mCommand = factory.createCommand();
    QObject *toInvoke = this;
    if (mCommand!=NULL)
    {
        connect(mCommand, SIGNAL(error(QString)), this, SLOT(onCommandError(QString)));
        if (mCommand->init(parsedArgs)==AbstractCommand::NoError)
        {
            connect(mCommand, SIGNAL(finished(int)), this, SLOT(onCommandFinished(int)));
            toInvoke = mCommand;
        }
    }

    QMetaObject::invokeMethod(toInvoke, "start", Qt::QueuedConnection);
    freeArguments(tempArgs);
    return (true);
}

KCmdLineArgs* CommandShell::getParsedArgs(int argc, char **argv)
{
    KCmdLineArgs::reset();
    KCmdLineArgs::init(argc, argv, &mAboutData, KCmdLineArgs::CmdLineArgNone);

    KCmdLineOptions options;
    options.add("!+command", ki18nc("@info:shell", "Command to execute"));
    options.add("+[options]", ki18nc( "@info:shell", "Options for command"));
    options.add("+[args]", ki18nc( "@info:shell", "Arguments for command"));
    options.add("", ki18nc("@info:shell",
                           "See 'help' for available commands"
                           "\n"
                           "See 'help command' for more information on a specific command"));
    KCmdLineArgs::addCmdLineOptions(options);
    return KCmdLineArgs::parsedArgs();
}

void CommandShell::onCommandFinished(int exitCode)
{
    if (mCommand!=NULL) mCommand->deleteLater();
    mCommand = NULL;
    QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
}

void CommandShell::onCommandError(const QString &error)
{
    ErrorReporter::error(error);
}

void CommandShell::freeArguments(const QVarLengthArray<char*> &args)
{
    for (int i = 0; i < args.size(); i++) {
        delete[] args.at(i);
    }
}
