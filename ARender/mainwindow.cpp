#include "mainwindow.h"
#include<QLabel>
//#include <_mingw_mac.h>
MainWindow::MainWindow(QWidget *parent)
    : FramelessWindow(20, parent)
{
    // Create main layout for page and sidebar
    _mainLayout = new QHBoxLayout(_windowWidget);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    _mainLayout->setSpacing(0);
    _windowWidget->setLayout(_mainLayout);
    // Create placeholder widget for pages
    _placeHolderWidget = new QWidget(_windowWidget);
    _placeHolderWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _placeHolderWidget->setMouseTracking(true);

    // Connect the resize event of the placeholder widget to the resizePages function using event filter
    _placeHolderWidget->installEventFilter(this);

    // Create sidebar
    _sideBar = new SideBar(_windowWidget);
    connect(_sideBar, &SideBar::onPageChanged, this, [=](PageWidget* previousPage, PageWidget* currentPage) {
        // Check for input validity
        if (previousPage == currentPage) {
            return;
        }
        if (previousPage != nullptr) {
            previousPage->offStage();
        }
        if (currentPage != nullptr) {
            currentPage->onStage();
        }
    });
    _sideBar->setMouseTracking(true);

    // Add sidebar and placeholder widget to main layout
    _mainLayout->addWidget(_sideBar);
    _mainLayout->addWidget(_placeHolderWidget);

    // Create about page and connect to side bar
    _aboutPage = new AboutPage(_placeHolderWidget);
    _aboutPage->setMouseTracking(true);
    _sideBar->addPage(_aboutPage);
}

MainWindow::~MainWindow()
{

}
