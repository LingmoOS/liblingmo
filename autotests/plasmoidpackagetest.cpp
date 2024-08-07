/*
    SPDX-FileCopyrightText: 2007 Bertjan Broeksema <b.broeksema@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmoidpackagetest.h"
#include "../config-lingmo.h"

#include <KJob>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <kzip.h>

#include <QDebug>

#include "applet.h"
#include "pluginloader.h"

void PlasmoidPackageTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void PlasmoidPackageTest::init()
{
    qDebug() << "PlasmoidPackage::init()";
    m_package = QString("Package");
    m_packageRoot = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/packageRoot";
    m_defaultPackage = Lingmo::PluginLoader::self()->loadPackage("Lingmo/Applet");
    cleanup(); // to prevent previous runs from interfering with this one
}

void PlasmoidPackageTest::cleanup()
{
    qDebug() << "cleaning up";
    // Clean things up.
    QDir(m_packageRoot).removeRecursively();
}

void PlasmoidPackageTest::createTestPackage(const QString &packageName)
{
    qDebug() << "Create test package" << m_packageRoot;
    QDir pRoot(m_packageRoot);
    // Create the root and package dir.
    if (!pRoot.exists()) {
        QVERIFY(QDir().mkpath(m_packageRoot));
    }

    // Create the package dir
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName));
    qDebug() << "Created" << (m_packageRoot + "/" + packageName);

    // Create the metadata.desktop file
    QFile file(m_packageRoot + "/" + packageName + "/metadata.desktop");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=" << packageName << "\n";
    out << "X-KDE-PluginInfo-Name=" << packageName << "\n";
    file.flush();
    file.close();

    qDebug() << "OUT: " << packageName;

    // Create the ui dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName + "/contents/ui"));

    // Create the main file.
    file.setFileName(m_packageRoot + "/" + packageName + "/contents/ui/main.qml");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out << "THIS IS A PLASMOID SCRIPT.....";
    file.flush();
    file.close();

    qDebug() << "THIS IS A PLASMOID SCRIPT THING";
    // Now we have a minimal plasmoid package which is valid. Let's add some
    // files to it for test purposes.

    // Create the images dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + packageName + "/contents/images"));
    file.setFileName(m_packageRoot + "/" + packageName + "/contents/images/image-1.svg");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out << "<svg>This is a test image</svg>";
    file.flush();
    file.close();

    file.setFileName(m_packageRoot + "/" + packageName + "/contents/images/image-2.svg");

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out.setDevice(&file);
    out << "<svg>This is another test image</svg>";
    file.flush();
    file.close();
}

void PlasmoidPackageTest::isValid()
{
    Lingmo::Package *p = new Lingmo::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);

    // A PlasmoidPackage is valid when:
    // - The package root exists.
    // - The package root consists an file named "ui/main.qml"
    QVERIFY(!p->isValid());

    // Create the root and package dir.
    QVERIFY(QDir().mkpath(m_packageRoot));
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package));

    // Should still be invalid.
    delete p;
    p = new Lingmo::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(!p->isValid());

    // Create the metadata.desktop file.
    QFile file(m_packageRoot + "/" + m_package + "/metadata.desktop");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=test\n";
    out << "Description=Just a test desktop file";
    file.flush();
    file.close();

    // Create the ui dir.
    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package + "/contents/ui"));

    // No main file yet so should still be invalid.
    delete p;
    p = new Lingmo::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(!p->isValid());

    // Create the main file.
    file.setFileName(m_packageRoot + "/" + m_package + "/contents/ui/main.qml");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    out.setDevice(&file);
    out << "THIS IS A PLASMOID SCRIPT.....\n";
    file.flush();
    file.close();

    file.setPermissions(QFile::ReadUser | QFile::WriteUser);
    // Main file exists so should be valid now.
    delete p;
    p = new Lingmo::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(p->isValid());
    QCOMPARE(p->contentsHash(), QString("db0b38c2b4fe21a9f37923cc25152340de055f6d"));
    delete p;
}

void PlasmoidPackageTest::filePath()
{
    return;
    // Package::filePath() returns
    // - {package_root}/{package_name}/path/to/file if the file exists
    // - QString() otherwise.
    Lingmo::Package *p = new Lingmo::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);

    QCOMPARE(p->filePath("scripts", "main"), QString());

    QVERIFY(QDir().mkpath(m_packageRoot + "/" + m_package + "/contents/ui/main.qml"));
    QFile file(m_packageRoot + "/" + m_package + "/contents/ui/main.qml");
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QTextStream out(&file);
    out << "THIS IS A PLASMOID SCRIPT.....";
    file.flush();
    file.close();

    // The package is valid by now so a path for code/main should get returned.
    delete p;
    p = new Lingmo::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);

    const QString path = QFileInfo(m_packageRoot + "/" + m_package + "/contents/ui/main.qml").canonicalFilePath();

    // Two ways to get the same info.
    // 1. Give the file type which refers to a class of files (a directory) in
    //    the package structure and the file name.
    // 2. Give the file type which refers to a file in the package structure.
    //
    // NOTE: scripts, main and mainscript are defined in packages.cpp and are
    //       specific for a PlasmoidPackage.
    QCOMPARE(p->filePath("scripts", "main"), path);
    QCOMPARE(p->filePath("mainscript"), path);
    delete p;
}

void PlasmoidPackageTest::entryList()
{
    // Create a package named @p packageName which is valid and has some images.
    createTestPackage(m_package);

    // Create a package object and verify that it is valid.
    Lingmo::Package *p = new Lingmo::Package(m_defaultPackage);
    p->setPath(m_packageRoot + '/' + m_package);
    QVERIFY(p->isValid());

    // Now we have a valid package that should contain the following files in
    // given filetypes:
    // fileTye - Files
    // scripts - {"main"}
    // images - {"image-1.svg", "image-2.svg"}
    QStringList files = p->entryList("scripts");
    QCOMPARE(files.size(), 1);
    QVERIFY(files.contains("main"));

    files = p->entryList("images");
    QCOMPARE(files.size(), 2);
    QVERIFY(files.contains("image-1.svg"));
    QVERIFY(files.contains("image-2.svg"));
    delete p;
}

void PlasmoidPackageTest::createAndInstallPackage()
{
    qDebug() << "                   ";
    qDebug() << "   CreateAndInstall ";
    createTestPackage("plasmoid_to_package");
    const QString packagePath = m_packageRoot + '/' + "testpackage.plasmoid";

    KZip creator(packagePath);
    QVERIFY(creator.open(QIODevice::WriteOnly));
    creator.addLocalDirectory(m_packageRoot + '/' + "plasmoid_to_package", ".");
    creator.close();
    KIO::NetAccess::del(QUrl::fromLocalFile(m_packageRoot + "/plasmoid_to_package"), 0);

    QVERIFY(QFile::exists(packagePath));

    KZip package(packagePath);
    QVERIFY(package.open(QIODevice::ReadOnly));
    const KArchiveDirectory *dir = package.directory();
    QVERIFY(dir); //
    QVERIFY(dir->entry("metadata.desktop"));
    const KArchiveEntry *contentsEntry = dir->entry("contents");
    QVERIFY(contentsEntry);
    QVERIFY(contentsEntry->isDirectory());
    const KArchiveDirectory *contents = static_cast<const KArchiveDirectory *>(contentsEntry);
    QVERIFY(contents->entry("ui"));
    QVERIFY(contents->entry("images"));

    m_defaultPackageStructure = new Lingmo::PackageStructure(this);
    Lingmo::Package *p = new Lingmo::Package(m_defaultPackageStructure);
    qDebug() << "Installing " << archivePath;
    // const QString packageRoot = "lingmo/plasmoids/";
    // const QString servicePrefix = "lingmo-applet-";
    KJob *job = p->install(archivePath, m_packageRoot);
    connect(job, SIGNAL(finished(KJob *)), SLOT(packageInstalled(KJob *)));

    // QVERIFY(p->isValid());
    delete p;
}

void PlasmoidPackageTest::packageInstalled(KJob *j)
{
    qDebug() << "!!!!!!!!!!!!!!!!!!!! package installed" << (j->error() == KJob::NoError);
    QVERIFY(j->error() == KJob::NoError);
    // QVERIFY(p->path());

    Lingmo::Package *p = new Lingmo::Package(m_defaultPackageStructure);
    KJob *jj = p->uninstall("org.kde.microblog-qml", m_packageRoot);
    // QObject::disconnect(j, SIGNAL(finished(KJob*)), this, SLOT(packageInstalled(KJob*)));
    connect(jj, SIGNAL(finished(KJob *)), SLOT(packageInstalled(KJob *)));
}

void PlasmoidPackageTest::packageUninstalled(KJob *j)
{
    qDebug() << "!!!!!!!!!!!!!!!!!!!!! package uninstalled";
    QVERIFY(j->error() == KJob::NoError);
}

QTEST_MAIN(PlasmoidPackageTest)

#include "moc_plasmoidpackagetest.cpp"
