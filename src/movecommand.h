/*
    Copyright (C) 2013  Jonathan Marten <jjm@keelhaul.me.uk>

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

#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include "copycommand.h"


class MoveCommand : public CopyCommand
{
  Q_OBJECT

  public:
    explicit MoveCommand(QObject *parent = 0);
    ~MoveCommand();

    QString name() const;

  public Q_SLOTS:
    virtual void start();

  protected:
    virtual void setupCommandOptions(KCmdLineOptions &options);
};

#endif							// MOVECOMMAND_H
