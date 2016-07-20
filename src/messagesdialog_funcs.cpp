#include "messagesdialog.h"
#include "epsrproject.h"

MessagesDialog::MessagesDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    QByteArray messageText = mainWindow_->messageText();
    char *messages = messageText.data();
    ui.textBrowser->setText(messages);

//    connect(mainWindow_->messageText(),SIGNAL(there are none...), this, SLOT(refresh()));
}

void MessagesDialog::refresh()
{
    QByteArray messageText = mainWindow_->messageText();
    char *messages = messageText.data();
    ui.textBrowser->setText(messages);
}
