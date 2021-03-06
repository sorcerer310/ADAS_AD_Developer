#include "projectdialog.h"
#include "ui_projectdialog.h"

ProjectDialog::ProjectDialog(QSharedPointer<ProjectDataModel> pdm,QSharedPointer<RecentProjectDataModel> rpdm,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectDialog),
    pDataModel(pdm),
    rProjectDataModel(rpdm),
    rebackRecentProject(false)
{
    ui->setupUi(this);
        this->setAttribute(Qt::WA_QuitOnClose, false);
    initButton();
    initOpenPath();
    initStackedFirst();
    initStackedSecond();
    initValidator();
}


ProjectDialog::~ProjectDialog()
{
    delete ui;
}

///初始化所有按钮
void ProjectDialog::initButton()
{
    ui->pb_project_finish->hide();
    int nCount = ui->sw_project->count();

    //返回按钮事件
    connect(ui->pb_project_back,&QPushButton::clicked,this,[&]()
    {
        int nIndex = ui->sw_project->currentIndex();
        if(nIndex>0)
        {
            nIndex--;
            if(nIndex==0) ui->pb_project_back->setEnabled(false);
        }
        ui->sw_project->setCurrentIndex(nIndex);
    });

    //下一步按钮事件
    connect(ui->pb_project_next,&QPushButton::clicked,this,[&,nCount]()
    {
        int nIndex = ui->sw_project->currentIndex();
        if(nIndex<nCount)
        {
            nIndex++;
            ui->pb_project_back->setEnabled(true);
            ui->pb_project_next->setEnabled(true);
            if(nIndex==nCount-1) ui->pb_project_next->setEnabled(false);
        }
        ui->sw_project->setCurrentIndex(nIndex);
    });

    //取消按钮事件
    connect(ui->pb_project_cancel,&QPushButton::clicked,this,[&]()
    {
        emit projectDialogCanceled(rebackRecentProject);
        rebackRecentProject = false;
        this->close();
    });


    //完成按钮事件
    connect(ui->pb_project_finish,&QPushButton::clicked,this,[&]()
    {
        //create folder
        QDir folder;
        //        QString fullpath = projectPath+"/"+projectName+"/.ap";

        //创建项目文件夹
        QString projectFolder = projectPath+"/"+projectName;
        QString projectConfigFolder = projectFolder;

        bool existProjectFolder = folder.exists(projectFolder);
        if(existProjectFolder)
        {
            QMessageBox::information(this,tr("建立文件夹"),tr("文件夹已经存在，请选择其他路径"));
            return;
        }
        else
        {
            bool pfok = folder.mkdir(projectFolder);
            if(!pfok)
            {
                QMessageBox::information(this,tr("建立文件夹"),tr("建立文件夹失败：").append(projectFolder));
                return;
            }
        }


        //创建项目文件
        QFile file(projectConfigFolder+"/project.xml");
        bool file_exist = file.exists();
        if(!file_exist)
        {
            if(file.open(QFile::ReadWrite))
            {
                writeProjectXml(file);
                ui->sw_project->setCurrentIndex(0);
                pDataModel->setProject(projectName,projectPath+"/"+projectName);

                //为最近项目数据模型增加数据
                rProjectDataModel->sortProjectFirst(projectName,pDataModel);

                this->accept();
                return;
            }
        }

        //set current page index is 0;
        ui->sw_project->setCurrentIndex(0);


    });


    //项目StackedWidget切换事件
    connect(ui->sw_project,&QStackedWidget::currentChanged,this,[&](int currentPage)
    {
        switch(currentPage)
        {
        case 0:
            ui->pb_project_back->setEnabled(false);
            ui->pb_project_next->setEnabled(true);
            ui->le_project_name->setFocus();
            break;
        case 1:
            ui->pb_project_back->setEnabled(true);
            ui->pb_project_next->setEnabled(true);
            ui->le_config1->setFocus();
            ui->pb_project_finish->hide();
            break;
        case 2:
            ui->pb_project_back->setEnabled(true);
            ui->pb_project_next->setEnabled(false);
            ui->pb_project_finish->show();
            break;
        default:
            break;
        }
    });
}

void ProjectDialog::initOpenPath()
{
    connect(ui->pb_project_path,&QPushButton::clicked,this,[&]()
    {
        QString path = QFileDialog::getExistingDirectory(this,"选择目录","./");
        if(path.isEmpty()) return;
        else ui->le_project_path->setText(path);
    });
}

void ProjectDialog::initStackedFirst()
{
    connect(ui->le_project_name,&QLineEdit::textChanged,this,[&](const QString &str){
        projectName = str;
        ui->le_summary_project_name->setText(projectName);
    });
    connect(ui->le_project_path,&QLineEdit::textChanged,this,[&](const QString &str){projectPath = str;ui->le_summary_project_path->setText(projectPath);});
}

void ProjectDialog::initStackedSecond()
{
    connect(ui->le_config1,&QLineEdit::textChanged,this,[&](const QString &str){config1 = str;ui->le_summary_config1->setText(config1);});
    connect(ui->le_config2,&QLineEdit::textChanged,this,[&](const QString &str){config2=str;ui->le_summary_config2->setText(config2);});
}

void ProjectDialog::initValidator(){
    QRegExp pnameRegExp(R"([a-zA-Z0-9_]+)");
    ui->le_project_name->setValidator(new QRegExpValidator(pnameRegExp,ui->le_project_name));
    QRegExp ppathRegExp(R"([^<>/\\\|:""\*\?]+\.\w+$)");
    ui->le_project_path->setValidator(new QRegExpValidator(ppathRegExp,ui->le_project_path));
}

///写入项目配置xml文件
void ProjectDialog::writeProjectXml(QFile &file)
{
    QDomDocument doc;
    //写入头数据
    QDomProcessingInstruction instruction;
    instruction=doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);
    //根节点，记录项目名称
    QDomElement root = doc.createElement("Project");
    root.setAttribute("name",projectName);
    doc.appendChild(root);

    //FlowScene容器，可记录多个FlowScene,默认创建1个FlowScene
    QDomElement flowScenes = doc.createElement("FlowScenes");
    root.appendChild(flowScenes);
    QDomElement mainFlowScene = doc.createElement("FlowScene");
    mainFlowScene.setAttribute("name","mainScene");
    mainFlowScene.setAttribute("saveFile","mainScene.flow");
    flowScenes.appendChild(mainFlowScene);

    QTextStream out_stream(&file);
    doc.save(out_stream,4);
    file.close();
}

/**
 * @brief ProjectDialog::showFromRecentProjectDialog    通过最近项目窗口显示创建项目窗口
 */
void ProjectDialog::showFromRecentProjectDialog(){
    this->show();
    rebackRecentProject = true;
}
