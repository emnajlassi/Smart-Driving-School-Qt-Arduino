#ifndef EMPLOYE_H
#define EMPLOYE_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>


class employe
{
    QString nom, prenom, specialite, password, telephone, question , reponse, poste;
    int id_employe;

public:
    //constructeurs
    employe(){}
    employe (int id, QString nom, QString prenom, QString telephone, QString password, QString specialite, QString question, QString reponse, QString poste);

    // getters
    QString getNom(){ return nom;}
    QString getPrenom(){ return prenom;}
    QString getSpecialite(){ return specialite;}
    QString getPassword(){ return password;}
    QString getQuestion(){ return question;}
    QString getReponse(){ return reponse;}
    QString getPoste(){ return poste;}
    int getID(){ return id_employe;}
    QString getTelephone(){ return telephone;}

    //Setters
    void setNom(QString n){nom=n;}
    void setPrenom(QString p){prenom=p;}
    void setSpecialite(QString s){specialite=s;}
    void setPassword(QString pass){password=pass;}
    void setQuestion(QString q){question=q;}
    void setReponse(QString r){reponse=r;}
    void setPoste(QString pos){question=pos;}
    void setID(int id){id_employe=id;}
    void setTelephone(QString tele){telephone=tele;}

    // CRUD
    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* afficher();


    //login
    //bool verifierLogin(int id, QString password);
    /*bool verifierInfosOublie(int id, QString nom, QString prenom, QString telephone, QString specialite);*/
    QString verifierLoginNom(int id, QString password);
    bool doesEmployeeExist(int id);
    QString getSecurityQuestion(int id);
    bool verifyQuestion(int id, QString question);
    bool verifyAnswer(int id, QString question, QString answer);
    bool updatePassword(int id, QString newPassword);
    QString getPosteById(int id);


    //recherche
    QSqlQueryModel* rechercher(const QString &key);
    //tri
    QSqlQueryModel* trier(const QString &critere);
    //export pdf
    bool exporterPDF(const QString &filePath);
    //stat
    QSqlQueryModel* statistiquesParSpecialite() const;


};

#endif // EMPLOYE_H
