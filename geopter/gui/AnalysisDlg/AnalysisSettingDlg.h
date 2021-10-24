#ifndef AnalysisSettingDlg_H
#define AnalysisSettingDlg_H

#include <QDialog>

class AnalysisSettingDlg : public QDialog
{
    Q_OBJECT

public:
    AnalysisSettingDlg(QWidget *parent);

    virtual void updateParentDockContent();
    virtual int parentDockType() const;
};

#endif // AnalysisSettingDlg_H
