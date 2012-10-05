/**************************************************************************
**
** This file is part of the Qt Build Suite
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file.
** Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**************************************************************************/

#include "tst_language.h"

QHash<QString, ResolvedProduct::Ptr> TestLanguage::productsFromProject(ResolvedProject::Ptr project)
{
    QHash<QString, ResolvedProduct::Ptr> result;
    foreach (ResolvedProduct::Ptr product, project->products)
        result.insert(product->name, product);
    return result;
}

ResolvedModule::ConstPtr TestLanguage::findModuleByName(ResolvedProduct::Ptr product, const QString &name)
{
    foreach (const ResolvedModule::ConstPtr &module, product->modules)
        if (module->name == name)
            return module;
    return ResolvedModule::ConstPtr();
}

QVariant TestLanguage::productPropertyValue(ResolvedProduct::Ptr product, QString propertyName)
{
    QStringList propertyNameComponents = propertyName.split(QLatin1Char('.'));
    if (propertyNameComponents.count() > 1)
        propertyNameComponents.prepend(QLatin1String("modules"));
    return getConfigProperty(product->configuration->value(), propertyNameComponents);
}

void TestLanguage::initTestCase()
{
    //Logger::instance().setLogSink(new ConsolePrintLogSink);
    //Logger::instance().setLevel(LoggerTrace);
    loader = new Loader();
    loader->setSearchPaths(QStringList()
                           << QLatin1String(SRCDIR "../../../share/qbs")
                           << QLatin1String(SRCDIR "testdata"));
    buildConfig = Configuration::Ptr(new Configuration);
    QVariantMap cfgValue;
    setConfigProperty(cfgValue, QStringList() << "qbs" << "targetOS", "linux");
    buildConfig->setValue(cfgValue);
}

void TestLanguage::cleanupTestCase()
{
    delete loader;
}

void TestLanguage::conditionalDepends()
{
    bool exceptionCaught = false;
    ResolvedProduct::Ptr product;
    ResolvedModule::ConstPtr dependency;
    try {
        ProjectFile::Ptr projectFile = loader->loadProject(SRCDIR "testdata/conditionaldepends.qbs");
        ResolvedProject::Ptr project = loader->resolveProject(projectFile, "someBuildDirectory",
                                                              buildConfig);
        QVERIFY(project);
        QHash<QString, ResolvedProduct::Ptr> products = productsFromProject(project);

        product = products.value("conditionaldepends_derived");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QVERIFY(dependency);

        product = products.value("conditionaldepends_derived_false");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QCOMPARE(dependency, ResolvedModule::ConstPtr());

        product = products.value("product_props_true");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QVERIFY(dependency);

        product = products.value("product_props_false");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QCOMPARE(dependency, ResolvedModule::ConstPtr());

        product = products.value("project_props_true");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QVERIFY(dependency);

        product = products.value("project_props_false");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QCOMPARE(dependency, ResolvedModule::ConstPtr());

        product = products.value("module_props_true");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy2");
        QVERIFY(dependency);
        dependency = findModuleByName(product, "dummy");
        QVERIFY(dependency);

        product = products.value("module_props_false");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy2");
        QVERIFY(dependency);
        dependency = findModuleByName(product, "dummy");
        QCOMPARE(dependency, ResolvedModule::ConstPtr());

        product = products.value("contradictory_conditions1");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QVERIFY(dependency);

        product = products.value("contradictory_conditions2");
        QVERIFY(product);
        dependency = findModuleByName(product, "dummy");
        QVERIFY(dependency);
    } catch (const Error &e) {
        exceptionCaught = true;
        qDebug() << e.toString();
    }
    QCOMPARE(exceptionCaught, false);
}

void TestLanguage::productConditions()
{
    bool exceptionCaught = false;
    try {
        ProjectFile::Ptr projectFile = loader->loadProject(SRCDIR "testdata/productconditions.qbs");
        ResolvedProject::Ptr project = loader->resolveProject(projectFile, "someBuildDirectory",
                                                              buildConfig);
        QVERIFY(project);
        QHash<QString, ResolvedProduct::Ptr> products = productsFromProject(project);
        QCOMPARE(products.count(), 3);
        QVERIFY(products.value("product_no_condition"));
        QVERIFY(products.value("product_true_condition"));
        QVERIFY(products.value("product_condition_dependent_of_module"));
    }
    catch (const Error &e) {
        exceptionCaught = true;
        qDebug() << e.toString();
    }
    QCOMPARE(exceptionCaught, false);
}

void TestLanguage::propertiesBlocks_data()
{
    QTest::addColumn<QString>("propertyName");
    QTest::addColumn<QStringList>("expectedValues");

    QTest::newRow("init") << QString() << QStringList();
    QTest::newRow("property_overwrite") << QString("dummy.defines") << QStringList("OVERWRITTEN");
    QTest::newRow("property_overwrite_no_outer") << QString("dummy.defines") << QStringList("OVERWRITTEN");
    QTest::newRow("property_append_to_outer") << QString("dummy.defines") << (QStringList() << QString("ONE") << QString("TWO"));

    QTest::newRow("multiple_exclusive_properties") << QString("dummy.defines") << QStringList("OVERWRITTEN");
    QTest::newRow("multiple_exclusive_properties_no_outer") << QString("dummy.defines") << QStringList("OVERWRITTEN");
    QTest::newRow("multiple_exclusive_properties_append_to_outer") << QString("dummy.defines") << (QStringList() << QString("ONE") << QString("TWO"));

    QTest::newRow("ambiguous_properties") << QString("dummy.defines") << (QStringList() << QString("ONE") << QString("TWO") << QString("THREE"));
    QTest::newRow("cleanup") << QString() << QStringList();
}

void TestLanguage::propertiesBlocks()
{
    QString productName = QString::fromLocal8Bit(QTest::currentDataTag());
    if (productName == "init") {
        bool exceptionCaught = false;
        try {
            ProjectFile::Ptr projectFile = loader->loadProject(SRCDIR "testdata/propertiesblocks.qbs");
            project = loader->resolveProject(projectFile, "someBuildDirectory", buildConfig);
            QVERIFY(project);
        } catch (const Error &e) {
            exceptionCaught = true;
            qDebug() << e.toString();
        }
        QCOMPARE(exceptionCaught, false);
        return;
    } else if (productName == "cleanup") {
        project = ResolvedProject::Ptr();
        return;
    }

    QFETCH(QString, propertyName);
    QFETCH(QStringList, expectedValues);
    QVERIFY(project);
    QHash<QString, ResolvedProduct::Ptr> products = productsFromProject(project);
    ResolvedProduct::Ptr product = products.value(productName);
    QVERIFY(product);
    QCOMPARE(product->name, productName);
    QVariant v = productPropertyValue(product, propertyName);
    QCOMPARE(v.toStringList(), expectedValues);
}

void TestLanguage::fileTags_data()
{
    QTest::addColumn<QStringList>("expectedFileTags");

    QTest::newRow("init") << QStringList();
    QTest::newRow("filetagger_project_scope") << (QStringList() << "cpp");
    QTest::newRow("filetagger_product_scope") << (QStringList() << "asm");
    QTest::newRow("unknown_file_tag") << (QStringList() << "unknown-file-tag");
    QTest::newRow("set_file_tag_via_group") << (QStringList() << "c++");
    QTest::newRow("add_file_tag_via_group") << (QStringList() << "cpp" << "zzz");
    QTest::newRow("add_file_tag_via_group_and_file_ref") << (QStringList() << "cpp" << "zzz");
    QTest::newRow("cleanup") << QStringList();
}

void TestLanguage::fileTags()
{
    QString productName = QString::fromLocal8Bit(QTest::currentDataTag());
    if (productName == QLatin1String("init")) {
        bool exceptionCaught = false;
        try {
            ProjectFile::Ptr projectFile = loader->loadProject(SRCDIR "testdata/filetags.qbs");
            project = loader->resolveProject(projectFile, "someBuildDirectory", buildConfig);
            QVERIFY(project);
        }
        catch (const Error &e) {
            exceptionCaught = true;
            qDebug() << e.toString();
        }
        QCOMPARE(exceptionCaught, false);
        return;
    } else if (productName == QLatin1String("cleanup")) {
        project = ResolvedProject::Ptr();
        return;
    }

    QVERIFY(project);
    QFETCH(QStringList, expectedFileTags);
    QHash<QString, ResolvedProduct::Ptr> products = productsFromProject(project);
    ResolvedProduct::Ptr product;
    QVERIFY(product = products.value(productName));
    QCOMPARE(product->sources.count(), 1);
    SourceArtifact::ConstPtr sourceFile = *product->sources.begin();
    QStringList fileTags = sourceFile->fileTags.toList();
    fileTags.sort();
    QCOMPARE(fileTags, expectedFileTags);
}

QTEST_MAIN(TestLanguage)
