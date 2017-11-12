#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

int N(12500);
int Ne(10000);
int Ni(2500);
int Ce(1000);
int Ci(250);
double Je(0.1);
double Ji(0.5);
double V_max(20.0);
double V_reset(0.0);
int tau(10);
int cycle_buffer(16);
double Vext(400.0);
//Soient les paramètres à utiliser pour l'intérieur du programme//


class Neuron {
	
	public:
	Neuron(bool excitatoire)
	: excitatoire(excitatoire) { setPotentiel(0.0); setSpike_time(2.0); taille_spikes_assimiles(cycle_buffer - 1); }
	//Je ne choisis que le booléen pour l'initialisation du neurone. Car les valeurs des autres attributs sont communs entre eux.//
	//Et j'initialise son potentiel, son spiketime ainsi que la taille de son "ring buffer".//
	
	//Des getters//
	bool getExcitatoire() const {
		return excitatoire;
	}
	vector<int> getCibles() const {
		return cibles;
	}
	double getPotentiel() const {
		return potentiel;
	}
	vector<int> getTableau_cibles() const {
		return tableau_cibles;
	}
	bool getRefactoire() const {
		return refactoire;
	}
	double getSpike_time() const {
		return spike_time;
	}
	vector<double> getSpikes_assimiles() const {
		return spikes_assimiles;
	}
	
	//Des setters//
	void setCibles(vector<int> x) {
		cibles = x;
	}
	void setPotentiel(double p) {
		potentiel = p;
	}
	void setTableau_cibles(vector<int> n) {
		tableau_cibles = n;
	}
	void setRefactoire(bool b) {
		refactoire = b;
	}
	void setSpike_time(double s) {
		spike_time = s;
	}
	void setSpikes_assimiles(vector <double> s) {
		spikes_assimiles = s;
	}
	
	//Je crée des méthodes pour accéder facilement à des éléments appartenant à des tableaux, ou pour modifier ceux-ci.//
	double acces_Tableau_cibles(int r) {
		return tableau_cibles[r];
	}
	double acces_spikes_assimiles(int r) {
		return spikes_assimiles[r];
	}
	void taille_spikes_assimiles(unsigned int r) {
		spikes_assimiles.resize(r);
	}
	void modifier_spikes_assimiles(int r, double t) {
		spikes_assimiles[r] = t;
	}
	
	//J'utilise la fonction fonction random pour remplir des cases, créer des connections avec d'autres neurones dont l'élément généré représente le nombre de connections d'entrée//
	void creer_cibles() {
		int m(0);
		vector<int> x;
		random_device rd;
		mt19937 gen(rd());
		
		if (getExcitatoire() == true) {
			uniform_int_distribution<> dis(0, Ce); 
			do {
				const int r(dis(gen));
				if (r >= 1) {
					x.push_back(r);
				}
				m = m + r;
			} while (m < Ce);
			x.push_back(Ce - m);
		} else {
			uniform_int_distribution<int> dis(0, Ci); 
			do {
				const int r(dis(gen));
				if (r >= 1) {
					x.push_back(r);
				}
				m = m + r;
			} while (m < Ci);
			x.push_back(Ci - m);
		}
		setCibles(x);
	}
	
	private: 
	bool excitatoire;
	vector <int> cibles;
	vector <int> tableau_cibles;
	double potentiel;
	bool refactoire;
	double spike_time;
	vector <double> spikes_assimiles;
	/*Les attributs. Si le bool est true, le neurone est excitatoire, inhibiteur sinon.
	 * Le vector cibles est celui que j'ai créé au-dessus. Le tableau_cibles est utilisé en dessous.
	 * Je choisis le potentiel de la membrane comme un double, le mode refractoire comme un booléen, le spike_time comme un double.
	 * Le vector spikes_assimiiles sert à enregistrer le nombre total de spikes que reçoit le neurone au cours d'une étape de la simulation. Il sera enregistré dans une case et influera sur le potentiel après le délai.*/
};

class Network {
	
	public:
	
	//Un getter et un setter//
	vector<Neuron*> getNeurones() const {
		return neurones;
	}
	
	void setNeurones(vector<Neuron*> n) {
		neurones = n;
	}
	
	Neuron* acces_Neuron(int r) {
		return neurones[r];
	}
	
	//Cette méthode sert à remplir le tableau de neurones. Je choisis les 1000 premiers comme excitateurs et les derniers comme inhibiteurs.//
	void creer_tableau_neurones() {
		vector<Neuron*> n;
		for (int i(0); i < Ne; i++) {
			Neuron* neurone = new Neuron(true);
			n.push_back(neurone);
		}
		for (int i(0); i < Ni; i++) {
			Neuron* neurone = new Neuron(false);
			n.push_back(neurone);
		}
		setNeurones(n);
	}
	
	//Une méthode qui répartit uniformément et aléatoirement les cibles d'un neurone dans le tableau du Network, qui remplit au passage son tableau_cibles.//
	void creer_tableau_cibles() {
		random_device rd;
		mt19937 gen(rd());
		for (auto n : neurones) {
			bernoulli_distribution d(n->getCibles().size()/N);
			vector <int> neu;
			int r(0);
			for (int i(0); i <= N - 1; i++) {
				if (d(gen)) {
					neu[i] = n->acces_Tableau_cibles(r);
					r = r + 1;
				}
			}
			n->setTableau_cibles(neu);
		}
	}
	
	//Création des spikes aléatoires avec Poisson.//
	void creer_spikes_aleatoires() {
		random_device rd;
		mt19937 gen(rd());
		poisson_distribution<> dis(Vext/V_max); 
		for (unsigned int i(0); i < getNeurones().size(); i++) {
			neurones[i]->setPotentiel(dis(gen));
		}
	}
			
	void update(double h, double stoptime) {
		double simtime(0.0);
		const int& i(10*simtime); /* Le i est pris comme référence car il servira juste en dessous pour le ring buffer. */
		do {
			for (auto n : neurones) {
				if (n->getRefactoire()) {
					if (n->getSpike_time() > 0.0) {
						n->setSpike_time(n->getSpike_time() - h);
					} else {
						n->setSpike_time(2.0);
						n->setRefactoire(false);
					}
				}
				//Je dois parcourir chaque neurone, contrôler qu'il est en réfractoire, auquel cas il doit attendre la fin de son spike_time, sa pause.//
				else { 
					if (n->getPotentiel() > V_max) {
					n->setPotentiel(V_reset);
					n->setRefactoire(true);
					if (n->getExcitatoire()) {
					for (auto p : n->getTableau_cibles()) {
						if (n->acces_Tableau_cibles(p) != 0) {
							if (acces_Neuron(p)->getRefactoire() == false) {
								acces_Neuron(p)->modifier_spikes_assimiles((i + 15) % cycle_buffer, acces_Neuron(p)->acces_spikes_assimiles((i + 15) % cycle_buffer + Je*n->acces_Tableau_cibles(p)));
							}
						}
					}
				} else {
					for (auto p : n->getTableau_cibles()) {
						if (n->acces_Tableau_cibles(p) != 0) {
							if (acces_Neuron(p)->getRefactoire() == false) {
								acces_Neuron(p)->modifier_spikes_assimiles((i + 15) % cycle_buffer, acces_Neuron(p)->acces_spikes_assimiles((i + 15) % cycle_buffer + Ji*n->acces_Tableau_cibles(p)));
								}
							}
						}
					}
				}
				// A l'inverse, s'il est actif, je vérifie son potentiel. Si celui-ci est assez grand, il y aura distribution de spikes aux autres éléments du tableau, d'après le tableau_cibles du neurone en question.
				// Je fais bien sûr la différence entre les excitateurs et les inhibiteurs, contrôle à chaque fois que les neurones ciblés soient en état de recevoir les spikes.
				// Après quoi, le neurone actuellement sélectionné dans la boucle entrera en période réfractoire.
				else {
					n->setPotentiel(exp(-h/tau)*n->getPotentiel() + n->acces_spikes_assimiles(i % cycle_buffer));
					}
					// Pour finir, s'il n'y a pas d'annonce de période réfractoire, je modifie le potentiel selon la formule du cours.//
				}
			} simtime = simtime + h;
			// Et ainsi de suite.
		} while (simtime < stoptime);
	}
			
	
	private:
	vector <Neuron*> neurones;
};

