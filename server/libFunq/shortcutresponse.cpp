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

#include "shortcutresponse.h"

#include "player.h"

#include <QApplication>
#include <QKeyEvent>
#include <QtTest>

ShortcutResponse::ShortcutResponse(JsonClient * client,
                                   const QtJson::JsonObject & command)
    : DelayedResponse(client), m_target(NULL) {
    if (command.contains("oid")) {
        WidgetLocatorContext<QWidget> ctx(static_cast<Player *>(jsonClient()),
                                          command, "oid");
        if (ctx.hasError()) {
            writeResponse(ctx.lastError);
            return;
        }
        m_target = ctx.widget;
    } else {
        m_target = qApp->activeWindow();
    }
    m_binding = QKeySequence::fromString(command["keysequence"].toString());
}

void ShortcutResponse::start() {
    QTimer::singleShot(0, this, SLOT(execute_repaint()));
}

void ShortcutResponse::execute_repaint() {
    if (m_target) {
        m_target->repaint();
    }
    QTimer::singleShot(100, this, SLOT(execute_grab_keyboard()));
}

void ShortcutResponse::execute_grab_keyboard() {
    if (m_target) {
        m_target->grabKeyboard();
    }
    QTimer::singleShot(100, this, SLOT(execute_press_keys()));
}

void ShortcutResponse::execute_press_keys() {
    // taken from
    // http://stackoverflow.com/questions/14283764/how-can-i-simulate-emission-of-a-standard-key-sequence
    for (int i = 0; i < m_binding.count(); ++i) {
        uint key = m_binding[i];
        Qt::KeyboardModifiers modifiers =
            static_cast<Qt::KeyboardModifiers>(key & Qt::KeyboardModifierMask);
        key = key & ~Qt::KeyboardModifierMask;
        if (m_target) {
            QTest::keyPress(m_target, static_cast<Qt::Key>(key), modifiers);
        }
    }
    QTimer::singleShot(100, this, SLOT(execute_release_keys()));
}

void ShortcutResponse::execute_release_keys() {
    for (int i = 0; i < m_binding.count(); ++i) {
        uint key = m_binding[i];
        Qt::KeyboardModifiers modifiers =
            static_cast<Qt::KeyboardModifiers>(key & Qt::KeyboardModifierMask);
        key = key & ~Qt::KeyboardModifierMask;
        if (m_target) {
            QTest::keyRelease(m_target, static_cast<Qt::Key>(key), modifiers);
        }
    }
    QTimer::singleShot(100, this, SLOT(execute_release_keyboard()));
}

void ShortcutResponse::execute_release_keyboard() {
    if (m_target) {
        m_target->releaseKeyboard();
    }
    writeResponse(QtJson::JsonObject());
}
