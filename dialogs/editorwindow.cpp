#include "editorwindow.h"
#include "ui_editorwindow.h"

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{
    ui->setupUi(this);
    initEditor();
    initAction();
}

EditorWindow::~EditorWindow()
{
    delete ui;
}

void EditorWindow::initAction(){
    connect(ui->actionNew,&QAction::triggered,this,[&](){
        newFile();
    });
    connect(ui->actionOpen,&QAction::triggered,this,[&](){
        open();
    });
    connect(ui->actionSave,&QAction::triggered,this,[&](){
        save();
    });
    connect(ui->actionCopy,&QAction::triggered,this,[&](){
        _editor->copy();
    });
    connect(ui->actionCut,&QAction::triggered,this,[&](){
       _editor->cut();
    });
    connect(ui->actionPaste,&QAction::triggered,this,[&](){
       _editor->paste();
    });
}

void EditorWindow::newFile()
{
    if (maybeSave()) {
        _editor->clear();
        setCurrentFile("");
    }
}

void EditorWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool EditorWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool EditorWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}


bool EditorWindow::maybeSave()
{
    if (_editor->isModified()) {
        int ret = QMessageBox::warning(this, tr("Application"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape);
        if (ret == QMessageBox::Yes)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void EditorWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    _editor->setText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool EditorWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << _editor->text();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void EditorWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    _editor->setModified(false);
    setWindowModified(false);

    QString shownName;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = strippedName(curFile);

    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Application")));
}

QString EditorWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}


void EditorWindow::openTextFile(const QString pathName){
    QFile file(pathName);
    if(file.open(QIODevice::ReadOnly|QIODevice::Text)){
        _editor->clear();
        _editor->append(file.readAll());
    }
    else
        qDebug() <<"The file open failed!";
}

void EditorWindow::initEditor(QWidget *parent){
    this->setAttribute(Qt::WA_QuitOnClose, false);
    //??????QsciScintilla?????????
    _editor = new QsciScintilla(parent);

    //????????????
    QsciLexerCPP *textLexer = new QsciLexerCPP(parent);
    _editor->setLexer(textLexer);
    textLexer->setPaper(QColor(250,250,250));                                       //??????
    textLexer->setColor(QColor(0,170,0),QsciLexerCPP::Comment);         //????????????

    //????????????
    QsciAPIs *apis = new QsciAPIs(textLexer);
    apis->add(QString("class"));
    apis->add(QString("api"));
    apis->add(QString("main"));
    apis->prepare();

    //????????????
    _editor->SendScintilla(QsciScintilla::SCI_SETCODEPAGE,QsciScintilla::SC_CP_UTF8);
    QFont line_font;
    line_font.setFamily("SimSun");
    line_font.setPointSize(11);
    _editor->setFont(line_font);
    _editor->setWrapMode(QsciScintilla::WrapWord);
    _editor->setEolMode(QsciScintilla::EolWindows);
    _editor->setTabWidth(4);
    _editor->setAutoIndent(true);
    _editor->setBackspaceUnindents(true);
    _editor->setTabIndents(true);
    _editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
    //?????????
    _editor->setCaretWidth(2);
    _editor->setCaretForegroundColor(QColor("darkCyan"));            //????????????
    _editor->setCaretLineVisible(true);                                                  //???????????????????????????
    _editor->setCaretLineBackgroundColor(QColor(255,255,255));   //????????????????????????
    //????????????
    _editor->setSelectionBackgroundColor(Qt::black);                     //?????????????????????
    _editor->setSelectionForegroundColor(Qt::white);                      //?????????????????????

    //????????????
    QFont margin_font;
    margin_font.setFamily("SimSun");
    margin_font.setPointSize(11);
    _editor->setMarginsFont(margin_font);
    _editor->setMarginType(0,QsciScintilla::NumberMargin);       //???????????????0?????????????????????
    _editor->setMarginLineNumbers(0,true);                                 //?????????0???????????????????????????True????????????
    _editor->setMarginWidth(0,15);                                                 //??????0????????????
    _editor->setMarginsBackgroundColor(Qt::gray);
    _editor->setMarginsForegroundColor(Qt::white);
    _editor->setFolding(QsciScintilla::BoxedTreeFoldStyle);          //????????????
    _editor->setFoldMarginColors(Qt::gray,Qt::lightGray);            //???????????????

    _editor->setAutoCompletionSource(QsciScintilla::AcsAll);     //????????????Ascii??????????????????
    _editor->setAutoCompletionThreshold(1);                                //??????????????????????????????????????????????????????

    this->setCentralWidget(_editor);
}
