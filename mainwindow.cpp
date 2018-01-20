#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QStringListModel"
#include "QMessageBox"
#include "QInputDialog"
#include <QTextStream>
#include <QLabel>
#include <QTextBrowser>

QStringList qFilter = QStringList() << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.bmp" << "*.BMP" << "*.wav" << "*.WAV" << "*.au" << "*.AU";
bool multifiles = true;

void outcallback( const char* ptr, std::streamsize count, void* pString )
{
  (void) count;
  QString* p = static_cast< QString* >( pString );
  p->append(ptr);
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->FreeSpaceProgressBar->setFormat("Select file in left section.");
    ui->centralWidget->setLayout(ui->gridLayout);
    sPath = getenv("HOME");
    selected_file=NULL;
    dirmodel = new QFileSystemModel(this);
    dirmodel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
    dirmodel->setNameFilters(qFilter);
    dirmodel->setRootPath(sPath);
    ui->FilesAndFoldersTreeView->setModel(dirmodel);
    ui->FilesAndFoldersTreeView->setRootIndex(dirmodel->index(sPath));
    filesmodel = new FilesToAddListModel(filenames);
    ui->FileTableView->setModel(filesmodel);
    files_v_header= ui->FileTableView->verticalHeader();
    connect(files_v_header, SIGNAL(sectionClicked(int)), this, SLOT(on_sectionClicked(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_ReloadButton_clicked()
{
    sPath=ui->FilesAndFolders_MainPath->text();
    if(QDir(sPath).exists()){
        dirmodel->setRootPath(sPath);
        ui->FilesAndFoldersTreeView->setRootIndex(dirmodel->index(sPath));
    }
    else{
        QWidget tmp;
        QMessageBox::critical(&tmp,"Error","Invalid Path!");
    }
}

void MainWindow::on_AddFilesButton_clicked()
{
    filenames.clear();
    filenames=QFileDialog::getOpenFileNames(this, tr("Open File"), sPath, "All files *.*");
    update_HideFilesModel(filenames);
}

void MainWindow::on_sectionClicked ( int logicalIndex )
{
    filesmodel->removeRow(logicalIndex);

    int count = filesmodel->rowCount() + 1;
    ui->FileTableView->verticalHeader()->setMaximumHeight(30 * count);

    ui->AddFilesButton->setEnabled(filesmodel->rowCount() < 1 || multifiles);
    update_FreeSpaceProgressBar();
}

void MainWindow::on_SettingsButton_clicked()
{
    dialog = new Dialog(this);
    dialog->setWindowTitle("Settings");
    dialog->setlabeltext("There will be some Settings.");
    dialog->setModal(true);
    dialog->show();
}

void MainWindow::on_HelpButton_clicked()
{
    dialog = new Dialog(this);
    dialog->setWindowTitle("Help");
    dialog->setlabeltext("There will be some Help.");
    dialog->setModal(true);
    dialog->show();
}

void MainWindow::on_CheckFileButton_clicked()
{
    if (selected_file != NULL)
    {
        GetContents();
    }
}

void MainWindow::on_EncodeButton_clicked()
{
    bool ok = false;
    passphrase=QInputDialog::getText(this,"Specify passphrase (leave empty for none)","Enter passphrase:",QLineEdit::Password, nullptr, &ok);
    if (!ok) return;
    //encode the data
    ExtractDialog *encode_dialog;
    encode_dialog =new ExtractDialog(this);
    encode_dialog->setWindowTitle("Encoder");
    encode_dialog->setModal(true);
    encode_dialog->setlabeltext("Encoding...");
    encode_dialog->show();
}

void MainWindow::on_FilesAndFoldersTreeView_clicked(const QModelIndex &index)
{
    if((ui->FilesAndFoldersTreeView->selectionModel()->isSelected(index))&&(QFileInfo(dirmodel->filePath(index)).isFile())){
        try
        {
            selected_file =  CvrStgFile::readFile ((dirmodel->filePath(index)).toUtf8().constData()) ;
            update_FreeSpaceProgressBar();
            ui->EncodeButton->setEnabled(true);
            ui->ExtractButton->setEnabled(true);
            ui->CheckFileButton->setEnabled(true);
        }
        catch (UnSupFileFormat e)
        {
            selected_file = NULL;
            ui->ExtractButton->setEnabled(false);
            ui->EncodeButton->setEnabled(false);
            ui->CheckFileButton->setEnabled(false);
            QWidget tmp;
            QMessageBox::critical(&tmp,"Error","Unsupported File!");
        }
    }
    else
    {
        ui->ExtractButton->setEnabled(false);
        ui->EncodeButton->setEnabled(false);
        selected_file = NULL;
    }

}

void MainWindow::update_HideFilesModel(QStringList filenames)
{
    int count = filesmodel->rowCount() + 1;
    ui->FileTableView->verticalHeader()->setMaximumHeight(30 * count);
    filesmodel->setStringList(filenames);
    ui->AddFilesButton->setEnabled(filesmodel->rowCount() < 1 || multifiles);
    update_FreeSpaceProgressBar();
}

void MainWindow::update_FreeSpaceProgressBar(){
    if(selected_file!=NULL){
        int temp=((filesmodel->get_sum_size())/(selected_file->getCapacity()))*100;
        ui->FreeSpaceProgressBar->setValue(temp);
        ui->FreeSpaceProgressBar->setFormat(QString::number(filesmodel->get_sum_size())+" / "+QString::number(selected_file->getCapacity()));
    }
}

void MainWindow::on_FilesAndFolders_MainPath_returnPressed()
{
    on_ReloadButton_clicked();
}

void MainWindow::GetContents()
{
    bool ok = false;
    passphrase = QInputDialog::getText(this,"Password Dialog","Enter passphrase (leave empty for none):",QLineEdit::Password, nullptr, &ok);
    if (!ok) return;
    try
    {
        QString fn;
        QStringList fnn;
        fnn.clear();

        QModelIndex index = ui->FilesAndFoldersTreeView->selectionModel()->selectedIndexes().at(0);
        std::string dirmodel_path = (dirmodel->filePath(index)).toUtf8().constData();
        Extractor ext (dirmodel_path, passphrase.toUtf8().constData());

        EmbData* embdata = ext.extract();

        fn = QString::fromUtf8(embdata->getFileName().data(), embdata->getFileName().size());
        if (!fnn.contains(fn))
        {
            fnn.append(fn);
        }
        if (fn.length() != 0)
        {
            update_HideFilesModel(fnn);
        }
        return;
    }
    catch (...)
    {

    }
}

void MainWindow::on_ExtractButton_clicked()
{
    if(selected_file!=NULL){
        bool ok = false;
        passphrase=QInputDialog::getText(this,"Password Dialog","Enter passphrase (leave empty for none):",QLineEdit::Password, nullptr, &ok);
        if (!ok) return;
        ExtractDialog *extract_dialog;
        extract_dialog=new ExtractDialog(this);
        extract_dialog->setWindowTitle("Extracter");
        extract_dialog->setModal(true);
        extract_dialog->show();
        try{
            QModelIndex index = ui->FilesAndFoldersTreeView->selectionModel()->selectedIndexes().at(0);
            std::string dirmodel_path = (dirmodel->filePath(index)).toUtf8().constData();
            Extractor ext (dirmodel_path, passphrase.toUtf8().constData());

            EmbData* embdata = ext.extract();

            // write data
            std::string fn ;
                // write extracted data to file with embedded file name
                fn = embdata->getFileName() ;
                if (fn.length() == 0) {
                    fn=(QInputDialog::getText(this,"Please specify a file name for the extracted data (there is no name embedded in the stego file).","Enter filename:",QLineEdit::Normal)).toUtf8().constData();
                }

            extract_dialog->setlabeltext( QString::fromStdString("Writing extracted data to: "+fn));

            QString * myString = new QString();
            myString->clear();

            StdRedirector<>* myRedirector = new StdRedirector<>( std::cerr, outcallback, myString );
            BinaryIO *io;
            try{
            io = new BinaryIO (dirmodel_path.substr(0,dirmodel_path.find_last_of("//"))+"/"+fn, BinaryIO::WRITE) ;
            }
            catch(SteghideError e){
                 QMessageBox::StandardButton reply = QMessageBox::question(this, "Overwrite file?", myString->toUtf8().constData(), QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    Args.Force.setValue(true);
                    io = new BinaryIO (fn, BinaryIO::WRITE);
                    Args.Force.setValue(false);
                }
                else
                {
                    extract_dialog->close();
                    return;
                }
                delete myRedirector;
                myString->clear();
            }

            if(io->is_open()){
                std::vector<BYTE> data = embdata->getData() ;
                float progress=0;
                unsigned long k;
                unsigned long size = data.size();
                for (std::vector<BYTE>::iterator i = data.begin() ; i != data.end() ; i++) {
                    io->write8 (*i);
                    k=((i-data.begin())/100);
                    progress=((k/(data.size()/100))*100);
                    extract_dialog->setprogressbarvalue(progress);
                }
                io->close() ;
                extract_dialog->setlabeltext( QString::fromStdString("wrote extracted data to \""+fn+"\"."));
            }
            else{
                extract_dialog->setlabeltext( "ERROR" );
            }
        }
        catch (...)
        {
            extract_dialog->setlabeltext( "ERROR" );
            extract_dialog->close();
            QMessageBox::StandardButton reply = QMessageBox::critical(this, "Extraction Failed", "Extraction failed, Retry with different password?", QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                on_ExtractButton_clicked();
            }
        }
    }
}
