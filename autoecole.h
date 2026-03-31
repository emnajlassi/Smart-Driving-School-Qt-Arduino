#ifndef AUTOECOLE_H
#define AUTOECOLE_H
#include "employe.h"
#include <client.h>
#include <examen.h>
#include <vehicule.h>
#include <seance.h>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include "qlabel.h"
#include "seance.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPdfWriter>
#include <QWebEngineView>
#include <QList>
#include <QString>
#include <QDialog>
#include <QListWidget>
#include "arduino.h" // N'oubliez pas l'include!
#include <QSqlError>     // Ajout essentiel pour gérer les erreurs SQL


QT_BEGIN_NAMESPACE
namespace Ui {
class AutoEcole;
}
QT_END_NAMESPACE

class AutoEcole : public QMainWindow
{
    Q_OBJECT

public:
    AutoEcole(QWidget *parent = nullptr);
    ~AutoEcole();
    void afficherStatsEmploye();
    void afficherStatsExamen();
    void afficherStatsVehicule();
    void updateNavigationButtons(const QString &poste);
    void afficherMapVehicule();





private slots:
    //arduino
    void on_bouton_connect_arduino_clicked(); // Slot pour le bouton de connexion UI
    void lireDonneesArduino(); // Slot qui gère la communication série
    void update_arduino();
    //----------------------CRUD-EMPLOYE--------------------
    //ajouter
    void on_ajoutemploye_clicked();
    //supprimer
    void on_suppemploye_clicked();
    //modifier
    void on_updateemploye_clicked();
    //chercher
    void on_lineEdit_rechercheEmploye_textChanged(const QString &text);
    //tri
    void on_comboBox_triEmploye_currentIndexChanged(const QString &text);
    //export-pdf
    void on_btn_exportEmployePDF_clicked();
    //stat
    void on_pushButton_StatistiquesEmploye_clicked();
    //tab_select_employe
    void on_tableemploye_clicked(const QModelIndex &index);

//--------------------CRUD-CLIENT-----------------------
    //ajouter
    void on_pushButton_ajoutClient_clicked();
    //supprimer
    void on_pushButton_supprimerClient_clicked();
    //modifier
    void on_pushButton_modifierClient_clicked();
    //recherche
    void on_lineEdit_rechercheClient_textChanged(const QString &text);
    //tri
    void on_comboBox_triClient_currentIndexChanged(const QString &text);
    //export-pdf
    void on_btn_exportClientPDF_clicked();
    //stat
    void on_btnStatClient_clicked();
    void afficherStatsClients();
    //tab_select-client
    void on_tableClient_clicked(const QModelIndex &index);
//--------------------CRUD-EXAMEN-----------------------
    //ajouter
    void on_pushButton_ajouterExamen_clicked();
    //supprimer
    void on_supprimerExamen_clicked();
    //modifier
    void on_modifierExamen_clicked();
    //recherche
    void on_lineEdit_rechercheExamen_textChanged(const QString &text);
    //tri
    void on_comboBox_triExamen_currentIndexChanged(const QString &text);
    //export-pdf
    void on_btn_exportExamenPDF_clicked();
    //stat
    void on_btnStatExamen_clicked();
    // Bouton Exams en Retard
    void on_pushButton_exams_en_retard_clicked();
    void on_pushButton_67_clicked();
    void on_pushButton_68_clicked();
    //tab_select_examen
    void on_tableExamen_clicked(const QModelIndex &index);
//--------------------CRUD-VEHICULE-----------------------
    //ajouter
    void on_ajouterVehicule_clicked();
    //supprimer
    void on_supprimerVehicule_clicked();
    //modifier
    void on_modifierVehicule_clicked();
    //recherche
    void on_lineEdit_rechercheVehicule_textChanged(const QString &text);
    //tri
    void on_comboBox_triVehicule_currentIndexChanged(const QString &text);
    //export-pdf
    void on_btn_exportVehiculePDF_clicked();
    //stat
    void on_btnStatVehicule_clicked();
    void on_btnHistorique_clicked(); // <-- AJOUTER CELLE-CI
    void on_btn_mapVehicule_clicked(); // <-- AJOUTER CELLE-CI
    //tab_select_vehicule
    void on_tableVehicule_clicked(const QModelIndex &index);


    //--------------------CRUD-SEANCE-----------------------
    //ajouter
    void on_ajouterSeance_clicked();
    //supprimer
    void on_supprimerSeance_clicked();
    //modifier
    void on_modifierSeance_clicked();
    //recherche
    void on_lineEdit_rechercheSeance_textChanged(const QString &text);
    //tri
    void on_comboBox_triSeance_currentIndexChanged(const QString &text);
    //export-pdf
    void on_btn_exportSeancePDF_clicked();
    //stat
    void afficherStatsSeance(const QString &mode);
    void on_btnStatJour_clicked();
    void on_btnStatMois_clicked();
    // ========== AJOUTER CES TROIS LIGNES ==========
    void on_btnStatSemaine_clicked();
    //tab_select_seance
    void on_tableseance_clicked(const QModelIndex &index);



    //---------------------------------------------------NAVIGATION-----------------------------------------------------------
    //------------accueil-----------
    void on_pushButton_clicked();
    void on_pushButton1_clicked();
    void on_pushButton2_clicked();
    void on_pushButton3_clicked();
    void on_pushButton4_clicked();
    void on_pushButton5_clicked();
    void on_pushButton126_clicked();
    void on_pushButton127_clicked();
    void on_pushButton128_clicked();
    void on_pushButton129_clicked();
    void on_pushButton130_clicked();
    void on_pushButton184_clicked();
    //-----------client-------------
    void on_pushButton6_clicked();
    void on_pushButton7_clicked();
    void on_pushButton8_clicked();
    void on_pushButton9_clicked();
    void on_pushButton10_clicked();
    void on_pushButton11_clicked();
    void on_pushButton183_clicked();
    void on_pushButton131_clicked();
    void on_pushButton136_clicked();
    void on_pushButton132_clicked();
    //-----------employé-----------
    void on_pushButton12_clicked();
    void on_pushButton13_clicked();
    void on_pushButton14_clicked();
    void on_pushButton15_clicked();
    void on_pushButton16_clicked();
    void on_pushButton17_clicked();
    void on_pushButton182_clicked();
    void on_pushButton135_clicked();
    //-----------examen-------------
    void on_pushButton18_clicked();
    void on_pushButton19_clicked();
    void on_pushButton20_clicked();
    void on_pushButton21_clicked();
    void on_pushButton22_clicked();
    void on_pushButton23_clicked();
    void on_pushButton181_clicked();
    void on_pushButton137_clicked();
    void on_pushButton138_clicked();
    void on_pushButton139_clicked();
    //-------stat-seance-------
    void on_pushButton24_clicked();
    void on_pushButton25_clicked();
    void on_pushButton26_clicked();
    void on_pushButton27_clicked();
    void on_pushButton28_clicked();
    void on_pushButton29_clicked();
    void on_pushButton143_clicked();
    void on_pushButton180_clicked();
    //-----------seance------------
    void on_pushButton30_clicked();
    void on_pushButton31_clicked();
    void on_pushButton32_clicked();
    void on_pushButton33_clicked();
    void on_pushButton34_clicked();
    void on_pushButton35_clicked();
    void on_pushButton179_clicked();
    void on_pushButton144_clicked();
    void on_pushButton146_clicked();
    void on_pushButton145_clicked();
    //--------stat-client----------
    void on_pushButton36_clicked();
    void on_pushButton37_clicked();
    void on_pushButton38_clicked();
    void on_pushButton39_clicked();
    void on_pushButton40_clicked();
    void on_pushButton41_clicked();
    void on_pushButton178_clicked();
    void on_pushButton147_clicked();
    //--------stat-employé---------
    void on_pushButton42_clicked();
    void on_pushButton43_clicked();
    void on_pushButton44_clicked();
    void on_pushButton45_clicked();
    void on_pushButton46_clicked();
    void on_pushButton47_clicked();
    void on_pushButton177_clicked();
    void on_pushButton148_clicked();
    //--------stat-examen----------
    void on_pushButton48_clicked();
    void on_pushButton49_clicked();
    void on_pushButton50_clicked();
    void on_pushButton51_clicked();
    void on_pushButton52_clicked();
    void on_pushButton53_clicked();
    void on_pushButton176_clicked();
    void on_pushButton149_clicked();
    //--------stat-vehicule---------
    void on_pushButton54_clicked();
    void on_pushButton55_clicked();
    void on_pushButton56_clicked();
    void on_pushButton57_clicked();
    void on_pushButton58_clicked();
    void on_pushButton59_clicked();
    void on_pushButton175_clicked();
    void on_pushButton150_clicked();
    //-----calendrier-----
    void on_pushButton184_2_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton1_5_clicked();
    void on_pushButton2_5_clicked();
    void on_pushButton3_5_clicked();
    void on_pushButton4_5_clicked();
    void on_pushButton5_5_clicked();
    //-----------vehicule-----------
    void on_pushButton120_clicked();
    void on_pushButton121_clicked();
    void on_pushButton122_clicked();
    void on_pushButton123_clicked();
    void on_pushButton124_clicked();
    void on_pushButton125_clicked();
    void on_pushButton164_clicked();
    void on_pushButton161_clicked();
    void on_pushButton163_clicked();
    void on_pushButton162_clicked();
    //-----------login-------------
    void on_pushButton142_clicked();
    void on_pushButton_login_clicked();
    //--------forget-password-------
    void on_pushButton141_clicked();
    void on_pushButton_confirmanswer_clicked();
    void on_pushButton_changepassword_clicked();
    void on_pushButton_forget_password_clicked();
    void on_pushButton_confirmquestion_clicked();
    //*****************************************************************************
    void on_pushButton_confirmid_clicked();
    //-----------cazlendar seance--------------
    void onDateChanged(const QDate &date);
    void on_gocalendar_clicked();
    void on_annulercal_clicked();
    //void on_pushButton_10_clicked();
    //void on_pushButton1_4_clicked();
    //void on_pushButton2_4_clicked();
    //void on_pushButton3_4_clicked();
    //void on_pushButton4_4_clicked();
    //void on_pushButton5_4_clicked();
    //---------------------------weather----------------------
    void onWeatherDataReceived(QNetworkReply *reply);
    QString getWeatherCondition(int code);
    void updateCalendarWithWeather();
    // CLIENTUNO FUNCTION - CORRIGÉ LE NOM DU BOUTON
    void on_btnClientUno_clicked();




private:
    Ui::AutoEcole *ui;
    employe emp;
    Client cl;
    Examen ex;
    Vehicule veh;
    Seance s;
    Arduino A; // L'objet Arduino
    Arduino *arduino; // NEW: Add this for the new Arduino functions
    QByteArray serialBuffer; // <--- NOUVEAU : Buffer pour les données série fragmentées

    QWebEngineView *mapWindow;

    // Historique en mémoire
    QList<QString> historiqueActions;
    void ajouterHistorique(const QString& action, int matricule, const QString& details);
    void afficherHistorique();
    QString arduinoInputBuffer;
    QDialog *arduinoInputDialog;
    QLabel *arduinoIdDisplay;  // CORRIGÉ: QLabel maintenant inclus

    void setupArduinoConnections();
    void showArduinoInputDialog();
    void onArduinoDataReceived(const QString &data);
    void onArduinoKeyPressed(const QString &key);
    void onArduinoClientIdEntered(const QString &id);
    void onArduinoClientFound(int id, const QString &nom, int sessionsCount);
    void onArduinoClientNotFound(int id);
    void onArduinoClientWithoutSessions(int id, const QString &nom);
    void onArduinoError(const QString &error);
    void onArduinoConnected(bool connected);

    // LCD Control Functions (ADD THESE)
    void sendToLCD(const QString &command);
    void updateLCDDisplay(const QString &line1, const QString &line2 = "");
    void clearLCD();
    void showLCDMessage(const QString &message);
    void scrollLCD(const QString &text);

    Seance seance;
    int currentEmployeeId;
    QString getClientName(int idClient);
    QString getEmployeName(int idClient);
    QNetworkAccessManager *networkManager;
    QMap<QDate, QString> weatherCache;
    QString getWeatherIcon(const QString &condition);
    // Weather helper functions
    void fetchWeatherForDate(const QDate &date);

    // Calendar helper
    void colorizeCalendar();

    // ========== AJOUTER CES TROIS LIGNES ==========
    QWidget* creerWidgetStatsResumees();
    void afficherTableauStatsDetaille();
    bool exporterStatistiquesPDF(const QString &filePath);




};
#endif // AUTOECOLE_H
