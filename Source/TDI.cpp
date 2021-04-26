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
	bool disponible = true;
	int color;
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
int calcularFallos(region* nodo, int media);
void fusionar();
void megaFusion();

int nNodo = 0;
int PORCENTAJEPERMITIDO = 5; //Porcentaje de fallos permitido en el metodo "uniforme"
int RANGOFALLO = 10; //Se usara para establecer el rango permitido en el metodo "uniforme"
int LIMITE = 1; //Limite para la division de pixeles
int INCREMENTO = 10; //
int COLOR = INCREMENTO;
int FACTORDIVISION = 1;
int LIMITEMEGAFUSION = 100;
C_Image preview;
C_Image salidaMega;

//DEBUG
void dividirSimple(region* nodo);


int main(int argc, char** argv)
{
	//return Test(argc, argv);

	//Obtenemos imagen
	C_Image imagen;
	C_Image::IndexT row, col;

	string respuesta;

	while (true) {
		try
		{
			printf("Introduce el nombre del archivo: ");
			getline(cin, respuesta);

			imagen.ReadBMP(respuesta.c_str());
			break;
		}
		catch (exception e)
		{
			cout << "El nombre del archivo no es valido" << '\n';
		}
		
	}
	

	

	printf("Desea modificar las variables? (S/N): ");
	getline(cin, respuesta);

	for (auto& c : respuesta) c = toupper(c);

	if (respuesta == "S" || respuesta == "SI") {
		printf("Introduce el porcentaje de fallos permitido: ");
		getline(cin, respuesta);
		PORCENTAJEPERMITIDO = stoi(respuesta);

		printf("Introduce el rango de diferencia con la media permitido: ");
		getline(cin, respuesta);
		RANGOFALLO = stoi(respuesta);

		printf("Introduce el limite para la division de nodos: ");
		getline(cin, respuesta);
		LIMITE = stoi(respuesta);

		printf("Introduce el incremento para cada color de la salida: ");
		getline(cin, respuesta);
		INCREMENTO = stoi(respuesta);

		printf("Introduce el factor de division: ");
		getline(cin, respuesta);
		FACTORDIVISION = stoi(respuesta);

		printf("Introduce el limite para la mega fusion: ");
		getline(cin, respuesta);
		LIMITEMEGAFUSION = stoi(respuesta);
	}



	preview = imagen;
	salidaMega = imagen;
	salidaMega.SetValue(255);
	preview.SetValue(0);

	//histograma(&imagen);
	//inverso(&imagen);

	region raiz;
	raiz.num = nNodo;
	raiz.mat = imagen;

	dividir(&raiz);

	//DEBUG
	//dividirSimple(&raiz);

	fusionar();
	preview.palette.Read("PaletaSurtida256.txt");
	preview.WriteBMP("preview.bmp");

	megaFusion();
	//imagen.WriteBMP("cuadro_exportado.bmp");

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
	
	bool regionesVecinas = false;
		for (int i = 0; i < regiones.size(); i++) {
			//exportar(regiones[i]);
			for (int j = 0; j < regiones.size(); j++) {
				regionesVecinas = false;
				if (i != j && regiones[j]->disponible && regiones[i]->disponible) {
				
					if (vecinos(regiones[i], regiones[j])) {
						regionesVecinas = true;
					}else{
						//Subregiones de 1 con region de 2
						for (int k = 0; k < regiones[i]->subregiones.size(); k++) {
							if (vecinos(regiones[i]->subregiones[k], regiones[j])) {
								regionesVecinas = true;
								break;
							}
								
						}

						//Subregiones de 2 con region de 1
						for (int k = 0; k < regiones[j]->subregiones.size(); k++) {
							if (vecinos(regiones[j]->subregiones[k], regiones[i])) {
								regionesVecinas = true;
								break;
							}
								
						}

						//Subregiones de 1 con subregiones de 2
						for (int k = 0; k < regiones[i]->subregiones.size(); k++) {
							for (int z = 0; z < regiones[j]->subregiones.size(); z++) {
								if (vecinos(regiones[i]->subregiones[k], regiones[j]->subregiones[z])) {
									regionesVecinas = true;
									break;
								}
									
							}
						}
					}
					

					if (regionesVecinas) {
						parejaUniforme(regiones[i], regiones[j]);
					}
				}

			}

		}
	

	//DEBUG
	for (int i = 0; i < regiones.size(); i++) {
		if(regiones[i]->disponible){
			printf("\nNodo %i se fusiona con: ", regiones[i]->num);
			for (int j = 0; j < (*regiones[i]).subregiones.size(); j++) {
				printf("nodo %i ", (*regiones[i]).subregiones[j]->num);

			}
		}
		

	}
	

	//DEBUG
	/*
	int indice = 4;
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
		if (arriba1 <= arriba2 && abajo1 >= arriba2) { //que sea "<" significa que esta por encima y ">" que esta por debajo
			return true;
		}

		//Situaciones 3, 5
		if (arriba2 <= arriba1 && abajo2 >= arriba1) {
			return true;
		}
	}
	
	if (abs(arriba1 - abajo2) == 1 || abs(abajo1 - arriba2) == 1) { //Comprobamos si las regiones son vecinos verticales
		//situaciones 1, 2, 4
		if (izq1 <= izq2 && izq2 <= der1) {
			return true;
		}

		//Situaciones 4, 5

		if (izq2 <= izq1 && izq1 <= der2) {
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
	int media = (*nodo).mat.Mean();
	int fallos = 0;
	int pixeles = calcularPixeles(nodo);

	fallos = calcularFallos(nodo, media);

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

		
		/*
		for (int i = nodo->mat.FirstRow(); i <= nodo->mat.LastRow(); i++) {
			for (int j = nodo->mat.FirstCol(); j <= nodo->mat.LastCol(); j++) {
				preview(i, j) = nodo->num;
			}
		}*/
	}
}



void parejaUniforme(region* nodo1, region* nodo2) {


	//Calculamos la media de las dos regiones

	int media = ((*nodo1).suma + (*nodo2).suma) / ((*nodo1).pixeles + (*nodo2).pixeles);
	int fallos = 0;

	fallos += calcularFallos(nodo1, media);
	for (int i = 0; i < (*nodo1).subregiones.size(); i++) {
		fallos += calcularFallos((*nodo1).subregiones[i], media);
	}

	fallos += calcularFallos(nodo2, media);
	for (int i = 0; i < (*nodo2).subregiones.size(); i++) {
		fallos += calcularFallos((*nodo2).subregiones[i], media);
	}

	int porcentajeFallos = ((double)fallos / ((*nodo1).pixeles + (*nodo2).pixeles)) * 100;

	//DEBUG
	//printf("UNIFORME Nodo %i Pixeles = %i Fallos = %i Porcentaje = %i\n", (*nodo).num, pixeles, fallos, porcentajeFallos);

	if (porcentajeFallos > PORCENTAJEPERMITIDO) {
		//((*nodo).homogeneo = 0; //Indicamos que no es uniforme
	}
	else {
		if (nodo1->subregiones.empty()) {
			COLOR += INCREMENTO;
			nodo1->color = COLOR;

			for (int i = nodo1->mat.FirstRow(); i <= nodo1->mat.LastRow(); i++) {
				for (int j = nodo1->mat.FirstCol(); j <= nodo1->mat.LastCol(); j++) {
					preview(i, j) = nodo1->color;
				}
			}
		}
		(*nodo1).subregiones.push_back(nodo2);
		(*nodo1).pixeles += (*nodo2).pixeles;
		(*nodo1).suma += (*nodo2).suma;

		
		
		for (int i = 0; i < (*nodo2).subregiones.size(); i++) {
			(*nodo1).subregiones.push_back((*nodo2).subregiones[i]);
		}

		

		for (int k = 0; k < nodo1->subregiones.size(); k++) {
			for (int i = nodo1->subregiones[k]->mat.FirstRow(); i <= nodo1->subregiones[k]->mat.LastRow(); i++) {
				for (int j = nodo1->subregiones[k]->mat.FirstCol(); j <= nodo1->subregiones[k]->mat.LastCol(); j++) {
					preview(i, j) = nodo1->color;
				}
			}
		}
		

		(*nodo2).disponible = false;
		//regiones.erase(std::remove(regiones.begin(), regiones.end(), nodo2), regiones.end());

		//DEBUG
		//printf("Se puede unir el nodo %i con el nodo %i\n", (*nodo1).num, (*nodo2).num);
	}
}

void megaFusion() {
	int max = regiones[0]->pixeles;
	int indice = 0;
	bool actualizado;

	for(int g = 0; g < LIMITEMEGAFUSION; g++ ){
		actualizado = false;
		printf("Max entrada: %i\n", max);
		for (int i = 0; i < regiones.size(); i++) {
			if (regiones[i]->pixeles > max && regiones[i]->disponible) {
				indice = i;
				max = regiones[i]->pixeles;
				actualizado = true;
			}
		}
		if (!actualizado && LIMITEMEGAFUSION == 100) {
			printf("BREAK MEGAFUSION");
			break;
		}
		printf("Max escogido; %i\n", max);

		regiones[indice]->disponible = false;
		max = max/FACTORDIVISION;

		for (int i = regiones[indice]->mat.FirstRow(); i <= regiones[indice]->mat.LastRow(); i++) {
			for (int j = regiones[indice]->mat.FirstCol(); j <= regiones[indice]->mat.LastCol(); j++) {
				salidaMega(i, j) = 0;
			}
		}


		for (int k = 0; k < regiones[indice]->subregiones.size(); k++) {
			for (int i = regiones[indice]->subregiones[k]->mat.FirstRow(); i <= regiones[indice]->subregiones[k]->mat.LastRow(); i++) {
				for (int j = regiones[indice]->subregiones[k]->mat.FirstCol(); j <= regiones[indice]->subregiones[k]->mat.LastCol(); j++) {
					salidaMega(i, j) = 0;
				}
			}
		}

	

	}

	salidaMega.WriteBMP("salidaMega.bmp");

}

int calcularFallos(region* nodo, int media) {
	int fallos = 0;
	for (int row = (*nodo).mat.FirstRow(); row <= (*nodo).mat.LastRow(); row++) {
		for (int col = (*nodo).mat.FirstCol(); col <= (*nodo).mat.LastCol(); col++) {
			if ((*nodo).mat(row, col) < abs(media - RANGOFALLO) || (*nodo).mat(row, col) > (media + RANGOFALLO)) {
				fallos++;
			}
		}
	}

	return fallos;
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