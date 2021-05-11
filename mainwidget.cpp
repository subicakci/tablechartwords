#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent),
      m_isCaseSensitive(true)
{
    m_caseCheckBox->setChecked(m_isCaseSensitive);
    m_findWordsButton->setEnabled(false);

    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setHorizontalHeaderLabels({"Word Combination", "Percentage"});

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumSize(640, 480);

    QWidget *leftContentWidget = new QWidget();

    // file settings and options
    QFormLayout *fileSettingsLayout = new QFormLayout(leftContentWidget);
    fileSettingsLayout->addRow("File name : ", m_filenameLabel);
    fileSettingsLayout->addRow(m_caseCheckBox);
    fileSettingsLayout->addRow(m_openFileButton);
    fileSettingsLayout->addRow(m_findWordsButton);

    QGroupBox *fileSettings = new QGroupBox("File");
    fileSettings->setLayout(fileSettingsLayout);

    //connect objects' signals with their slots
    connect(m_openFileButton, &QPushButton::clicked, this, &MainWidget::handleOpenFileClicked);
    connect(m_findWordsButton, &QPushButton::clicked, this, &MainWidget::handleFindWordsClicked);
    connect(m_caseCheckBox, &QCheckBox::toggled, [this](bool set) {m_isCaseSensitive = set;});

    // create left layout
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(fileSettings);
    leftLayout->addWidget(m_tableWidget);
    leftContentWidget->setLayout(leftLayout);

    // create main layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(leftContentWidget, 1, 0);
    mainLayout->addWidget(m_chartView, 1, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(0, 0);
    setLayout(mainLayout);


}

MainWidget::~MainWidget()
{
}

// find all word combinations contain more than 3 letters
void MainWidget::findWordCombinations(HashWCollection &wcol)
{
    QStringList list;

    QRegExp rx("([a-zA-Z]{4,})"); // only letters and greater than 4
    int pos = 0;
    while ((pos = rx.indexIn(m_content, pos)) != -1) {

        list << rx.cap(1);
        pos += rx.matchedLength();
    }

    for (QStringList::iterator it = list.begin();it != list.end(); ++it)
    {
        QString word = m_isCaseSensitive ? *it : it->toLower();
        int word_size = word.size();

        for (int i = 0;i < word_size; i++)
        {
            for (int len = 4; len <= word_size - i; len++)
            {
                ++(wcol[word.mid(i,len)]);
            }
        }
    }

}

// find most frequent 10 words
VectorWCollection MainWidget::findTopFrequentWords(HashWCollection &wcol)
{
    VectorWCollection vwords;
    VectorWCollection vtopwords;
    // copy key-value pairs from the hash to the vector
    for (HashWCollection :: iterator it =wcol.begin(); it!=wcol.end(); it++)
    {
        vwords.push_front(qMakePair(it.key(), it.value()));
    }

    VectorWCollection::iterator middle_it = vwords.size() <= 10 ? vwords.end() : vwords.begin() + 10;

    // sort the vector by increasing order of its pair's second value
    std::partial_sort(vwords.begin(), middle_it, vwords.end(),
                      []( const QPair<QString, unsigned int> &a, const QPair<QString, unsigned int> &b ) {
                      return a.second > b.second; } );

    std::copy(vwords.begin(), middle_it,std::back_inserter(vtopwords));

    return vtopwords;
}


// fill the table with top words
void MainWidget::updateTableContent(const VectorWCollection &vwords)
{
    m_tableWidget->clearContents();
    m_tableWidget->setColumnCount(2);
    m_tableWidget->setRowCount(vwords.size());

    //calculate summation of top word frequencies to calculate rate (percentage)
    auto sum = std::accumulate(vwords.begin(), vwords.end(), 0,
                               [](uint sum, const QPair<QString, uint> &p) {return sum + p.second; });

    int index = 0;
    for (auto vword : vwords) {
        QTableWidgetItem *newKeyItem = new QTableWidgetItem(tr("%1").arg(vword.first ));
        QTableWidgetItem *newValueItem = new QTableWidgetItem(tr("%1").arg(QString::number((100.0 * vword.second / sum)) + "%"));
        m_tableWidget->setItem(index,0,newKeyItem);
        m_tableWidget->setItem(index++,1,newValueItem);
    }

}

// draw the bar chart with top words
void MainWidget::updateBarChartContent(const VectorWCollection &vwords)
{
    QChart *chart = new QChart();
    QBarSet *set0 = new QBarSet("Frequency");
    QStringList *categories = new QStringList();
    QHorizontalStackedBarSeries *series = new QHorizontalStackedBarSeries();

    //calculate summation of top word frequencies to calculate rate (percentage)
    auto sum = std::accumulate(vwords.begin(), vwords.end(), 0,
                               [](uint sum, const QPair<QString, uint> &p) {return sum + p.second; });

    for(auto vword: vwords)
    {
        set0->append (100.0 * vword.second / sum );
        categories->prepend(vword.first);
    }

    series->append(set0);
    chart->addSeries(series);

    QBarCategoryAxis *axisY = new QBarCategoryAxis();
    axisY->append(*categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0.0, 100.0);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTitle("Frequency of Word Combination");

    m_chartView->setChart(chart);

}


// open the file and read it
void MainWidget::handleOpenFileClicked()
{

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Text file"), "", tr("Text Files (*.txt)"));

    QFile file(fileName);
    QFileInfo fileInfo(file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        m_filenameLabel->setText("File not read!");
        m_findWordsButton->setEnabled(false);
        return;
    }

    m_content = file.readAll(); // read whole text into m_content
    m_filenameLabel->setText(fileInfo.fileName());
    m_findWordsButton->setEnabled(true);

    //clear the table and the barchart for new operation
    m_tableWidget->clearContents();
    m_chartView->setChart(new QChart);

    file.close();

}

//handle Find Words button and fill the table and chart with words
void MainWidget::handleFindWordsClicked()
{

    HashWCollection wcol; //hash map for word and frequency pair
    findWordCombinations(wcol);
    VectorWCollection topwords = findTopFrequentWords(wcol);

    updateTableContent(topwords);
    updateBarChartContent(topwords);

}

