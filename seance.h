#ifndef SEANCE_H
#define SEANCE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>
#include <QTime>
#include <QMap>
#include <QStringList>
#include <QList>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QBarCategoryAxis>

class Seance
{
    int id_seance, id_client, id_employe, matricule;
    QDate date_seance;
    QTime heure;

public:
    // Récupération des séances
    QList<Seance> getAllSeances();
    QList<Seance> getSeancesByDate(const QDate &date);

    // Constructeurs
    Seance() {}
    Seance(int id, QDate date, QTime heure, int idClient, int idEmploye, int matricule);

    // Getters
    int getIdSeance() const;
    QDate getDateSeance() const;
    QTime getHeure() const;
    int getIdClient() const;
    int getIdEmploye() const;
    int getMatricule() const;

    // Setters
    void setIdSeance(int id) { id_seance = id; }
    void setDateSeance(QDate d) { date_seance = d; }
    void setHeure(QTime h) { heure = h; }
    void setIdClient(int id) { id_client = id; }
    void setIdEmploye(int id) { id_employe = id; }
    void setMatricule(int m) { matricule = m; }

    // CRUD
    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* afficher();

    // Rechercher
    QSqlQueryModel* rechercher(const QString &critere);

    // Tri
    QSqlQueryModel* trier(const QString &critere);

    // Export PDF
    bool exporterPDF(const QString &filePath);

    // ========== STATISTIQUES AMÉLIORÉES ==========

    // Statistiques par période
    QSqlQueryModel* statistiquesParPeriode(const QString &periode);

    // Statistiques avancées (retourne map avec différentes métriques)
    QMap<QString, int> statsAvancees();

    // Génération de graphiques
    QChartView* genererGraphiqueBarres(const QString &periode);
    QChartView* genererGraphiqueLigne(const QString &periode);

    // Méthodes compatibles avec l'ancien code
    void statsParJour(QStringList &labels, QList<int> &counts);
    void statsParMois(QStringList &labels, QList<int> &counts);
    QChartView* genererCourbe(const QStringList &labels,
                              const QList<int> &counts,
                              const QString &titre);

    // Calendar et noms client/employé
    QString getClientName(int idClient);
    QString getEmployeName(int idEmploye);
    QMap<QDate, QList<Seance>> groupSeancesByDate();
    QMap<QDate, int> getSeancesCountByDate();
};

#endif // SEANCE_H
