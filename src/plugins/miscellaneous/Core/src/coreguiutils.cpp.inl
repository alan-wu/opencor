/*******************************************************************************

Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

//==============================================================================
// Core GUI utilities
//==============================================================================

QMainWindow * mainWindow()
{
    // Retrieve and return our main window

    static bool firstTime = true;
    static QMainWindow *res = 0;

    if (firstTime) {
        foreach (QWidget *widget, qApp->topLevelWidgets()) {
            if (widget->inherits("OpenCOR::MainWindow")) {
                res = qobject_cast<QMainWindow *>(widget);

                break;
            }
        }

        firstTime = false;
    }

    return res;
}

//==============================================================================

bool aboutToQuit()
{
    // Return whether we are about to quit

    return qApp->property("OpenCOR::aboutToQuit()").toBool();
}

//==============================================================================

QSize minimumWidgetSize(QWidget *pWidget)
{
    // Determine the minimum size of the given widget
    // Note: this is based on QWidgetPrivate::adjustedSize()...

    if (!pWidget)
        return QSize();

    QSize minimumSize = pWidget->sizeHint();

    if (pWidget->isWindow()) {
        Qt::Orientations expandingDirections;
        QLayout *layout = pWidget->layout();

        if (layout) {
            if (layout->hasHeightForWidth())
                minimumSize.setHeight(layout->totalHeightForWidth(minimumSize.width()));

            expandingDirections = layout->expandingDirections();
        } else {
            if (pWidget->sizePolicy().hasHeightForWidth())
                minimumSize.setHeight(pWidget->heightForWidth(minimumSize.width()));

            expandingDirections = pWidget->sizePolicy().expandingDirections();
        }

        if (expandingDirections & Qt::Horizontal)
            minimumSize.setWidth(qMax(minimumSize.width(), 200));

        if (expandingDirections & Qt::Vertical)
            minimumSize.setHeight(qMax(minimumSize.height(), 100));

        QRect screen = QApplication::desktop()->screenGeometry(pWidget->pos());

        minimumSize.setWidth(qMin(minimumSize.width(), screen.width()*2/3));
        minimumSize.setHeight(qMin(minimumSize.height(), screen.height()*2/3));
    }

    if (!minimumSize.isValid()) {
        QRect childrenRect = pWidget->childrenRect();

        if (!childrenRect.isNull())
            minimumSize = childrenRect.size()+QSize(2*childrenRect.x(), 2*childrenRect.y());
    }

    return minimumSize;
}

//==============================================================================

void adjustWidgetSize(QWidget *pWidget)
{
    // Retrieve the minimum size of the given , and resize and recenter it, if
    // needed

    QSize minimumSize = minimumWidgetSize(pWidget);

    if (!minimumSize.isValid())
        return;

    QSize oldSize = pWidget->size();
    QSize newSize = QSize(qMax(minimumSize.width(), oldSize.width()),
                          qMax(minimumSize.height(), oldSize.height()));

    if (newSize != oldSize) {
        // Note: if the given widget is not visible then we only resize it
        //       otherwise we also recenter it since being visible we are sure
        //       that it has an original position to recenter against...

        if (pWidget->isVisible()) {
            pWidget->setGeometry(pWidget->geometry().x()-((newSize.width()-oldSize.width()) >> 1),
                                 pWidget->geometry().y()-((newSize.height()-oldSize.height()) >> 1),
                                 newSize.width(), newSize.height());
        } else {
            pWidget->resize(newSize);
        }
    }
}

//==============================================================================

void showEnableAction(QAction *pAction, const bool &pVisible,
                      const bool &pEnabled)
{
    // Show/enable or hide/disable the given action

    pAction->setVisible(pVisible);
    pAction->setEnabled(pVisible && pEnabled);
}

//==============================================================================

QColor baseColor()
{
    // Return the base colour

    return qApp->palette().color(QPalette::Base);
}

//==============================================================================

QColor borderColor()
{
    // Return the mid colour, which we consider as the colour to be used for a
    // 'normal' border
    // Note: this is not quite the correct colour, but nonetheless the one that
    //       is closest to it. Indeed, to use a hidden widget of sorts to
    //       retrieve the colour of a border works fine on all our supported
    //       operating systems except Windows 10 (while it works fine on
    //       previous versions of Windows) where it has a side effect that
    //       prevents OpenCOR from retrieving the size of its main window. Not
    //       only that, but we can still see the 'hidden' widget...

    return qApp->palette().color(QPalette::Mid);
}

//==============================================================================

QColor highlightColor()
{
    // Return the highlight colour

    return qApp->palette().color(QPalette::Highlight);
}

//==============================================================================

QColor shadowColor()
{
    // Return the shadow colour

    return qApp->palette().color(QPalette::Shadow);
}

//==============================================================================

QColor windowColor()
{
    // Return the window colour

    return qApp->palette().color(QPalette::Window);
}

//==============================================================================

QMessageBox::StandardButton showMessageBox(QWidget *pParent,
                                           const QMessageBox::Icon &pIcon,
                                           const Qt::TextInteractionFlags &pFlags,
                                           const QString &pTitle,
                                           const QString &pText,
                                           const QMessageBox::StandardButtons &pButtons,
                                           const QMessageBox::StandardButton &pDefaultButton)
{
    // Create, show and return the result of a message box with the given
    // properties

    QMessageBox messageBox(pIcon, pTitle, pText, QMessageBox::NoButton, pParent);

    messageBox.setTextInteractionFlags(pFlags);

    QDialogButtonBox *buttonBox = messageBox.findChild<QDialogButtonBox*>();

    Q_ASSERT(buttonBox);

    uint mask = QMessageBox::FirstButton;

    while (mask <= QMessageBox::LastButton) {
        uint sb = pButtons & mask;

        mask <<= 1;

        if (!sb)
            continue;

        QPushButton *button = messageBox.addButton((QMessageBox::StandardButton)sb);

        if (messageBox.defaultButton())
            continue;

        if (   (   (pDefaultButton == QMessageBox::NoButton)
                && (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole))
            || (   (pDefaultButton != QMessageBox::NoButton) &&
                   (sb == uint(pDefaultButton)))) {
            messageBox.setDefaultButton(button);
        }
    }

    if (messageBox.exec() == -1)
        return QMessageBox::Cancel;

    return messageBox.standardButton(messageBox.clickedButton());
}

//==============================================================================

QMessageBox::StandardButton informationMessageBox(const QString &pTitle,
                                                  const QString &pText,
                                                  const QMessageBox::StandardButtons &pButtons,
                                                  const QMessageBox::StandardButton &pDefaultButton)
{
    // Return the result of an information message box

    return showMessageBox(mainWindow(), QMessageBox::Information,
                          Qt::LinksAccessibleByMouse, pTitle, pText, pButtons,
                          pDefaultButton);
}

//==============================================================================

QMessageBox::StandardButton questionMessageBox(const QString &pTitle,
                                               const QString &pText,
                                               const QMessageBox::StandardButtons &pButtons,
                                               const QMessageBox::StandardButton &pDefaultButton)
{
    // Return the result of a question message box

    return showMessageBox(mainWindow(), QMessageBox::Question,
                          Qt::LinksAccessibleByMouse, pTitle, pText, pButtons,
                          pDefaultButton);
}

//==============================================================================

QMessageBox::StandardButton warningMessageBox(const QString &pTitle,
                                              const QString &pText,
                                              const QMessageBox::StandardButtons &pButtons,
                                              const QMessageBox::StandardButton &pDefaultButton)
{
    // Return the result of a warning message box

    return showMessageBox(mainWindow(), QMessageBox::Warning,
                          Qt::TextSelectableByMouse|Qt::LinksAccessibleByMouse,
                          pTitle, pText, pButtons, pDefaultButton);
}

//==============================================================================

QMessageBox::StandardButton criticalMessageBox(const QString &pTitle,
                                               const QString &pText,
                                               const QMessageBox::StandardButtons &pButtons,
                                               const QMessageBox::StandardButton &pDefaultButton)
{
    // Return the result of a critical message box

    return showMessageBox(mainWindow(), QMessageBox::Critical,
                          Qt::TextSelectableByMouse|Qt::LinksAccessibleByMouse,
                          pTitle, pText, pButtons, pDefaultButton);
}

//==============================================================================

void aboutMessageBox(const QString &pTitle, const QString &pText)
{
    // Show an about message box

#ifdef Q_OS_MAC
    static QPointer<QMessageBox> oldMessageBox;

    if (oldMessageBox && !oldMessageBox->text().compare(pText)) {
        oldMessageBox->show();
        oldMessageBox->raise();
        oldMessageBox->activateWindow();

        return;
    }
#endif

    QMessageBox *messageBox = new QMessageBox(  pTitle, pText, QMessageBox::Information, 0, 0, 0, mainWindow()
#ifdef Q_OS_MAC
                                              , Qt::WindowTitleHint|Qt::WindowSystemMenuHint
#endif
                                             );

    messageBox->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    messageBox->setAttribute(Qt::WA_DeleteOnClose);

    QIcon icon = messageBox->windowIcon();
    QSize size = icon.actualSize(QSize(64, 64));

    messageBox->setIconPixmap(icon.pixmap(size));

#ifdef Q_OS_MAC
    oldMessageBox = messageBox;

    QDialogButtonBox *buttonBox = messageBox->findChild<QDialogButtonBox*>();

    Q_ASSERT(buttonBox);

    buttonBox->setCenterButtons(true);

    messageBox->show();
#else
    messageBox->exec();
#endif
}

//==============================================================================
// End of file
//==============================================================================
