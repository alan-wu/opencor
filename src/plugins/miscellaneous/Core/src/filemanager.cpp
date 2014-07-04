/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// File manager
//==============================================================================

#include "cliutils.h"
#include "filemanager.h"

//==============================================================================

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTimer>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

FileManager::FileManager() :
    mActive(true),
    mFiles(QList<File *>()),
    mFilesReadable(QMap<QString, bool>()),
    mFilesWritable(QMap<QString, bool>())
{
    // Create our timer

    mTimer = new QTimer(this);

    // A connection to handle the timing out of our timer

    connect(mTimer, SIGNAL(timeout()),
            this, SLOT(checkFiles()));
}

//==============================================================================

FileManager::~FileManager()
{
    // Delete some internal objects

    delete mTimer;

    // Remove all the managed files

    foreach (File *file, mFiles)
        delete file;
}

//==============================================================================

FileManager * FileManager::instance()
{
    // Return the 'global' instance of our file manager class

    static FileManager instance;

    return static_cast<FileManager *>(Core::globalInstance("OpenCOR::Core::FileManager::instance()",
                                                           &instance));
}

//==============================================================================

FileManager::Status FileManager::manage(const QString &pFileName,
                                        const File::Type &pType,
                                        const QString &pUrl)
{
    // Manage the given file, should it not be already managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);

    if (QFile::exists(nativeFileName)) {
        if (isManaged(nativeFileName)) {
            // The file is already managed, so...

            return AlreadyManaged;
        } else {
            // The file isn't already managed, so add it to our list of managed
            // files and let people know about it being now managed

            mFiles << new File(nativeFileName, pType, pUrl);

            if (!mTimer->isActive())
                mTimer->start(1000);

            emit fileManaged(nativeFileName);

            return Added;
        }
    } else {
        // The file doesn't exist, so...

        return DoesNotExist;
    }
}

//==============================================================================

FileManager::Status FileManager::unmanage(const QString &pFileName)
{
    // Unmanage the given file, should it be managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);
    File *file = isManaged(nativeFileName);

    if (file) {
        // The file is managed, so we can remove it

        mFiles.removeAt(mFiles.indexOf(file));

        delete file;

        if (mFiles.isEmpty())
            mTimer->stop();

        emit fileUnmanaged(nativeFileName);

        return Removed;
    } else {
        // The file isn't managed, so...

        return NotManaged;
    }
}

//==============================================================================

File * FileManager::isManaged(const QString &pFileName) const
{
    // Return whether the given file is managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);

    foreach (File *file, mFiles)
        if (!file->fileName().compare(nativeFileName))
            // The file has been found meaning it is managed

            return file;

    // The file couldn't be found meaning it's not managed

    return 0;
}

//==============================================================================

bool FileManager::isActive() const
{
    // Return whether we are active

    return mActive;
}

//==============================================================================

void FileManager::setActive(const bool &pActive)
{
    // Set whether we are active

    mActive = pActive;
}

//==============================================================================

QString FileManager::sha1(const QString &pFileName) const
{
    // Return the SHA-1 value of the given file, should it be managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);
    File *file = isManaged(nativeFileName);

    if (file)
        return file->sha1();
    else
        return QString();
}

//==============================================================================

void FileManager::reset(const QString &pFileName)
{
    // Reset the given file, should it be managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);
    File *file = isManaged(nativeFileName);

    if (file)
        file->reset();
}

//==============================================================================

int FileManager::newIndex(const QString &pFileName) const
{
    // Return the given file's new index, if it is being managed

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->newIndex();
    else
        return 0;
}

//==============================================================================

QString FileManager::url(const QString &pFileName) const
{
    // Return the given file's URL, if it is being managed

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->url();
    else
        return QString();
}

//==============================================================================

bool FileManager::isDifferent(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, is different from
    // its corresponding physical version

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->isDifferent();
    else
        return false;
}

//==============================================================================

bool FileManager::isNew(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, is new

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->isNew();
    else
        return false;
}

//==============================================================================

bool FileManager::isRemote(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, is a remote one

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->isRemote();
    else
        return false;
}

//==============================================================================

bool FileManager::isModified(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, has been modified

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->isModified();
    else
        return false;
}

//==============================================================================

bool FileManager::isNewOrModified(const QString &pFileName) const
{
    // Return whether the given file is new or modified

    return isNew(pFileName) || isModified(pFileName);
}

//==============================================================================

void FileManager::setModified(const QString &pFileName, const bool &pModified)
{
    // Set the modified state of the given file, should it be managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);
    File *file = isManaged(nativeFileName);

    if (file && file->setModified(pModified))
        emit fileModified(nativeFileName);
}

//==============================================================================

bool FileManager::isReadable(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, is readable

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->isReadable();
    else
        return false;
}

//==============================================================================

bool FileManager::isWritable(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, is writable

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->isWritable();
    else
        return false;
}

//==============================================================================

bool FileManager::isReadableAndWritable(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, is readable and
    // writable

    return isReadable(pFileName) && isWritable(pFileName);
}

//==============================================================================

bool FileManager::isLocked(const QString &pFileName) const
{
    // Return whether the given file, if it is being managed, is locked

    File *file = isManaged(nativeCanonicalFileName(pFileName));

    if (file)
        return file->isLocked();
    else
        return false;
}

//==============================================================================

FileManager::Status FileManager::setLocked(const QString &pFileName,
                                           const bool &pLocked)
{
    // Set the locked status of the given file, should it be managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);
    File *file = isManaged(nativeFileName);

    if (file) {
        File::Status status = file->setLocked(pLocked);

        if (status == File::LockedSet)
            emit filePermissionsChanged(nativeFileName);

        if (status == File::LockedNotNeeded)
            return LockedNotNeeded;
        else if (status == File::LockedSet)
            return LockedSet;
        else
            return LockedNotSet;
    } else {
        return NotManaged;
    }
}

//==============================================================================

void FileManager::reload(const QString &pFileName)
{
    // Make sure that the given file is managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);
    File *file = isManaged(nativeFileName);

    if (file) {
        // The file is managed, so reset its settings and let people know that
        // it should be reloaded

        file->reset();

        emit fileReloaded(nativeFileName);
    }
}

//==============================================================================

bool FileManager::newFile(const QString &pContents, QString &pFileName)
{
    // Create a new file

    QTemporaryFile file(QDir::tempPath()+QDir::separator()+"XXXXXX.tmp");

    if (file.open()) {
        file.setAutoRemove(false);
        // Note: by default, a temporary file is to autoremove itself, but we
        //       clearly don't want that here...

        QTextStream fileOut(&file);

        fileOut << pContents;

        file.close();

        pFileName = file.fileName();

        return true;
    } else {
        pFileName = QString();

        return false;
    }
}

//==============================================================================

FileManager::Status FileManager::create(const QString &pUrl,
                                        const QString &pContents)
{
    // Create a new file

    QString createdFileName;

    if (newFile(pContents, createdFileName)) {
        // Let people know that we have created a file

        emit fileCreated(createdFileName, pUrl);

        return Created;
    } else {
        return NotCreated;
    }
}

//==============================================================================

FileManager::Status FileManager::rename(const QString &pOldFileName,
                                        const QString &pNewFileName)
{
    // Make sure that the given 'old' file is managed

    QString oldNativeFileName = nativeCanonicalFileName(pOldFileName);
    File *file = isManaged(oldNativeFileName);

    if (file) {
        // The 'old' file is managed, so rename it and let people know about it

        QString newNativeFileName = nativeCanonicalFileName(pNewFileName);

        if (file->setFileName(newNativeFileName)) {
            emit fileRenamed(oldNativeFileName, newNativeFileName);

            return Renamed;
        } else {
            return RenamingNotNeeded;
        }
    } else {
        return NotManaged;
    }
}

//==============================================================================

FileManager::Status FileManager::duplicate(const QString &pFileName)
{
    // Make sure that the given file is managed

    QString nativeFileName = nativeCanonicalFileName(pFileName);
    File *file = isManaged(nativeFileName);

    if (file) {
        // The file is managed, so retrieve its contents

        QString fileContents;

        if (Core::readTextFromFile(pFileName, fileContents)) {
            // Now, we can create a new file, which contents will be that of our
            // given file

            QString duplicatedFileName;

            if (newFile(fileContents, duplicatedFileName)) {
                // Let people know that we have duplicated a file

                emit fileDuplicated(duplicatedFileName);

                return Duplicated;
            } else {
                return NotDuplicated;
            }
        } else {
            return NotDuplicated;
        }
    } else {
        return NotManaged;
    }
}

//==============================================================================

int FileManager::count() const
{
    // Return the number of files currently being managed

    return mFiles.count();
}

//==============================================================================

void FileManager::checkFiles()
{
    // We only want to check our files if we are active and if there is no
    // currently active dialog box

    if (!mActive || QApplication::activeModalWidget())
        return;

    // Check our various files, as well as their locked status, but only if they
    // are not being ignored

    foreach (File *file, mFiles) {
        QString fileName = file->fileName();

        switch (file->check()) {
        case File::Changed:
            // The file has changed, so let people know about it

            emit fileChanged(fileName);

            break;
        case File::Deleted:
            // The file has been deleted, so let people know about it

            emit fileDeleted(fileName);

            break;
        default:
            // The file is unchanged, so do nothing...

            ;
        }

        bool fileReadable = isReadable(fileName);
        bool fileWritable = isWritable(fileName);

        if (    (fileReadable != mFilesReadable.value(fileName, false))
            ||  (fileWritable != mFilesWritable.value(fileName, false))
            || !(   mFilesReadable.contains(fileName)
                 && mFilesWritable.contains(fileName))) {

            emit filePermissionsChanged(fileName);

            mFilesReadable.insert(fileName, fileReadable);
            mFilesWritable.insert(fileName, fileWritable);
        }
    }
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
