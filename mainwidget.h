#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include <QtCharts>

typedef QHash<QString, uint> HashWCollection;
typedef QVector<QPair<QString, uint>> VectorWCollection;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
    void findWordCombinations(HashWCollection &);
    VectorWCollection findTopFrequentWords(HashWCollection &);
    void updateTableContent(const VectorWCollection &);
    void updateBarChartContent(const VectorWCollection &);

private slots:
    void handleOpenFileClicked();
    void handleFindWordsClicked();

private:
    QPushButton *m_openFileButton = new QPushButton("&Open File", this);
    QPushButton *m_findWordsButton = new QPushButton("&Find Words", this);
    QCheckBox *m_caseCheckBox = new QCheckBox("C&ase sensitive", this);
    QLabel *m_filenameLabel= new QLabel(this);

    QTableWidget *m_tableWidget= new QTableWidget(this);
    QChartView *m_chartView = new QChartView(this);


    QString m_content;
    bool m_isCaseSensitive;

};
#endif // MAINWIDGET_H
