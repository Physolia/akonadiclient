/*
 * Copyright (C) 2017 Jonathan Marten <jjm@keelhaul.me.uk>
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

#pragma once

#include <qstringlist.h>

#include <Akonadi/Item>
#include <Akonadi/Tag>

#include "abstractcommand.h"

class DumpCommand : public AbstractCommand
{
    Q_OBJECT

public:
    explicit DumpCommand(QObject *parent = nullptr);
    ~DumpCommand() override = default;

    [[nodiscard]] QString name() const override;

public Q_SLOTS:
    void start() override;

protected:
    AbstractCommand::Error initCommand(QCommandLineParser *parser) override;
    void setupCommandOptions(QCommandLineParser *parser) override;

private:
    bool mMaildir;
    bool mAkonadiCategories;

    QString mDirectoryArg;
    Akonadi::Item::List mItemList;
    Akonadi::Tag::List mTagList;
    QStringList mCreatedDirs;

private:
    void writeItem(const Akonadi::Item &item, const QString &parent);

private Q_SLOTS:
    void onCollectionFetched(KJob *job);
    void onItemsFetched(KJob *job);
    void onTagsFetched(KJob *job);
    void onParentPathFetched(KJob *job);
    void processNextItem();
};
