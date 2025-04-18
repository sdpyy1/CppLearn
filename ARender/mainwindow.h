#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "framelesswindow.h"

#include <QMainWindow>
#include "aboutpage.h"
#include "sidebar.h"
#include "editorpage.h"
QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

class MainWindow : public FramelessWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QHBoxLayout* _mainLayout = nullptr;
    SideBar* _sideBar = nullptr;
    AboutPage* _aboutPage = nullptr;
    EditorPage* _editorPage = nullptr;

    // Place holder widget for resizing pages
    QWidget* _placeHolderWidget = nullptr;


private:
    void resizePages(QResizeEvent* event);

private:
    virtual void showEvent(QShowEvent* event) override;
    virtual bool eventFilter(QObject* object, QEvent* event) override;
};
#endif // MAINWINDOW_H
