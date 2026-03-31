#ifndef EXAMEN_H
#define EXAMEN_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>


class Examen
{
private:
    int id_examen,id_client;
    QString type, resultat;
    QDate date_examen;

public:
    // Constructeurs
    Examen() {}
    Examen(int id_examen, int id_client, QString type, QString resultat, QDate date);

    // Getters
    int getIdExamen(){return id_examen;}
    int getIdClient(){return id_client;}
    QString getType(){return type;}
    QString getResultat(){return resultat;}
    QDate getDate(){return date_examen;}

    // Setters
    void setIdExamen(int id){id_examen = id;}
    void setIdClient(int id){id_client = id;}
    void setType(QString t){type = t;}
    void setResultat(QString r){resultat = r;}
    void setDate(QDate d){date_examen = d;}

    // CRUD
    bool ajouter();
    bool supprimer(int id_examen);
    bool modifier(int id_examen);
    QSqlQueryModel* afficher();

    //recherche
    QSqlQueryModel* rechercher(const QString &critere);
    //tri
    QSqlQueryModel* trier(const QString &critere);
    //export-pdf
    bool exporterPDF(const QString &filePath);
    //stat
    // --- Statistiques Réussite / Échec ---
    QChartView* genererDonutReussite();

    // métier
    QSqlQueryModel* detecterNonRealisesALaDatePrevue();

};

#endif // EXAMEN_H
