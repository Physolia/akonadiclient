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

#ifndef COLLECTIONRESOLVEJOB_H
#define COLLECTIONRESOLVEJOB_H

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/collectionpathresolver.h>
#include <KCompositeJob>

class HackedCollectionPathResolver : public Akonadi::CollectionPathResolver
{
    Q_OBJECT

public:
    explicit HackedCollectionPathResolver(const QString &path, QObject *parent = nullptr);
    explicit HackedCollectionPathResolver(const Akonadi::Collection &col, QObject *parent = nullptr);

protected:
    bool addSubjob(KJob *job) override;
};

class CollectionResolveJob : public KCompositeJob
{
    Q_OBJECT

public:
    explicit CollectionResolveJob(const QString &userInput, QObject *parent = nullptr);
    virtual ~CollectionResolveJob() = default;

    void start() override;

    bool hasUsableInput();
    bool hadSlash() const
    {
        return (mHadSlash);
    }
    Akonadi::Collection collection() const
    {
        return (mCollection);
    }
    QString formattedCollectionName() const;

    static Akonadi::Item parseItem(const QString &userInput, bool showError = false);
    static Akonadi::Collection parseCollection(const QString &userInput);

protected Q_SLOTS:
    void slotResult(KJob *job) override;

private:
    const QString mUserInput;
    Akonadi::Collection mCollection;
    bool mHadSlash;

private:
    void fetchBase();
};

#endif                          // COLLECTIONRESOLVEJOB_H
