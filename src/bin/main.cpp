/*
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QProcess>
#include <QtQml/qqml.h>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>

static const char *HOMESCREEN_CODE = "homescreen";
static const char *MESSAGES_CODE = "messages";
static const char *PHONE_CODE = "phone";
static const char *SILICA_CODE = "silica";

static const char *PAYPAL_DONATE = "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&"
                                   "hosted_button_id=R6AJV4U2G33XG";

static void toggleSet(QSet<QString> &set, const QString &entry)
{
    if (set.contains(entry)) {
        set.remove(entry);
    } else {
        set.insert(entry);
    }
}

class Helper: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool appsNeedRestart READ isAppsNeedRestart NOTIFY appsNeedRestartChanged)
    Q_PROPERTY(bool homescreenNeedRestart READ isHomescreenNeedRestart NOTIFY homescreenNeedRestartChanged)
public:
    explicit Helper(QObject *parent = 0);
    bool isAppsNeedRestart() const;
    bool isHomescreenNeedRestart() const;
public slots:
    void patchToggleService(const QString &patch, const QString &code);
    void restartServices();
    void restartLipstick();
signals:
    void appsNeedRestartChanged();
    void homescreenNeedRestartChanged();
private:
    QSet<QString> m_homescreenPatches;
    QSet<QString> m_voiceCallPatches;
    QSet<QString> m_messagesPatches;
    bool m_appsNeedRestart;
    bool m_homescreenNeedRestart;
};

Helper::Helper(QObject *parent)
    : QObject(parent), m_appsNeedRestart(false), m_homescreenNeedRestart(false)
{
}

bool Helper::isAppsNeedRestart() const
{
    return m_appsNeedRestart;
}

bool Helper::isHomescreenNeedRestart() const
{
    return m_homescreenNeedRestart;
}

void Helper::patchToggleService(const QString &patch, const QString &code)
{
    if (code == HOMESCREEN_CODE || code == SILICA_CODE) {
        toggleSet(m_homescreenPatches, patch);
    } else if (code == MESSAGES_CODE) {
        toggleSet(m_messagesPatches, patch);
    } else if (code == PHONE_CODE) {
        toggleSet(m_voiceCallPatches, patch);
    }

    bool newAppsNeedRestart = (!m_messagesPatches.isEmpty() || !m_voiceCallPatches.isEmpty());
    bool newHomescreenNeedRestart = !m_homescreenPatches.isEmpty();

    if (m_appsNeedRestart != newAppsNeedRestart) {
        m_appsNeedRestart = newAppsNeedRestart;
        emit appsNeedRestartChanged();
    }

    if (m_homescreenNeedRestart != newHomescreenNeedRestart) {
        m_homescreenNeedRestart = newHomescreenNeedRestart;
        emit homescreenNeedRestartChanged();
    }
}

void Helper::restartServices()
{
    if (!m_messagesPatches.isEmpty()) {
        QStringList arguments;
        arguments << "jolla-messages";
        QProcess::execute("killall", arguments);
        m_messagesPatches.clear();
    }

    if (!m_voiceCallPatches.isEmpty()) {
        QStringList arguments;
        arguments << "voicecall-ui";
        QProcess::execute("killall", arguments);
        m_voiceCallPatches.clear();
    }

    if (m_appsNeedRestart) {
        m_appsNeedRestart = false;
        emit appsNeedRestartChanged();
    }

    if (m_homescreenNeedRestart) {
        QStringList arguments;
        arguments << "--user" << "restart" << "lipstick.service";
        QProcess::startDetached("systemctl", arguments);
    }

    if (m_homescreenNeedRestart) {
        m_homescreenNeedRestart = false;
        emit homescreenNeedRestartChanged();
    }
}

void Helper::restartLipstick()
{
    QStringList arguments;
    arguments << "--user" << "restart" << "lipstick.service";
    QProcess::startDetached("systemctl", arguments);
}

int main(int argc, char *argv[])
{
    qmlRegisterType<Helper>("org.SfietKonstantin.patchmanager", 1, 0, "Helper");
    QGuiApplication *app = SailfishApp::application(argc, argv);
    QQuickView *view = SailfishApp::createView();
    view->engine()->rootContext()->setContextProperty("PAYPAL_DONATE", PAYPAL_DONATE);
    view->setSource(QUrl("/usr/share/patchmanager/qml/main.qml"));
    view->show();
    return app->exec();
}

#include "main.moc"
