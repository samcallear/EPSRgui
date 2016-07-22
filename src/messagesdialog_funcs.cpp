#include "messagesdialog.h"
#include "epsrproject.h"

MessagesDialog::MessagesDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::Tool);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

//    connect(mainWindow_->messageText(),SIGNAL(there are none...), this, SLOT(refresh()));
}

void MessagesDialog::refreshMessages()
{
    QByteArray messageText = mainWindow_->messageText();
    char *messages = messageText.data();
    ui.textBrowser->setText(messages);
    ui.textBrowser->verticalScrollBar()->setValue(ui.textBrowser->verticalScrollBar()->maximum());
}
