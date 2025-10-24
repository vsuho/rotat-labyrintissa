/* Kääntäminen g++ -w lipulla tällä hetkellä */

/* Harjoituksen 2 ohjelmarunko */
/* Tee runkoon tarvittavat muutokset kommentoiden */
/* Pistesuorituksista kooste tähän alkuun */
/* 2p = täysin tehty suoritus, 1p = osittain tehty suoritus */
/* Tarkemmat ohjeet Moodlessa */

/* --- OMA KOOSTE PISTESUORITUKSISTA --- */
/* 1) Toimiva rinnakkainen prosessitoteutus, jaettua muistia käyttäen, jossa parent odottaa kaikkien lasten pääsemistä suuaukolle. 2p */
/* 2) Toimiva rinnakkainen säietoteutus. 2p */
/* 3) Toimiva prosessien välinen jaetun muistin toteutus labyrintin kokoiselle rottien sijaintikartalle, toteuta eksklusiivinen kirjoitus.
En ole ihan varma onko sijaintikartta toteutettu oikein, mutta eksklusiivinen kirjoitus semaforin avulla toteutuu. 1-2p. */
/* 4) Toimiva säikeiden välinen toteutus labyrintin kokoiselle rottien sijantikartalle, toteuta eksklusiivinen kirjoitus. Samat perustelut kuin ylempänä, 1-2p. */

/* Lisäohjeita, vinkkejä ja apuja löytyy koodin joukosta */
/* OPISKELIJA: merkityt kohdat eritoten kannattaa katsoa huolella */

//peruskirjastot mitä tarvii aika lailla aina kehitystyössä
//OPISKELIJA: lisää tarvitsemasi peruskirjastot
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

//rinnakaisuuden peruskirjastot
//OPISKLEIJA: lisää tarvittaessa lisää kirjastoja, muista käyttää -pthread lippua käännöksessä ja tarvittaessa -lrt lippua myös
//Huom! Rinnakkaisuusasioista on eri versioita (c++, POSIX/pthread, SystemV)
//Kaikkien käyttö on sallittua
#include <sys/shm.h> //jaetun muistin kirjasto: shmget(), shmmat()
#include <fcntl.h> //S_IRUSR | S_IWUSR määrittelyt esim jaettua muistia varten
#include <unistd.h> //esim fork() määrittely ja muut prosessimäärittelyt
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h> //pthread perussäiemäärittelyt
using namespace std;

//kaikkialla tarvittavat tunnisteet  määritellään globaalilla alueella
//OPISKELIJA: muistele harjoituksista miten eri asiat määriteltiin
//OPISKELIJA: tehtäväsi on sijoittaa itse labyrinttikin jaettuun muistialueeseen ja käyttää sitä sieltä
//->eli:labyrintin käyttö globaalilta alueelta jaetun muistin käyttöön
//vinkki: kannattaa määritellä pointteri nimeltä labyrintti kaksiuloitteiseen taulukkoon jolloin nykytoteutus toimii sellaisenaan
//parent/main alustaa jaettun muistin (eli kirjoittaa sinne) nuo labyrintin alkiot
//poista labyrintti kokonaan globaalilta alueelta, ohjelman pitäisi toimia 
//mieti harjoituksista opitun perusteella paljonko labyrintti vähintään tarvitsee jaettua muistia
//tee kaikki ratkaisut niin että ohjelma toimii millä tahansa labyrintilla

//definet voi jättää globaalille alueelle, ne on sitten tiedossa koko tiedostossa
#define KORKEUS 100 //rivien määrä alla
#define LEVEYS 100 //sarakkaiden määrä alla
pthread_mutex_t säieLukko = PTHREAD_MUTEX_INITIALIZER;

//apuja: voit testata ratkaisujasi myös alla olevalla yksinkertaisemmalla labyrintilla 
//#define KORKEUS 7
//#define LEVEYS 7
int alkupLabyrintti[KORKEUS][LEVEYS] = {
    {1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,2,0,2,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,2,0,0,1,0,0,0,1,0,0,2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,1},
    {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,2,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,2,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,2,0,0,0,0,0,0,1,0,0,0,0,2,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,1},
    {1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,2,0,0,1,0,0,0,1,2,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,2,0,0,1,1},
    {1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,1},
    {1,0,0,0,1,0,1,2,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,2,0,0,1,0,0,2,0,0,1,0,0,2,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,2,0,2,0,0,0,0,1,0,1,0,0,2,0,0,0,0,1,0,0,0,1,1},
    {1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1},
    {1,0,1,0,0,0,1,0,1,0,0,2,0,0,0,0,1,0,0,0,0,0,0,2,0,2,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,2,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,2,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,2,0,0,1,0,1,0,0,0,1,1},
    {1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1},
    {1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,2,0,0,0,0,0,0,0,2,1,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,2,0,0,1,0,0,0,1,0,0,2,1,0,0,0,1,0,0,2,0,0,1,0,0,0,1,0,0,0,1,1},
    {1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,2,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,1,0,0,2,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1},
    {1,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,2,1,0,0,0,1,0,0,0,1,0,0,2,1,0,0,2,0,2,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,1,1},
    {1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1},
    {1,0,1,0,0,2,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,2,0,0,1,2,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,2,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0,1,1},
    {1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1},
    {1,2,0,0,1,0,0,0,0,0,1,2,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,2,0,0,1,0,0,0,1,0,0,2,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,2,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,2,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1},
    {1,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,2,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,2,1,2,0,0,0,0,1,0,1,0,1,1},
    {1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1},
    {1,0,1,0,1,0,1,0,0,0,0,0,0,2,1,0,1,0,0,2,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,2,0,0,1,1},
    {1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1},
    {1,2,0,0,1,0,0,0,0,2,0,0,0,0,1,0,0,2,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,2,0,0,1,0,0,2,0,0,1,0,0,0,1,0,0,2,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,2,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,2,0,0,1,0,0,0,0,2,1,2,0,0,0,0,0,0,1,1},
    {1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1},
    {1,0,1,0,0,2,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,2,1,0,0,0,1,0,0,0,1,0,0,2,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,1},
    {1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1},
    {1,0,1,0,0,0,1,0,1,0,0,2,0,0,1,0,0,0,1,2,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,2,0,0,1,0,1,0,1,0,0,0,0,2,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,1,0,0,0,0,2,1,0,1,0,0,0,1,2,0,0,1,1},
    {1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1},
    {1,0,0,0,1,0,1,0,1,0,0,2,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,2,0,0,1,0,0,2,1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,2,1,0,1,0,0,2,1,0,0,0,1,1},
    {1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1},
    {1,0,0,0,1,2,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,2,0,0,0,0,0,0,1,0,0,2,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,1},
    {1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1},
    {1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,2,0,0,0,2,0,0,1,0,0,0,1,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1},
    {1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1},
    {1,0,1,0,0,0,1,0,0,0,0,2,0,0,1,0,0,0,1,0,0,2,1,0,1,0,0,0,0,2,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,2,0,0,1,0,0,2,0,0,1,0,0,0,1,1},
    {1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1},
    {1,2,0,0,1,0,0,0,0,0,1,0,0,0,0,2,0,0,1,0,0,0,1,2,0,0,0,0,1,0,0,0,0,0,0,0,0,2,0,0,1,0,1,0,1,2,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,2,0,0,1,0,0,0,1,1},
    {1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1},
    {1,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,2,0,0,1,0,0,2,0,0,1,0,0,2,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1},
    {1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1},
    {1,0,1,0,0,2,1,2,0,0,1,0,1,0,0,0,1,0,0,2,0,0,1,0,0,0,1,0,1,0,0,2,0,0,0,0,1,0,1,2,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2,0,0,1,0,0,0,0,0,1,0,1,0,1,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1},
    {1,0,0,0,1,0,1,0,1,0,0,2,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,2,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,2,1,1},
    {1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1},
    {1,0,0,0,0,2,1,0,0,0,1,0,0,0,1,0,0,2,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,2,1,0,1,0,0,0,1,0,0,0,1,0,1,2,0,0,1,0,0,0,1,0,0,0,1,2,0,0,1,0,0,0,1,0,1,0,1,0,0,2,0,0,0,2,1,0,1,2,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1},
    {1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1},
    {1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1,2,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,2,1,0,0,0,0,0,1,0,1,2,0,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,2,0,0,1,0,0,0,1,0,0,0,1,0,1,1},
    {1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1},
    {1,0,1,0,0,2,1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,2,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,2,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,1,0,0,0,1,0,0,0,1,1},
    {1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,1,0,1,2,0,2,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,1},
    {1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,2,1,0,1,0,1,2,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,2,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,1},
    {1,0,0,0,1,0,0,2,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,2,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,2,1,0,1,0,1,1},
    {1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,2,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,2,1,0,0,2,0,2,0,2,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,2,0,0,0,0,1,0,1,2,0,0,0,0,1,0,1,0,0,2,0,0,1,0,0,0,1,2,0,0,1,0,0,2,0,0,1,0,1,1},
    {1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1},
    {1,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,2,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,2,0,0,1,0,1,0,1,0,0,0,1,2,0,0,1,0,0,0,1,0,0,0,1,0,1,2,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,2,1,1},
    {1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1},
    {1,2,0,0,1,0,1,2,0,2,0,0,1,0,1,0,0,0,0,2,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,2,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,2,1,0,1,0,1,0,1,1},
    {1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1},
    {1,0,0,0,1,0,0,2,0,0,1,0,1,0,0,2,0,0,1,0,0,0,0,0,1,0,0,0,0,2,0,0,1,0,1,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,2,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,2,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,1},
    {1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1},
    {1,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,2,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,2,0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,2,0,0,1,0,0,0,1,0,0,0,1,0,1,1},
    {1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1},
    {1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,2,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,0,2,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,1},
    {1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1},
    {1,0,1,0,0,0,1,2,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,2,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,2,0,0,1,0,0,0,0,2,1,0,0,0,0,2,1,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,1},
    {1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1},
    {1,0,1,0,0,2,1,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,2,1,0,0,0,1,0,0,2,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,0,2,0,0,1,0,1,1},
    {1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1},
    {1,0,1,0,1,0,1,0,0,2,0,0,0,0,1,0,1,2,0,0,1,2,0,0,1,0,0,2,0,0,0,0,1,0,0,0,1,0,0,2,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,2,0,0,1,0,1,0,0,0,1,0,0,0,1,1},
    {1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1},
    {1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,2,0,2,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,2,0,0,0,0,0,0,1,0,0,2,0,0,0,0,0,0,0,2,1,0,1,0,0,2,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,1},
    {1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1},
    {1,2,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,2,1,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,1,0,0,0,0,2,1,0,0,2,1,0,0,0,0,0,1,0,1,2,0,0,1,1},
    {1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1},
    {1,0,1,2,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,0,2,1,0,1,0,1,0,1,0,0,2,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,2,0,0,0,0,0,0,1,0,0,2,0,2,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1},
    {1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
    {1,0,0,0,1,0,1,0,1,0,0,2,1,0,0,2,0,0,0,0,1,0,1,0,1,0,1,2,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,2,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,1},
    {1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1},
    {1,2,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,2,1,2,0,0,0,2,0,0,0,0,1,0,0,0,0,0,1,2,0,2,0,0,0,0,1,0,0,2,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,1,2,0,2,1,1},
    {1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,1},
    {1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1},
    {1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,2,0,2,0,0,1,0,1,0,1,0,0,0,0,2,1,0,0,0,0,0,1,0,1,0,0,0,1,0,0,2,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,2,1,0,0,0,1,0,0,0,1,2,0,0,1,0,1,0,1,0,0,0,1,0,1,1},
    {1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1},
    {1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,2,0,0,1,0,0,0,1,0,0,0,0,0,0,2,0,2,0,0,1,0,0,0,1,0,0,0,0,2,0,0,1,0,0,2,0,0,1,0,0,0,0,2,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,1},
    {1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
    {1,0,1,0,1,0,1,0,1,0,0,2,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,2,0,0,0,0,1,2,0,0,1,0,0,0,1,0,0,0,0,2,1,0,0,2,0,0,1,0,0,2,0,0,1,0,1,1},
    {1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1},
    {1,0,1,0,1,0,1,0,0,0,0,0,1,2,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,2,0,2,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,1},
    {1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1},
    {1,0,0,2,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,1,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,2,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,2,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1,1},
};


/*int alkupLabyrintti[KORKEUS][LEVEYS] = {
                        {1,1,1,1,1,1,1},
                        {1,0,1,0,1,0,4},
                        {1,0,1,0,1,0,1},
                        {1,2,0,2,0,2,1},
                        {1,0,1,0,1,0,1},
                        {1,0,1,0,1,0,1},
                        {1,1,1,3,1,1,1}};
                        */
                        

const char* Semafori = "/mySem";


//karttasijainnin tallettamiseen käytettävä rakenne, luotaessa alustuu vasempaan alakulmaan
//HUOM! ykoordinaatti on peilikuva taulukon rivi-indeksiin
//PasiM: TODO, voisi yksinkertaistaa että ykoord olisi sama kuin rivi-indeksi
struct Sijainti {
    int ykoord {0};
    int xkoord {0};
};

//rotan liikkeen suunnan määrittelyyn käytettävä rakenne
//huom! suunnat ovat absoluuttisia kuin kompassissa
//UP -> ykoord++ (yindex--)
//DOWN -> ykoord-- (yindex++)
//LEFT -> xkoord--
//RIGHT -> xkoord++
//DEFAULT -> suunta tuntematon
enum LiikkumisSuunta {
    UP, DOWN, LEFT, RIGHT, DEFAULT
};

//tämän hetken toteutuksessa tämä on tarpeeton rakenne, voisi käyttää rotan omassa kartan ymmärryksen kasvatuksessa
enum Ristausve {
    WALL = 1,
    OPENING = 0
};

//tämä rakenne on jokaisesta risteyksestä jokaiseen suuntaan omansa
//VINKKI: tutkituksi merkittyyn/merkittävään suuntaan ei rotta koskaan lähde enää tutkimaan ;)
struct Suunta {
    Ristausve jatkom; //tutkituille suunnille tämä on määritelty OPENING arvolle
    bool tutkittu {false}; //alkuarvona tutkimaton
};

//TÄRKEIN kaikista rakenteista, kertoo miten risteys on opittu juuri kyseisen rotan taholta
//tutkittavana - arvo kertoo minne suuntaan juuri tämä rotta viimeksi lähtenyt ko risteyksestä tutkimaan
struct Ristaus {
    Sijainti kartalla;
    Suunta up;
    Suunta down;
    Suunta left;
    Suunta right;
    LiikkumisSuunta tutkittavana = DEFAULT; //alkuarvo, kertoo while-silmukan alussa olevalle risteyskoodille että rotta tuli ensimmäistä kertaa ko risteykseen
};

//poikkeuksenheittämistä varten, ei käytössä tällä hetkellä
//PasiM, TODO: poikkeukset
struct Karttavirhe {
    int koodi {0};
    string msg;
};

//Struct jota käytetään palauttamaan aloitaRotta() funktiosta rotan sijainnin ja liikkujen määrän
struct RottaTulos {
    int liikkuCount;
    Sijainti rotanSijainti;
};


//tämä on esittely aloitusalgoritmifunktiolle mitä siis kutsutaan oli kyseessä prosessi eli säietoteutus
//tällä hetkellä palauttaa kyseisen rotan liikkujen määrän labyrintin selvittämiseksi
//OPISKELIJA: yhtenä mielenkiintoisena haasteena voisi olla liikkujen määrän optimointi rottien yhteistyötä kehittämällä
//int aloitaRotta();

//OPISKELIJA: lisää tarvittavat muut funktioesittelyt tähän niin koodin järjestykellä tiedostossa ei ole merkitystä


//Rotta-ohjelman algoritmimäärittelyt
//näitä kutsutaan-oli rinnakkaisuus mitä vain
//OPISKELIJA: kun siirrät labyrintin jaettuun muistiin, siihen osoittavaa pointteria varmaankin pitää kuljetella niihin funktioihin missä labyrinttia tutkitaan
//VINKKI: määrittele pointteri niin että olemassa olevissa algoritmeissa oleva koodi toimii sellaisenaan

//etsii kartasta jotain spesifistä, palauttaa sen koordinaatit
Sijainti etsiKartasta(int kohde, int (*labyrintti)[LEVEYS]){
    Sijainti kartalla;
    for (int y = 0; y<KORKEUS ; y++) {
        for (int x = 0; x<LEVEYS ; x++){
            if (labyrintti[y][x] == kohde) {
                kartalla.xkoord = x;
                kartalla.ykoord = KORKEUS-1-y;
                return kartalla;
            }  
        }
    }
    return kartalla;
}

//etsitään labyrintin aloituskohta, merkitty 3:lla
Sijainti findBegin(int (*labyrintti)[LEVEYS]){
    Sijainti alkusijainti;
    alkusijainti = etsiKartasta(3, labyrintti);
    return alkusijainti;
} 

//TÄRKEÄÄ: reitti on risteyspino - sen muutokset täytyy liikkua tiedonvälityksen mukana, siksi liikkuu viittaus siihen
//OPISKELIJA: rotan liikkumislogiikkaan ei (välttämättä) tarvitse kajota, ainoastaan päätöksentekoon risteyksiin liittyen
//OPISKELIJA: väistämislogiikan tekeminen voi olla poikkeus ylläolevaan
//OHJE: taulukon indeksit ja koordinaatit ovat y-suunnassa peilikuvana
//eli: ykoordinaatti alkaa alhaalta kasvaa ylöspäin, yindeksi alkaa ylhäältä kasvaa alaspäin
//OPISKLEIJA: käytä siis labyrinttia jaetusta muistista ja tee tarvittaessa siihen liittyvät muutokset

//tutkitaan mitä nykypaikan yläpuolella on, prevDir kertoo minkä suuntainen oli viimeisin kyseisen rotan liikku
bool tutkiUp(Sijainti nykysijainti, auto& reitti, LiikkumisSuunta prevDir, int (*labyrintti)[LEVEYS]){
    int yindex = KORKEUS-1-nykysijainti.ykoord-1;
    if (yindex < 0) return false; //ulos kartalta - ei mahdollista 
    if (labyrintti[yindex][nykysijainti.xkoord] == 1) return false; //labyrintin seinä
    //tulossa uuteen ristaukseen, siihen siirrytään aina silmukan lopuksi nykytoteutuksessa
    if (labyrintti[yindex][nykysijainti.xkoord] == 2 && prevDir != DOWN) {
        Ristaus ristaus;
        ristaus.kartalla.ykoord = nykysijainti.ykoord+1;
        ristaus.kartalla.xkoord = nykysijainti.xkoord;
        ristaus.down.tutkittu = true;
        ristaus.down.jatkom = OPENING;
        reitti.push_back(ristaus); //lisätään risteys pinon päällimmäiseksi
// debuggausapuja..:
//        for (auto rist : reitti){
//            cout << "Risteys: " << rist.kartalla.ykoord << "," << rist.kartalla.xkoord << endl;
//        }
        return true;
    }
    return true;
}
//..alapuolella
bool tutkiDown(Sijainti nykysijainti, auto& reitti, LiikkumisSuunta prevDir, int (*labyrintti)[LEVEYS]){
    int yindex = KORKEUS-1-nykysijainti.ykoord+1;
    if (yindex > KORKEUS-1) return false; //ulos kartalta - ei mahdollista 
    if (labyrintti[yindex][nykysijainti.xkoord] == 1) return false;
    //tulossa uuteen ristaukseen
    if (labyrintti[yindex][nykysijainti.xkoord] == 2 && prevDir != UP){
        Ristaus ristaus;
        ristaus.kartalla.ykoord = nykysijainti.ykoord-1;
        ristaus.kartalla.xkoord = nykysijainti.xkoord;
        ristaus.up.tutkittu = true;
        ristaus.up.jatkom = OPENING;
        reitti.push_back(ristaus);
//        for (auto rist : reitti){
//            cout << "Risteys: " << rist.kartalla.ykoord << "," << rist.kartalla.xkoord << endl;
//        }
        return true;
    }
    return true;
}

//..vasemmalla
bool tutkiLeft(Sijainti nykysijainti, auto& reitti, LiikkumisSuunta prevDir, int (*labyrintti)[LEVEYS]){
    int yindex = KORKEUS-1-nykysijainti.ykoord;
    int xindex = nykysijainti.xkoord-1;
    if (xindex < 0) return false; //ulos kartalta - ei mahdollista
    if (labyrintti[yindex][xindex] == 1) return false;
    //tulossa uuteen ristaukseen
    if (labyrintti[yindex][xindex] == 2 && prevDir != RIGHT){
        Ristaus ristaus;
        ristaus.kartalla.ykoord = nykysijainti.ykoord;
        ristaus.kartalla.xkoord = nykysijainti.xkoord-1;
        ristaus.right.tutkittu = true;
        ristaus.right.jatkom = OPENING;
        reitti.push_back(ristaus);
//        for (auto rist : reitti){
//            cout << "Risteys: " << rist.kartalla.ykoord << "," << rist.kartalla.xkoord << endl;
//        }
        return true;
    }
    return true;
}

//..oikealla
bool tutkiRight(Sijainti nykysijainti, auto& reitti, LiikkumisSuunta prevDir, int (*labyrintti)[LEVEYS]){
    int yindex = KORKEUS-1-nykysijainti.ykoord;
    int xindex = nykysijainti.xkoord+1;
    if (xindex > LEVEYS) return false; //ulos kartalta - ei mahdollista
    if (labyrintti[yindex][xindex] == 1) return false;
    //tulossa uuteen ristaukseen
    if (labyrintti[yindex][xindex] == 2 && prevDir != LEFT){
        Ristaus ristaus;
        ristaus.kartalla.ykoord = nykysijainti.ykoord;
        ristaus.kartalla.xkoord = nykysijainti.xkoord+1;
        ristaus.left.tutkittu = true;
        ristaus.left.jatkom = OPENING;
        reitti.push_back(ristaus);
//        for (auto rist : reitti){
//            cout << "Risteys: " << rist.kartalla.ykoord << "," << rist.kartalla.xkoord << endl;
//        }
        return true;
    }
    return true;
}

//tämä funktio palauttaa aina seuraavan lähtösuunnan ilman lisäehtoja
//OPISKELIJA: älä muuta tätä funktiota suoraan vaan tee omille mahdollisille lisäehdoille oma(t) funktio(t) loogisesti oikeisiin paikkoihin
LiikkumisSuunta findNext(bool onkoRistaus, Sijainti nykysijainti, LiikkumisSuunta prevDir, auto& reitti, int (*labyrintti)[LEVEYS]){
    if (!onkoRistaus) {        
        if (tutkiLeft(nykysijainti, reitti, prevDir, labyrintti) && prevDir != RIGHT){
        return LEFT;
        }
        if (tutkiUp(nykysijainti, reitti, prevDir, labyrintti) && prevDir != DOWN){
        return UP;
        }
        if (tutkiDown(nykysijainti, reitti, prevDir, labyrintti) && prevDir != UP){
        return DOWN;
        }
        if (tutkiRight(nykysijainti, reitti, prevDir, labyrintti) && prevDir != LEFT){
        return RIGHT;
        }
        return DEFAULT; //UMPIKUJA - palaa viimeisimpään risteykseen returnin jälkeen
    }
    else if (onkoRistaus) {
        if (tutkiLeft(nykysijainti, reitti, prevDir, labyrintti) && reitti.back().tutkittavana != LEFT && !reitti.back().left.tutkittu){
        return LEFT;
        }
        if (tutkiUp(nykysijainti, reitti, prevDir, labyrintti) && reitti.back().tutkittavana != UP && !reitti.back().up.tutkittu){
        return UP;
        }
        if (tutkiDown(nykysijainti, reitti, prevDir, labyrintti) && reitti.back().tutkittavana != DOWN && !reitti.back().down.tutkittu){
        return DOWN;
        }
        if (tutkiRight(nykysijainti, reitti, prevDir, labyrintti) && reitti.back().tutkittavana != RIGHT && !reitti.back().right.tutkittu){
        return RIGHT;
        }
        return DEFAULT; //palatun ristauksen kaikki suunnat käyty,return jälkeen risteyksen voi poistaa pinosta ja palata sitä edelliseen risteykseen
    }
}

//näissä rotta liikkuu varsinaisesti eli sijainti päivitetään
Sijainti moveUp(Sijainti nykysijainti){
    nykysijainti.ykoord++;
    return nykysijainti;
}
Sijainti moveDown(Sijainti nykysijainti){
    nykysijainti.ykoord--;
    return nykysijainti;
}
Sijainti moveLeft(Sijainti nykysijainti){
    nykysijainti.xkoord--;
    return nykysijainti;
}
Sijainti moveRight(Sijainti nykysijainti){
    nykysijainti.xkoord++;
    return nykysijainti;
}

//PasiM TODO:jatkokehitys:
//toteuta funktiopointterivektorit
//funktiopointterivektorilla voi tiivistää koodia melkoisesti

//TÄRKEÄÄ
//risteyskoodi alapuolella, eli tämä ajetaan silmukan alussa kun ollaan jo risteyksessä (eli liikuttu siihen edellisen silmukan lopussa)
//OPISKELIJA: tästä kutsut tarvittavaan omaan lisättyyn logiikkaan (jos lisäehtoja/optimointia labyrintissa liikkumiselle)
//back() viittaa vektorin viimeisimpään lisättyyn alkioon (=pinon päälle)
//pop_back() poistaa viimeisimmän lisätyn alkion (=pinon päältä)
//tutkittavana - attribuutti kertoo (jää muistiin risteyksestä) minne suuntaan risteyksestä nyt lähdettiinkään
//palauttaa suunnan mihin risteyksestä lähdetään
LiikkumisSuunta doRistaus(Sijainti risteyssijainti, LiikkumisSuunta prevDir, auto& reitti, int (*labyrintti)[LEVEYS]){
    LiikkumisSuunta nextDir; 
    nextDir = findNext(true, risteyssijainti, prevDir, reitti, labyrintti); 
    //HUOM! päätös risteyksessä toimimisesta tehdään alla
    //OPISKELIJA: voit vaikuttaa päätöksentekoon, lisää oma toiminnallisuus omaan funktioonsa
    if (nextDir == LEFT) reitti.back().tutkittavana = LEFT;
    else if (nextDir == UP) reitti.back().tutkittavana = UP;
    else if (nextDir == RIGHT) reitti.back().tutkittavana = RIGHT;
    else if (nextDir == DOWN) reitti.back().tutkittavana = DOWN;
    else if (nextDir == DEFAULT) reitti.pop_back(); //kaikki suunnat oli jo tutkittu
    return nextDir; //kertoo eteenpäin miten päätettiin toimia risteyksessä
}
/*
 * Tästä alkaa rotan matka labyrintissa - tätä kutsutaan niin prosessista kuin threadistä samalla tavalla
 * Pääsilmukka on yksi liikkumisvuoro: vuoro alkaa analysoinnista nykyisen sijainnin suhteen ja vuoro loppuu siirtymiseen uuteen sijaintiin
 */

//OPISKELIJA: erottele tarvittaessa koodi mitä ajetaan prosessitoteutuksessa ja mitä thread-toteutuksessa
//OPISKELIJA: tarvitset siihen komentoriviltä annettavan parametrin mitä kuljetat sinne minne sitä tarvitaan

//HUOM! olennaista on että kurssilla opeteltuja asioita on sovellettu (mahdollisimman yksinkertainen koodi toimii)
//Siis kutsu alla olevaa niin forkatusta lapsesta kuin luodusta threadistä!
//palauttaa yksittäisen rotan liikkujen määrän
//parametrina tässä siis voi/pitää antaa esimerkiksi että ollaanko prosessina vai threadinä liikkeellä
//kuljeta parametria/parametreja tarvittavissa paikoissa ohjelmassa
//ohjelman lopussa reitti vektorissa (käsitellään pinona) on oikean reitin risteykset ainoastaan
RottaTulos aloitaRotta(int (*labyrintti)[LEVEYS], int (*sijaintiKartta)[LEVEYS], sem_t* sem, int rotta_ID){
    RottaTulos tulos;
    tulos.liikkuCount = 0;
    vector<Ristaus> reitti; //pinona käytettävä rotan kulkema reitti (pinossa kuljetut risteykset)
    Sijainti rotanSijainti = findBegin(labyrintti); //hae labyrintin alku
    LiikkumisSuunta prevDir {DEFAULT}; //edellinen suunta:jottei kuljeta edestakaisin vahingossa
    LiikkumisSuunta nextDir {DEFAULT}; //seuraava suunta
    //pyöri labyrintissa kunnes ulostulo on löytynyt
    while (labyrintti[KORKEUS-1-rotanSijainti.ykoord][rotanSijainti.xkoord] != 4) { //ulospääsyn löytymisehto
//DEBUGAUSAPUJA..getpid() palauttaa ajajan
        pid_t prosessi = getpid();
        //cout << "Olen prosessi: " << prosessi << endl;
        //alla vaihtoehtoisesti n-kertainen for-loop testauksia varten
        //    for (int i = 0 ; i < 50 ; i++){
        //risteykset on labyrinttiin merkitty 2:lla ohjelmoinnin helpottamiseksi
        //risteyksen tutkimiselle on oma koodi alla
        if (labyrintti[KORKEUS-1-rotanSijainti.ykoord][rotanSijainti.xkoord] == 2){
            nextDir = doRistaus(rotanSijainti, prevDir, reitti, labyrintti);
        }
        //muuten tämä, eli "muu kuin risteys" koodi
        else nextDir = findNext(false /* ei risteys */, rotanSijainti, prevDir, reitti, labyrintti);
        //huom! nykyimplementaatiossa jos vuoron alussa (eli yläpuolisissa kutsuissa) liikkuminen on todettu mahdolliseksi se myös tullaan tekemään, muuta tätä tarvittaessa
        //huom! jos on valittu risteykseenmeno niin ristaus on jo lisätty ristauspinoon tässä vaiheessa
        //huom! päätökset on jo tehty, tässä on vain päätösten toimeenpano
        switch (nextDir) {
        case UP:
        rotanSijainti = moveUp(rotanSijainti);
        prevDir = UP;
        break;
        case DOWN:
        rotanSijainti = moveDown(rotanSijainti);
        prevDir = DOWN;
        break;
        case LEFT:
        rotanSijainti = moveLeft(rotanSijainti);
        prevDir = LEFT;
        break;
        case RIGHT:
        rotanSijainti = moveRight(rotanSijainti);
        prevDir = RIGHT;
        break;
        //jos on palautettu DEFAULT niin on joko kyse
        //a)UMPIKUJASTA: (DEFAULT PALAUTETTU findNext() -> paluu edelliseen risteykseen
        //b)AIEMMIN PALATTIIN JO RISTEYKSEEN JONKA KAIKKI SUUNNAT tutkittu-attribuutti true: (DEFAULT PALAUTETTU doRistaus() -> poista risteys pinosta ja palaa edelliseen risteykseen
        case DEFAULT: //=paluu edelliseen risteykseen jossa käymättömiä reittejä
        cout << "Umpikuja: " << "Ruutu: " << rotanSijainti.ykoord << "," << rotanSijainti.xkoord << endl; 
        rotanSijainti.ykoord = reitti.back().kartalla.ykoord;
        rotanSijainti.xkoord = reitti.back().kartalla.xkoord;
        cout << "Palattu: " << "Ruutu: " << rotanSijainti.ykoord << "," << rotanSijainti.xkoord << endl;
            switch (reitti.back().tutkittavana){
            case UP:
                reitti.back().up.tutkittu = true;
                reitti.back().up.jatkom = OPENING;
                break;
            case DOWN:
                reitti.back().down.tutkittu = true;
                reitti.back().down.jatkom = OPENING;
                break;
            case LEFT:
                reitti.back().left.tutkittu = true;
                reitti.back().left.jatkom = OPENING;
                break;

            case RIGHT:
                reitti.back().right.tutkittu = true;
                reitti.back().right.jatkom = OPENING;
                break;
            default:
            cout << "Ei pitäisi tapahtua! Joku ongelma jos tämä tulostus tulee!" << endl;
            break;
        } 
        break;
    }
//DEBUGGAUSAPUJA
        //cout << "Rotan sijainti nyt: " << rotanSijainti.ykoord << "," << rotanSijainti.xkoord << endl;
        tulos.rotanSijainti = rotanSijainti;
        //cout << "Rotan sijainti nyt: " << tulos.rotanSijainti.ykoord << "," << tulos.rotanSijainti.xkoord << endl;

        //Yksittäisen rotan eksklusiivinen kirjoitus jaetun muistin sijaintiKartalle
        //Prosessin tapauksessa käytetään semaforia kriittisen alueen suojaamiseen (merkataan rotan id sijaintikarttaan)
        if (sem) {
            sem_wait(sem);
            sijaintiKartta[rotanSijainti.ykoord][rotanSijainti.xkoord] = rotta_ID;
            sem_post(sem);
        } else { //Säietapauksessa käytetään mutexia kriittisen alueen suojaamiseen (merkataan rotan id sijaintikarttaan)
            pthread_mutex_lock(&säieLukko);
            sijaintiKartta[rotanSijainti.ykoord][rotanSijainti.xkoord] = rotta_ID;
            pthread_mutex_unlock(&säieLukko);
        }

        cout << "Rotan " << rotta_ID << " sijainti: "  << rotanSijainti.ykoord << ", " << rotanSijainti.xkoord << endl;
        //Tulostetaan kaksiulotteisen taulukon mukainen sijaintikartta, joka näyttää rotan merkitsemällään sijainnilla labyrintissa
        //Ei toimi hyvin 100x100 taulukon kanssa suuren koon vuoksi
        /*
        cout << "Rottien sijaintikartta:" << endl;
         
        for (int y = 0; y < KORKEUS; y++) {
            for (int x = 0; x < LEVEYS; x++)
                cout << sijaintiKartta[y][x] << ' ';
            cout << '\n';
        }
            */
            
            

    //päivitetään liikkujen laskuri, tällä hetkellä yksi risteykseen paluu on yksi liikku
    tulos.liikkuCount++;
    //cout << "Käytettyjä liikkuja: " << tulos.liikkuCount << endl;
    //OPISKELIJA: päivitä rotan hengähtämistaukoa haluamallasi tavalla (se voi vaihdella eri rotilla)
    usleep(10);
} // for //while    
    //ohjelman lopuksi palautetaan liikkujen määrä rottakohtaisesti
    //OPISKELIJA: voisit muuttaa paluuarvon rakenteeksi jossa olisi liikkujen määrän lisäksi myös oikea reitti eli jäljelle jäänyt risteyspino, pystyt sitten käyttämään sitä prosesseissa tai säikeissä
    //return liikkuCount, rotanSijainti.ykoord, rotanSijainti.xkoord;
    //cout << "TULOS: " << tulos.rotanSijainti.xkoord << endl;
    return tulos;
}

    //Ohjelman toteutus prosesseina
int prosessiToteutus(int (*labyrintti)[LEVEYS], int (*sijaintiKartta)[LEVEYS], int shm_lab, int shm_sij, int maara, sem_t* sem) {
   
     //Luodaan lapset vektori, johon kerätään kaikki luodut lapsiprosessit
     vector<pid_t> lapset;

    sijaintiKartta = (int (*)[LEVEYS]) shmat(shm_sij, NULL, 0);

    if (sijaintiKartta == (void*)-1) {
        perror("shmat (sijaintiKartta)");
        return 1;
    }

     //Luodaan lapsiprosesseja käyttäjän syöttämä määrä
     for (int i = 0; i < maara; i++) {
     pid_t pid = fork();
     if (pid == -1) { perror("fork"); return 1; };

     //Lapsi
     if (pid == 0) {
        //cout << "Olen lapsi! pid: " << pid << endl;

       RottaTulos tulos = aloitaRotta(labyrintti, sijaintiKartta, sem, i + 1);

        cout << "Rotta " << i + 1 << " valmis! Lopullinen sijainti: " << tulos.rotanSijainti.ykoord << ", " << tulos.rotanSijainti.xkoord << endl;

        _exit(0);
        //return 0;
        //Vanhempi
     } else {
       // cout << "Olen vanhempi! pid: " << getpid() << endl;
       //Tallennetaan lapsiprosessit vektoriin
       lapset.push_back(pid);
     }

}
    //Vanhempi odottaa kaikkien lasten päättymistä
    for (pid_t pid : lapset) {
        waitpid(pid, nullptr, 0);
    }
    return 0;
}

//Struct jossa kaikki säiToteutus() argumentit
struct SäieArgs {
    int (*labyrintti)[LEVEYS];
    int (*sijaintiKartta)[LEVEYS];
    int rotta_ID;
};

    //Ohjelman toteutus säikeinä
void* säieToteutus(void* arg) {
    SäieArgs* args = (SäieArgs*)arg;
    cout << "Säie: " << args->rotta_ID << endl;
    aloitaRotta(args->labyrintti, args->sijaintiKartta, nullptr, args->rotta_ID);
    cout << "Säie " << args->rotta_ID << " valmis!" << endl;
    return nullptr;
}

//OPISKELIJA: nykyinen main on näin yksinkertainen, tästä pitää muokata se rinnakkaisuuden pohja
//argc= argumenttien määrä komentoriviltä
//argv[]= parametrien indeksit esim argv[1]
int main(int argc, char* argv[]){

    //Muuttujat syötetylle rottien määrälle ja tilalle
    int maara;
    string tila;
    //Kysytään käyttäjältä argumentit ohjelman ajamiseen
        cout << "Valitaan ajotapa (p tai s) sekä rottien määrä" << endl;
        cout << "Ajotapa: ";
        cin >> tila;
        //Ehtolause sille, ettei kysellä rottien määrää väärän syötteen jälkeen
        if (tila == "p" || tila == "s") {
        cout << "Rottien määrä: ";
        cin >> maara;
        }

     //Jaettu muisti
     //Luodaan jaettuMuisti muuttuja, joka on siis jaetun muistin koko tavuina
     //Luodaan int tyyppinen jaetun muistin tunniste shm_id
    size_t jaettuMuisti = sizeof(int) * KORKEUS * LEVEYS;
    int shm_lab;
    //Haetaan haluttu jaetun muisti shm_id:n käyttöön
    shm_lab = shmget(IPC_PRIVATE, jaettuMuisti, S_IRUSR | S_IWUSR);
    //Luodaan int tyyppinen pointteri nimeltä labyrintti, jonka kautta 2D taulukkoa käytetään ohjelmassa
    //Liitetään (shmat = attach) kyseinen pointteri jaettuun muistiin
    int (*labyrintti)[LEVEYS];
    labyrintti = (int (*)[LEVEYS]) shmat(shm_lab, NULL, 0);

    //Kopioidaan alkuperäinen labyrintti jaettuun muistiin
    memcpy(labyrintti, alkupLabyrintti, jaettuMuisti);

    //Luodaan jaetun muistin tunniste shm_sij
    int shm_sij;
    shm_sij = shmget(IPC_PRIVATE, jaettuMuisti, S_IRUSR | S_IWUSR);

    //Luodaan sijaintikartta ja käytetään jaettua muistialuetta
    int (*sijaintiKartta)[LEVEYS];
    sijaintiKartta = (int (*)[LEVEYS]) shmat(shm_sij, NULL, 0);
    memset(sijaintiKartta, 0, jaettuMuisti);

    //Avataan semafori, jolla suojataan kriittinen alue
     sem_unlink(Semafori);
     sem_t* sem = sem_open(Semafori, O_CREAT | O_EXCL, 0666, 1);
     if (sem == SEM_FAILED) { perror("sem_open"); return 1; }


    //Jos valittiin säikeet
    if (tila == "s") {
        cout << "Ajetaan säikeillä: " << maara << " rottaa!" << endl;
        //Luodaan vektori jonne luodut säikeet tallennetaan
        vector<pthread_t> säikeet(maara);
        vector<SäieArgs> args(maara);
 
        //Luodaan käyttäjän syöttämä määrä säikeitä
        for (int i = 0; i < maara; i++) {
            args[i].labyrintti = labyrintti;
            args[i].sijaintiKartta = sijaintiKartta;
            args[i].rotta_ID = i + 1;
            pthread_create(&säikeet[i], nullptr, säieToteutus, &args[i]);
        }

        //Odotetaan että kaikki luodut säikeet ovat löytäneet suuaukolle
        for (int i = 0; i < maara; i++) {
            pthread_join(säikeet[i], nullptr);
        }

        //Tuhotaan mutex käytön jälkeen
        pthread_mutex_destroy(&säieLukko);
    }
    //Jos valittiin prosessit
    else if (tila == "p") {
        cout << "Ajetaan prosesseilla: " << maara << " rottaa!" << endl;
        prosessiToteutus(labyrintti, sijaintiKartta, shm_lab, shm_sij, maara, sem);
    }
    //Jos jotain muuta (virheellistä)
    else {
        cout << "Väärä syöte: " << tila << endl;
        return 1;
    }

    //Suljetaan ja poistetaan semafori
    sem_close(sem);
    sem_unlink(Semafori);

    //Siivotaan käytetty jaettu muisti
    shmdt(labyrintti);
    shmdt(sijaintiKartta);
    shmctl(shm_lab, IPC_RMID, nullptr);
    shmctl(shm_sij, IPC_RMID, nullptr);

    cout << "Kaikki rotat ulkona!" << endl;

    return 0;
    //tämän tulee kertoa että kaikki rotat ovat päässeet ulos labyrintista
    //viimeinen jäädytetty kuva sijaintikartasta olisi hyvä olla todistamassa sitä
}
