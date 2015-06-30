#include <stdlib.h>
#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include <math.h>
#include <queue>
#include <map>
#include <algorithm>

using namespace std;


/*VARIABLES*/
double tiempo_total = 0;
double largo_timer = 0;
bool A_libre = true;
bool B_libre = true;

int ACK_esperado_por_B = 0;	//es un identificador
int ACK_esperado_por_A = 0;	//es un identificador
int ultimo_ACK_enviado = 0;
int ultimo_frm_enviado = 0;
int mensajes_en_ventana = 0;

int ultimo_indice_ventana_enviado = 0;
int indice_ventana_A = 0;

int contador_ventana = 0;
bool llego_ACK_bien = false;
int parametro_timer = 0;

class timer
{
public:
	double hora_comienzo;
	void empieza(){
		hora_comienzo = clock();
	}
	double tiempo_pasado(){
		return ((double)clock() - hora_comienzo) / CLOCKS_PER_SEC;
	}
	bool termino_timer(){
		if (largo_timer >= tiempo_pasado()){
			return true;
		}
		return false;
	}
};
/*-------------------------------------------------------------------------------------*/


/*VARIABLES RELOJ*/
double A_recibe = 0;
double A_libera = 0;
double B_recibe = 0;
double B_libera = 0;
double A_recibe_ACK = 0;
double vence_timer = 0;

double RELOJ = 0;


/*VARIABLES ESTADISTICAS*/
int sum_cola_A = 0;
int numSumColaA = 0;

vector <int> frames_recibidos_por_B;

timer medirCola;
double tiempoParaMedirCola;

double tiempo_en_sistema = 0;
int cont_tiempo_sistema = 0;

int primeros_20 = 0;
timer permanencia_mensajes[8];
double tiempo_transferencia = 0;
double prom_tiempo_transferencia = 0;
int total_msjs_transferidos = 0;
vector <double> permanencia_por_corrida;
double permanencia_en_sistema;


/*COLAS*/
vector<int> TODOS_LOS_MENSAJES;	//cola con todos los mensajes que han sido mandados
queue<int> cola_mensajes_A;
queue< pair<int, bool> > cola_mensajes_B;
timer timers_ventana[8];
vector<pair<int, bool> > ventana;
vector <double> tiempoPromedio;



/*METODOS*/
double minimo(double uno, double dos){
	if (uno<dos){
		return uno;
	}
	else {
		return dos;
	}
}


double numero_aleatorio_exp(){
	double r = (double)rand() / (double)RAND_MAX;
	return (double)-(log(1 - r)) * 2;
}
double numero_aleatorio_normal(){
	double r1 = (double)rand() / (double)RAND_MAX;
	double r2 = (double)rand() / (double)RAND_MAX;

	double z = sqrt(-2 * log(r1))*sin(r2);

	return (double)(25 + z);
}
double numero_aleatorio_TI(){
	double r = (double)rand() / (double)RAND_MAX;
	return (double)sqrt(5 * r + 4);
}


/*---Metodos para estadisticas y para intervalo de confianza----------------------------*/

void mostrarFramesRecibidos(){
	if (!frames_recibidos_por_B.empty()){
		vector <int> ::reverse_iterator iter = frames_recibidos_por_B.rbegin();
		int cont = 0;
		//cout << "Ultimos frames recibidos por B: " << endl;
		while (iter != frames_recibidos_por_B.rend() && cont < 20){
			//cout << cont << ") " << *iter << " ";
			cont++;
			++iter;
		}
		//cout << endl;
	}
	else
		;//cout << "B no ha recibido frames" << endl;

}
void inicializarVariables(int numCorridas){
	ventana.resize(8);
	medirCola.empieza();
	tiempoParaMedirCola = 10;
	permanencia_por_corrida.resize(numCorridas); //es el numero de veces que se va a correr
}

void estadisticasFinales(){
	permanencia_en_sistema = 0;
	double tiempo_transmision = 0;
	double tiempo_servicio = 0;
	double eficiencia = 0;
	if (numSumColaA != 0){
		//cout << "Tamano promedio cola A " << sum_cola_A / numSumColaA << endl;
	}
	if (cont_tiempo_sistema != 0){
		permanencia_en_sistema = tiempo_en_sistema / cont_tiempo_sistema;
		//cout << "Tiempo promedio que permanece un mensaje \n en el sistema:   " << permanencia_en_sistema << endl;
	}
	if (total_msjs_transferidos != 0){
		tiempo_transmision = prom_tiempo_transferencia / total_msjs_transferidos;
		//cout << "Tiempo promedio de transmision para un mensaje:  " << tiempo_transmision << endl;
	}
	tiempo_servicio = permanencia_en_sistema - tiempo_transmision;
	//cout << "Tiempo promedio de servicio para un mensaje:  " << tiempo_servicio << endl;
	if (tiempo_servicio != 0){
		eficiencia = tiempo_transmision / tiempo_servicio;
		//cout << "Eficiencia del servidor:  " << eficiencia << endl;

	}

}

void obtenerIntervaloDeConfianza(int numCorridas) {
	//en nuestro caso num corridas sera igual a 10
	//funciona con intervalo de confianza de  1 – 0.05 = 0,95
	double mediaMuestral = 0;
	double mediaMuestralTemp = 0;
	double varianzaMuestral = 0;
	double sumaTemp = 0;
	double intervalo_de_confianza = 0;
	double limSup = 0;
	double limInf = 0;
	//extremos del intervalo
	if (numCorridas <= permanencia_por_corrida.size()){

		for (int i = 0; i < numCorridas; i++){

			mediaMuestralTemp += permanencia_por_corrida.at(i);
		}
		mediaMuestral = mediaMuestralTemp / numCorridas;
		for (int i = 0; i < numCorridas; i++){

			sumaTemp += (permanencia_por_corrida.at(i) - mediaMuestral)*(permanencia_por_corrida.at(i) - mediaMuestral);
		}
		varianzaMuestral = sumaTemp / (numCorridas - 1);
		limInf = mediaMuestral - 2.26 * sqrt(varianzaMuestral / numCorridas);
		limSup = mediaMuestral + 2.26 * sqrt(varianzaMuestral / numCorridas);
		if (limInf < limSup){
			intervalo_de_confianza = limSup - limInf;
			cout << "LimInf " << limInf << "limSup " << limSup << endl;
			cout << "El intervalo de confianza es " << intervalo_de_confianza << endl;

		}
		else intervalo_de_confianza = limInf - limSup;
		cout << "LimInf " << limSup << "limSup " << limInf << endl;
		cout << "El intervalo de confianza es " << intervalo_de_confianza << endl;
	}
}
/*--------------------------------------------------------------------------------------*/


void A_recibe_mensaje(int identificador){
	RELOJ = A_recibe;
	cout << "Esta en el evento A recibe mensaje, el reloj es " << RELOJ / (double)CLOCKS_PER_SEC << endl;
	mensajes_en_ventana++;
	if (mensajes_en_ventana > 8){
		cola_mensajes_A.push(identificador);
	}
	else{
		permanencia_mensajes[indice_ventana_A].empieza();
		ventana.at(indice_ventana_A) = make_pair(identificador, false);
		if (A_libre){
			tiempo_transferencia = numero_aleatorio_exp() + 1;
			prom_tiempo_transferencia += tiempo_transferencia;
			total_msjs_transferidos++;
		}
		indice_ventana_A++;//dice qué tan llena está la ventana
		if (indice_ventana_A == 8){
			indice_ventana_A = 0;
		}
	}
	A_libera = A_recibe + (double)tiempo_transferencia;
	A_recibe = A_recibe + numero_aleatorio_normal();
}


void A_se_libera(){
	int indice = 0;
	for (unsigned int i = 0; i < ventana.size(); i++){
		if (ventana[i].first == ACK_esperado_por_B){
			indice = i;
		}
	}

	RELOJ = A_libera;
	cout << "Esta en el evento A se libera, el reloj es " << RELOJ / (double)CLOCKS_PER_SEC << endl;
	int lo_que_pasa = rand() % 99;	//numero entre 0 y 99
	//cout << "Timers " << timers_ventana[indice].hora_comienzo << endl;

	if (lo_que_pasa > 10 && lo_que_pasa < 15){	//se perdio
		vence_timer = clock();
		parametro_timer = ventana[indice].first;

		permanencia_mensajes[indice].hora_comienzo = 0;
		cont_tiempo_sistema--;		//si se va a perder entonces quita los timers
	}
	else if (lo_que_pasa < 10){ 	//llego con error
		ventana.at(indice).second = true;
		B_recibe = RELOJ + 1;
		ACK_esperado_por_B = ventana.at(indice).first;
	}
	else {	//llego bien
		B_recibe = RELOJ + 1;
		ACK_esperado_por_B = ventana.at(indice).first;
		ACK_esperado_por_A = ACK_esperado_por_B + 1;
	}

	ultimo_indice_ventana_enviado = indice;//para B_recibe_frame
	A_libera = A_recibe + numero_aleatorio_exp() + 1;
}

void B_recibe_frame(){

	RELOJ = B_recibe;
	cout << "Esta en el evento B recibe frame, reloj es " << RELOJ / (double)CLOCKS_PER_SEC << endl;

	frames_recibidos_por_B.push_back(ventana[ultimo_indice_ventana_enviado].first);	//ESTADISTICAS
	double tiempoRevision = numero_aleatorio_TI();
	if (B_libre){
		B_libera = RELOJ + 0.25 + (double)tiempoRevision;
		B_recibe = A_libera + 1;
	}
	else
		cola_mensajes_B.push(ventana[ultimo_indice_ventana_enviado]);
	mostrarFramesRecibidos();
}

void B_se_libera(){
	RELOJ = B_libera;
	cout << "Esta en el evento B se libera " << RELOJ / (double)CLOCKS_PER_SEC << endl;
	int se_pierde = rand() % 99;
	if (se_pierde > 15){ 	//llegó bien
		//cout << "El ultimo valor de ACK mandado por B es " << ultimo_ACK_enviado << endl;
		if (ventana[ultimo_indice_ventana_enviado].second == true){		//si llega con error el frame
			ultimo_ACK_enviado = ACK_esperado_por_B;
		}
		else {
			ultimo_ACK_enviado = ACK_esperado_por_B + 1;
		}
		A_recibe_ACK = B_libera + 1;
	}
	else {	//se perdio
		vence_timer = clock();
		parametro_timer = ACK_esperado_por_B;
	}
	B_libera = RELOJ + 0.25 + numero_aleatorio_TI();

}

void A_recibeACK(){
	RELOJ = A_recibe_ACK;
	cout << "Esta en el evento A recibe ACK, el reloj es" << RELOJ / (double)CLOCKS_PER_SEC << endl;
	int entraron = 0;
	if (ultimo_ACK_enviado >= ACK_esperado_por_A){

		llego_ACK_bien = true;
		if (ultimo_ACK_enviado > ACK_esperado_por_A){
			entraron = ultimo_ACK_enviado - ACK_esperado_por_A;
		}
		else{
			entraron = 1;
		}
		for (int n = 0; n <entraron; n++){
			//se usa luego para calcular el tiempo promedio sistema
			tiempoPromedio.push_back(timers_ventana[n].tiempo_pasado());
		}
		mensajes_en_ventana -= entraron;

		//Corrimientos:
		ventana.erase(ventana.begin(), ventana.begin() + entraron);//los que siguen se van a ingresar al final
		ventana.resize(8);

		for (int i = 0; i <mensajes_en_ventana; i++){
			timers_ventana[i] = timers_ventana[i + entraron];

			//tiempo_en_sistema += permanencia_mensajes[i].tiempo_pasado();//ESTADISTICAS
			cont_tiempo_sistema++;
			permanencia_mensajes[i] = permanencia_mensajes[i + entraron];

		}
		for (int j = mensajes_en_ventana; j < 8; j++){
			//(timers_ventana[j]).hora_comienzo = clock();

			//permanencia_mensajes[j].hora_comienzo = 0;//ESTADISTICAS
		}
	}
	else{
		for (int i = 0; i < ultimo_frm_enviado; i++){
			//teniendo que ultimo_frm_enviado siempre sera menor que 8
			timers_ventana[i].hora_comienzo = clock();
		}
	}

	if (medirCola.tiempo_pasado() >= tiempoParaMedirCola){
		if (!cola_mensajes_A.empty()){
			//cout << "Largo mensaje cola A " << cola_mensajes_A.size() << endl;
			sum_cola_A += cola_mensajes_A.size();
			numSumColaA++;
			if (A_libre){
				pair <int, bool> msjTemp;
				msjTemp.second = false;
				for (int i = mensajes_en_ventana; i < 8; i++){
					msjTemp.first = cola_mensajes_A.front();
					cola_mensajes_A.pop();
					ventana.at(i) = msjTemp;

				}
				A_libera = RELOJ / (double)CLOCKS_PER_SEC;
			}
		}
	}
	A_recibe_ACK = B_libera + 1;
}

void seVenceTimer(int indice){
	RELOJ = vence_timer;
	if (timers_ventana[indice].termino_timer() && indice < mensajes_en_ventana){

		cout << "Esta en el evento, el reloj es " << RELOJ / (double)CLOCKS_PER_SEC << endl;
		for (int i = indice; i < mensajes_en_ventana; i++){
			timers_ventana[i].hora_comienzo = 0;
		}
		//porque se tienen que mandar todos los mensajes de la ventana que se creia habian sido enviados
		for (unsigned int i = 0; i < ventana.size(); i++){
			if (ventana[i].first == ACK_esperado_por_B){
				contador_ventana = i;
			}
		}
	}
	vence_timer = 138;//infinito
}



void simulacion(){
	double el_minimo = minimo(A_recibe,
		minimo(A_libera,
		minimo(B_recibe,
		minimo(B_libera,
		minimo(A_recibe_ACK, vence_timer)))));

	if (el_minimo == A_recibe){	//para este entonces, ya está en la cola de todos los mensajes
		A_recibe_mensaje(TODOS_LOS_MENSAJES.back());
		contador_ventana++;
		if (contador_ventana == 8){
			contador_ventana = 0;
		}
	}
	else if (el_minimo == A_libera){
		if (contador_ventana == 8 && llego_ACK_bien){
			contador_ventana = 0;
			A_se_libera();
			llego_ACK_bien = false;
		}
		else {
			contador_ventana++;
			A_se_libera();
		}
	}
	else if (el_minimo == B_recibe){
		B_recibe_frame();
	}
	else if (el_minimo == B_libera){
		B_se_libera();
	}
	else if (el_minimo == A_recibe_ACK){
		A_recibeACK();
	}
	else if (el_minimo == vence_timer){
		seVenceTimer(parametro_timer);
	}
}

int main()
{
	RELOJ = clock() / (double)CLOCKS_PER_SEC;
	srand((unsigned)time(NULL));	//se inicializa para todos los rand del sistema

	A_recibe = clock() / (double)CLOCKS_PER_SEC;
	A_libera = A_recibe * 50000;
	B_recibe = A_recibe * 50000;
	B_libera = A_recibe * 50000;
	A_recibe_ACK = A_recibe * 50000;
	vence_timer = A_recibe * 50000;

	int corridas_solicitadas = 0;
	int corridas = 0;
	cout << "Numero de corridas:  ";
	cin >> corridas_solicitadas;

	cout << "Tiempo en segundos para correr: ";
	cin >> tiempo_total;
	cout << "Tiempo del timer: ";
	cin >> largo_timer;


	inicializarVariables(corridas_solicitadas);
	clock_t tiempo_principio = clock();

	//ESTADISTICAS
	/*A_recibe_mensaje(5, 0);
	A_se_libera(0);
	B_recibe_frame(ventana.at(0));
	B_se_libera(0, 0);
	A_recibeACK(0);
	estadisticasFinales();*/

	int num_mensaje = 0;
	while (corridas < corridas_solicitadas){
		num_mensaje = 0;
		while (true){

			//PROGRAMA
			TODOS_LOS_MENSAJES.push_back(num_mensaje);

			simulacion();

			if ((clock() - tiempo_principio) / (double)CLOCKS_PER_SEC > tiempo_total){
				break;
			}
			num_mensaje++;
		}


		A_recibe = RELOJ;
		A_libera = tiempo_total * 3;
		B_recibe = tiempo_total * 3;
		B_libera = tiempo_total * 3;
		A_recibe_ACK = tiempo_total * 3;
		vence_timer = tiempo_total * 3;

		tiempo_principio = clock();
		permanencia_por_corrida.at(corridas) = permanencia_en_sistema;
		num_mensaje = 0;
		corridas++;
	}

	system("pause");
	return 0;
}
