#include <C_Image.hpp>
#include <vector>
#include <iostream>
int Test(int argc, char **argv);

struct region {
	vector<region*> subregiones;
	C_Matrix mat;
	int homogeneo = -1;
	/*	0 - no cumple el criterio
	*	1 - cumple el criterio
	*	-1 - no se ha comprobado
	*/
	int num = -1;
	int pixeles;
	int suma;
};
vector<region*> regiones;

void histograma(C_Image* imagen);
void inverso(C_Image* imagen);
void dividir(region* nodo);
void exportar(region* nodo);
void uniforme(region* nodo);
void parejaUniforme(region* nodo1, region* nodo2);
bool vecinos(region* nodo1, region* nodo2);
int calcularPixeles(region* nodo);
void prepararRegion(region* nodo);
void fusionar();

int nNodo = 0;
int PORCENTAJEPERMITIDO = 10; //Porcentaje de fallos permitido en el metodo "uniforme"
int RANGOFALLO = 10; //Se usara para establecer el rango permitido en el metodo "uniforme"
int LIMITE = 15; //Limite para la division de pixeles

//DEBUG
void dividirSimple(region* nodo);


int main(int argc, char** argv)
{
	//return Test(argc, argv);

	//Obtenemos imagen
	C_Image imagen;
	C_Image::IndexT row, col;

	imagen.ReadBMP("Hercules_Gris.bmp");

	//histograma(&imagen);
	//inverso(&imagen);

	region raiz;
	raiz.num = nNodo;
	raiz.mat = imagen;

	dividir(&raiz);

	//DEBUG
	//dividirSimple(&raiz);

	fusionar();



	//imagen.WriteBMP("patata_inverso2.bmp");

}


void dividir(region* nodo) {

	//DEBUG
	if (((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) < LIMITE) {
		printf("LIMITE Nodo:%i	%i - %i = %i\n", (*nodo).num, (*nodo).mat.LastRow(), (*nodo).mat.FirstRow(), (*nodo).mat.LastRow() - (*nodo).mat.FirstRow());
		return;
	}if (((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) < LIMITE) {
		printf("LIMITE Nodo:%i	%i - %i = %i\n", (*nodo).num, (*nodo).mat.LastCol(), (*nodo).mat.FirstCol(), (*nodo).mat.LastCol() - (*nodo).mat.FirstCol());
		return;
	}


	if ((*nodo).homogeneo == -1)
		uniforme(nodo);


	if ((*nodo).homogeneo == 0) {
		//DEGUG
		printf("DIVIDIENDO Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", (*nodo).num, (*nodo).mat.FirstRow(), (*nodo).mat.LastRow(), (*nodo).mat.FirstCol(), (*nodo).mat.LastCol());

		region* hijo1 = new region;
		region* hijo2 = new region;
		region* hijo3 = new region;
		region* hijo4 = new region;

		//Arriba izquierda
		int primeraFila = (*nodo).mat.FirstRow();
		int ultimaFila = ((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow();
		int primeraCol = (*nodo).mat.FirstCol();
		int ultimaCol = ((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol();
		C_Matrix mat1((*nodo).mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);


		//Arriba derecha
		primeraFila = (*nodo).mat.FirstRow();
		ultimaFila = ((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow();
		primeraCol = (((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol()) + 1; //Para que no coincida con mat1
		ultimaCol = (*nodo).mat.LastCol();
		C_Matrix mat2((*nodo).mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);

		//Abajo izquierda
		primeraFila = (((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow()) + 1; //Para que no coincida con mat1
		ultimaFila = (*nodo).mat.LastRow();
		primeraCol = (*nodo).mat.FirstCol();
		ultimaCol = ((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol();
		C_Matrix mat3((*nodo).mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);

		//Abajo derecha
		primeraFila = (((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow()) + 1; //Para que no coincida con mat2
		ultimaFila = (*nodo).mat.LastRow();
		primeraCol = (((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol()) + 1; //Para que no coincida con mat3
		ultimaCol = (*nodo).mat.LastCol();
		C_Matrix mat4((*nodo).mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);

		(*hijo1).mat = mat1;
		(*hijo2).mat = mat2;
		(*hijo3).mat = mat3;
		(*hijo4).mat = mat4;

		prepararRegion(hijo1);
		prepararRegion(hijo2);
		prepararRegion(hijo3);
		prepararRegion(hijo4);

		//DEBUG
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", (*hijo1).num, (*hijo1).mat.FirstRow(), (*hijo1).mat.LastRow(), (*hijo1).mat.FirstCol(), (*hijo1).mat.LastCol());
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", (*hijo2).num, (*hijo2).mat.FirstRow(), (*hijo2).mat.LastRow(), (*hijo2).mat.FirstCol(), (*hijo2).mat.LastCol());
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", (*hijo3).num, (*hijo3).mat.FirstRow(), (*hijo3).mat.LastRow(), (*hijo3).mat.FirstCol(), (*hijo3).mat.LastCol());
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", (*hijo4).num, (*hijo4).mat.FirstRow(), (*hijo4).mat.LastRow(), (*hijo4).mat.FirstCol(), (*hijo4).mat.LastCol());

		dividir(hijo1);
		dividir(hijo2);
		dividir(hijo3);
		dividir(hijo4);
	}

}

void fusionar() {

	//Recorremos el vector, comprobando por parejas si son vecinos. En el caso de que los sean comprobamos si cumplen el criterio
	for (int i = 0; i < regiones.size(); i++) {
		exportar(regiones[i]);
		for (int j = 0; j < regiones.size(); j++) {
			if (i != j) {
				if (vecinos(regiones[i], regiones[j])) {
					parejaUniforme(regiones[i], regiones[j]);
				}
			}
				
		}
		
	}

	//DEBUG
	/*
	int indice = 1;
	printf("Nodo %i\n", (*regiones[indice]).num);
	for (int i = 0; i < regiones.size(); i++) {
		exportar(regiones[i]);
		if (vecinos(regiones[i], regiones[indice])) {
			printf("Vecino con %i\n", (*regiones[i]).num);
		}
	}*/

}

void prepararRegion(region* nodo){
	nNodo++;
	(*nodo).num = nNodo;
	(*nodo).pixeles = calcularPixeles(nodo);
	(*nodo).suma = (*nodo).mat.Sum();
}

bool vecinos(region* nodo1, region* nodo2) {

	int izq1 = (*nodo1).mat.FirstCol();
	int der1 = (*nodo1).mat.LastCol();
	int izq2 = (*nodo2).mat.FirstCol();
	int der2 = (*nodo2).mat.LastCol();

	int arriba1 = (*nodo1).mat.FirstRow();
	int abajo1 = (*nodo1).mat.LastRow();
	int arriba2 = (*nodo2).mat.FirstRow();
	int abajo2 = (*nodo2).mat.LastRow();


	if (abs(izq1 - der2) == 1 || abs(der1 - izq2) == 1) { //Comprobamos si las regiones son vecinos horizontales

		//Situaciones 1, 2, 4
		if (arriba1 <= arriba2 && abajo1 > arriba2) { //que sea "<" significa que esta por encima y ">" que esta por debajo
			return true;
		}

		//Situaciones 3, 5
		if (arriba2 <= arriba1 && abajo2 > arriba1) {
			return true;
		}
	}
	
	if (abs(arriba1 - abajo2) == 1 || abs(abajo1 - arriba2) == 1) { //Comprobamos si las regiones son vecinos verticales
		//situaciones 1, 2, 4
		if (izq1 <= izq2 && izq2 < der1) {
			return true;
		}

		//Situaciones 4, 5
		if (izq2 <= izq1 && izq1 < der2) {
			return true;
		}
	}

	return false;
}


void exportar(region* nodo) {
	C_Image salida((*nodo).mat);
	C_Image::IndexT row, col;
	string nombre = std::to_string((*nodo).num) + ".bmp";

	salida.WriteBMP(nombre.c_str());

}

int calcularPixeles(region* nodo) {
	return ((*nodo).mat.LastRow() - (*nodo).mat.FirstRow() + 1) * ((*nodo).mat.LastCol() - (*nodo).mat.FirstCol() + 1);
}

//Comprueba si una region es de color más o menos uniforme dentro de un rango
void uniforme(region* nodo) {
	int muestra = (*nodo).mat.Mean(); //media
	int fallos = 0;
	int pixeles = calcularPixeles(nodo);

	for (int row = (*nodo).mat.FirstRow(); row <= (*nodo).mat.LastRow(); row++) {
		for (int col = (*nodo).mat.FirstCol(); col <= (*nodo).mat.LastCol(); col++) {
			if ((*nodo).mat(row, col) < (muestra - RANGOFALLO) || (*nodo).mat(row, col) > (muestra + RANGOFALLO)) {
				fallos++;
			}
		}
	}

	int porcentajeFallos = ((double)fallos / pixeles) * 100;

	//DEBUG
	//printf("UNIFORME Nodo %i Pixeles = %i Fallos = %i Porcentaje = %i\n", (*nodo).num, pixeles, fallos, porcentajeFallos);

	if (porcentajeFallos > PORCENTAJEPERMITIDO) {
		(*nodo).homogeneo = 0; //Indicamos que no es uniforme
	}
	else {
		//Si el nodo es homogeneo lo añadimos al vector de regiones homogeneas
		(*nodo).homogeneo = 1; //Indicamos que es uniforme
		regiones.push_back(nodo);
	}
}



void parejaUniforme(region* nodo1, region* nodo2) {


	//Calculamos la media de las dos regiones

	int muestra = ((*nodo1).suma + (*nodo2).suma) / ((*nodo1).pixeles + (*nodo2).pixeles);
	int fallos = 0;


	//Comprobamos la parte del nodo1
	for (int row = (*nodo1).mat.FirstRow(); row <= (*nodo1).mat.LastRow(); row++) {
		for (int col = (*nodo1).mat.FirstCol(); col <= (*nodo1).mat.LastCol(); col++) {
			if ((*nodo1).mat(row, col) < (muestra - RANGOFALLO) || (*nodo1).mat(row, col) > (muestra + RANGOFALLO)) {
				fallos++;
			}
		}
	}

	for (int row = (*nodo2).mat.FirstRow(); row <= (*nodo2).mat.LastRow(); row++) {
		for (int col = (*nodo2).mat.FirstCol(); col <= (*nodo2).mat.LastCol(); col++) {
			if ((*nodo2).mat(row, col) < (muestra - RANGOFALLO) || (*nodo2).mat(row, col) > (muestra + RANGOFALLO)) {
				fallos++;
			}
		}
	}


	int porcentajeFallos = ((double)fallos / ((*nodo1).pixeles + (*nodo2).pixeles)) * 100;

	//DEBUG
	//printf("UNIFORME Nodo %i Pixeles = %i Fallos = %i Porcentaje = %i\n", (*nodo).num, pixeles, fallos, porcentajeFallos);

	if (porcentajeFallos > PORCENTAJEPERMITIDO) {
		//((*nodo).homogeneo = 0; //Indicamos que no es uniforme
	}
	else {
		(*nodo1).subregiones.push_back(nodo2);
		regiones.erase(std::remove(regiones.begin(), regiones.end(), nodo2), regiones.end());

		//DEBUG
		printf("Se puede unir el nodo %i con el nodo %i\n", (*nodo1).num, (*nodo2).num);
	}
}



void histograma(C_Image* imagen) {
	int Histograma[256] = { 0 };
	int HistogramaRelativo[256] = { 0 };


	for (int row = (*imagen).FirstRow(); row <= (*imagen).LastRow(); row++)
		for (int col = (*imagen).FirstCol(); col <= (*imagen).LastCol(); col++)
			Histograma[(int)(*imagen)(row, col)]++;

	double max = 0;
	for (int i = 0; i < 256; i++) {
		if (Histograma[i] > max)
			max = Histograma[i];
	}

	for (int i = 0; i < 256; i++) {
		HistogramaRelativo[i] = (Histograma[i] / max) * 100;
	}

	for (int i = 0; i < 256; i++) {
		printf("%i	%i	", i, Histograma[i]);

		for (int j = 0; j < HistogramaRelativo[i]; j++)
			cout << "*";

		cout << endl;
	}
}

void inverso(C_Image* imagen) {
	for (int row = (*imagen).FirstRow(); row <= (*imagen).LastRow(); row++)
		for (int col = (*imagen).FirstCol(); col <= (*imagen).LastCol(); col++)
			(*imagen)(row, col) = 255 - (*imagen)(row, col);
}

//DEBUG
void dividirSimple(region* nodo) {
	int limite = 3;
	//int limite = 5; //4X4

	if (nodo->num >= limite) {
		return;
	}

	region* hijo1 = new region;
	region* hijo2 = new region;
	region* hijo3 = new region;
	region* hijo4 = new region;

	//Arriba izquierda
	int primeraFila = (*nodo).mat.FirstRow();
	int ultimaFila = ((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow();
	int primeraCol = (*nodo).mat.FirstCol();
	int ultimaCol = ((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol();
	C_Matrix mat1((*nodo).mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);


	//Arriba derecha
	primeraFila = (*nodo).mat.FirstRow();
	ultimaFila = ((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow();
	primeraCol = (((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol()) + 1; //Para que no coincida con mat1
	ultimaCol = (*nodo).mat.LastCol();
	C_Matrix mat2((*nodo).mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);

	//Abajo izquierda
	primeraFila = (((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow()) + 1; //Para que no coincida con mat1
	ultimaFila = (*nodo).mat.LastRow();
	primeraCol = (*nodo).mat.FirstCol();
	ultimaCol = ((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol();
	C_Matrix mat3((*nodo).mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);

	//Abajo derecha
	primeraFila = (((*nodo).mat.LastRow() - (*nodo).mat.FirstRow()) / 2 + (*nodo).mat.FirstRow()) + 1; //Para que no coincida con mat2
	ultimaFila = (*nodo).mat.LastRow();
	primeraCol = (((*nodo).mat.LastCol() - (*nodo).mat.FirstCol()) / 2 + (*nodo).mat.FirstCol()) + 1; //Para que no coincida con mat3
	ultimaCol = (*nodo).mat.LastCol();
	C_Matrix mat4((*nodo).mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);

	(*hijo1).mat = mat1;
	(*hijo2).mat = mat2;
	(*hijo3).mat = mat3;
	(*hijo4).mat = mat4;

	nNodo++;
	(*hijo1).num = nNodo;
	nNodo++;
	(*hijo2).num = nNodo;
	nNodo++;
	(*hijo3).num = nNodo;
	nNodo++;
	(*hijo4).num = nNodo;


	regiones.erase(std::remove(regiones.begin(), regiones.end(), nodo), regiones.end());


	regiones.push_back(hijo1);
	regiones.push_back(hijo2);
	regiones.push_back(hijo3);
	regiones.push_back(hijo4);

	dividirSimple(hijo1);
	dividirSimple(hijo2);
	dividirSimple(hijo3);
	dividirSimple(hijo4);
}