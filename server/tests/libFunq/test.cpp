/*
Copyright: SCLE SFE
Contributor: Julien Pagès <j.parkouss@gmail.com>

This software is a computer program whose purpose is to test graphical
applications written with the QT framework (http://qt.digia.com/).

This software is governed by the CeCILL v2.1 license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL v2.1 license and that you accept its terms.
*/

#include <QBuffer>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QShortcut>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTabBar>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtTest/QtTest>

#ifdef QT_QUICK_LIB
#include <QQuickItem>
#include <QQuickView>
#endif

#include "eventsprocessedresponse.h"
#include "objectpath.h"
#include "player.h"
#include "shortcutresponse.h"

class TestDragNDropWidget : public QWidget {
public:
    explicit TestDragNDropWidget(QWidget * parent = NULL) : QWidget(parent) {
        m_lineEditDrag = new QLineEdit;
        m_lineEditDrop = new QLineEdit;

        QHBoxLayout * layout = new QHBoxLayout;

        layout->addWidget(m_lineEditDrag);
        layout->addWidget(m_lineEditDrop);

        setLayout(layout);

        m_lineEditDrag->setDragEnabled(true);

        m_lineEditDrag->setObjectName("drag");
        m_lineEditDrop->setObjectName("drop");
        setObjectName("testdnd");
    }

    QLineEdit * m_lineEditDrag;
    QLineEdit * m_lineEditDrop;
};

class TestSlot : public QWidget {
    Q_OBJECT
public:
    QVariant m_variant;

public slots:
    const QVariant editVariant(const QVariant & variant) {
        m_variant = variant;
        return QVariant(123);
    }
};

class LibFunqTest : public QObject {
    Q_OBJECT
private slots:
    void test_objectPath_objectName_noname() {
        QObject obj;

        QString name = ObjectPath::objectName(&obj);
        QCOMPARE(name, QString("QObject"));
    }
    void test_objectPath_objectName_named() {
        QObject obj;
        obj.setObjectName("NAMEd");

        QString name = ObjectPath::objectName(&obj);
        QCOMPARE(name, QString("NAMEd"));
    }
    void test_objectPath_objectName_noname_with_siblings() {
        QObject parent;
        QObject obj(&parent);
        QObject obj2(&parent);

        QCOMPARE(ObjectPath::objectName(&obj), QString("QObject"));
        QCOMPARE(ObjectPath::objectName(&obj2), QString("QObject-1"));
    }
    void test_objectPath_objectName_named_with_siblings() {
        QObject parent;
        QObject obj(&parent);
        QObject obj2(&parent);

        obj.setObjectName("NAMEd");
        obj2.setObjectName("NAMEd2");

        QCOMPARE(ObjectPath::objectName(&obj), QString("NAMEd"));
        QCOMPARE(ObjectPath::objectName(&obj2), QString("NAMEd2"));
    }
    void test_objectPath_objectName_named_same_with_siblings() {
        QObject parent;
        QObject obj(&parent);
        QObject obj2(&parent);

        obj.setObjectName("NAMEd");
        obj2.setObjectName("NAMEd");

        QCOMPARE(ObjectPath::objectName(&obj), QString("NAMEd"));
        QCOMPARE(ObjectPath::objectName(&obj2), QString("NAMEd-1"));
    }
    void test_objectPath_objectPath_simple() {
        QObject parent;
        QObject obj(&parent);
        QObject obj2(&parent);

        obj.setObjectName("NAMEd");
        obj2.setObjectName("NAMEd");

        QCOMPARE(ObjectPath::objectPath(&obj), QString("QObject::NAMEd"));
        QCOMPARE(ObjectPath::objectPath(&obj2), QString("QObject::NAMEd-1"));
    }
    void test_objectPath_objectPath_simple_with_sep() {
        QObject parent;
        QObject obj(&parent);
        QObject obj2(&parent);

        obj.setObjectName("::NAMEd");
        obj2.setObjectName("NAMEd");

        QCOMPARE(ObjectPath::objectPath(&obj), QString("QObject:::_:NAMEd"));
        QCOMPARE(ObjectPath::objectPath(&obj2), QString("QObject::NAMEd"));
    }
    void test_objectPath_findObject_simple() {
        QMainWindow parent;
        QObject obj(&parent);
        QObject obj2(&parent);

        obj2.setObjectName("NAMEd");

        QCOMPARE(ObjectPath::findObject("QMainWindow::NAMEd"), &obj2);
    }
    void test_objectPath_findObject_with_sep() {
        QMainWindow parent;
        QObject obj(&parent);
        QObject obj2(&parent);

        obj2.setObjectName("::NAMEd");

        QCOMPARE(ObjectPath::findObject("QMainWindow:::_:NAMEd"), &obj2);
    }
    void test_objectpath_graphicsItemId() {
        QGraphicsView view;
        QGraphicsScene scene;
        view.setScene(&scene);

        QGraphicsRectItem item;
        QGraphicsRectItem child(&item);

        scene.addItem(&item);

        QCOMPARE(ObjectPath::graphicsItemId(&item), (qulonglong)&item);
        QCOMPARE(ObjectPath::graphicsItemId(&child), (qulonglong)&child);
    }
    void test_objectpath_graphicsItemFromId() {
        QGraphicsView view;
        QGraphicsScene scene;
        view.setScene(&scene);

        QGraphicsRectItem notInScene;
        QGraphicsRectItem parent;
        QGraphicsRectItem item(&parent);
        QGraphicsRectItem item2(&parent);

        scene.addItem(&parent);

        QCOMPARE(ObjectPath::graphicsItemFromId(&view, (qulonglong)&parent),
                 &parent);
        QCOMPARE(ObjectPath::graphicsItemFromId(&view, (qulonglong)&item),
                 &item);
        QCOMPARE(ObjectPath::graphicsItemFromId(&view, (qulonglong)&item2),
                 &item2);
        QCOMPARE(ObjectPath::graphicsItemFromId(&view, (qulonglong)&notInScene),
                 (QGraphicsItem *)NULL);
    }
    /*
     *
     * TESTS for player.cpp
     *
     */
    void test_player_widget_by_path() {
        QMainWindow w;
        QObject o(&w);

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject command;
        command["path"] = "QMainWindow::QObject";

        QtJson::JsonObject result = player.widget_by_path(command);

        QVERIFY(result["oid"].value<qulonglong>() != 0);
        QCOMPARE(player.registeredObject(result["oid"].value<qulonglong>()),
                 &o);
    }

    void test_player_widget_by_path_wrong_path() {
        QMainWindow w;
        QObject o(&w);

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject command;
        command["path"] = "QMainWindow::QObject3569";

        QtJson::JsonObject result = player.widget_by_path(command);

        QCOMPARE(result["success"].toBool(), false);
        QCOMPARE(result["errName"].toString(), QString("InvalidWidgetPath"));
    }

    void test_player_object_properties() {
        QMainWindow w;
        QObject o(&w);
        o.setObjectName("toto");

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::toto";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];

        QtJson::JsonObject result = player.object_properties(command);

        QCOMPARE(result["objectName"].toString(), QString("toto"));
    }

    void test_player_not_registered_object() {
        QMainWindow w;
        QObject o(&w);

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject command;
        QtJson::JsonObject result = player.object_properties(command);

        QCOMPARE(result["success"].toBool(), false);
        QCOMPARE(result["errName"].toString(), QString("NotRegisteredObject"));
    }

    void test_player_deleted_object() {
        QMainWindow w;
        QObject * o = new QObject(&w);

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::QObject";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];

        delete o;
        QtJson::JsonObject result = player.object_properties(command);

        QCOMPARE(result["success"].toBool(), false);
        QCOMPARE(result["errName"].toString(), QString("NotRegisteredObject"));
    }

    void test_player_active_widget() {
        QMainWindow w;

        w.show();
#if QT_VERSION >= 0x050000
        QTest::qWaitForWindowExposed(&w);
#else
        QTest::qWaitForWindowShown(&w);
#endif
        QApplication::setActiveWindow(&w);  // required with Xvfb
        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject command;

        QtJson::JsonObject result = player.active_widget(command);

        QVERIFY(result["oid"].value<qulonglong>() != 0);
        QCOMPARE(player.registeredObject(result["oid"].value<qulonglong>()),
                 &w);
    }

    void test_player_object_set_properties() {
        QMainWindow w;
        QObject o(&w);
        o.setObjectName("toto");

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::toto";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        QtJson::JsonObject properties;
        properties["objectName"] = "titi";
        command["oid"] = resultPath["oid"];
        command["properties"] = properties;

        player.object_set_properties(command);

        qApp->processEvents();

        QCOMPARE(o.objectName(), QString("titi"));
    }

    void test_player_widgets_list() {
        QMainWindow mw;
        QWidget w(&mw);

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject command;
        QtJson::JsonObject result = player.widgets_list(command);

        QtJson::JsonObject mwResult = result["QMainWindow"].toMap();
        QtJson::JsonObject childrenResult = mwResult["children"].toMap();
        QVERIFY(childrenResult.contains("QWidget"));
        QCOMPARE(mwResult["classes"].toStringList(),
                 QStringList() << "QMainWindow"
                               << "QWidget"
                               << "QObject");
        QCOMPARE(childrenResult["QWidget"].toMap()["classes"].toStringList(),
                 QStringList() << "QWidget"
                               << "QObject");
    }

    void test_player_widgets_list_with_oid() {
        QMainWindow mw;
        QWidget w(&mw);

        QBuffer buffer;

        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];
        QtJson::JsonObject result = player.widgets_list(command);

        QtJson::JsonObject wResult = result["QWidget"].toMap();
        QVERIFY(wResult["children"].toMap().isEmpty());
        QCOMPARE(wResult["classes"].toStringList(),
                 QStringList() << "QWidget"
                               << "QObject");
    }

    void test_player_widget_click() {
        QMainWindow mw;
        QPushButton * btn = new QPushButton("myBtn");
        mw.setCentralWidget(btn);

        QSignalSpy spy(btn, SIGNAL(clicked()));

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::QPushButton";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];
        player.widget_click(command);

        qApp->processEvents();
        QCOMPARE(spy.count(), 1);
    }

    void test_player_widget_close() {
        QMainWindow mw;
        mw.show();
#if QT_VERSION >= 0x050000
        QTest::qWaitForWindowExposed(&mw);
#else
        QTest::qWaitForWindowShown(&mw);
#endif

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];

        QCOMPARE(mw.isVisible(), true);

        player.widget_close(command);

        qApp->processEvents();
        QCOMPARE(mw.isVisible(), false);
    }

    void test_player_call_slot() {
        QMainWindow mw;
        TestSlot testslot;
        testslot.setObjectName("test_slot");
        mw.setCentralWidget(&testslot);

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::test_slot";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];
        command["slot_name"] = "editVariant";
        command["params"] = 23;

        EventsProcessedResponse * response =
            qobject_cast<EventsProcessedResponse *>(player.call_slot(command));
        QVariant result_slot = response->response()["result_slot"];
        QCOMPARE(result_slot, QVariant(123));
        QCOMPARE(testslot.m_variant, QVariant(23));
    }

    void test_player_widget_keyclick() {
        QMainWindow mw;
        QLineEdit * line = new QLineEdit();
        mw.setCentralWidget(line);

        QSignalSpy spy(line, SIGNAL(textEdited(const QString &)));

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::QLineEdit";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];
        command["text"] = "this is a new text";
        player.widget_keyclick(command);

        qApp->processEvents();
        QVERIFY(spy.count() > 0);
        QCOMPARE(line->text(), QString("this is a new text"));
    }

    void test_player_shortcut() {
        QMainWindow mw;
        QShortcut shortcut(Qt::Key_F2, &mw, 0, 0, Qt::ApplicationShortcut);
        mw.show();
#if QT_VERSION >= 0x050000
        QTest::qWaitForWindowExposed(&mw);
#else
        QTest::qWaitForWindowShown(&mw);
#endif
        QSignalSpy spy(&shortcut, SIGNAL(activated()));

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];
        command["keysequence"] = "F2";

        DelayedResponse * dresponse = player.shortcut(command);

        dresponse->start();

        QEventLoop loop;
        QObject::connect(
            dresponse, SIGNAL(aboutToWriteResponse(const QtJson::JsonObject &)),
            &loop, SLOT(quit()));
        loop.exec();

        QCOMPARE(spy.count(), 1);
    }

    void test_player_shortcut_15_times() {
        for (int i = 0; i < 15; i++) {
            test_player_shortcut();
        }
    }

    void test_player_tabbar_list() {
        QMainWindow mw;
        QTabBar tb(&mw);

        QStringList tabtexts = QStringList() << "toto"
                                             << "titi"
                                             << "tutu";

        foreach (const QString & txt, tabtexts) { tb.addTab(txt); }

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::QTabBar";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];

        QtJson::JsonObject result = player.tabbar_list(command);

        QCOMPARE(tabtexts, result["tabtexts"].toStringList());
    }

    void test_player_headerview_list() {
        QTableWidget table;
        QStringList columns = QStringList() << "C1"
                                            << "C2"
                                            << "C3";
        QStringList rows = QStringList() << "R1"
                                         << "R2";
        table.setColumnCount(columns.count());
        table.setRowCount(rows.count());
        table.setHorizontalHeaderLabels(columns);
        table.setVerticalHeaderLabels(rows);
        for (int i = 0; i < columns.count(); i++) {
            for (int j = 0; j < rows.count(); j++) {
                table.setItem(i, j, new QTableWidgetItem(""));
            }
        }
        table.horizontalHeader()->setObjectName("H");
        table.verticalHeader()->setObjectName("V");

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath, resultPath, command, result;

        commandPath["path"] = "QTableWidget::H";
        resultPath = player.widget_by_path(commandPath);
        command["oid"] = resultPath["oid"];
        result = player.headerview_list(command);
        QCOMPARE(result["headertexts"].toStringList(), columns);

        commandPath["path"] = "QTableWidget::V";
        resultPath = player.widget_by_path(commandPath);
        command["oid"] = resultPath["oid"];
        result = player.headerview_list(command);
        QCOMPARE(result["headertexts"].toStringList(), rows);
    }

    void test_player_headerview_click() {
        QTableWidget table;
        QStringList columns = QStringList() << "C1"
                                            << "C2"
                                            << "C3";
        QStringList rows = QStringList() << "R1"
                                         << "R2";
        table.setColumnCount(columns.count());
        table.setRowCount(rows.count());
        table.setHorizontalHeaderLabels(columns);
        table.setVerticalHeaderLabels(rows);
        for (int i = 0; i < columns.count(); i++) {
            for (int j = 0; j < rows.count(); j++) {
                table.setItem(i, j, new QTableWidgetItem(""));
            }
        }
        table.horizontalHeader()->setObjectName("H");
        table.verticalHeader()->setObjectName("V");

        table.resize(800, 600);

        table.show();
#if QT_VERSION >= 0x050000
        QTest::qWaitForWindowExposed(&table);
#else
        QTest::qWaitForWindowShown(&table);
#endif

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath, resultPath, command;

        for (int i = 0; i < columns.count(); i++) {
            QSignalSpy hspy(table.horizontalHeader(),
                            SIGNAL(sectionClicked(int)));
            commandPath["path"] = "QTableWidget::H";
            resultPath = player.widget_by_path(commandPath);
            command["oid"] = resultPath["oid"];
            command["indexOrName"] = i;
            player.headerview_click(command);
            qApp->processEvents();
            QCOMPARE(hspy.count(), 1);
            QCOMPARE(hspy.first().first().toInt(), i);
        }

        for (int i = 0; i < rows.count(); i++) {
            QSignalSpy vspy(table.verticalHeader(),
                            SIGNAL(sectionClicked(int)));
            commandPath["path"] = "QTableWidget::V";
            resultPath = player.widget_by_path(commandPath);
            command["oid"] = resultPath["oid"];
            command["indexOrName"] = i;
            player.headerview_click(command);
            qApp->processEvents();
            QCOMPARE(vspy.count(), 1);
            QCOMPARE(vspy.first().first().toInt(), i);
        }
    }

    void test_player_headerview_click_by_name() {
        QTableWidget table;
        QStringList columns = QStringList() << "C1"
                                            << "C2"
                                            << "C3";
        QStringList rows = QStringList() << "R1"
                                         << "R2";
        table.setColumnCount(columns.count());
        table.setRowCount(rows.count());
        table.setHorizontalHeaderLabels(columns);
        table.setVerticalHeaderLabels(rows);
        for (int i = 0; i < columns.count(); i++) {
            for (int j = 0; j < rows.count(); j++) {
                table.setItem(i, j, new QTableWidgetItem(""));
            }
        }
        table.horizontalHeader()->setObjectName("H");
        table.verticalHeader()->setObjectName("V");

        table.resize(800, 600);

        table.show();
#if QT_VERSION >= 0x050000
        QTest::qWaitForWindowExposed(&table);
#else
        QTest::qWaitForWindowShown(&table);
#endif

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath, resultPath, command;

        QSignalSpy hspy(table.horizontalHeader(), SIGNAL(sectionClicked(int)));
        commandPath["path"] = "QTableWidget::H";
        resultPath = player.widget_by_path(commandPath);
        command["oid"] = resultPath["oid"];
        command["indexOrName"] = "C2";
        player.headerview_click(command);
        qApp->processEvents();
        QCOMPARE(hspy.count(), 1);
        QCOMPARE(hspy.first().first().toInt(), 1);
    }

    void test_player_model_items() {
        QMainWindow mw;
        QTableView view(&mw);

        QStandardItemModel model(4, 4);
        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                QStandardItem * item = new QStandardItem(
                    QString("row %0, column %1").arg(row).arg(column));
                model.setItem(row, column, item);
            }
        }

        view.setModel(&model);

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject commandPath;
        commandPath["path"] = "QMainWindow::QTableView";

        QtJson::JsonObject resultPath = player.widget_by_path(commandPath);

        QtJson::JsonObject command;
        command["oid"] = resultPath["oid"];

        QtJson::JsonObject result = player.model_items(command);

        QList<QVariant> items = result["items"].toList();

        QCOMPARE(items.count(), 4 * 4);
    }

#if QT_VERSION < 0x050000
    /* TODO: this test crash on ubuntu Using Qt version 5.2.1 in
     * /usr/lib/x86_64-linux-gnu */
    void test_drag_ndrop() {
        TestDragNDropWidget dndwidget;

        dndwidget.show();
#if QT_VERSION >= 0x050000
        QTest::qWaitForWindowExposed(&dndwidget);
#else
        QTest::qWaitForWindowShown(&dndwidget);
#endif

        dndwidget.m_lineEditDrag->setText(
            "HELLO, I HOPE I WILL BE DRAGGED AND DROPPED !");
        dndwidget.m_lineEditDrag->selectAll();

        qApp->processEvents();

        QBuffer buffer;
        Player player(&buffer);

        QtJson::JsonObject command;
        QtJson::JsonObject commandPath;
        commandPath["path"] = "testdnd::drag";

        command["srcoid"] = player.widget_by_path(commandPath)["oid"];

        commandPath["path"] = "testdnd::drop";
        command["destoid"] = player.widget_by_path(commandPath)["oid"];

        DelayedResponse * dresponse = player.drag_n_drop(command);

        QCOMPARE(dndwidget.m_lineEditDrop->text(), QString(""));

        dresponse->start();

        QEventLoop loop;
        QObject::connect(
            dresponse, SIGNAL(aboutToWriteResponse(const QtJson::JsonObject &)),
            &loop, SLOT(quit()));
        loop.exec();

        QCOMPARE(dndwidget.m_lineEditDrop->text(),
                 QString("HELLO, I HOPE I WILL BE DRAGGED AND DROPPED !"));
    }

    void test_drag_ndrop_15_times() {
        for (int i = 0; i < 15; i++) {
            test_drag_ndrop();
        }
    }
#endif

#ifdef QT_QUICK_LIB
    /* QtQuick tests */

    void test_quick_item_by_path() {
        QQuickView view;
        view.setSource(QUrl::fromLocalFile(SOURCE_DIR "sample1.qml"));
        view.show();
        QTest::qWaitForWindowExposed(&view);

        QBuffer buffer;
        Player player(&buffer);

        // register the quick view
        QtJson::JsonObject command_get_view;
        command_get_view["path"] = "QQuickView";
        QtJson::JsonObject result_view =
            player.widget_by_path(command_get_view);

        // search for quick_object
        QtJson::JsonObject command;
        command["quick_window_oid"] = result_view["oid"].value<qulonglong>();
        command["path"] = "QQuickItem::QQuickRectangle";

        QtJson::JsonObject result = player.quick_item_find(command);

        QVERIFY(result["oid"].value<qulonglong>() != 0);
        QCOMPARE(
            player.registeredObject(result["oid"].value<qulonglong>()),
            view.contentItem()->childItems().first()->childItems().first());

        // search for object that does not exists
        command["path"] = "tototiti::tutu";
        result = player.quick_item_find(command);

        QCOMPARE(result["success"].toBool(), false);
        QCOMPARE(result["errName"].toString(), QString("InvalidQuickItem"));
    }

    void test_quick_item_find_by_id() {
        QQuickView view;
        view.setSource(QUrl::fromLocalFile(SOURCE_DIR "find_by_id.qml"));
        view.show();
        QTest::qWaitForWindowExposed(&view);

        QBuffer buffer;
        Player player(&buffer);

        qulonglong view_id = player.registerObject(&view);

        QtJson::JsonObject command;
        QtJson::JsonObject result;
        qulonglong oid;
        QQuickItem * item;

        // search for root object
        command["quick_window_oid"] = view_id;
        command["qid"] = "root";

        result = player.quick_item_find(command);
        oid = result["oid"].value<qulonglong>();
        QVERIFY(oid != 0);

        item = (QQuickItem *)oid;
        QCOMPARE(item->property("objectName").toString(), QString("MyRoot"));

        // search for rect object
        command["qid"] = "rect";

        result = player.quick_item_find(command);
        oid = result["oid"].value<qulonglong>();
        QVERIFY(oid != 0);

        item = (QQuickItem *)oid;
        QCOMPARE(item->property("objectName").toString(), QString("MyRect"));

        // search for rect object with full id
        command["qid"] = "root.rect";

        result = player.quick_item_find(command);
        oid = result["oid"].value<qulonglong>();
        QVERIFY(oid != 0);

        item = (QQuickItem *)oid;
        QCOMPARE(item->property("objectName").toString(), QString("MyRect"));

        // search for object that does not exists
        command["qid"] = "rootrect";

        result = player.quick_item_find(command);
        QCOMPARE(result["success"].toBool(), false);
        QCOMPARE(result["errName"].toString(), QString("InvalidQuickItem"));
    }

    void test_quick_item_click() {
        QQuickView view;
        view.setSource(QUrl::fromLocalFile(SOURCE_DIR "test_click.qml"));
        view.show();
        QTest::qWaitForWindowExposed(&view);

        QBuffer buffer;
        Player player(&buffer);

        qulonglong view_id = player.registerObject(&view);

        // search for quick_object
        QtJson::JsonObject command;
        command["quick_window_oid"] = view_id;
        command["path"] = "QQuickItem::QQuickRectangle";

        QtJson::JsonObject result = player.quick_item_find(command);

        QQuickItem * item =
            view.contentItem()->childItems().first()->childItems().first();
        QCOMPARE(item->property("color").toString(), QString("#272822"));

        // click on it
        QtJson::JsonObject command_click;
        command_click["oid"] = result["oid"].value<qulonglong>();
        player.quick_item_click(command_click);

        qApp->processEvents();

        QCOMPARE(item->property("color").toString(), QString("#ffffff"));
    }
#endif
};

QTEST_MAIN(LibFunqTest)
#include "test.moc"
