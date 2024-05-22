#include "ProjectSearch.h"
#include "ui_ProjectSearchGUI.h"

ProjectSearch::ProjectSearch(QWidget *parent) : QWidget(parent), ui(new Ui::ProjectSearchGUI) {
    ui->setupUi(this);
}

ProjectSearch::~ProjectSearch() { delete ui; }
