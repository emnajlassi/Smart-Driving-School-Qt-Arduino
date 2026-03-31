#ifndef CLIENT_H
#define CLIENT_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QSqlRecord>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
class Client
{
    int id_client;
    QString nom, prenom, niveau, email, telephone;

public:
    //constructeurs
    Client() {}
    Client(int id, QString nom, QString prenom, QString niveau, QString telephone, QString email);

    // getters
    QString getNom(){ return nom;}
    QString getPrenom(){ return prenom;}
    QString getNiveau(){ return niveau;}
    QString getEmail(){ return email;}
    int getID(){ return id_client;}
    QString getTelephone(){ return telephone;}

    //Setters
    void setNom(QString n){nom=n;}
    void setPrenom(QString p){prenom=p;}
    void setSpecialite(QString niv){niveau=niv;}
    void setPassword(QString e){email=e;}
    void setID(int id){id_client=id;}
    void setTelephone(QString tele){telephone=tele;}

    // CRUD
    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* afficher();

    //recherche
    QSqlQueryModel* rechercher(const QString &critere);
    //tri
    QSqlQueryModel* trier(const QString &critere);
    //export-pdf
    bool exporterPDF(const QString &filePath);
    //stat
    QSqlQueryModel* statistiquesParNiveau();

    // QR CODE FUNCTIONS
    QPixmap genererVraiQRCode(int size = 300) const;
    bool genererBitakitTaarifPDF(const QString &filePath) const;

    // ADDED: Email function
    bool envoyerEmailConfirmation() const;  // Changé le nom
    //métier-avancé-examen
    // métier
    QSqlQueryModel* identifierClientsEnDifficulte();

};
#endif // CLIENT_H
