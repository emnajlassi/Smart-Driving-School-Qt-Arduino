#include "autoecole.h"
#include "ui_autoecole.h"
#include "employe.h"
#include "client.h"
#include "examen.h"
#include "vehicule.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlRecord>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QChart>
#include <QRandomGenerator>
#include <random>
#include <QStandardItemModel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebEngineView>
#include <QWebChannel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlError> // NÉCESSAIRE
#include <QByteArray>
#include <QTimer>



AutoEcole::AutoEcole(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AutoEcole)
    , arduino(nullptr)
{
    ui->setupUi(this);

    // Debug: List ALL buttons in the UI
    qDebug() << "=== LISTING ALL BUTTONS ===";
    QList<QPushButton*> allButtons = ui->centralwidget->findChildren<QPushButton*>();
    for (QPushButton *btn : allButtons) {
        qDebug() << "Button:" << btn->objectName() << "| Text:" << btn->text();
    }
    qDebug() << "=== END LIST ===";
    //arduino
    // ============ ARDUINO INITIALIZATION ============
    /* QObject::connect(A.getSerialPort(), &QSerialPort::readyRead, this, &AutoEcole::lireDonneesArduino);

    // Tenter la connexion au démarrage
    int ret = A.connect_arduino();
    if (ret == 1) {
        ui->label_statut_arduino->setText("Connecté");
        ui->label_statut_arduino->setStyleSheet("color: green;");
    } else {
        ui->label_statut_arduino->setText("Déconnecté/Erreur");
        ui->label_statut_arduino->setStyleSheet("color: red;");
    }*/
    // Arduino Connection
    // Arduino Connection
    int ret = A.connect_arduino();
    switch(ret) {
    case 0:
        qDebug() << "Arduino is available and connected to: " << A.getarduino_port_name();
        break;
    case 1:
        qDebug() << "Arduino is available but not connected to: " << A.getarduino_port_name();
        break;
    case -1:
        qDebug() << "Arduino is not available";
    }

    // Connect the readyRead signal from the serial port to the lireDonneesArduino slot
    QObject::connect(A.getSerialPort(), &QSerialPort::readyRead, this, &AutoEcole::lireDonneesArduino);




    ////weather/
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &AutoEcole::onWeatherDataReceived);
    // ---------------- AFFICHER-EMPLOYE ----------------
    connect(ui->tableemploye, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(on_tableemploye_clicked(const QModelIndex &)));

    ui->tableemploye->setModel(emp.afficher());
    //tri-employé
    connect(ui->comboBox_triEmploye, &QComboBox::currentTextChanged,
            this, &AutoEcole::on_comboBox_triEmploye_currentIndexChanged);
    //login****password
    ui->lineEdit_passwordlogin->setEchoMode(QLineEdit::Password);

    // ---------------- AFFICHER-CLIENT ----------------
    connect(ui->tableClient, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(on_tableClient_clicked(const QModelIndex &)));

    ui->tableClient->setModel(cl.afficher());
    //tri-client
    connect(ui->comboBox_triClient, &QComboBox::currentTextChanged,
            this, &AutoEcole::on_comboBox_triClient_currentIndexChanged);
    // ---------------- AFFICHER-EXAMEN ----------------
    connect(ui->tableExamen, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(on_tableExamen_clicked(const QModelIndex &)));

    ui->tableExamen->setModel(ex.afficher());
    //tri-examen
    connect(ui->comboBox_triExamen, &QComboBox::currentTextChanged,
            this, &AutoEcole::on_comboBox_triExamen_currentIndexChanged);
    // ---------------- AFFICHER-VEHICULE ----------------
    connect(ui->tableVehicule, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(on_tableVehicule_clicked(const QModelIndex &)));

    ui->tableVehicule->setModel(veh.afficher());
    //tri-vehicule
    connect(ui->comboBox_triVehicule, &QComboBox::currentTextChanged,
            this, &AutoEcole::on_comboBox_triVehicule_currentIndexChanged);
    // ---------------- AFFICHER-SEANCE ----------------
    connect(ui->tableseance, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(on_tableSeance_clicked(const QModelIndex &)));

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        // Replace X with your Seance page index
        if (index == 7) {
            ui->tableseance->setModel(s.afficher());
        }
    });

    //tri-seance
    connect(ui->comboBox_triSeance, &QComboBox::currentTextChanged,
            this, &AutoEcole::on_comboBox_triSeance_currentIndexChanged);

    // ========== CREATE NEW ARDUINO OBJECT ==========
    arduino = new Arduino();

    // ========== SETUP ARDUINO CONNECTIONS ==========
    setupArduinoConnections();

    qDebug() << "Système initialisé avec Arduino support.";
    // ============ CALENDAR INITIALIZATION ============
    // Colorize calendar with seances
    colorizeCalendar();

    // Detect click on calendar
    connect(ui->calendarSeances, &QCalendarWidget::clicked,
            this, &AutoEcole::onDateChanged);

    // Optional: Show today's seances on startup
    onDateChanged(QDate::currentDate());
    //weather
    QDate today = QDate::currentDate();
    for (int i = 0; i < 7; i++) {
        fetchWeatherForDate(today.addDays(i));
    }
    // Initialiser l'historique
    historiqueActions.clear();
    historiqueActions.prepend(QString("[%1] SYSTEM: Session démarrée")
                                  .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss")));
}

AutoEcole::~AutoEcole()
{
    delete ui;
    delete arduino;
}

// ========== SETUP ARDUINO CONNECTIONS ==========
void AutoEcole::setupArduinoConnections()
{
    if (!arduino) return;

    // Connect Arduino signals to slots
    connect(arduino, &Arduino::dataReceived, this, &AutoEcole::onArduinoDataReceived);
    connect(arduino, &Arduino::keyPressed, this, &AutoEcole::onArduinoKeyPressed);
    connect(arduino, &Arduino::clientIdEntered, this, &AutoEcole::onArduinoClientIdEntered);
    connect(arduino, &Arduino::clientFound, this, &AutoEcole::onArduinoClientFound);
    connect(arduino, &Arduino::clientNotFound, this, &AutoEcole::onArduinoClientNotFound);
    connect(arduino, &Arduino::clientWithoutSessions, this, &AutoEcole::onArduinoClientWithoutSessions);
    connect(arduino, &Arduino::errorOccurred, this, &AutoEcole::onArduinoError);
    connect(arduino, &Arduino::connectionChanged, this, &AutoEcole::onArduinoConnected);
}

// ==================== FONCTIONS HISTORIQUE ====================

void AutoEcole::ajouterHistorique(const QString& action, int matricule, const QString& details) {
    QString historiqueEntry = QString("[%1] %2 - Véhicule %3: %4")
                                  .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss"))
                                  .arg(action)
                                  .arg(matricule)
                                  .arg(details);

    historiqueActions.prepend(historiqueEntry);

    // Limiter à 100 entrées maximum
    if (historiqueActions.size() > 100) {
        historiqueActions.removeLast();
    }
}

void AutoEcole::afficherHistorique() {
    // Créer un widget pour afficher l'historique
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Historique des Actions - Véhicules");
    dialog->setMinimumSize(900, 600);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Liste pour afficher l'historique
    QListWidget *listWidget = new QListWidget();
    listWidget->setStyleSheet("QListWidget { font-family: 'Courier New'; font-size: 10pt; }");

    // Ajouter toutes les actions à la liste
    for (const QString &action : historiqueActions) {
        QListWidgetItem *item = new QListWidgetItem(action);

        // Colorer selon le type d'action
        if (action.contains("AJOUT")) {
            item->setBackground(QColor(200, 255, 200)); // Vert clair
        } else if (action.contains("SUPPRESSION")) {
            item->setBackground(QColor(255, 200, 200)); // Rouge clair
        } else if (action.contains("MODIFICATION")) {
            item->setBackground(QColor(255, 255, 200)); // Jaune clair
        } else if (action.contains("SYSTEM")) {
            item->setBackground(QColor(200, 200, 255)); // Bleu clair
        }

        listWidget->addItem(item);
    }

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnFermer = new QPushButton("Fermer");
    QPushButton *btnVider = new QPushButton("Vider l'historique");

    connect(btnFermer, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(btnVider, &QPushButton::clicked, [this, listWidget]() {
        historiqueActions.clear();
        listWidget->clear();
        historiqueActions.prepend(QString("[%1] SYSTEM: Historique vidé")
                                      .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss")));
        listWidget->addItem(historiqueActions.first());
    });

    buttonLayout->addWidget(btnVider);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnFermer);

    layout->addWidget(new QLabel("Historique des actions sur les véhicules (les plus récentes en premier):"));
    layout->addWidget(listWidget);
    layout->addLayout(buttonLayout);

    dialog->exec();
}

void AutoEcole::on_btnHistorique_clicked()
{
    afficherHistorique();
}
//-------------------------arduino-----------------------------------------
void AutoEcole::update_arduino()
{
    QByteArray data = A.read_from_arduino();
    if (data.isEmpty()) return;

    qDebug() << "Arduino Data: " << data;

    // Logic for handling the incoming distance data
    QString message = QString::fromStdString(data.toStdString());
    if (message.contains("Distance:")) {
        // Parse distance
        QStringList parts = message.split("Distance: ");
        if (parts.size() > 1) {
            QString valStr = parts.last();
            QString distStr = valStr.split(" cm").first().trimmed();
            bool ok;
            int distance = distStr.toInt(&ok);

            if (ok) {
                // Example logic: if distance < 10 cm, alert!
                if (distance > 0 && distance < 10) {
                    // Display warning message
                    QMessageBox::warning(this, "Attention", "Distance trop proche: " + QString::number(distance) + " cm");
                }
            }
        }
    }
}

// ========== SLOTS ARDUINO ==========
void AutoEcole::onArduinoDataReceived(const QString &data)
{
    qDebug() << "Donnée Arduino reçue:" << data;
    statusBar()->showMessage("Arduino: " + data, 3000);
}

void AutoEcole::onArduinoKeyPressed(const QString &key)
{
    qDebug() << "Key pressed:" << key;

    if (arduinoInputDialog && arduinoInputDialog->isVisible()) {
        if (key == "*") {
            // Backspace
            if (!arduinoInputBuffer.isEmpty()) {
                arduinoInputBuffer.chop(1);
                qDebug() << "Backspace. Buffer now:" << arduinoInputBuffer;
            }
            // ========== ADD THIS BLOCK ==========
            // Update LCD display with current input
            QString lcdLine1 = "ID Client:";
            QString lcdLine2 = "";

            // Build display with placeholders
            for (int i = 0; i < 4; i++) {
                if (i < arduinoInputBuffer.length()) {
                    lcdLine2 += arduinoInputBuffer[i];
                } else {
                    lcdLine2 += "_";
                }
                if (i < 3) lcdLine2 += " ";
            }

            updateLCDDisplay(lcdLine1, lcdLine2);
        }
        else if (key.length() == 1 && key[0].isDigit()) {
            // Digit pressed (0-9)
            if (arduinoInputBuffer.length() < 4) {
                arduinoInputBuffer += key;
                qDebug() << "Digit added. Buffer now:" << arduinoInputBuffer;
            } else {
                qDebug() << "Maximum 4 digits reached";
            }
        }

        // Update display
        if (arduinoIdDisplay) {
            QString display = arduinoInputBuffer;
            for (int i = arduinoInputBuffer.length(); i < 4; i++) {
                display += "_";
            }

            // Add spaces between characters
            QString formatted;
            for (int i = 0; i < display.length(); i++) {
                if (i > 0) formatted += " ";
                formatted += display[i];
            }

            arduinoIdDisplay->setText(formatted);

            // Visual feedback
            if (!arduinoInputBuffer.isEmpty()) {
                arduinoIdDisplay->setStyleSheet(
                    "font-size: 24px; font-weight: bold; color: #00FF00; "
                    "background-color: #333; border: 2px solid #00FF00; "
                    "border-radius: 5px; padding: 10px;"
                    );
            } else {
                arduinoIdDisplay->setStyleSheet(
                    "font-size: 24px; font-weight: bold; color: #FFA500; "
                    "background-color: #333; border: 2px solid #666; "
                    "border-radius: 5px; padding: 10px;"
                    );
            }
        }
    } else {
        qDebug() << "Key pressed but dialog not open";
    }
}

void AutoEcole::onArduinoClientIdEntered(const QString &id)
{
    bool ok;
    int clientId = id.toInt(&ok);

    if (ok && clientId > 0) {
        qDebug() << "ID client entré via Arduino:" << clientId;

        // Update LCD
        updateLCDDisplay("Verif ID:", QString::number(clientId));
        sendToLCD("QT_ID_RECEIVED");

        if (arduinoInputDialog) {
            arduinoInputDialog->accept();
        }

        // Check if client exists
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) FROM CLIENT WHERE ID_CLIENT = :id");
        query.bindValue(":id", clientId);

        if (query.exec() && query.next()) {
            if (query.value(0).toInt() > 0) {
                // Client exists - check for sessions
                QSqlQuery sessionQuery;
                sessionQuery.prepare("SELECT COUNT(*) FROM SEANCE WHERE ID_CLIENT = :id");
                sessionQuery.bindValue(":id", clientId);

                // FIXED: Declare sessionCount variable
                int sessionCount = 0;

                if (sessionQuery.exec() && sessionQuery.next()) {
                    sessionCount = sessionQuery.value(0).toInt();
                }

                // Get client name
                QSqlQuery nameQuery;
                nameQuery.prepare("SELECT NOM, PRENOM FROM CLIENT WHERE ID_CLIENT = :id");
                nameQuery.bindValue(":id", clientId);

                QString nomComplet = "Client " + QString::number(clientId);
                if (nameQuery.exec() && nameQuery.next()) {
                    nomComplet = nameQuery.value(0).toString() + " " + nameQuery.value(1).toString();
                }

                if (sessionCount > 0) {
                    QString message = QString("✅ Client %1 (ID: %2) a %3 séance(s)")
                                          .arg(nomComplet).arg(clientId).arg(sessionCount);

                    QMessageBox::information(this, "Client trouvé", message);
                    statusBar()->showMessage(message, 5000);

                    // LCD feedback
                    QString lcdMessage = QString("Client %1 OK").arg(clientId);
                    updateLCDDisplay(lcdMessage, QString("%1 seances").arg(sessionCount));
                    A.write_to_arduino("RESULT:OK\n");
                } else {
                    QString message = QString("ℹ️ Le client %1 (ID: %2) existe mais n'a <b>aucune séance</b> programmée.")
                                          .arg(nomComplet).arg(clientId);

                    QMessageBox::information(this, "Client sans Séances", message);
                    statusBar()->showMessage("Client sans séances: " + nomComplet + " (ID: " + QString::number(clientId) + ")", 5000);

                    // LCD feedback
                    updateLCDDisplay("Client trouve", "Pas de seances");
                    A.write_to_arduino("RESULT:NO_SESSIONS\n");
                }
            } else {
                // Client doesn't exist
                QMessageBox::warning(this, "Client non trouvé",
                                     QString("❌ Aucun client avec l'ID %1").arg(clientId));
                statusBar()->showMessage("Client non trouvé: ID " + QString::number(clientId), 3000);

                // LCD feedback
                updateLCDDisplay("Client non", "trouve");
                A.write_to_arduino("RESULT:NOT_FOUND\n");
            }
        }
    }
}

void AutoEcole::onArduinoClientFound(int id, const QString &nom, int sessionsCount)
{
    QString message = QString("✅ Client %1 (ID: %2) a %3 séance(s)")
                          .arg(nom).arg(id).arg(sessionsCount);

    QMessageBox::information(this, "Client trouvé", message);
    statusBar()->showMessage(message, 5000);
}

void AutoEcole::onArduinoClientNotFound(int id)
{
    QMessageBox::warning(this, "Client non trouvé",
                         QString("❌ Aucun client avec l'ID %1").arg(id));
    statusBar()->showMessage("Client non trouvé: ID " + QString::number(id), 3000);
}

void AutoEcole::onArduinoClientWithoutSessions(int id, const QString &nom)
{
    QString message = QString("ℹ️ Le client %1 (ID: %2) existe mais n'a <b>aucune séance</b> programmée.")
                          .arg(nom).arg(id);

    QMessageBox::information(this, "Client sans Séances", message);
    statusBar()->showMessage("Client sans séances: " + nom + " (ID: " + QString::number(id) + ")", 5000);
}

void AutoEcole::onArduinoError(const QString &error)
{
    qDebug() << "Erreur Arduino:" << error;
    statusBar()->showMessage("Erreur Arduino: " + error, 5000);
}

void AutoEcole::onArduinoConnected(bool connected)
{
    if (connected) {
        qDebug() << "Arduino connecté";
        statusBar()->showMessage("Arduino connecté", 3000);
    } else {
        qDebug() << "Arduino déconnecté";
        statusBar()->showMessage("Arduino déconnecté", 3000);
    }
}

// ========== FONCTION PRINCIPALE CLIENTUNO ==========
// ========== FONCTION PRINCIPALE CLIENTUNO ==========
void AutoEcole::on_btnClientUno_clicked()
{
    qDebug() << "=== DEBUG: on_btnClientUno_clicked() STARTED ===";
    // ADD THESE 3 LINES:
    clearLCD();
    updateLCDDisplay("Systeme Pret", "Entrez ID");
    sendToLCD("QT_READY");
    qDebug() << "This function is being called!";

    // Simple test - show a message box immediately
    QMessageBox::information(this, "Test", "Button clicked! Function is working!");

    qDebug() << "Checking Arduino connection...";

    // Check if Arduino is connected
    if (!A.getSerialPort() || !A.getSerialPort()->isOpen()) {
        qDebug() << "Arduino NOT connected or serial port not available";

        QMessageBox::warning(this, "⚠️ Arduino Non Connecté",
                             "Le clavier Arduino n'est pas connecté.\n"
                             "Veuillez:\n"
                             "1. Vérifier le branchement USB de l'Arduino\n"
                             "2. Cliquer sur 'Connecter Arduino' dans le menu\n"
                             "3. Redémarrer l'application si nécessaire.");

        qDebug() << "=== DEBUG: on_btnClientUno_clicked() ENDED (Arduino not connected) ===";
        return;
    }

    qDebug() << "Arduino is connected to port:" << A.getarduino_port_name();

    // Show the first message box
    QMessageBox msgBox;
    msgBox.setWindowTitle("🔍 Vérification Client-Séance");
    msgBox.setText("Prêt à utiliser le clavier Arduino !\n\n"
                   "Instructions:\n"
                   "1. Utilisez le clavier 4x4 pour entrer l'ID client (2-4 chiffres)\n"
                   "2. Appuyez sur '#' pour valider\n"
                   "3. Appuyez sur '*' pour effacer un chiffre\n\n"
                   "Appuyez sur OK quand vous êtes prêt.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);

    qDebug() << "Showing first message box...";

    int result = msgBox.exec();
    if (result == QMessageBox::Cancel) {
        qDebug() << "User cancelled";
        // ADD THIS LINE:
        updateLCDDisplay("Operation", "Annulee");
        return;
    }
    qDebug() << "Message box result:" << result;

    if (result == QMessageBox::Cancel) {
        qDebug() << "User cancelled";
        qDebug() << "=== DEBUG: on_btnClientUno_clicked() ENDED (User cancelled) ===";
        return;
    }

    qDebug() << "Sending READY signal to Arduino...";

    // Send signal to Arduino
    if (A.getSerialPort()->isWritable()) {
        A.write_to_arduino("QT_READY_FOR_INPUT\n");
        qDebug() << "Sent: QT_READY_FOR_INPUT";
    } else {
        qDebug() << "ERROR: Cannot write to Arduino!";
    }

    qDebug() << "Showing Arduino input dialog...";
    showArduinoInputDialog();

    qDebug() << "=== DEBUG: on_btnClientUno_clicked() ENDED SUCCESSFULLY ===";
}

void AutoEcole::showArduinoInputDialog()
{
    arduinoInputDialog = new QDialog(this);
    arduinoInputDialog->setWindowTitle("Entrée Clavier Arduino");
    arduinoInputDialog->setModal(true);
    arduinoInputDialog->setFixedSize(400, 250);
    clearLCD();

    QVBoxLayout *layout = new QVBoxLayout(arduinoInputDialog);

    QLabel *titleLabel = new QLabel("⚡ Attente de l'ID client", arduinoInputDialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: white;"
        );

    arduinoIdDisplay = new QLabel("_ _ _ _", arduinoInputDialog);
    arduinoIdDisplay->setAlignment(Qt::AlignCenter);
    arduinoIdDisplay->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #FFA500; "
        "background-color: #333; border: 2px solid #666; "
        "border-radius: 5px; padding: 10px;"
        );
    arduinoIdDisplay->setMinimumHeight(60);

    QLabel *instructionLabel = new QLabel(
        "Utilisez le clavier Arduino:\n"
        "• Entrez 1-4 chiffres (0-9)\n"
        "• '#' = Valider\n"
        "• '*' = Effacer\n",
        arduinoInputDialog
        );
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("color: white;");
    instructionLabel->setWordWrap(true);

    QPushButton *cancelButton = new QPushButton("Annuler", arduinoInputDialog);
    cancelButton->setStyleSheet("padding: 8px; font-size: 14px;");

    layout->addWidget(titleLabel);
    layout->addWidget(arduinoIdDisplay);
    layout->addWidget(instructionLabel);
    layout->addWidget(cancelButton);

    arduinoInputBuffer = "";

    connect(cancelButton, &QPushButton::clicked, arduinoInputDialog, &QDialog::reject);

    connect(arduinoInputDialog, &QDialog::rejected, this, [this]() {
        // Use A instead of arduino
        A.write_to_arduino("QT_INPUT_CANCELLED\n");
        statusBar()->showMessage("Entrée Arduino annulée", 3000);
        qDebug() << "Arduino input cancelled by user";
    });

    arduinoInputDialog->setStyleSheet("background-color: #2b2b2b;");

    QTimer::singleShot(30000, arduinoInputDialog, [this]() {
        if (arduinoInputDialog && arduinoInputDialog->isVisible()) {
            arduinoInputDialog->reject();
            QMessageBox::warning(this, "❌ Délai Dépassé",
                                 "Aucune entrée reçue de l'Arduino dans le délai imparti (30 secondes).");
            qDebug() << "Arduino input timeout after 30 seconds";
        }
    });

    qDebug() << "Opening Arduino input dialog...";
    arduinoInputDialog->exec();

    if (arduinoInputDialog) {
        delete arduinoInputDialog;
        arduinoInputDialog = nullptr;
        arduinoIdDisplay = nullptr;
    }
}

// ==========================================================
// SLOT : CONNEXION MANUELLE ARDUINO
// ==========================================================
void AutoEcole::on_bouton_connect_arduino_clicked()
{
    if (A.getSerialPort()->isOpen()) {
        // Déconnexion
        A.close_arduino();
        ui->label_statut_arduino->setText("Déconnecté");
        ui->label_statut_arduino->setStyleSheet("color: red;");
    } else {
        // Connexion
        int ret = A.connect_arduino();
        if (ret == 1) {
            ui->label_statut_arduino->setText("Connecté : " + A.getPortName());
            ui->label_statut_arduino->setStyleSheet("color: green;");
        } else {
            QMessageBox::warning(this, "Erreur Connexion", "Impossible de se connecter à l'Arduino. Vérifiez le branchement.");
            ui->label_statut_arduino->setText("Déconnecté/Erreur");
            ui->label_statut_arduino->setStyleSheet("color: red;");
        }
    }
}
// Dans autoecole.cpp

// ==========================================================
// SLOT : LECTURE DES DONNÉES ARDUINO (Correction fragmentation)
// ==========================================================
void AutoEcole::lireDonneesArduino()
{
    // 1. Lire toutes les données disponibles et les ajouter au buffer
    QByteArray newData = A.getSerialPort()->readAll();
    serialBuffer.append(newData);

    // Ligne de débogage pour voir tout ce qui est dans le buffer (à retirer après)
    qDebug() << "Buffer actuel (fragmenté):" << serialBuffer;

    // 2. Traiter les données ligne par ligne (l'Arduino envoie un '\n' à la fin)
    while (serialBuffer.contains('\n'))
    {
        // Trouver la fin de la première ligne complète
        int lineEndIndex = serialBuffer.indexOf('\n');

        // Extraire la ligne complète (Ex: "UID:233b1802\r")
        QByteArray lineData = serialBuffer.left(lineEndIndex).trimmed();

        // Retirer la ligne traitée du buffer (+1 pour le '\n')
        serialBuffer.remove(0, lineEndIndex + 1);

        // Convertir la ligne complète en QString et la netteler
        QString dataString = QString::fromUtf8(lineData).trimmed();

        qDebug() << "Ligne complète extraite et traitée:" << dataString; // Cette ligne est maintenant complète !

        // ========== RFID FUNCTIONALITY (EXISTING) ==========
        // 3. Vérification du format (Ex: "UID:233B1802")
        if (dataString.startsWith("UID:"))
        {
            QString uid = dataString.section(':', 1).trimmed().toUpper();
            qDebug() << "UID RFID extrait pour recherche DB:" << uid;

            // --- 4. Logique de vérification SQL ---
            QSqlQuery query;
            query.prepare("SELECT ID_EMPLOYE, NOM, PRENOM FROM EMPLOYE WHERE UID_RFID = :uid");
            query.bindValue(":uid", uid);

            if (query.exec())
            {
                if (query.next())
                {
                    // Succès
                    QString prenom = query.value("PRENOM").toString();
                    QString nom = query.value("NOM").toString();

                    QMessageBox::information(this, "Accès Réussi",
                                             "Bonjour " + prenom + " " + nom + "! Accès autorisé.");
                    A.write_to_arduino("OPEN");
                }
                else
                {
                    // UID Non trouvé
                    QMessageBox::critical(this, "Accès Refusé", "Badge inconnu ou non associé à un compte Employé.");
                    A.write_to_arduino("DENY");
                }
            }
            else
            {
                // Erreur SQL critique
                QMessageBox::critical(this, "Erreur DB",
                                      "Erreur lors de la vérification de l'UID: " + query.lastError().text());
                A.write_to_arduino("DENY");
            }

            // On quitte après avoir traité le badge pour ne pas traiter d'autres fragments du buffer
            return;
        }

        // ========== NEW: KEYPAD FUNCTIONALITY (ADDED) ==========
        // Check for single digit (0-9)
        if (dataString.length() == 1 && dataString[0].isDigit()) {
            qDebug() << "Keypad digit:" << dataString;
            onArduinoKeyPressed(dataString);
        }
        // Check for special keys
        else if (dataString == "*") {
            qDebug() << "Keypad: BACKSPACE (*)";
            onArduinoKeyPressed("*");
        }
        else if (dataString == "#") {
            qDebug() << "Keypad: ENTER/SUBMIT (#)";
            if (!arduinoInputBuffer.isEmpty()) {
                qDebug() << "Submitting ID:" << arduinoInputBuffer;
                onArduinoClientIdEntered(arduinoInputBuffer);
            }
        }
        // Check for formatted key messages (alternative format)
        else if (dataString.startsWith("KEY:")) {
            QString key = dataString.mid(4);
            qDebug() << "Formatted key:" << key;
            onArduinoKeyPressed(key);
        }
        else if (dataString.startsWith("ENTER:")) {
            QString id = dataString.mid(6);
            qDebug() << "Formatted enter:" << id;
            onArduinoClientIdEntered(id);
        }
        // ========== END NEW FUNCTIONALITY ==========
    }
}

// ========== LCD CONTROL FUNCTIONS ==========

void AutoEcole::sendToLCD(const QString &command)
{
    if (A.getSerialPort()->isOpen()) {
        QString fullCommand = command + "\n";
        A.write_to_arduino(fullCommand.toUtf8());
        qDebug() << "Sent to LCD:" << command;
    }
}

void AutoEcole::clearLCD()
{
    sendToLCD("LCD_CLEAR");
}

void AutoEcole::updateLCDDisplay(const QString &line1, const QString &line2)
{
    if (line2.isEmpty()) {
        sendToLCD(QString("LCD_DISP:%1:").arg(line1));
    } else {
        sendToLCD(QString("LCD_DISP:%1:%2").arg(line1, line2));
    }
}

void AutoEcole::showLCDMessage(const QString &message)
{
    if (message.length() <= 16) {
        // Single line message
        updateLCDDisplay(message);
    } else {
        // Split long message into two lines
        QString line1 = message.left(16);
        QString line2 = message.mid(16, 16);
        updateLCDDisplay(line1, line2);
    }
}

void AutoEcole::scrollLCD(const QString &text)
{
    sendToLCD(QString("LCD_SCROLL:%1").arg(text));
}





//==================================================================CRUD-EMPLOYE===================================================================

// ---------------- AJOUTER ----------------
void AutoEcole::on_ajoutemploye_clicked()
{
    QString idText = ui->lineEdit_idemp->text().trimmed();
    QString nom = ui->lineEdit_nom->text().trimmed();
    QString prenom = ui->lineEdit_prenom->text().trimmed();
    QString specialite = ui->comboBox_specialite->currentText().trimmed();
    QString password = ui->lineEdit_motdepasse->text().trimmed();
    QString telephone = ui->lineEdit_telephone->text().trimmed();
    QString question = ui->lineEdit_question->text().trimmed();
    QString reponse = ui->lineEdit_reponse->text().trimmed();
    QString poste = ui->comboBox_poste->currentText().trimmed();


    //Vérifier si un champ est vide
    if (idText.isEmpty() || nom.isEmpty() || prenom.isEmpty() ||
        telephone.isEmpty() || password.isEmpty() ||question.isEmpty()||poste.isEmpty()  || reponse.isEmpty()||
        specialite.isEmpty() || specialite == "— Sélectionner —") {
        QMessageBox::warning(this, "Champs vides",
                             "Veuillez remplir tous les champs avant d’ajouter un employé !");
        return;
    }


    int id = idText.toInt();
    employe emp(id, nom, prenom, telephone, password, specialite, question, reponse,poste);
    bool test = emp.ajouter();

    if (test) {
        ui->tableemploye->setModel(emp.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Ajout effectué.\nClick Cancel to exit."), QMessageBox::Cancel);
        ui->lineEdit_idemp->clear();
        ui->lineEdit_nom->clear();
        ui->lineEdit_prenom->clear();
        ui->lineEdit_telephone->clear();
        ui->lineEdit_motdepasse->clear();
        ui->lineEdit_question->clear();
        ui->lineEdit_reponse->clear();
        ui->comboBox_specialite->setCurrentIndex(0);
        ui->comboBox_poste->setCurrentIndex(0);

    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Ajout non effectué.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
}

// ---------------- SUPPRIMER ----------------
void AutoEcole::on_suppemploye_clicked()
{
    QString idText = ui->lineEdit_idemp->text().trimmed();

    //Vérifier si l'ID est vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID de l'employé à supprimer !");
        return;
    }

    int id = idText.toInt();
    bool test = emp.supprimer(id);

    if (test) {
        ui->tableemploye->setModel(emp.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Suppression effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Suppression non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }

    ui->lineEdit_idemp->clear();
    ui->lineEdit_nom->clear();
    ui->lineEdit_prenom->clear();
    ui->lineEdit_telephone->clear();
    ui->lineEdit_motdepasse->clear();
    ui->lineEdit_question->clear();
    ui->lineEdit_reponse->clear();
    ui->comboBox_specialite->setCurrentIndex(0);
    ui->comboBox_poste->setCurrentIndex(0);


}

// ---------------- MODIFIER ----------------
void AutoEcole::on_updateemploye_clicked()
{
    QString idText = ui->lineEdit_idemp->text().trimmed();
    QString nom = ui->lineEdit_nom->text().trimmed();
    QString prenom = ui->lineEdit_prenom->text().trimmed();
    QString password = ui->lineEdit_motdepasse->text().trimmed();
    QString question = ui->lineEdit_question->text().trimmed();
    QString reponse = ui->lineEdit_reponse->text().trimmed();
    QString telephone = ui->lineEdit_telephone->text().trimmed();
    QString specialite;
    if (ui->comboBox_specialite->currentIndex() <= 0)
        specialite = "";
    else
        specialite = ui->comboBox_specialite->currentText().trimmed();

    // Get the poste from the new ComboBox
    QString poste;
    if (ui->comboBox_poste->currentIndex() <= 0)
        poste = "";
    else
        poste = ui->comboBox_poste->currentText().trimmed();

    //Vérifier si ID vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID de l'employé à modifier !");
        return;
    }

    int id = idText.toInt();

    //Vérifier si l'ID existe dans la base
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM employe WHERE id_employe = :id");
    check.bindValue(":id", id);
    if (!check.exec() || !check.next() || check.value(0).toInt() == 0) {
        QMessageBox::warning(this, "ID invalide",
                             "Aucun employé trouvé avec cet ID !");
        return;
    }

    //Vérifier si aucun autre champ n’est rempli
    if (nom.isEmpty() && prenom.isEmpty() && password.isEmpty() &&
        telephone.isEmpty() && question.isEmpty()&& reponse.isEmpty() && specialite.isEmpty()&& poste.isEmpty()) {
        QMessageBox::warning(this, "Aucune modification",
                             "Veuillez remplir au moins un champ à modifier (autre que l'ID) !");
        return;
    }

    employe emp(id, nom, prenom, telephone, password, specialite, question, reponse, poste);
    bool test = emp.modifier(id);

    if (test) {
        ui->tableemploye->setModel(emp.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Modification effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
        ui->lineEdit_idemp->clear();
        ui->lineEdit_nom->clear();
        ui->lineEdit_prenom->clear();
        ui->lineEdit_telephone->clear();
        ui->lineEdit_motdepasse->clear();
        ui->lineEdit_question->clear();
        ui->lineEdit_reponse->clear();
        ui->comboBox_specialite->setCurrentIndex(0);
        ui->comboBox_poste->setCurrentIndex(0); // Reset the poste ComboBox
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Modification non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
}

// When a row in the table is clicked, populate the form fields with the data of the selected row
void AutoEcole::on_tableemploye_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    const QAbstractItemModel *model = ui->tableemploye->model();
    int row = index.row();

    // Set the data for Name, Surname, etc., in the corresponding lineEdits
    ui->lineEdit_idemp->setText(model->data(model->index(row, 0)).toString());
    ui->lineEdit_nom->setText(model->data(model->index(row, 1)).toString());
    ui->lineEdit_prenom->setText(model->data(model->index(row, 2)).toString());
    ui->comboBox_specialite->setCurrentText(model->data(model->index(row, 3)).toString());
    ui->lineEdit_telephone->setText(model->data(model->index(row, 4)).toString());
    ui->comboBox_poste->setCurrentText(model->data(model->index(row, 5)).toString());


    // Do not set the password, question, and response in the lineEdits (remove these lines)
    // ui->lineEdit_motdepasse->setText(model->data(model->index(row, 9)).toString());
    // ui->lineEdit_question->setText(model->data(model->index(row, 5)).toString());
    // ui->lineEdit_reponse->setText(model->data(model->index(row, 6)).toString());
}


// ------------------------------------------------------------ LOGIN -------------------------------------------------------------------------
void AutoEcole::on_pushButton_login_clicked()
{
    int id = ui->lineEdit_idlogin->text().toInt();  // Get ID from the login page
    QString password = ui->lineEdit_passwordlogin->text();  // Get password

    // Validate input fields
    if (id == 0 || password.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un identifiant et un mot de passe.");
        return; // Exit the function if either is empty
    }

    // Attempt to verify the login credentials
    QString nomComplet = emp.verifierLoginNom(id, password);

    if (!nomComplet.isEmpty()) {
        // Store the ID for later use
        currentEmployeeId = id;

        // Get the poste (role) of the employee
        QString poste = emp.getPosteById(id);

        // Enable/disable the navigation buttons based on the poste
        updateNavigationButtons(poste);  // Call the function to update the buttons

        // Navigate to the main page
        ui->label_72->setText("Bonjour " + nomComplet + ", voici un aperçu de la gestion de l'auto-école.");
        ui->stackedWidget->setCurrentIndex(2); // → accueil principal

        // Clear the login fields
        ui->lineEdit_idlogin->clear();
        ui->lineEdit_passwordlogin->clear();
    } else {
        QMessageBox::warning(this, "Erreur", "Identifiant ou mot de passe incorrect !");
    }
}
void AutoEcole::updateNavigationButtons(const QString &poste)
{
    // Based on the poste, enable/disable the buttons accordingly
    if (poste == "Gestion Employé") {
        ui->pushButton->setEnabled(true);
        ui->pushButton1->setEnabled(true);
        ui->pushButton127->setEnabled(true);
        ui->pushButton184->setEnabled(true);
        ui->pushButton182->setEnabled(true);
        ui->pushButton12->setEnabled(true);
        ui->pushButton43->setEnabled(true);
        ui->pushButton42->setEnabled(true);
        ui->pushButton177->setEnabled(true);

        ui->pushButton126->setEnabled(false);
        ui->pushButton128->setEnabled(false);
        ui->pushButton129->setEnabled(false);
        ui->pushButton130->setEnabled(false);
        ui->pushButton2->setEnabled(false);
        ui->pushButton3->setEnabled(false);
        ui->pushButton4->setEnabled(false);
        ui->pushButton5->setEnabled(false);
        ui->pushButton14->setEnabled(false);
        ui->pushButton15->setEnabled(false);
        ui->pushButton16->setEnabled(false);
        ui->pushButton17->setEnabled(false);
        ui->pushButton47->setEnabled(false);
        ui->pushButton46->setEnabled(false);
        ui->pushButton45->setEnabled(false);
        ui->pushButton44->setEnabled(false);


    }
    else if (poste == "Gestion Client") {
        ui->pushButton1->setEnabled(false);
        ui->pushButton3->setEnabled(false);
        ui->pushButton4->setEnabled(false);
        ui->pushButton5->setEnabled(false);
        ui->pushButton127->setEnabled(false);
        ui->pushButton130->setEnabled(false);
        ui->pushButton128->setEnabled(false);
        ui->pushButton129->setEnabled(false);
        ui->pushButton7->setEnabled(false);
        ui->pushButton9->setEnabled(false);
        ui->pushButton10->setEnabled(false);
        ui->pushButton11->setEnabled(false);
        ui->pushButton41->setEnabled(false);
        ui->pushButton40->setEnabled(false);
        ui->pushButton39->setEnabled(false);
        ui->pushButton37->setEnabled(false);

        ui->pushButton->setEnabled(true);
        ui->pushButton184->setEnabled(true);
        ui->pushButton2->setEnabled(true);
        ui->pushButton126->setEnabled(true);
        ui->pushButton6->setEnabled(true);
        ui->pushButton183->setEnabled(true);
        ui->pushButton178->setEnabled(true);
        ui->pushButton38->setEnabled(true);
        ui->pushButton36->setEnabled(true);

    }
    else if (poste == "Gestion Véhicule") {
        ui->pushButton127->setEnabled(false);
        ui->pushButton126->setEnabled(false);
        ui->pushButton129->setEnabled(false);
        ui->pushButton130->setEnabled(false);
        ui->pushButton121->setEnabled(false);
        ui->pushButton122->setEnabled(false);
        ui->pushButton124->setEnabled(false);
        ui->pushButton125->setEnabled(false);
        ui->pushButton55->setEnabled(false);
        ui->pushButton56->setEnabled(false);
        ui->pushButton58->setEnabled(false);
        ui->pushButton59->setEnabled(false);
        ui->pushButton1->setEnabled(false);
        ui->pushButton2->setEnabled(false);
        ui->pushButton4->setEnabled(false);
        ui->pushButton5->setEnabled(false);

        ui->pushButton->setEnabled(true);
        ui->pushButton184->setEnabled(true);
        ui->pushButton3->setEnabled(true);
        ui->pushButton128->setEnabled(true);
        ui->pushButton120->setEnabled(true);
        ui->pushButton164->setEnabled(true);
        ui->pushButton57->setEnabled(true);
        ui->pushButton54->setEnabled(true);
        ui->pushButton175->setEnabled(true);

    }
    else if (poste == "Gestion Séance") {
        ui->pushButton->setEnabled(true);
        ui->pushButton4->setEnabled(true);
        ui->pushButton129->setEnabled(true);
        ui->pushButton184->setEnabled(true);
        ui->pushButton30->setEnabled(true);
        ui->pushButton179->setEnabled(true);
        ui->pushButton180->setEnabled(true);
        ui->pushButton28->setEnabled(true);
        ui->pushButton24->setEnabled(true);
        ui->pushButton184_2->setEnabled(true);
        ui->pushButton_11->setEnabled(true);
        ui->pushButton4_5->setEnabled(true);


        ui->pushButton126->setEnabled(false);
        ui->pushButton127->setEnabled(false);
        ui->pushButton130->setEnabled(false);
        ui->pushButton128->setEnabled(false);
        ui->pushButton5->setEnabled(false);
        ui->pushButton3->setEnabled(false);
        ui->pushButton2->setEnabled(false);
        ui->pushButton1->setEnabled(false);
        ui->pushButton31->setEnabled(false);
        ui->pushButton32->setEnabled(false);
        ui->pushButton33->setEnabled(false);
        ui->pushButton35->setEnabled(false);
        ui->pushButton29->setEnabled(false);
        ui->pushButton27->setEnabled(false);
        ui->pushButton26->setEnabled(false);
        ui->pushButton25->setEnabled(false);
        ui->pushButton1_5->setEnabled(false);
        ui->pushButton2_5->setEnabled(false);
        ui->pushButton3_5->setEnabled(false);
        ui->pushButton5_5->setEnabled(false);

    }
    else if (poste == "Gestion Examen") {
        ui->pushButton1->setEnabled(false);
        ui->pushButton2->setEnabled(false);
        ui->pushButton3->setEnabled(false);
        ui->pushButton4->setEnabled(false);
        ui->pushButton126->setEnabled(false);
        ui->pushButton127->setEnabled(false);
        ui->pushButton128->setEnabled(false);
        ui->pushButton129->setEnabled(false);
        ui->pushButton21->setEnabled(false);
        ui->pushButton20->setEnabled(false);
        ui->pushButton19->setEnabled(false);
        ui->pushButton22->setEnabled(false);
        ui->pushButton51->setEnabled(false);
        ui->pushButton50->setEnabled(false);
        ui->pushButton49->setEnabled(false);
        ui->pushButton52->setEnabled(false);

        ui->pushButton->setEnabled(true);
        ui->pushButton130->setEnabled(true);
        ui->pushButton5->setEnabled(true);
        ui->pushButton184->setEnabled(true);
        ui->pushButton18->setEnabled(true);
        ui->pushButton181->setEnabled(true);
        ui->pushButton53->setEnabled(true);
        ui->pushButton48->setEnabled(true);
        ui->pushButton176->setEnabled(true);

    }
    else {
        // If the poste doesn't match any known category, disable all buttons
        ui->pushButton->setEnabled(false);
        ui->pushButton1->setEnabled(false);
        ui->pushButton2->setEnabled(false);
        ui->pushButton3->setEnabled(false);
        ui->pushButton4->setEnabled(false);
        ui->pushButton5->setEnabled(false);
        ui->pushButton6->setEnabled(false);
        ui->pushButton7->setEnabled(false);

        ui->pushButton9->setEnabled(false);
        ui->pushButton10->setEnabled(false);
        ui->pushButton11->setEnabled(false);
        ui->pushButton12->setEnabled(false);

        ui->pushButton14->setEnabled(false);
        ui->pushButton15->setEnabled(false);
        ui->pushButton16->setEnabled(false);
        ui->pushButton17->setEnabled(false);
        ui->pushButton18->setEnabled(false);
        ui->pushButton19->setEnabled(false);
        ui->pushButton20->setEnabled(false);
        ui->pushButton21->setEnabled(false);
        ui->pushButton22->setEnabled(false);

        ui->pushButton24->setEnabled(false);
        ui->pushButton25->setEnabled(false);
        ui->pushButton26->setEnabled(false);
        ui->pushButton27->setEnabled(false);
        ui->pushButton28->setEnabled(false);
        ui->pushButton29->setEnabled(false);
        ui->pushButton30->setEnabled(false);
        ui->pushButton31->setEnabled(false);
        ui->pushButton32->setEnabled(false);
        ui->pushButton33->setEnabled(false);

        ui->pushButton35->setEnabled(false);
        ui->pushButton36->setEnabled(false);
        ui->pushButton37->setEnabled(false);
        ui->pushButton38->setEnabled(false);
        ui->pushButton39->setEnabled(false);
        ui->pushButton40->setEnabled(false);
        ui->pushButton41->setEnabled(false);
        ui->pushButton42->setEnabled(false);
        ui->pushButton43->setEnabled(false);
        ui->pushButton44->setEnabled(false);
        ui->pushButton45->setEnabled(false);
        ui->pushButton46->setEnabled(false);
        ui->pushButton47->setEnabled(false);
        ui->pushButton48->setEnabled(false);
        ui->pushButton49->setEnabled(false);
        ui->pushButton50->setEnabled(false);
        ui->pushButton51->setEnabled(false);
        ui->pushButton52->setEnabled(false);
        ui->pushButton53->setEnabled(false);
        ui->pushButton54->setEnabled(false);
        ui->pushButton55->setEnabled(false);
        ui->pushButton56->setEnabled(false);
        ui->pushButton57->setEnabled(false);
        ui->pushButton58->setEnabled(false);
        ui->pushButton59->setEnabled(false);

        ui->pushButton120->setEnabled(false);
        ui->pushButton121->setEnabled(false);
        ui->pushButton122->setEnabled(false);

        ui->pushButton124->setEnabled(false);
        ui->pushButton125->setEnabled(false);
        ui->pushButton126->setEnabled(false);
        ui->pushButton127->setEnabled(false);
        ui->pushButton128->setEnabled(false);
        ui->pushButton129->setEnabled(false);
        ui->pushButton130->setEnabled(false);

        ui->pushButton164->setEnabled(false);

        ui->pushButton175->setEnabled(false);
        ui->pushButton176->setEnabled(false);
        ui->pushButton177->setEnabled(false);
        ui->pushButton178->setEnabled(false);
        ui->pushButton179->setEnabled(false);
        ui->pushButton180->setEnabled(false);
        ui->pushButton181->setEnabled(false);
        ui->pushButton182->setEnabled(false);
        ui->pushButton183->setEnabled(false);
        ui->pushButton184->setEnabled(false);
        ui->pushButton1_5->setEnabled(false);
        ui->pushButton2_5->setEnabled(false);
        ui->pushButton3_5->setEnabled(false);
        ui->pushButton5_5->setEnabled(false);
        ui->pushButton184_2->setEnabled(false);
        ui->pushButton_11->setEnabled(false);
        ui->pushButton4_5->setEnabled(false);

    }
}
// --------------------------- FORGOT PASSWORD ------------------------------
void AutoEcole::on_pushButton_forget_password_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);  // Go to the ID entry page
}

// Declare a variable to track the number of failed attempts
int failedAttemptsQuestion = 0;
int failedAttemptsAnswer = 0;

// --------------------------- CONFIRM ID ---------------------------------------
void AutoEcole::on_pushButton_confirmid_clicked()
{
    int id = ui->lineEdit_id->text().toInt();  // Get ID from the user input

    // Check if the ID exists
    if (emp.doesEmployeeExist(id)) {
        currentEmployeeId = id;

        // Clear ComboBox and populate it with security questions
        ui->comboBox_question->clear();
        ui->comboBox_question->addItem("---- Sélectionner ----");  // Default option

        // Fetch the security question for the entered ID
        QString question = emp.getSecurityQuestion(id);
        ui->comboBox_question->addItem(question);  // Add the correct security question to ComboBox

        // Add other random questions to the ComboBox
        QStringList randomQuestions = {
            "Quel est votre animal préféré?",
            "Quel est le nom de votre école?",
            "Quel est votre plat préféré?",
            "Quel est votre fruit préféré?",
            "Quel est le prénom de votre meilleur ami?",
            "Où êtes-vous né?",
            "Quel est le nom de votre premier professeur?"
        };

        // Shuffle the random questions
        std::shuffle(randomQuestions.begin(), randomQuestions.end(), std::default_random_engine(QRandomGenerator::global()->bounded(0, 100)));

        // Add shuffled random questions to ComboBox
        ui->comboBox_question->addItems(randomQuestions);

        // Go to the next page where the user can select the question
        ui->stackedWidget->setCurrentIndex(23);  // Question selection page
    } else {
        QMessageBox::warning(this, "Erreur", "Identifiant non trouvé.");
    }
}

// --------------------------- CONFIRM QUESTION ---------------------------------------
void AutoEcole::on_pushButton_confirmquestion_clicked()
{
    QString selectedQuestion = ui->comboBox_question->currentText();
    int id = currentEmployeeId;  // Use the stored ID

    if (selectedQuestion == "---- Sélectionner ----") {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une question.");
        return;
    }

    // Verify if the selected question is correct
    if (emp.verifyQuestion(id, selectedQuestion)) {
        // Reset the failed attempt counter for question verification
        failedAttemptsQuestion = 0;

        // Go to the next page to answer the question
        ui->stackedWidget->setCurrentIndex(24);  // Answer page
    } else {
        // Increment the failed attempts counter
        failedAttemptsQuestion++;
        qDebug() << "Incorrect question. Attempt " << failedAttemptsQuestion << " of 3.";

        if (failedAttemptsQuestion >= 3) {
            // After 3 failed attempts, go back to the login page
            QMessageBox::warning(this, "Erreur", "Trop de tentatives échouées. Retour à la page de connexion.");
            ui->stackedWidget->setCurrentIndex(0);  // Go to the login page
            failedAttemptsQuestion = 0;  // Reset the counter
        } else {
            QMessageBox::warning(this, "Erreur", "Question incorrecte.");
        }
    }
}

// --------------------------- CONFIRM ANSWER ---------------------------------------
void AutoEcole::on_pushButton_confirmanswer_clicked()
{
    QString answer = ui->lineEdit_answer->text().trimmed();  // Get the answer from lineEdit
    int id = currentEmployeeId;  // Use the stored ID
    QString selectedQuestion = ui->comboBox_question->currentText();  // Get selected question

    // Check if the answer is correct
    if (emp.verifyAnswer(id, selectedQuestion, answer)) {
        // Reset the failed attempt counter for answer verification
        failedAttemptsAnswer = 0;

        // Go to the next page for password change
        ui->stackedWidget->setCurrentIndex(25);  // Password change page
    } else {
        // Increment the failed attempts counter
        failedAttemptsAnswer++;
        qDebug() << "Incorrect answer. Attempt " << failedAttemptsAnswer << " of 3.";

        if (failedAttemptsAnswer >= 3) {
            // After 3 failed attempts, go back to the login page
            QMessageBox::warning(this, "Erreur", "Trop de tentatives échouées. Retour à la page de connexion.");
            ui->stackedWidget->setCurrentIndex(0);  // Go to the login page
            failedAttemptsAnswer = 0;  // Reset the counter
        } else {
            QMessageBox::warning(this, "Erreur", "Réponse incorrecte.");
        }
    }
}

// --------------------------- CHANGE PASSWORD --------------------------------------
void AutoEcole::on_pushButton_changepassword_clicked()
{
    QString newPassword = ui->lineEdit_newPassword->text().trimmed();  // Get the new password
    int id = currentEmployeeId;  // Get the ID again

    if (!newPassword.isEmpty()) {
        // Update the password in the database
        if (emp.updatePassword(id, newPassword)) {
            QMessageBox::information(this, "Succès", "Mot de passe mis à jour.");
            ui->stackedWidget->setCurrentIndex(0);  // Go back to login page
        } else {
            QMessageBox::warning(this, "Erreur", "Échec de la mise à jour du mot de passe.");
        }
    } else {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un nouveau mot de passe.");
    }
}

//-------------------------------------------------------------------rechercher-employe--------------------------------------------------------------

void AutoEcole::on_lineEdit_rechercheEmploye_textChanged(const QString &text)
{
    employe emp;
    ui->tableemploye->setModel(emp.rechercher(text));
}

//--------------------------------------------------------------------tri-employe---------------------------------------------------------------------
void AutoEcole::on_comboBox_triEmploye_currentIndexChanged(const QString &text)
{
    if (text.trimmed().isEmpty() || text == "— Sélectionner —") {
        ui->tableemploye->setModel(emp.afficher());
        return;
    }

    ui->tableemploye->setModel(emp.trier(text));
}

//------------------------------------export-pdf-------------------------
void AutoEcole::on_btn_exportEmployePDF_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter en PDF",
        QDir::homePath() + "/employes_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
        "Fichiers PDF (*.pdf)"
        );

    if (filePath.isEmpty())
        return;

    employe e;
    if (e.exporterPDF(filePath))
        QMessageBox::information(this, "Succès", "Exportation PDF réussie !");
    else
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'exportation du PDF !");
}

//-----------------------------stat-employe--------------------------------------
void AutoEcole::on_pushButton_StatistiquesEmploye_clicked()
{
    afficherStatsEmploye();
}

// =====================================================================
//                AFFICHAGE STATISTIQUES EMPLOYÉS
// =====================================================================
void AutoEcole::afficherStatsEmploye()
{
    // 1) Nettoyer l'ancien contenu du layout
    QLayoutItem *item;
    while ((item = ui->verticalLayoutStatsEmploye->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    // 2) Récupérer les stats depuis la base
    employe e;
    QSqlQueryModel *model = e.statistiquesParSpecialite();

    if (model->rowCount() == 0) {
        QMessageBox::information(this, "Statistiques",
                                 "Aucun employé trouvé pour générer les statistiques.");
        delete model;
        return;
    }

    // Ordre fixe des catégories
    QStringList specialitesFixes = {"code", "conduite voiture", "moto", "poids lourd"};

    // Initialiser à zéro
    QMap<QString,int> counts;
    for (const QString &sp : specialitesFixes)
        counts[sp] = 0;

    // Remplir depuis la base
    int totalEmp = 0;
    for (int i = 0; i < model->rowCount(); ++i) {
        QString sp = model->record(i).value("specialite").toString().toLower();
        int n = model->record(i).value("total").toInt();
        if (counts.contains(sp)) {
            counts[sp] = n;
            totalEmp += n;
        }
    }

    delete model;

    if (totalEmp == 0) {
        QMessageBox::information(this, "Statistiques",
                                 "Toutes les spécialités sont vides.");
        return;
    }

    // --- COULEURS pour les 4 barres ---
    QList<QColor> couleurs;
    couleurs << QColor("#61C4FF")   // code
             << QColor("#FFA53D")   // conduite voiture
             << QColor("#30B45B")   // moto
             << QColor("#FF4F7B");  // poids lourd

    // 3) Construire la série de barres
    QBarSeries *series = new QBarSeries();
    QStringList categories;
    int nbCats = specialitesFixes.size();

    for (int i = 0; i < nbCats; ++i) {
        QString sp = specialitesFixes[i];
        QBarSet *set = new QBarSet(sp);

        for (int j = 0; j < nbCats; ++j) {
            if (j == i)
                *set << counts.value(sp);
            else
                *set << 0;
        }

        set->setBrush(couleurs[i]);
        set->setLabelBrush(Qt::white);
        set->setLabelFont(QFont("Poppins", 15, QFont::Bold));

        series->append(set);
        categories << sp;
    }

    series->setLabelsVisible(true);
    series->setLabelsFormat("@value");
    series->setBarWidth(0.7);//***************************************************************************
    // après avoir rempli `couleurs` ET créé/ajouté les QBarSet dans `series`
    for (int i = 0; i < series->barSets().size(); ++i) {
        QBarSet *set = series->barSets().at(i);
        QColor baseColor = couleurs.at(i);      // couleur d'origine

        // s'assurer que la couleur de base est bien appliquée
        set->setBrush(QBrush(baseColor));

        QObject::connect(set, &QBarSet::hovered, this,
                         [set, baseColor](bool hovered, int index) {
                             Q_UNUSED(index);

                             if (hovered) {
                                 // un peu plus clair au survol
                                 set->setBrush(QBrush(baseColor.lighter(130)));
                             } else {
                                 // revenir exactement à la couleur de base
                                 set->setBrush(QBrush(baseColor));
                             }
                         }
                         );
    }
    //Créer le QChart avec animation d'apparition
    QChart *chart = new QChart();
    chart->addSeries(series);

    //Animation apparition : les barres montent du bas vers le haut
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chart->setTitle("Répartition des employés par spécialité");
    chart->setTitleBrush(Qt::white);
    chart->legend()->setVisible(false);
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(40, 20, 40, 20));
    chart->setBackgroundRoundness(0);

    // Créer le chartView après configuration
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(350);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setStyleSheet("background: transparent;");


    // Axe X
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(Qt::white);
    axisX->setTitleBrush(Qt::white);
    axisX->setTitleText("Spécialité");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Axe Y
    int maxCount = 0;
    for (int v : counts.values())
        if (v > maxCount) maxCount = v;

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, qMax(1, maxCount + 1));
    axisY->setLabelsColor(Qt::white);
    axisY->setTitleBrush(Qt::white);
    axisY->setTitleText("Nombre d'employés");
    axisY->setGridLineColor(QColor("#4A4A6A"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // ChartView
    //QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(350);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setStyleSheet("background: transparent;");

    // --- 5) TABLEAU SOUS LE SCHÉMA ---
    QTableWidget *table = new QTableWidget(nbCats, 3);
    table->setHorizontalHeaderLabels({"Spécialité", "Nombre", "%"});
    table->setStyleSheet(
        "QTableWidget { color: white; background-color: transparent; gridline-color: white; font-size: 14px; }"
        "QHeaderView::section { background-color: transparent; color: white; font-size: 14px; }"
        "QTableWidget::item { color: white; }"
        );
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);

    for (int i = 0; i < specialitesFixes.size(); ++i) {
        QString sp = specialitesFixes[i];
        int n = counts[sp];
        double pct = (totalEmp > 0) ? n * 100.0 / totalEmp : 0.0;

        table->setItem(i, 0, new QTableWidgetItem(sp));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(n)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(pct, 'f', 1) + " %"));
    }

    // on le laisse s'adapter au layout, juste une largeur max pour qu'il reste compact
    table->setMaximumWidth(450);
    table->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // 6) Ajouter dans le layout (table directement sous le graphique)
    ui->verticalLayoutStatsEmploye->addWidget(chartView);
    ui->verticalLayoutStatsEmploye->addWidget(table, 0, Qt::AlignHCenter);

}

//==================================================================CRUD-CLIENT===================================================================

// ---------------- AJOUTER ----------------

void AutoEcole::on_pushButton_ajoutClient_clicked()
{
    QString idText = ui->lineEdit_idClient->text().trimmed();
    QString nom = ui->lineEdit_nomClient->text().trimmed();
    QString prenom = ui->lineEdit_prenomClient->text().trimmed();
    QString niveau = ui->comboBox_niveauClient->currentText().trimmed();
    QString telephone = ui->lineEdit_telephoneClient->text().trimmed();
    QString email = ui->lineEdit_emailClient->text().trimmed();

    // Check for empty fields
    if (idText.isEmpty() || nom.isEmpty() || prenom.isEmpty() ||
        niveau.isEmpty() || niveau == "— Sélectionner —" ||
        telephone.isEmpty() || email.isEmpty())
    {
        QMessageBox::warning(this, "Empty Fields",
                             "Please fill all fields before adding a client!");
        return;
    }

    int id = idText.toInt();
    Client cl(id, nom, prenom, niveau, telephone, email);
    bool test = cl.ajouter();

    if (test) {
        ui->tableClient->setModel(cl.afficher());

        // Show sending notification
        QMessageBox::information(this, "Sending Email",
                                 "Sending confirmation email to:\n" + email);

        // Try to send email
        bool emailSent = cl.envoyerEmailConfirmation();

        // Show result to user
        if (emailSent) {
            QMessageBox::information(this, "✅ Success",
                                     "✅ Client added successfully!\n"
                                     "✅ Email sent to: " + email);
        } else {
            QMessageBox::warning(this, "⚠️ Partial Success",
                                 "✅ Client added successfully!\n"
                                 "❌ But failed to send email to: " + email +
                                     "\n\nCheck email settings or internet connection");
        }

        // Rest of your existing code for Bitakit Taarif...
        auto reponse = QMessageBox::question(this,
                                             "Bitakit Taarif",
                                             "Voulez-vous générer la carte d'identité numérique (QR scannable) de ce client ?",
                                             QMessageBox::Yes | QMessageBox::No);

        if (reponse == QMessageBox::Yes) {
            QString defaultName = QDir::homePath() + "/Bitakit_Taarif_Client_" + QString::number(id) + ".pdf";
            QString filePath = QFileDialog::getSaveFileName(this,
                                                            "Enregistrer la Bitakit Taarif",
                                                            defaultName,
                                                            "Fichiers PDF (*.pdf)");

            if (!filePath.isEmpty()) {
                if (cl.genererBitakitTaarifPDF(filePath)) {
                    QMessageBox::information(this, "Terminé",
                                             "Bitakit Taarif générée avec succès !\n"
                                             "Ouvrez le PDF et scannez le QR code avec votre téléphone → les infos du client s'afficheront comme une carte de contact.");
                } else {
                    QMessageBox::warning(this, "Erreur", "Échec de génération du PDF.");
                }
            }
        }

        // Clear fields
        ui->lineEdit_idClient->clear();
        ui->lineEdit_nomClient->clear();
        ui->lineEdit_prenomClient->clear();
        ui->lineEdit_telephoneClient->clear();
        ui->lineEdit_emailClient->clear();
        ui->comboBox_niveauClient->setCurrentIndex(0);
    } else {
        QMessageBox::critical(this, "❌ Error",
                              "Failed to add client!\nPlease check the data and try again.");
    }
}

// ---------------- SUPPRIMER ----------------
void AutoEcole::on_pushButton_supprimerClient_clicked()
{
    QString idText = ui->lineEdit_idClient->text().trimmed();

    //Vérifier si l'ID est vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID du client à supprimer !");
        return;
    }

    int id = idText.toInt();
    bool test = cl.supprimer(id);

    if (test) {
        ui->tableClient->setModel(cl.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Suppression effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Suppression non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }

    ui->lineEdit_idClient->clear();
}

// ---------------- MODIFIER ----------------
void AutoEcole::on_pushButton_modifierClient_clicked()
{
    QString idText = ui->lineEdit_idClient->text().trimmed();
    QString nom = ui->lineEdit_nomClient->text().trimmed();
    QString prenom = ui->lineEdit_prenomClient->text().trimmed();
    QString niveau = ui->comboBox_niveauClient->currentText().trimmed();
    QString telephone = ui->lineEdit_telephoneClient->text().trimmed();
    QString email = ui->lineEdit_emailClient->text().trimmed();


    //Vérifier si ID vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID du client à modifier !");
        return;
    }

    int id = idText.toInt();

    //Vérifier si l'ID existe dans la base
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM client WHERE id_client = :id");
    check.bindValue(":id", id);
    if (!check.exec() || !check.next() || check.value(0).toInt() == 0) {
        QMessageBox::warning(this, "ID invalide",
                             "Aucun client trouvé avec cet ID !");
        return;
    }

    //Vérifier si aucun autre champ n’est rempli
    if (nom.isEmpty() && prenom.isEmpty() && telephone.isEmpty() &&
        email.isEmpty() && (niveau.isEmpty() || niveau == "— Sélectionner —"))
    {
        QMessageBox::warning(this, "Aucune modification",
                             "Veuillez remplir au moins un champ à modifier (autre que l'ID) !");
        return;
    }

    Client cl(id, nom, prenom, niveau, telephone, email);
    bool test = cl.modifier(id);

    if (test) {
        ui->tableClient->setModel(cl.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Modification effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);

        ui->lineEdit_idClient->clear();
        ui->lineEdit_nomClient->clear();
        ui->lineEdit_prenomClient->clear();
        ui->lineEdit_telephoneClient->clear();
        ui->lineEdit_emailClient->clear();
        ui->comboBox_niveauClient->setCurrentIndex(0);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Modification non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
}
// When a row in the Client table is clicked, populate the form fields
void AutoEcole::on_tableClient_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    const QAbstractItemModel *model = ui->tableClient->model();
    int row = index.row();

    // Fill the form fields with the row values
    ui->lineEdit_idClient->setText(model->data(model->index(row, 0)).toString());
    ui->lineEdit_nomClient->setText(model->data(model->index(row, 1)).toString());
    ui->lineEdit_prenomClient->setText(model->data(model->index(row, 2)).toString());
    ui->comboBox_niveauClient->setCurrentText(model->data(model->index(row, 3)).toString());
    ui->lineEdit_telephoneClient->setText(model->data(model->index(row, 4)).toString());
    ui->lineEdit_emailClient->setText(model->data(model->index(row, 5)).toString());
}


//--------------------------recherche-client-------------------------
void AutoEcole::on_lineEdit_rechercheClient_textChanged(const QString &text)
{
    Client cl;
    ui->tableClient->setModel(cl.rechercher(text));
}
//---------------------------tri-client-----------------------------

void AutoEcole::on_comboBox_triClient_currentIndexChanged(const QString &text)
{
    if (text.trimmed().isEmpty() || text == "— Sélectionner —") {
        ui->tableClient->setModel(cl.afficher());
        return;
    }

    ui->tableClient->setModel(cl.trier(text));
}

//------------------------------------export-pdf-------------------------
void AutoEcole::on_btn_exportClientPDF_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter en PDF",
        QDir::homePath() + "/clients_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
        "Fichiers PDF (*.pdf)"
        );

    if (filePath.isEmpty())
        return;

    Client c;
    if (c.exporterPDF(filePath))
        QMessageBox::information(this, "Succès", "Exportation PDF réussie !");
    else
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'exportation du PDF !");
}
//------------------------------------stat-client-------------------------
void AutoEcole::on_btnStatClient_clicked()
{
    afficherStatsClients();
}
void AutoEcole::afficherStatsClients()
{
    // 1) Nettoyer l'ancien contenu du layout
    QLayoutItem *item;
    while ((item = ui->layoutStatsClients->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    // 2) Récupérer les stats depuis la base via Client
    Client c;
    QSqlQueryModel *model = c.statistiquesParNiveau();

    if (!model || model->rowCount() == 0) {
        QMessageBox::information(this, "Statistiques",
                                 "Aucun client trouvé pour générer les statistiques.");
        delete model;
        return;
    }

    // Nombre total de clients
    int totalClients = 0;
    for (int i = 0; i < model->rowCount(); ++i)
        totalClients += model->record(i).value("total").toInt();

    // 3) Créer la série de camembert
    QPieSeries *series = new QPieSeries();

    QStringList couleurs = {"#7BA0E8", "#F4A026", "#B084F9"};
    int indexCouleur = 0;

    QString niveauPlusFreq;
    int maxCount = -1;

    for (int i = 0; i < model->rowCount(); ++i) {

        QString niveau = model->record(i).value("niveau").toString();
        int total      = model->record(i).value("total").toInt();

        // pourcentage
        double pct = (totalClients > 0)
                         ? (total * 100.0 / totalClients)
                         : 0.0;

        // label final : Niveau : nb (xx.x%)
        QString label = QString("%1 : %2 (%3%)")
                            .arg(niveau)
                            .arg(total)
                            .arg(QString::number(pct, 'f', 1));

        QPieSlice *slice = series->append(label, total);

        QColor col(couleurs.at(indexCouleur % couleurs.size()));
        indexCouleur++;

        slice->setBrush(col);
        slice->setLabelVisible(true);
        slice->setLabelColor(Qt::white);
        slice->setLabelFont(QFont("Poppins", 11, QFont::Bold));
        slice->setLabelPosition(QPieSlice::LabelOutside);

        // Effet hover : la part se détache un peu + bordure
        QObject::connect(slice, &QPieSlice::hovered,
                         this, [slice, col](bool hovered) {
                             slice->setExploded(hovered);
                             slice->setExplodeDistanceFactor(0.10);
                             slice->setBorderWidth(hovered ? 3 : 1);
                             slice->setBorderColor(hovered ? QColor("#341671") : Qt::transparent);
                             slice->setBrush(hovered ? col.lighter(130) : col);
                         });

        // Calcul du niveau le plus fréquent
        if (total > maxCount) {
            maxCount = total;
            niveauPlusFreq = niveau;
        }
    }

    // 4) Créer le chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle("");                 // pas de titre
    chart->setBackgroundVisible(false);  // fond transparent du chart

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(500, 300);
    chartView->setStyleSheet("background: transparent;");

    // 5) Ajouter au layout de la zone graphique
    ui->layoutStatsClients->addWidget(chartView);

    // 6) Mettre à jour les 3 labels de droite

    // Total des clients
    ui->lblTotalClients->setText(QString::number(totalClients));

    // Niveau le plus fréquent
    ui->lblNiveauFrequent->setText(niveauPlusFreq);

    // Pourcentage de progression moyen
    double interCount = 0, avCount = 0;

    for (int i = 0; i < model->rowCount(); ++i) {
        QString niv = model->record(i).value("niveau").toString();
        int count   = model->record(i).value("total").toInt();

        if (niv == "Intermédiaire")
            interCount = count;
        else if (niv == "Avancé")
            avCount = count;
    }

    double progression = (totalClients > 0)
                             ? (interCount * 50.0 + avCount * 100.0) / totalClients
                             : 0.0;

    ui->lblProgression->setText(QString::number(progression, 'f', 1) + " %");

    // 7) Libérer le modèle (plus besoin)
    delete model;
}


//==================================================================CRUD-EXAMEN====================================================================

// ---------------- AJOUTER ----------------
void AutoEcole::on_pushButton_ajouterExamen_clicked()
{
    QString idText = ui->lineEdit_idExamen->text().trimmed();
    QString idClientText = ui->lineEdit_idClientExamen->text().trimmed();
    QString type = ui->comboBox_typeExamen->currentText().trimmed();
    QString resultat = ui->comboBox_resultatExamen->currentText().trimmed();
    QDate date = ui->dateEdit_examen->date();

    //Vérifier si des champs sont vides
    if (idText.isEmpty() || idClientText.isEmpty() ||
        type.isEmpty() || type == "— Sélectionner —" ||
        resultat.isEmpty() || resultat == "— Sélectionner —")
    {
        QMessageBox::warning(this, "Champs vides",
                             "Veuillez remplir tous les champs avant d'ajouter un examen !");
        return;
    }

    int id_examen = idText.toInt();
    int id_client = idClientText.toInt();

    Examen ex(id_examen, id_client, type, resultat, date);
    bool test = ex.ajouter();

    if (test) {
        ui->tableExamen->setModel(ex.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Ajout effectué.\nClick Cancel to exit."), QMessageBox::Cancel);
        ui->lineEdit_idExamen->clear();
        ui->lineEdit_idClientExamen->clear();
        ui->comboBox_typeExamen->setCurrentIndex(0);
        ui->comboBox_resultatExamen->setCurrentIndex(0);
        ui->dateEdit_examen->setDate(QDate::currentDate());
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Ajout non effectué.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
}

// ---------------- SUPPRIMER ----------------
void AutoEcole::on_supprimerExamen_clicked()
{
    QString idText = ui->lineEdit_idExamen->text().trimmed();

    //Vérifier si l'ID est vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID de l'examen à supprimer !");
        return;
    }

    int id_examen = idText.toInt();
    bool test = ex.supprimer(id_examen);

    if (test) {
        ui->tableExamen->setModel(ex.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Suppression effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Suppression non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }

    ui->lineEdit_idExamen->clear();
    ui->lineEdit_idClientExamen->clear();
    ui->comboBox_typeExamen->setCurrentIndex(0);
    ui->comboBox_resultatExamen->setCurrentIndex(0);
    ui->dateEdit_examen->setDate(QDate::currentDate());
}

// ---------------- MODIFIER ----------------
void AutoEcole::on_modifierExamen_clicked()
{
    QString idText = ui->lineEdit_idExamen->text().trimmed();
    QString idClientText = ui->lineEdit_idClientExamen->text().trimmed();
    QString type = ui->comboBox_typeExamen->currentText().trimmed();
    QString resultat = ui->comboBox_resultatExamen->currentText().trimmed();
    QDate date = ui->dateEdit_examen->date();

    //Vérifier si ID vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID de l'examen à modifier !");
        return;
    }

    int id_examen = idText.toInt();
    int id_client = idClientText.toInt();

    //Vérifier si l'ID existe dans la base
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM examen WHERE id_examen = :id");
    check.bindValue(":id", id_examen);
    if (!check.exec() || !check.next() || check.value(0).toInt() == 0) {
        QMessageBox::warning(this, "ID invalide",
                             "Aucun examen trouvé avec cet ID !");
        return;
    }

    //Vérifier si aucun autre champ n'est rempli
    if ((idClientText.isEmpty() || id_client == 0) &&
        (type.isEmpty() || type == "— Sélectionner —") &&
        (resultat.isEmpty() || resultat == "— Sélectionner —") &&
        !date.isValid())
    {
        QMessageBox::warning(this, "Aucune modification",
                             "Veuillez remplir au moins un champ à modifier (autre que l'ID) !");
        return;
    }

    Examen ex(id_examen, id_client, type, resultat, date);
    bool test = ex.modifier(id_examen);

    if (test) {
        ui->tableExamen->setModel(ex.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Modification effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
        ui->lineEdit_idExamen->clear();
        ui->lineEdit_idClientExamen->clear();
        ui->comboBox_typeExamen->setCurrentIndex(0);
        ui->comboBox_resultatExamen->setCurrentIndex(0);
        ui->dateEdit_examen->setDate(QDate::currentDate());
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Modification non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
}
void AutoEcole::on_tableExamen_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    const QAbstractItemModel *model = ui->tableExamen->model();
    int row = index.row();

    // ID Examen (col 0)
    ui->lineEdit_idExamen->setText(model->data(model->index(row, 0)).toString());

    // Type (col 1)
    ui->comboBox_typeExamen->setCurrentText(model->data(model->index(row, 1)).toString());

    // Date (col 2)
    QVariant dateValue = model->data(model->index(row, 2));
    if (dateValue.isValid())
        ui->dateEdit_examen->setDate(dateValue.toDate());

    // ID Client (col 3)
    ui->lineEdit_idClientExamen->setText(model->data(model->index(row, 3)).toString());

    // Résultat (col 4)
    ui->comboBox_resultatExamen->setCurrentText(model->data(model->index(row, 4)).toString());
}


//-----------------------------rechercher-examen--------------------------------------
void AutoEcole::on_lineEdit_rechercheExamen_textChanged(const QString &text)
{
    Examen ex;
    ui->tableExamen->setModel(ex.rechercher(text));
}
//-----------------------------tri-examen--------------------------------------
void AutoEcole::on_comboBox_triExamen_currentIndexChanged(const QString &text)
{
    Examen ex;
    if (text.trimmed().isEmpty() || text == "— Sélectionner —") {
        ui->tableExamen->setModel(ex.afficher());
        return;
    }

    ui->tableExamen->setModel(ex.trier(text));
}
// -------------------------- EXPORT-PDF EXAMEN --------------------------
void AutoEcole::on_btn_exportExamenPDF_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter en PDF",
        QDir::homePath() + "/examens_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
        "Fichiers PDF (*.pdf)"
        );

    if (filePath.isEmpty())
        return;

    Examen ex;
    if (ex.exporterPDF(filePath))
        QMessageBox::information(this, "Succès", "Exportation PDF réussie !");
    else
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'exportation du PDF !");
}
//-------------------------stat--------------------------------------
// Quand tu cliques sur le bouton "Statistiques"
void AutoEcole::on_btnStatExamen_clicked()
{
    afficherStatsExamen();
}

void AutoEcole::afficherStatsExamen()
{
    // 1) Nettoyer l'ancien donut
    QLayoutItem *item;
    while ((item = ui->layoutStatsExamen->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    // 2) Générer le nouveau donut
    Examen e;
    QChartView *view = e.genererDonutReussite();

    if (!view) {
        QMessageBox::information(this,
                                 "Statistiques examen",
                                 "Aucun examen trouvé.");
        return;
    }

    // 3) L'ajouter dans le layout
    ui->layoutStatsExamen->addWidget(view);
}

//------------------------------------métier--------------------------------
void AutoEcole::on_pushButton_exams_en_retard_clicked()
{
    Examen ex;
    QSqlQueryModel *model = ex.detecterNonRealisesALaDatePrevue();
    if (model && model->rowCount() == 0) {
        QMessageBox::information(this, "Examens en retard", "Aucun examen en retard détecté.");
    }
    ui->tableExamen->setModel(model);
}

void AutoEcole::on_pushButton_67_clicked()
{
    on_pushButton_exams_en_retard_clicked();
}

void AutoEcole::on_pushButton_68_clicked()
{
    QSqlQueryModel *model = cl.identifierClientsEnDifficulte();
    if (model && model->rowCount() == 0) {
        QMessageBox::information(this, "Clients en difficulté", "Aucun client en difficulté détecté.");
    }
    ui->tableClient->setModel(model);
    ui->stackedWidget->setCurrentWidget(ui->client);
}
//==================================================================CRUD-VEHICULE===================================================================

// ---------------- AJOUTER ----------------

void AutoEcole::on_ajouterVehicule_clicked()
{
    QString matriculeText = ui->lineEdit_matricule->text().trimmed();
    QString marque = ui->lineEdit_marque->text().trimmed();
    QString modele = ui->lineEdit_modele->text().trimmed();
    QString kilometrage = ui->lineEdit_kilometrage->text().trimmed();
    QString type = ui->comboBox_add_typeVehicule->currentText().trimmed();
    QString localisation = ui->comboBox_add_localisation->currentText().trimmed();

    //Vérifier si un champ est vide
    if (matriculeText.isEmpty() || marque.isEmpty() || modele.isEmpty() ||
        kilometrage.isEmpty() || type.isEmpty() || type == "– Sélectionner –" ||
        localisation.isEmpty() || localisation == "– Sélectionner –") {
        QMessageBox::warning(this, "Champs vides",
                             "Veuillez remplir tous les champs avant d'ajouter un véhicule !");
        return;
    }

    int matricule = matriculeText.toInt();

    Vehicule veh(matricule, marque, modele, type, kilometrage, localisation);
    bool test = veh.ajouter();

    if (test) {
        ui->tableVehicule->setModel(veh.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Ajout effectué.\nClick Cancel to exit."), QMessageBox::Cancel);

        // ENREGISTRER DANS L'HISTORIQUE
        QString details = QString("%1 %2 - Type: %3 - %4 km - Localisation: %5")
                              .arg(marque).arg(modele).arg(type).arg(kilometrage).arg(localisation);
        ajouterHistorique("AJOUT", matricule, details);

        ui->lineEdit_matricule->clear();
        ui->lineEdit_marque->clear();
        ui->lineEdit_modele->clear();
        ui->lineEdit_kilometrage->clear();
        ui->comboBox_add_typeVehicule->setCurrentIndex(0);
        ui->comboBox_add_localisation->setCurrentIndex(0);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Ajout non effectué.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
}

// ---------------- SUPPRIMER ----------------
void AutoEcole::on_supprimerVehicule_clicked()
{
    QString matriculeText = ui->lineEdit_matricule->text().trimmed();

    //Vérifier si ID vide
    if (matriculeText.isEmpty()) {
        QMessageBox::warning(this, "Matricule manquant",
                             "Veuillez entrer le matricule du véhicule à supprimer !");
        return;
    }

    int matricule = matriculeText.toInt();
    Vehicule veh;
    bool test = veh.supprimer(matricule);

    if (test) {
        ui->tableVehicule->setModel(veh.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Suppression effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);

        // ENREGISTRER DANS L'HISTORIQUE
        QString details = QString("Véhicule supprimé de la base de données");
        ajouterHistorique("SUPPRESSION", matricule, details);

    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Suppression non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }

    ui->lineEdit_matricule->clear();
    ui->lineEdit_marque->clear();
    ui->lineEdit_modele->clear();
    ui->lineEdit_kilometrage->clear();
    ui->comboBox_add_typeVehicule->setCurrentIndex(0);
    ui->comboBox_add_localisation->setCurrentIndex(0);
}

// ---------------- MODIFIER ----------------

void AutoEcole::on_modifierVehicule_clicked()
{
    QString matriculeText = ui->lineEdit_matricule->text().trimmed();
    QString marque = ui->lineEdit_marque->text().trimmed();
    QString modele = ui->lineEdit_modele->text().trimmed();
    QString kilometrage = ui->lineEdit_kilometrage->text().trimmed();
    QString type;
    if (ui->comboBox_add_typeVehicule->currentIndex() <= 0)
        type = "";
    else
        type = ui->comboBox_add_typeVehicule->currentText().trimmed();

    QString localisation;
    if (ui->comboBox_add_localisation->currentIndex() <= 0)
        localisation = "";
    else
        localisation = ui->comboBox_add_localisation->currentText().trimmed();

    //Vérifier si matricule vide
    if (matriculeText.isEmpty()) {
        QMessageBox::warning(this, "Matricule manquant",
                             "Veuillez entrer le matricule du véhicule à modifier !");
        return;
    }

    int matricule = matriculeText.toInt();

    //Vérifier si le matricule existe
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM vehicule WHERE matricule = :mat");
    check.bindValue(":mat", matricule);
    if (!check.exec() || !check.next() || check.value(0).toInt() == 0) {
        QMessageBox::warning(this, "Matricule invalide",
                             "Aucun véhicule trouvé avec ce matricule !");
        return;
    }

    //Vérifier si aucun autre champ n'est rempli
    if (marque.isEmpty() && modele.isEmpty() && kilometrage.isEmpty() &&
        type.isEmpty() && localisation.isEmpty()) {
        QMessageBox::warning(this, "Aucune modification",
                             "Veuillez remplir au moins un champ à modifier (autre que le matricule) !");
        return;
    }

    Vehicule veh(matricule, marque, modele, type, kilometrage, localisation);
    bool test = veh.modifier(matricule);

    if (test) {
        ui->tableVehicule->setModel(veh.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Modification effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);

        // ENREGISTRER DANS L'HISTORIQUE
        QString details = QString("Modification: Marque=%1, Modèle=%2, Type=%3, Km=%4, Localisation=%5")
                              .arg(marque.isEmpty() ? "(inchangé)" : marque)
                              .arg(modele.isEmpty() ? "(inchangé)" : modele)
                              .arg(type.isEmpty() ? "(inchangé)" : type)
                              .arg(kilometrage.isEmpty() ? "(inchangé)" : kilometrage)
                              .arg(localisation.isEmpty() ? "(inchangé)" : localisation);
        ajouterHistorique("MODIFICATION", matricule, details);

        ui->lineEdit_matricule->clear();
        ui->lineEdit_marque->clear();
        ui->lineEdit_modele->clear();
        ui->lineEdit_kilometrage->clear();
        ui->comboBox_add_typeVehicule->setCurrentIndex(0);
        ui->comboBox_add_localisation->setCurrentIndex(0);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Modification non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
}
// Quand une ligne du tableau Véhicule est cliquée, remplir les champs du formulaire
void AutoEcole::on_tableVehicule_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    const QAbstractItemModel *model = ui->tableVehicule->model();
    int row = index.row();

    // Matricule (col 0)
    ui->lineEdit_matricule->setText(model->data(model->index(row, 0)).toString());

    // Marque (col 1)
    ui->lineEdit_marque->setText(model->data(model->index(row, 1)).toString());

    // Modèle (col 2)
    ui->lineEdit_modele->setText(model->data(model->index(row, 2)).toString());

    // Type (col 3)
    ui->comboBox_add_typeVehicule->setCurrentText(model->data(model->index(row, 3)).toString());

    // Kilométrage (col 4)
    ui->lineEdit_kilometrage->setText(model->data(model->index(row, 4)).toString());

    // Localisation (col 5)
    ui->comboBox_add_localisation->setCurrentText(model->data(model->index(row, 5)).toString());
}


//------------------------------------export-pdf-------------------------
void AutoEcole::on_btn_exportVehiculePDF_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter en PDF",
        QDir::homePath() + "/vehicules_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
        "Fichiers PDF (*.pdf)"
        );

    if (filePath.isEmpty())
        return;

    Vehicule v;
    if (v.exporterPDF(filePath))
        QMessageBox::information(this, "SuccÃ¨s", "Exportation PDF rÃ©ussie !");
    else
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'exportation du PDF !");
}
// ------------------------------ STAT VEHICULE ------------------------------

void AutoEcole::on_btnStatVehicule_clicked()
{
    afficherStatsVehicule();
}
void AutoEcole::afficherStatsVehicule()
{
    // ---------- CLEAN OLD CONTENT ----------
    QLayoutItem *item;
    while ((item = ui->layoutChartVehicule->takeAt(0)) != nullptr)
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    Vehicule v;
    QSqlQueryModel *model = v.statistiquesVehicule();

    // ---------- DONUT ----------
    QChartView *donut = v.genererDonut(model);
    donut->setMinimumSize(600, 600);   // bigger + clear
    ui->layoutChartVehicule->addWidget(donut);

    // ---------- FILL LABELS ----------
    ui->lblNbManuelle->setText("0");
    ui->lblNbAutomatique->setText("0");
    ui->lblNbMoto->setText("0");
    // ------- LEGEND BUBBLES UNDER DONUT -------
    QHBoxLayout *legend = new QHBoxLayout();

    auto addLegend = [&](QString color, QString text){
        QWidget *item = new QWidget();
        QHBoxLayout *h = new QHBoxLayout(item);

        QLabel *dot = new QLabel();
        dot->setFixedSize(18,18);
        dot->setStyleSheet("background-color:" + color +
                           "; border-radius: 9px;");

        QLabel *lbl = new QLabel(text);
        lbl->setStyleSheet("color:white; font-size:22px;");

        h->addWidget(dot);
        h->addSpacing(10);
        h->addWidget(lbl);

        legend->addWidget(item);
        legend->addSpacing(40);
    };


    // Ajouter sous le donut
    ui->legendVehiculeLayout->addLayout(legend);


    for (int i = 0; i < model->rowCount(); i++)
    {
        QString type = model->record(i).value(0).toString();
        int total = model->record(i).value(1).toInt();

        if (type == "Manuelle")
            ui->lblNbManuelle->setText(QString::number(total));

        if (type == "Automatique")
            ui->lblNbAutomatique->setText(QString::number(total));

        if (type == "Moto")
            ui->lblNbMoto->setText(QString::number(total));
    }
}

void AutoEcole::on_lineEdit_rechercheVehicule_textChanged(const QString &text)
{
    Vehicule veh;
    ui->tableVehicule->setModel(veh.rechercher(text));
}
void AutoEcole::on_comboBox_triVehicule_currentIndexChanged(const QString &text)
{
    Vehicule v;
    if (text.trimmed().isEmpty() || text == "— Sélectionner —") {
        ui->tableVehicule->setModel(v.afficher());
        return;
    }

    ui->tableVehicule->setModel(v.trier(text));
}



//==================================================================CRUD-SEANCE===================================================================

// ---------------- AJOUTER ----------------
void AutoEcole::on_ajouterSeance_clicked()
{
    QString idText = ui->lineEdit_idSeance->text().trimmed();
    QString idClientText = ui->lineEdit_idClient_2->text().trimmed();
    QString idEmployeText = ui->lineEdit_idEmploye->text().trimmed();
    QString matriculeText = ui->lineEdit_matricule_2->text().trimmed();
    QDate date = ui->dateEdit_seance->date();
    QTime heure = ui->timeEdit_seance->time();

    //Vérifier si un champ est vide
    if (idText.isEmpty() || idClientText.isEmpty() || idEmployeText.isEmpty() ||
        matriculeText.isEmpty()) {
        QMessageBox::warning(this, "Champs vides",
                             "Veuillez remplir tous les champs avant d'ajouter une séance !");
        return;
    }

    int id = idText.toInt();
    int idClient = idClientText.toInt();
    int idEmploye = idEmployeText.toInt();
    int matricule = matriculeText.toInt();

    Seance s(id, date, heure, idClient, idEmploye, matricule);
    bool test = s.ajouter();

    if (test) {
        ui->tableseance->setModel(s.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Ajout effectué.\nClick Cancel to exit."), QMessageBox::Cancel);
        ui->lineEdit_idSeance->clear();
        ui->lineEdit_idClient_2->clear();
        ui->lineEdit_idEmploye->clear();
        ui->lineEdit_matricule_2->clear();
        ui->dateEdit_seance->setDate(QDate::currentDate());
        ui->timeEdit_seance->setTime(QTime::currentTime());
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Ajout non effectué.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
    if (test) {
        ui->tableseance->setModel(s.afficher());
        colorizeCalendar(); // ← ADD THIS LINE
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Ajout effectué.\nClick Cancel to exit."),
                                 QMessageBox::Cancel);
    }
}

// ---------------- SUPPRIMER ----------------
void AutoEcole::on_supprimerSeance_clicked()
{
    QString idText = ui->lineEdit_idSeance->text().trimmed();

    //Vérifier si ID vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID de la séance à supprimer !");
        return;
    }

    int id = idText.toInt();
    Seance s;
    bool test = s.supprimer(id);

    if (test) {
        ui->tableseance->setModel(s.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Suppression effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Suppression non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
    if (test) {
        ui->tableseance->setModel(s.afficher());
        colorizeCalendar(); // ← ADD THIS LINE
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Suppression effectuée.\nClick Cancel to exit."),
                                 QMessageBox::Cancel);
    }

    ui->lineEdit_idSeance->clear();
    ui->lineEdit_idClient_2->clear();
    ui->lineEdit_idEmploye->clear();
    ui->lineEdit_matricule_2->clear();
    ui->dateEdit_seance->setDate(QDate::currentDate());
    ui->timeEdit_seance->setTime(QTime::currentTime());
}

// ---------------- MODIFIER ----------------
void AutoEcole::on_modifierSeance_clicked()
{
    QString idText = ui->lineEdit_idSeance->text().trimmed();
    QString idClientText = ui->lineEdit_idClient_2->text().trimmed();
    QString idEmployeText = ui->lineEdit_idEmploye->text().trimmed();
    QString matriculeText = ui->lineEdit_matricule_2->text().trimmed();
    QDate date = ui->dateEdit_seance->date();
    QTime heure = ui->timeEdit_seance->time();

    //Vérifier si ID vide
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez entrer l'ID de la séance à modifier !");
        return;
    }

    int id = idText.toInt();
    int idClient = idClientText.toInt();
    int idEmploye = idEmployeText.toInt();
    int matricule = matriculeText.toInt();

    //Vérifier si l'ID existe dans la base
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM seance WHERE id_seance = :id");
    check.bindValue(":id", id);
    if (!check.exec() || !check.next() || check.value(0).toInt() == 0) {
        QMessageBox::warning(this, "ID invalide",
                             "Aucune séance trouvée avec cet ID !");
        return;
    }

    //Vérifier si aucun autre champ n'est rempli
    if (idClientText.isEmpty() && idEmployeText.isEmpty() && matriculeText.isEmpty() &&
        (!date.isValid() || date == QDate::currentDate()) &&
        (!heure.isValid() || heure == QTime::currentTime()))
    {
        QMessageBox::warning(this, "Aucune modification",
                             "Veuillez remplir au moins un champ à modifier (autre que l'ID) !");
        return;
    }

    Seance s(id, date, heure, idClient, idEmploye, matricule);
    bool test = s.modifier(id);

    if (test) {
        ui->tableseance->setModel(s.afficher());
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Modification effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
        ui->lineEdit_idSeance->clear();
        ui->lineEdit_idClient_2->clear();
        ui->lineEdit_idEmploye->clear();
        ui->lineEdit_matricule_2->clear();
        ui->dateEdit_seance->setDate(QDate::currentDate());
        ui->timeEdit_seance->setTime(QTime::currentTime());
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Modification non effectuée.\nClick Cancel to exit."), QMessageBox::Cancel);
    }
    if (test) {
        ui->tableseance->setModel(s.afficher());
        colorizeCalendar(); // ← ADD THIS LINE
        QMessageBox::information(nullptr, QObject::tr("OK"),
                                 QObject::tr("Modification effectuée.\nClick Cancel to exit."),
                                 QMessageBox::Cancel);
        // ... clear fields ...
    }
}
// Quand une ligne du tableau Séance est cliquée, remplir les champs du formulaire
void AutoEcole::on_tableseance_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    const QAbstractItemModel *model = ui->tableseance->model();
    int row = index.row();

    // ID séance
    ui->lineEdit_idSeance->setText(model->data(model->index(row, 0)).toString());

    // ----- DATE (col 1) -----
    QString dateStr = model->data(model->index(row, 1)).toString();
    QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
    if (date.isValid())
        ui->dateEdit_seance->setDate(date);

    // ----- HEURE (col 2) -----
    QString timeStr = model->data(model->index(row, 2)).toString();
    QTime time = QTime::fromString(timeStr, "hh:mm");
    if (time.isValid())
        ui->timeEdit_seance->setTime(time);

    // ID Client
    ui->lineEdit_idClient_2->setText(model->data(model->index(row, 3)).toString());

    // ID Employé
    ui->lineEdit_idEmploye->setText(model->data(model->index(row, 4)).toString());

    // Matricule
    ui->lineEdit_matricule_2->setText(model->data(model->index(row, 5)).toString());
}



//-----------------------------rechercher-seance--------------------------------------
void AutoEcole::on_lineEdit_rechercheSeance_textChanged(const QString &text)
{
    Seance s;
    ui->tableseance->setModel(s.rechercher(text));
}

//---------------------------tri-seance-----------------------------
void AutoEcole::on_comboBox_triSeance_currentIndexChanged(const QString &text)
{
    Seance s;
    if (text.trimmed().isEmpty() || text == "— Sélectionner —") {
        ui->tableseance->setModel(s.afficher());
        return;
    }
    ui->tableseance->setModel(s.trier(text));
}

//------------------------------------export-pdf-------------------------
void AutoEcole::on_btn_exportSeancePDF_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter en PDF",
        QDir::homePath() + "/seances_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
        "Fichiers PDF (*.pdf)"
        );

    if (filePath.isEmpty())
        return;

    Seance s;
    if (s.exporterPDF(filePath))
        QMessageBox::information(this, "Succès", "Exportation PDF réussie !");
    else
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'exportation du PDF !");
}
//------------------------------------stat-seance---------------------------------
// ==================== STATISTIQUES SÉANCES AMÉLIORÉES ====================

// Fonction principale pour afficher les statistiques
void AutoEcole::afficherStatsSeance(const QString &mode)
{
    // Nettoyer le layout
    QLayout *lay = ui->layoutChartSeance;
    while (QLayoutItem *item = lay->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    Seance s;
    QChartView *view = nullptr;

    // Générer le graphique selon le mode
    if (mode == "jour") {
        view = s.genererGraphiqueBarres("jour");
    }
    else if (mode == "mois") {
        view = s.genererGraphiqueLigne("mois");
    }
    else if (mode == "semaine") {
        view = s.genererGraphiqueBarres("semaine");
    }

    if (view) {
        view->setMinimumSize(800, 500);
        lay->addWidget(view);

        // Ajouter un widget de statistiques résumées
        QWidget *statsWidget = creerWidgetStatsResumees();
        lay->addWidget(statsWidget);
    } else {
        QLabel *noData = new QLabel("Aucune donnée disponible");
        noData->setStyleSheet("color: white; font-size: 16px;");
        noData->setAlignment(Qt::AlignCenter);
        lay->addWidget(noData);
    }
}

// Créer un widget avec les statistiques résumées
// ==================== VERSION SIMPLE - HAUTE LISIBILITÉ ==========

QWidget* AutoEcole::creerWidgetStatsResumees()
{
    QWidget *widget = new QWidget();
    widget->setStyleSheet("background: transparent;");
    QHBoxLayout *mainLayout = new QHBoxLayout(widget);
    mainLayout->setSpacing(20);

    Seance s;
    QMap<QString, int> stats = s.statsAvancees();

    // Style ultra-simple mais très lisible
    QString cardStyle =
        "QWidget {"
        "  background-color: rgba(255, 255, 255, 0.15);"
        "  border: 3px solid rgba(255, 255, 255, 0.3);"
        "  border-radius: 10px;"
        "}";

    auto createSimpleCard = [cardStyle](QString emoji, QString text, int value, QString color) -> QWidget* {
        QWidget *card = new QWidget();
        card->setStyleSheet(cardStyle);
        card->setMinimumSize(200, 130);

        QVBoxLayout *layout = new QVBoxLayout(card);

        // Icône + Titre sur une ligne
        QHBoxLayout *headerLayout = new QHBoxLayout();
        QLabel *icon = new QLabel(emoji);
        icon->setStyleSheet("font-size: 24px; background: transparent; color: white;");
        QLabel *title = new QLabel(text);
        title->setStyleSheet("font-size: 15px; font-weight: bold; background: transparent; color: white;");
        headerLayout->addWidget(icon);
        headerLayout->addWidget(title);
        headerLayout->addStretch();

        // Valeur GRANDE
        QLabel *valueLabel = new QLabel(QString::number(value));
        valueLabel->setStyleSheet(QString(
                                      "font-size: 56px;"
                                      "font-weight: bold;"
                                      "color: %1;"
                                      "background: transparent;"
                                      ).arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);

        layout->addLayout(headerLayout);
        layout->addWidget(valueLabel);

        return card;
    };

    mainLayout->addWidget(createSimpleCard("📊", "Total", stats.value("total", 0), "#FFC300"));
    mainLayout->addWidget(createSimpleCard("📅", "Semaine", stats.value("semaine", 0), "#61C4FF"));
    mainLayout->addWidget(createSimpleCard("🔜", "À venir", stats.value("avenir", 0), "#30B45B"));
    mainLayout->addWidget(createSimpleCard("🎯", "Aujourd'hui", stats.value("aujourdhui", 0), "#FF6B6B"));

    return widget;
}

// ==================== BOUTONS DE NAVIGATION ====================

void AutoEcole::on_btnStatJour_clicked()
{
    afficherStatsSeance("jour");
}

void AutoEcole::on_btnStatMois_clicked()
{
    afficherStatsSeance("mois");
}

// Nouveau bouton pour stats par semaine (à ajouter dans votre UI)
void AutoEcole::on_btnStatSemaine_clicked()
{
    afficherStatsSeance("semaine");
}

// ==================== TABLEAU DÉTAILLÉ DES STATISTIQUES ====================

void AutoEcole::afficherTableauStatsDetaille()
{
    // Créer un tableau avec les détails des statistiques
    QTableWidget *table = new QTableWidget();
    table->setStyleSheet(
        "QTableWidget {"
        "  background-color: rgba(255, 255, 255, 0.05);"
        "  color: white;"
        "  gridline-color: #4A4A6A;"
        "  font-size: 13px;"
        "  border: none;"
        "}"
        "QHeaderView::section {"
        "  background-color: rgba(255, 195, 0, 0.2);"
        "  color: white;"
        "  padding: 8px;"
        "  border: 1px solid #4A4A6A;"
        "  font-weight: bold;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px;"
        "}"
        );

    Seance s;
    QMap<QString, int> stats = s.statsAvancees();

    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Métrique", "Valeur"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);

    QStringList metriques = {
        "Total des séances",
        "Séances aujourd'hui",
        "Séances cette semaine",
        "Séances ce mois",
        "Séances à venir",
        "Séances passées"
    };

    QStringList keys = {
        "total",
        "aujourdhui",
        "semaine",
        "mois",
        "avenir",
        "passees"
    };

    table->setRowCount(metriques.size());

    for (int i = 0; i < metriques.size(); i++) {
        QTableWidgetItem *item1 = new QTableWidgetItem(metriques[i]);
        QTableWidgetItem *item2 = new QTableWidgetItem(
            QString::number(stats.value(keys[i], 0))
            );

        item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item2->setTextAlignment(Qt::AlignCenter);

        // Colorer les valeurs
        QColor color = QColor("#FFC300");
        if (keys[i] == "avenir") color = QColor("#30B45B");
        if (keys[i] == "aujourdhui") color = QColor("#FF6B6B");

        QFont font = item2->font();
        font.setBold(true);
        item2->setFont(font);
        item2->setForeground(QBrush(color));

        table->setItem(i, 0, item1);
        table->setItem(i, 1, item2);
    }

    table->resizeColumnsToContents();
    table->setMinimumWidth(400);
    table->setMaximumWidth(600);

    // Ajouter au layout (vous devez avoir un layout dédié dans votre UI)
    // ui->layoutTableauStats->addWidget(table);
}

// ==================== EXPORT STATISTIQUES PDF ====================

bool AutoEcole::exporterStatistiquesPDF(const QString &filePath)
{
    Seance s;
    QMap<QString, int> stats = s.statsAvancees();

    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);
    if (!painter.isActive())
        return false;

    QFont titleFont("Arial", 18, QFont::Bold);
    QFont headerFont("Arial", 12, QFont::Bold);
    QFont textFont("Arial", 10);

    int y = 300;

    // Titre
    painter.setFont(titleFont);
    painter.drawText(QRect(0, y, pdf.width(), 300),
                     Qt::AlignHCenter,
                     "RAPPORT STATISTIQUES DES SÉANCES");
    y += 500;

    // Date de génération
    painter.setFont(QFont("Arial", 9, QFont::StyleItalic));
    painter.drawText(QRect(0, y, pdf.width(), 200),
                     Qt::AlignHCenter,
                     "Généré le " + QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm"));
    y += 400;

    // Ligne de séparation
    painter.drawLine(200, y, pdf.width() - 200, y);
    y += 300;

    // Statistiques
    painter.setFont(headerFont);
    painter.drawText(200, y, "RÉSUMÉ DES SÉANCES");
    y += 350;

    painter.setFont(textFont);

    QStringList metrics = {
        QString("Total des séances : %1").arg(stats.value("total", 0)),
        QString("Séances aujourd'hui : %1").arg(stats.value("aujourdhui", 0)),
        QString("Séances cette semaine : %1").arg(stats.value("semaine", 0)),
        QString("Séances ce mois : %1").arg(stats.value("mois", 0)),
        QString("Séances à venir : %1").arg(stats.value("avenir", 0)),
        QString("Séances passées : %1").arg(stats.value("passees", 0))
    };

    for (const QString &metric : metrics) {
        painter.drawText(400, y, metric);
        y += 280;
    }

    painter.end();
    return true;
}
//--------------calendar seance---------//////////////////////////////////////////////////////////////////////
// Add these missing implementations to autoecole.cpp

// ==================== WEATHER FUNCTIONS ====================

void AutoEcole::fetchWeatherForDate(const QDate &date)
{
    // Using Open-Meteo API (free, no API key required)
    QString lat = "36.8065";  // Tunis latitude
    QString lon = "10.1815";  // Tunis longitude

    QString url = QString("https://api.open-meteo.com/v1/forecast?"
                          "latitude=%1&longitude=%2&daily=temperature_2m_max,"
                          "temperature_2m_min,weathercode&timezone=Africa/Tunis"
                          "&start_date=%3&end_date=%3")
                      .arg(lat, lon, date.toString("yyyy-MM-dd"));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "AutoEcole/1.0");

    QNetworkReply *reply = networkManager->get(request);
    reply->setProperty("date", date);
}

void AutoEcole::onWeatherDataReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Weather API error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QDate date = reply->property("date").toDate();
    QByteArray data = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    if (obj.contains("daily")) {
        QJsonObject daily = obj["daily"].toObject();

        // Get weather code (WMO Weather interpretation codes)
        int weatherCode = daily["weathercode"].toArray()[0].toInt();
        double tempMax = daily["temperature_2m_max"].toArray()[0].toDouble();
        double tempMin = daily["temperature_2m_min"].toArray()[0].toDouble();

        // Convert weather code to readable text
        QString condition = getWeatherCondition(weatherCode);
        QString weatherInfo = QString("%1\n%2°C - %3°C")
                                  .arg(condition)
                                  .arg(QString::number(tempMin, 'f', 0))
                                  .arg(QString::number(tempMax, 'f', 0));

        weatherCache[date] = weatherInfo;
    }

    reply->deleteLater();
}

QString AutoEcole::getWeatherCondition(int code)
{
    // WMO Weather interpretation codes
    if (code == 0) return "☀️ Ensoleillé";
    if (code <= 3) return "⛅ Partiellement nuageux";
    if (code <= 48) return "🌫️ Brouillard";
    if (code <= 67) return "🌧️ Pluie";
    if (code <= 77) return "🌨️ Neige";
    if (code <= 82) return "🌧️ Averses";
    if (code <= 86) return "🌨️ Averses de neige";
    if (code >= 95) return "⛈️ Orage";
    return "🌤️ Variable";
}

void AutoEcole::updateCalendarWithWeather()
{
    // This function is called after weather data is received
    // It updates the calendar display with the new weather info
    colorizeCalendar();
}

// ==================== CALENDAR FUNCTIONS ====================

void AutoEcole::colorizeCalendar()
{
    // Reset all dates to default first
    ui->calendarSeances->setDateTextFormat(QDate(), QTextCharFormat());

    // Create a Seance object to call member methods
    Seance s;

    // Get grouped seances from Seance class
    QMap<QDate, QList<Seance>> seancesByDate = s.groupSeancesByDate();

    // Apply colors and build tooltips with weather info
    for (auto it = seancesByDate.begin(); it != seancesByDate.end(); ++it) {
        QTextCharFormat fmt;
        fmt.setFontWeight(QFont::Bold);

        QDate date = it.key();
        QList<Seance> seances = it.value();
        int count = seances.count();

        // Color coding based on number of seances
        if (count == 1) {
            fmt.setBackground(QColor("#A8E6CF"));
            fmt.setForeground(Qt::black);
        } else if (count == 2) {
            fmt.setBackground(QColor("#FFD93D"));
            fmt.setForeground(Qt::black);
        } else if (count >= 3) {
            fmt.setBackground(QColor("#FF6B6B"));
            fmt.setForeground(Qt::white);
        }

        // Build tooltip with seances AND weather
        QString tooltip;

        // Add weather info if available
        if (weatherCache.contains(date)) {
            tooltip += "🌤️ MÉTÉO:\n" + weatherCache[date] + "\n\n";
        }

        tooltip += QString("📅 %1 SÉANCE(S):\n").arg(count);

        for (int i = 0; i < seances.size(); i++) {
            const Seance &seance = seances[i];
            // Use method from Seance object
            QString clientName = s.getClientName(seance.getIdClient());
            QString heure = seance.getHeure().toString("HH:mm");
            tooltip += QString("• %1 - %2\n").arg(heure, clientName);
        }

        fmt.setToolTip(tooltip);
        ui->calendarSeances->setDateTextFormat(date, fmt);
    }

    // Also mark dates with just weather (no seances)
    QDate today = QDate::currentDate();
    for (int i = 0; i < 14; i++) {
        QDate d = today.addDays(i);
        if (!seancesByDate.contains(d) && weatherCache.contains(d)) {
            QTextCharFormat fmt;
            fmt.setToolTip("🌤️ MÉTÉO:\n" + weatherCache[d]);
            ui->calendarSeances->setDateTextFormat(d, fmt);
        }
    }
}

void AutoEcole::onDateChanged(const QDate &date)
{
    Seance s;  // Create Seance object
    QList<Seance> list = s.getSeancesByDate(date);

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(6);
    model->setHorizontalHeaderLabels({
        "ID Séance", "Heure", "ID Client", "ID Employé", "Matricule", "Date"
    });

    // Display weather info for selected date
    if (weatherCache.contains(date)) {
        qDebug() << "Weather for" << date << ":" << weatherCache[date];
    }

    if (list.isEmpty()) {
        ui->tableseance->setModel(model);
        return;
    }

    int row = 0;
    for (const Seance &seance : list) {
        model->setItem(row, 0, new QStandardItem(QString::number(seance.getIdSeance())));
        model->setItem(row, 1, new QStandardItem(seance.getHeure().toString("HH:mm")));
        model->setItem(row, 2, new QStandardItem(QString::number(seance.getIdClient())));
        model->setItem(row, 3, new QStandardItem(QString::number(seance.getIdEmploye())));
        model->setItem(row, 4, new QStandardItem(QString::number(seance.getMatricule())));
        model->setItem(row, 5, new QStandardItem(date.toString("dd/MM/yyyy")));
        row++;
    }

    ui->tableseance->setModel(model);
    ui->tableseance->resizeColumnsToContents();
}

// ==================== CALENDAR NAVIGATION ====================

void AutoEcole::on_gocalendar_clicked()
{
    // Fetch fresh weather data
    QDate today = QDate::currentDate();
    for (int i = 0; i < 14; i++) {
        QDate d = today.addDays(i);
        if (!weatherCache.contains(d)) {
            fetchWeatherForDate(d);
        }
    }

    // Refresh calendar with latest data
    colorizeCalendar();
    onDateChanged(QDate::currentDate());
    ui->stackedWidget->setCurrentWidget(ui->w_calendar);
}

void AutoEcole::on_annulercal_clicked()
{
    ui->stackedWidget->setCurrentIndex(7);  // Return to seance page
}


void AutoEcole::afficherMapVehicule()
{
    // Créer une fenêtre pour la carte
    mapWindow = new QWebEngineView();

    // Créer le HTML de la carte avec les données JSON
    QString htmlContent = R"(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Carte des Véhicules - Smart Driving School</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.9.4/leaflet.min.css" />
    <script src="https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.9.4/leaflet.min.js"></script>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Poppins', sans-serif; background: linear-gradient(135deg, #1a1f4d 0%, #2d2f5f 100%); height: 100vh; overflow: hidden; }
        .container { display: flex; height: 100vh; gap: 10px; padding: 10px; }
        .map-section { flex: 1; border-radius: 12px; overflow: hidden; box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3); }
        #map { width: 100%; height: 100%; }
        .sidebar { width: 320px; background: rgba(255, 255, 255, 0.95); border-radius: 12px; padding: 20px; overflow-y: auto; box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3); display: flex; flex-direction: column; }
        .sidebar h2 { color: #1a1f4d; margin-bottom: 15px; font-size: 18px; border-bottom: 2px solid #FF9500; padding-bottom: 10px; }
        .filter-group { margin-bottom: 15px; }
        .filter-group label { display: block; font-size: 12px; color: #666; margin-bottom: 5px; font-weight: 600; }
        .filter-group select { width: 100%; padding: 8px 10px; border: 1.5px solid #ddd; border-radius: 6px; font-size: 13px; background: white; cursor: pointer; }
        .vehicle-info { background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%); border-radius: 8px; padding: 15px; margin-top: 15px; display: none; }
        .vehicle-info.active { display: block; }
        .vehicle-info h3 { color: #1a1f4d; margin-bottom: 10px; font-size: 15px; }
        .vehicle-info-item { display: flex; justify-content: space-between; margin-bottom: 8px; font-size: 12px; }
        .vehicle-info-item strong { color: #1a1f4d; }
        .vehicle-info-item span { color: #FF9500; font-weight: 600; }
        .vehicle-list { flex: 1; overflow-y: auto; margin-top: 10px; }
        .vehicle-item { background: white; border: 1px solid #e0e0e0; border-radius: 6px; padding: 10px; margin-bottom: 8px; cursor: pointer; border-left: 4px solid #FF9500; }
        .vehicle-item:hover { box-shadow: 0 4px 12px rgba(255, 149, 0, 0.2); }
        .vehicle-item.active { background: #FFF3E0; border-left-color: #FF6B00; }
        .vehicle-item-matricule { font-weight: 700; color: #1a1f4d; font-size: 13px; margin-bottom: 3px; }
        .vehicle-item-info { font-size: 11px; color: #666; }
        .badge { display: inline-block; background: #FF9500; color: white; padding: 2px 6px; border-radius: 3px; font-size: 10px; margin-right: 5px; }
        .stats { background: linear-gradient(135deg, #1a1f4d 0%, #2d2f5f 100%); color: white; padding: 12px; border-radius: 6px; margin-bottom: 15px; text-align: center; }
        .stats-value { font-size: 24px; font-weight: 700; color: #FF9500; }
        .stats-label { font-size: 11px; margin-top: 3px; }
        .popup-content { font-size: 12px; color: #333; max-height: 300px; overflow-y: auto; }
        .popup-content strong { color: #1a1f4d; display: block; margin-top: 5px; }
        .location-vehicles { margin-top: 10px; border-top: 1px solid #eee; padding-top: 10px; }
        .location-vehicle-item { background: #f8f9fa; padding: 8px; margin: 5px 0; border-radius: 4px; border-left: 3px solid #FF9500; }

        /* Styles pour les marqueurs groupés */
        .cluster-marker {
            background: #FF9500;
            border: 3px solid white;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.3);
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
            font-size: 14px;
        }
        .cluster-marker.tunis { background: #0C2A6E; }
        .cluster-marker.sousse { background: #5FB6ED; }
        .cluster-marker.bizerte { background: #3A3F47; }
    </style>
</head>
<body>
    <div class="container">
        <div class="map-section"><div id="map"></div></div>
        <div class="sidebar">
            <h2>🚗 Véhicules par Localisation</h2>
            <div class="stats">
                <div class="stats-value" id="totalVehicles">0</div>
                <div class="stats-label">Véhicules totaux</div>
            </div>
            <div class="filter-group">
                <label>Filtrer par type:</label>
                <select id="typeFilter">
                    <option value="">Tous les types</option>
                    <option value="Manuelle">Manuelle</option>
                    <option value="Automatique">Automatique</option>
                    <option value="Moto">Moto</option>
                </select>
            </div>
            <div class="vehicle-info" id="vehicleInfo">
                <h3>Détails du véhicule</h3>
                <div class="vehicle-info-item"><strong>Matricule:</strong><span id="infoMatricule">-</span></div>
                <div class="vehicle-info-item"><strong>Marque:</strong><span id="infoMarque">-</span></div>
                <div class="vehicle-info-item"><strong>Modèle:</strong><span id="infoModele">-</span></div>
                <div class="vehicle-info-item"><strong>Type:</strong><span id="infoType">-</span></div>
                <div class="vehicle-info-item"><strong>Kilométrage:</strong><span id="infoKilometrage">-</span></div>
                <div class="vehicle-info-item"><strong>Localisation:</strong><span id="infoLocalisation">-</span></div>
            </div>
            <div class="vehicle-list" id="vehicleList"></div>
        </div>
    </div>
    <script>
        const map = L.map('map').setView([36.3932, 10.1592], 8);
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '© OpenStreetMap contributors', maxZoom: 19
        }).addTo(map);

        let vehicles = %VEHICLES_JSON%;
        let markers = {};
        let selectedVehicleId = null;

        // Fonction pour regrouper les véhicules par localisation
        function groupVehiclesByLocation() {
            const grouped = {};
            vehicles.forEach(vehicle => {
                const location = vehicle.localisation;
                if (!grouped[location]) {
                    grouped[location] = {
                        vehicles: [],
                        latitude: vehicle.latitude,
                        longitude: vehicle.longitude
                    };
                }
                grouped[location].vehicles.push(vehicle);
            });
            return grouped;
        }

        // Fonction pour créer un marqueur groupé
        function createClusterIcon(location, count) {
            let className = 'cluster-marker';
            if (location.toLowerCase() === 'tunis') className += ' tunis';
            else if (location.toLowerCase() === 'sousse') className += ' sousse';
            else if (location.toLowerCase() === 'bizerte') className += ' bizerte';

            return L.divIcon({
                className: className,
                html: `<div style="display: flex; align-items: center; justify-content: center; width: 100%; height: 100%;">${count}</div>`,
                iconSize: [40, 40],
                iconAnchor: [20, 20]
            });
        }

        function renderMap() {
            Object.values(markers).forEach(marker => map.removeLayer(marker));
            markers = {};
            const typeFilter = document.getElementById('typeFilter').value;

            // Grouper les véhicules par localisation
            const groupedVehicles = groupVehiclesByLocation();

            Object.keys(groupedVehicles).forEach(location => {
                const group = groupedVehicles[location];

                // Filtrer les véhicules dans ce groupe
                const filteredVehicles = group.vehicles.filter(vehicle =>
                    (typeFilter === '' || vehicle.type === typeFilter)
                );

                if (filteredVehicles.length > 0) {
                    const marker = L.marker([group.latitude, group.longitude], {
                        icon: createClusterIcon(location, filteredVehicles.length)
                    }).addTo(map);

                    // Créer le contenu du popup avec tous les véhicules de cette localisation
                    let popupContent = `<div class="popup-content">
                        <strong style="font-size: 14px; color: #1a1f4d; margin-bottom: 10px; display: block;">
                            📍 ${location} - ${filteredVehicles.length} véhicule(s)
                        </strong>
                        <div class="location-vehicles">`;

                    filteredVehicles.forEach(vehicle => {
                        popupContent += `
                            <div class="location-vehicle-item">
                                <strong>Matricule: ${vehicle.matricule}</strong>
                                <strong>${vehicle.marque} ${vehicle.modele}</strong>
                                <strong>Type: ${vehicle.type}</strong>
                                <strong>Kilométrage: ${vehicle.kilometrage} km</strong>
                            </div>`;
                    });

                    popupContent += `</div></div>`;
                    marker.bindPopup(popupContent);

                    // Stocker le marqueur
                    markers[location] = marker;
                }
            });
        }

        function renderVehicleList() {
            const list = document.getElementById('vehicleList');
            list.innerHTML = '';
            const typeFilter = document.getElementById('typeFilter').value;

            // Grouper les véhicules par localisation pour l'affichage
            const groupedVehicles = groupVehiclesByLocation();

            Object.keys(groupedVehicles).forEach(location => {
                const group = groupedVehicles[location];
                const filteredVehicles = group.vehicles.filter(vehicle =>
                    (typeFilter === '' || vehicle.type === typeFilter)
                );

                if (filteredVehicles.length > 0) {
                    // Créer un item de groupe pour la localisation
                    const groupItem = document.createElement('div');
                    groupItem.className = 'vehicle-item';
                    groupItem.innerHTML = `
                        <div class="vehicle-item-matricule">📍 ${location}</div>
                        <div class="vehicle-item-info">
                            <span class="badge">${filteredVehicles.length} véhicule(s)</span>
                        </div>
                    `;
                    groupItem.addEventListener('click', () => {
                        // Centrer la carte sur cette localisation
                        if (markers[location]) {
                            map.setView([group.latitude, group.longitude], 12);
                            markers[location].openPopup();
                        }
                    });
                    list.appendChild(groupItem);

                    // Ajouter chaque véhicule de cette localisation
                    filteredVehicles.forEach(vehicle => {
                        const item = document.createElement('div');
                        item.className = 'vehicle-item' + (selectedVehicleId === vehicle.matricule ? ' active' : '');
                        item.style.marginLeft = '20px';
                        item.style.borderLeftColor = '#ccc';
                        item.innerHTML = `
                            <div class="vehicle-item-matricule">MAT: ${vehicle.matricule}</div>
                            <div class="vehicle-item-info">
                                <span class="badge">${vehicle.type}</span>
                                <span>${vehicle.marque} ${vehicle.modele}</span>
                            </div>
                            <div class="vehicle-item-info" style="color: #666; margin-top: 3px;">
                                ${vehicle.kilometrage} km
                            </div>
                        `;
                        item.addEventListener('click', (e) => {
                            e.stopPropagation();
                            selectVehicle(vehicle.matricule);
                        });
                        list.appendChild(item);
                    });
                }
            });
        }

        function selectVehicle(matricule) {
            selectedVehicleId = matricule;
            const vehicle = vehicles.find(v => v.matricule === matricule);
            if (vehicle) {
                document.getElementById('vehicleInfo').classList.add('active');
                document.getElementById('infoMatricule').textContent = vehicle.matricule;
                document.getElementById('infoMarque').textContent = vehicle.marque;
                document.getElementById('infoModele').textContent = vehicle.modele;
                document.getElementById('infoType').textContent = vehicle.type;
                document.getElementById('infoKilometrage').textContent = vehicle.kilometrage + ' km';
                document.getElementById('infoLocalisation').textContent = vehicle.localisation;

                // Centrer la carte sur le véhicule sélectionné
                if (vehicle.latitude && vehicle.longitude) {
                    map.setView([vehicle.latitude, vehicle.longitude], 12);
                }
            }
            renderVehicleList();
        }

        function updateStats() {
            const typeFilter = document.getElementById('typeFilter').value;
            const filtered = vehicles.filter(v =>
                (typeFilter === '' || v.type === typeFilter)
            );
            document.getElementById('totalVehicles').textContent = filtered.length;
        }

        document.getElementById('typeFilter').addEventListener('change', () => {
            renderMap();
            renderVehicleList();
            updateStats();
            selectedVehicleId = null;
            document.getElementById('vehicleInfo').classList.remove('active');
        });

        // Initialisation
        renderMap();
        renderVehicleList();
        updateStats();
    </script>
</body>
</html>
)";

    // Récupérer les véhicules en JSON
    Vehicule v;
    QString vehiclesJson = v.getVehiclesAsJSON();

    // Remplacer le placeholder par les vraies données
    htmlContent.replace("%VEHICLES_JSON%", vehiclesJson);

    // Charger le contenu HTML
    mapWindow->setHtml(htmlContent);
    mapWindow->setWindowTitle("Carte des Véhicules - Smart Driving School");
    mapWindow->resize(1200, 700);
    mapWindow->show();
}
void AutoEcole::on_btn_mapVehicule_clicked()
{
    afficherMapVehicule();
}



// ===================== NAVIGATION ==================
//-----accueil-----

void AutoEcole::on_pushButton_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton1_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton2_clicked(){ ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton3_clicked(){ ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton4_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton5_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton126_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton127_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton128_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton129_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton130_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton184_clicked(){ui->stackedWidget->setCurrentIndex(0);}

//-----client-----

void AutoEcole::on_pushButton6_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton7_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton8_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton9_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton10_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton11_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton183_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton131_clicked(){ui->stackedWidget->setCurrentIndex(18);}
void AutoEcole::on_pushButton136_clicked(){ui->stackedWidget->setCurrentIndex(19);}
void AutoEcole::on_pushButton132_clicked(){ui->stackedWidget->setCurrentIndex(17);}

//-----employé-----

void AutoEcole::on_pushButton12_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton13_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton14_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton15_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton16_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton17_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton182_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton135_clicked(){ui->stackedWidget->setCurrentIndex(6);}

//-----examen-----

void AutoEcole::on_pushButton18_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton19_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton20_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton21_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton22_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton23_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton181_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton137_clicked(){ui->stackedWidget->setCurrentIndex(16);}
void AutoEcole::on_pushButton138_clicked(){ui->stackedWidget->setCurrentIndex(15);}
void AutoEcole::on_pushButton139_clicked(){ui->stackedWidget->setCurrentIndex(14);}

//-----stat-seance-----

void AutoEcole::on_pushButton24_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton25_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton26_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton27_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton28_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton29_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton143_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton180_clicked(){ui->stackedWidget->setCurrentIndex(0);}

//-----seance-----

void AutoEcole::on_pushButton30_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton31_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton32_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton33_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton34_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton35_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton179_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton144_clicked(){ui->stackedWidget->setCurrentIndex(8);}
void AutoEcole::on_pushButton146_clicked(){ui->stackedWidget->setCurrentIndex(9);}
void AutoEcole::on_pushButton145_clicked(){ui->stackedWidget->setCurrentIndex(10);}

//----calendrier-----
void AutoEcole::on_pushButton184_2_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton_11_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton1_5_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton2_5_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton3_5_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton4_5_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton5_5_clicked(){ui->stackedWidget->setCurrentIndex(11);}

//--------stat-client----------

void AutoEcole::on_pushButton36_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton37_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton38_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton39_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton40_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton41_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton178_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton147_clicked(){ui->stackedWidget->setCurrentIndex(12);}

//--------stat-employé---------

void AutoEcole::on_pushButton42_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton43_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton44_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton45_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton46_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton47_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton177_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton148_clicked(){ui->stackedWidget->setCurrentIndex(3);}

//--------stat-examen----------

void AutoEcole::on_pushButton48_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton49_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton50_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton51_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton52_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton53_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton176_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton149_clicked(){ui->stackedWidget->setCurrentIndex(11);}

//--------stat-vehicule----------

void AutoEcole::on_pushButton54_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton55_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton56_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton57_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton58_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton59_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton175_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton150_clicked(){ui->stackedWidget->setCurrentIndex(13);}


//--------vehicule----------

void AutoEcole::on_pushButton120_clicked(){ui->stackedWidget->setCurrentIndex(2);}
void AutoEcole::on_pushButton121_clicked(){ui->stackedWidget->setCurrentIndex(3);}
void AutoEcole::on_pushButton122_clicked(){ui->stackedWidget->setCurrentIndex(12);}
void AutoEcole::on_pushButton123_clicked(){ui->stackedWidget->setCurrentIndex(13);}
void AutoEcole::on_pushButton124_clicked(){ui->stackedWidget->setCurrentIndex(7);}
void AutoEcole::on_pushButton125_clicked(){ui->stackedWidget->setCurrentIndex(11);}
void AutoEcole::on_pushButton164_clicked(){ui->stackedWidget->setCurrentIndex(0);}
void AutoEcole::on_pushButton161_clicked(){ui->stackedWidget->setCurrentIndex(21);}
void AutoEcole::on_pushButton163_clicked(){ui->stackedWidget->setCurrentIndex(20);}
void AutoEcole::on_pushButton162_clicked(){ui->stackedWidget->setCurrentIndex(22);}

//--------login----------

void AutoEcole::on_pushButton142_clicked(){ui->stackedWidget->setCurrentIndex(1);}
//void AutoEcole::on_pushButton_login_clicked(){ui->stackedWidget->setCurrentIndex(2);}

//--------forget-password----------

void AutoEcole::on_pushButton141_clicked(){ui->stackedWidget->setCurrentIndex(0);}
