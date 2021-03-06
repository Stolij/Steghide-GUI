#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QStringListModel"
#include "QMessageBox"
#include "QInputDialog"
#include "encodeoptionsdialog.h"
#include <steghide-src/Embedderlib.h>
#include <QTextStream>
#include <QLabel>
#include <QTextBrowser>
#include <QComboBox>

QStringList qFilter = QStringList() << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.bmp" << "*.BMP" << "*.wav" << "*.WAV" << "*.au" << "*.AU";
bool multifiles = false;

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
    ui->FilesAndFolders_MainPath->setText(sPath);
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
        QMessageBox::critical(this, "Error", "Invalid Path!");
        ui->FilesAndFolders_MainPath->setText(getenv("HOME"));
    }
}

void MainWindow::on_AddFilesButton_clicked()
{
    filenames.clear();
    filenames=QFileDialog::getOpenFileNames(this, tr("Open File"), sPath, "All files *.*");
    update_HideFilesModel(filenames);
}

void MainWindow::on_sectionClicked (int logicalIndex)
{
    filesmodel->removeRow(logicalIndex);
    filenames.removeAt(logicalIndex);

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
    if (filenames.size() < 1)
    {
        QMessageBox::critical(this, "No file selected", "Please select a file first!");
        return;
    }
    EncodeOptionsDialog *sdialog = new EncodeOptionsDialog(this);
    if (sdialog->exec() != 1) return;
    //encode the data
    QString password = sdialog->getPassword();
    EncryptionAlgorithm encryptionAlgorithm = EncryptionAlgorithm((EncryptionAlgorithm::IRep)(sdialog->getEncryptionAlgorithm()));
    EncryptionMode encryptionMode = EncryptionMode((EncryptionMode::IRep)(sdialog->getEncryptionMode()));
    try
    {
        QString embedfile = filenames[0];
        Arguments* a = new Arguments();
        EmbedderLib *embedder = new EmbedderLib(selected_file->getName(), embedfile.toStdString(), password.toStdString(), encryptionAlgorithm, encryptionMode);
        embedder->Embed();
        QMessageBox::information(this, "Embedding Succesful", "Embedding Succesful! \n\nencryption algorithm: "+QString::fromStdString(encryptionAlgorithm.getStringRep())+"\nencryption mode: "+QString::fromStdString(encryptionMode.getStringRep()));
    }
    catch (SteghideError& e)
    {
        QString errmessage = QString::fromStdString(e.getMessage());
        QMessageBox::critical(this, "Embedding Failed", "Embedding Failed! ("+ errmessage +")");
    }
}

void MainWindow::on_FilesAndFoldersTreeView_clicked(const QModelIndex &index)
{
    if((ui->FilesAndFoldersTreeView->selectionModel()->isSelected(index))&&(QFileInfo(dirmodel->filePath(index)).isFile())){
        try
        {
            selected_file = CvrStgFile::readFile((dirmodel->filePath(index)).toUtf8().constData()) ;
            ui->FilesAndFolders_MainPath->setText(dirmodel->filePath(index).toUtf8().constData());
            update_FreeSpaceProgressBar();
            ui->EncodeButton->setEnabled(true);
            ui->ExtractButton->setEnabled(true);
            ui->CheckFileButton->setEnabled(true);
        }
        catch (UnSupFileFormat e)
        {
            selected_file = NULL;
            ui->FilesAndFolders_MainPath->setText(sPath);
            ui->ExtractButton->setEnabled(false);
            ui->EncodeButton->setEnabled(false);
            ui->CheckFileButton->setEnabled(false);
            QMessageBox::critical(this, "Error", "Unsupported File!");
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
        double temp = ((double)(filesmodel->get_sum_size()) / (double)(selected_file->getCapacity())) * 100;
        temp = (temp <= 100) ? temp : 100;
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
        QString filename;
        QStringList filenamelist;
        filenamelist.clear();

        QModelIndex index = ui->FilesAndFoldersTreeView->selectionModel()->selectedIndexes().at(0);
        std::string dirmodel_path = (dirmodel->filePath(index)).toUtf8().constData();
        Extractor extractor (dirmodel_path, passphrase.toUtf8().constData());

        EmbData* embdata = extractor.extract();

        filename = QString::fromUtf8(embdata->getFileName().data(), embdata->getFileName().size());
        if (!filenamelist.contains(filename) && filename.length() != 0)
        {
            filenamelist.append(filename);
            update_HideFilesModel(filenamelist);
        }        
        return;
    }
    catch (...)
    {
        QMessageBox::StandardButton reply = QMessageBox::critical(this, "Get Contents Failed", "Getting contents failed, Retry with different password?", QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            GetContents();
        }
    }
}

bool MainWindow::getAnswer(std::string message)
{
    QString qmessage = QString::fromStdString(message);
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Question", qmessage, QMessageBox::Yes|QMessageBox::No);
    return reply;
}

void MainWindow::on_ExtractButton_clicked()
{
    if(selected_file!=NULL){
        bool ok = false;
        passphrase=QInputDialog::getText(this,"Password Dialog","Enter passphrase (leave empty for none):",QLineEdit::Password, nullptr, &ok);
        if (!ok) return;
        ExtractDialog *extract_dialog;
        extract_dialog=new ExtractDialog(this);
        extract_dialog->setWindowTitle("Extractor");
        extract_dialog->setModal(true);
        extract_dialog->show();
        try{
            QModelIndex index = ui->FilesAndFoldersTreeView->selectionModel()->selectedIndexes().at(0);
            std::string dirmodel_path = (dirmodel->filePath(index)).toUtf8().constData();
            Extractor ext (dirmodel_path, passphrase.toUtf8().constData());

            EmbData* embdata = ext.extract();

            // write data
            std::string filename ;
                // write extracted data to file with embedded file name
                filename = embdata->getFileName() ;
                if (filename.length() == 0) {
                    filename=(QInputDialog::getText(this,"Please specify a file name for the extracted data (embedded name wasn't found).","Enter filename:",QLineEdit::Normal)).toUtf8().constData();
                }

            extract_dialog->setlabeltext( QString::fromStdString("Writing extracted data to: "+filename));

            QString * myString = new QString();
            myString->clear();

            StdRedirector<>* myRedirector = new StdRedirector<>( std::cerr, outcallback, myString );
            BinaryIO *io;
            try{
            io = new BinaryIO (dirmodel_path.substr(0,dirmodel_path.find_last_of("//"))+"/"+filename, BinaryIO::WRITE) ;
            }
            catch(SteghideError e){
                 QMessageBox::StandardButton reply = QMessageBox::question(this, "Overwrite file?", myString->toUtf8().constData(), QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    Args.Force.setValue(true);
                    io = new BinaryIO (filename, BinaryIO::WRITE);
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
                //unsigned long size = data.size();
                for (std::vector<BYTE>::iterator i = data.begin() ; i != data.end() ; i++) {
                    io->write8 (*i);
                    k=((i-data.begin())/100);
                    progress=((k/(data.size()/100))*100);
                    extract_dialog->setprogressbarvalue(progress);
                }
                io->close() ;
                extract_dialog->setlabeltext( QString::fromStdString("wrote extracted data to \""+filename+"\"."));
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
