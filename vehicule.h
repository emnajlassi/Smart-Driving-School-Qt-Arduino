#ifndef VEHICULE_H
#define VEHICULE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

class Vehicule
{
    int matricule;
    QString marque, modele, type, kilometrage, localisation;

public:
    Vehicule() {}
    Vehicule(int matricule, QString marque, QString modele, QString type, QString kilometrage, QString localisation);

    // Getters
    int getMatricule() { return matricule; }
    QString getMarque() { return marque; }
    QString getModele() { return modele; }
    QString getType() { return type; }
    QString getKilometrage() { return kilometrage; }
    QString getLocalisation() { return localisation; }

    // Setters
    void setMatricule(int m) { matricule = m; }
    void setMarque(QString ma) { marque = ma; }
    void setModele(QString mo) { modele = mo; }
    void setType(QString t) { type = t; }
    void setKilometrage(QString k) { kilometrage = k; }
    void setLocalisation(QString l) { localisation = l; }

    // CRUD
    bool ajouter();
    bool supprimer(int matricule);
    bool modifier(int matricule);
    QSqlQueryModel* afficher();

    // Recherche
    QSqlQueryModel* rechercher(const QString &critere);

    // Tri
    QSqlQueryModel* trier(const QString &critere);

    // Export PDF
    bool exporterPDF(const QString &filePath);

    // Statistiques
    QSqlQueryModel* statistiquesVehicule();
    QChartView* genererDonut(QSqlQueryModel* model);

    // Conversion JSON pour la carte
    QString getVehiclesAsJSON();
};

#endif // VEHICULE_H
