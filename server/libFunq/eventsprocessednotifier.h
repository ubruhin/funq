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

#ifndef EVENTS_PROCESSED_NOTIFIER_H
#define EVENTS_PROCESSED_NOTIFIER_H

#include <QObject>
#include <QPointer>

/**
 * @brief Class to notify when all events in the event queue are processed
 *
 * Many actions of libFunq add events to the event queue with qApp->postEvent(),
 * for example to simulate mouse events. But as postEvents() is non-blocking
 * (i.e. returns immediately), we don't know when the event is actually
 * processed. To avoid race conditions, we want to wait until the event is fully
 * processed before sending the TCP response to the client.
 *
 * A simple method to wait until the event is processed is to use
 * qApp->sendEvent() instead of qApp->postEvent(). But this does not work if the
 * application under test performs a blocking action in the event handler (e.g.
 * by opening a QMessageBox), this would freeze libFunq and the TCP response is
 * never sent.
 *
 * So this class provides an alternative way to wait until an event is fully
 * processed. Basically the start() method adds a dummy event to the event
 * queue, with the smallest possible priority to make it the last event
 * processed, after any other event with higher priority. The event() method
 * receives the dummy event once its scheduled, and calls the events_processed()
 * slot to notify about the processed event.
 *
 * But the slot is not called directly - instead, a QTimer with interval 0 is
 * used to call the slot. This way, any previously scheduled signal will be
 * processed first. So, if the application under test has emitted a signal with
 * a queued connection, the corresponding slot is guaranteed to be called before
 * we emit the events_processed() slot.
 *
 * All these tricks help us to let the application under test process as many
 * events and queued signal-slot connections as possible before libFunq sends
 * the TCP response to the client. This reduces the risk of race conditions.
 */
class EventsProcessedNotifier : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructor
     */
    explicit EventsProcessedNotifier(QObject * parent = 0);

    /**
     * @brief Start with waiting for all events being processed.
     */
    void start();

protected:
    /**
     * @brief The event filter method.
     */
    bool event(QEvent * e);

signals:
    /**
     * @brief This signal will be emitted once all events are processed.
     */
    void events_processed();

private:
    QEvent * m_event;
};

#endif  // EVENTS_PROCESSED_NOTIFIER_H
