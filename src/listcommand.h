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

#ifndef LISTCOMMAND_H
#define LISTCOMMAND_H

#include "abstractcommand.h"

class KJob;

class ListCommand : public AbstractCommand
{
    Q_OBJECT

public:
    explicit ListCommand(QObject *parent = nullptr);
    ~ListCommand() override = default;

    QString name() const override;

public Q_SLOTS:
    void start() override;

protected:
    void setupCommandOptions(QCommandLineParser *parser) override;
    int initCommand(QCommandLineParser *parser) override;

private:
    bool mListItems;
    bool mListCollections;
    bool mListDetails;

private:
    void fetchCollections();
    void fetchItems();

private Q_SLOTS:
    void onBaseFetched(KJob *job);
    void onCollectionsFetched(KJob *job);
    void onItemsFetched(KJob *job);
};

#endif // LISTCOMMAND_H
