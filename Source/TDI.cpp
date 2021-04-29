#include <C_Image.hpp>
#include <vector>
#include <iostream>
int Test(int argc, char** argv);

struct region {
	vector<region*> subregiones; //Vector donde se incluyen las regiones al unir nodos
	C_Matrix mat; //Matriz de instensidades
	int num = -1; //Id de la region
	int pixeles; //Total de pixeles (incluye los de las subregiones)
	int suma; //Suma de las instensidades de los pixeles (incluye los de las subregiones)
	bool disponible = true; //Indica si una region esta disponible para fusionarse con otra
	int color = -1; //Indica el color de una region (se asigna en la fusion)
	int homogeneo = -1; //Indica si una region cumple el criterio de homogenidad 
	/*	0 - no cumple el criterio
	*	1 - cumple el criterio
	*	-1 - no se ha comprobado
	*/
};

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
void separacionFondo();

int nNodo = 0;
int COLOR = 0;
int PORCENTAJEDIVISION = 5; //Porcentaje de fallos permitido en el metodo uniforme()
int PORCENTAJEFUSION= 5; //Porcentaje de fallos permitido en el metodo uniforme()
int RANGOFALLODIVISION = 10; //Se usara para establecer el rango permitido en el metodo uniforme()
int RANGOFALLOFUSION = 10; //Se usara para establecer el rango permitido en el metodo uniforme()
int LIMITE = 1; //Limite para la division de pixeles
int FACTORDIVISION = 1; //Limite para la comprobacion en el metodo megaFusion
int LIMITESEPARACIONFONDO = 100;
bool separacionFondoElegida = false;
bool modoDivision;
vector<int> coloresDisponibles; //Vector en el que se guardan los colores que se han dejado de usar para poder reutilizarlos mas tarde
vector<region*> regiones; //Vector en el que guardamos todas las regiones homogeneas
C_Image salidaSegmentacion;
C_Image salidaSeparacionFondo;

//DEBUG
void dividirSimple(region* nodo);


int main(int argc, char** argv)
{
	//return Test(argc, argv);

	//Proceso de toma de datos
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

	printf("Desea modificar las variables para la SEGMENTACION? (S/N): ");
	getline(cin, respuesta);

	for (auto& c : respuesta) c = toupper(c);

	if (respuesta == "S" || respuesta == "SI") {
		printf("Introduce el porcentaje de fallos permitido en la DIVISION: ");
		getline(cin, respuesta);
		PORCENTAJEDIVISION = stoi(respuesta);

		printf("Introduce el rango de diferencia con la media permitido en la DIVISION: ");
		getline(cin, respuesta);
		RANGOFALLODIVISION = stoi(respuesta);

		printf("Introduce el porcentaje de fallos permitido en la FUSION: ");
		getline(cin, respuesta);
		PORCENTAJEFUSION = stoi(respuesta);

		printf("Introduce el rango de diferencia con la media permitido en la FUSION: ");
		getline(cin, respuesta);
		RANGOFALLOFUSION = stoi(respuesta);

		printf("Introduce el limite para la division de nodos: ");
		getline(cin, respuesta);
		LIMITE = stoi(respuesta);
	}

	printf("Desea realizar la SEPARACION DE FONDO? (S/N): ");
	getline(cin, respuesta);
	for (auto& c : respuesta) c = toupper(c);

	if (respuesta == "S" || respuesta == "SI") {
		separacionFondoElegida = true;

		printf("Desea modificar las variables para la SEPARACION DE FONDO? (S/N): ");
		getline(cin, respuesta);

		for (auto& c : respuesta) c = toupper(c);

		if (respuesta == "S" || respuesta == "SI") {
			printf("Introduce el factor de division para la separacion de fondo: ");
			getline(cin, respuesta);
			FACTORDIVISION = stoi(respuesta);

			printf("Introduce el limite para la separacion de fondo: ");
			getline(cin, respuesta);
			LIMITESEPARACIONFONDO = stoi(respuesta);
		}
	}

	//Donde se guardara el resultado de la segmentacion
	salidaSegmentacion = imagen; //Partimos de la imagen original
	salidaSegmentacion.SetValue(0); //Ponemos todos los valores de la matriz a 0 (negro)

	//Donde se guardara el resultado de la separacion de fondo
	salidaSeparacionFondo = imagen; //Partimos de la imagen original
	salidaSeparacionFondo.SetValue(255);  //Ponemos todos los valores de la matriz a 255 (blanco)

	//El nodo region raiz sera la imagen original
	region raiz;
	raiz.num = nNodo;
	raiz.mat = imagen;

	modoDivision = true; //Sirve para poder diferenciar que variables usar mas tarde
	dividir(&raiz);

	//DEBUG: metodo que divide sin tener en cuenta el criterio
	//dividirSimple(&raiz);

	modoDivision = false;
	fusionar();

	//Exportamos el resultado de la segmentacion
	salidaSegmentacion.palette.Read("PaletaSurtida256.txt");
	salidaSegmentacion.WriteBMP("salidaSegmentacion.bmp");

	printf("Numero de colores utilizados: %i\n", COLOR);

	if (separacionFondoElegida) {
		separacionFondo();
	}

}

/*Metodo recursivo que divide una region en otras 4 si no se cumple el criterio establecido
*/
void dividir(region* nodo) {

	//Se detiene la division si una region tiene un ancho o alto menor que el limite establecido
	if ((nodo->mat.LastRow() - nodo->mat.FirstRow()) < LIMITE) {
		regiones.push_back(nodo);
		//DEGUG: proceso de resta para comprobar si se cumple el limite
		printf("LIMITE Nodo:%i	%i - %i = %i\n", nodo->num, nodo->mat.LastRow(), nodo->mat.FirstRow(), nodo->mat.LastRow() - nodo->mat.FirstRow());
		return;
	}if ((nodo->mat.LastCol() - nodo->mat.FirstCol()) < LIMITE) {
		regiones.push_back(nodo);
		//DEGUG: proceso de resta para comprobar si se cumple el limite
		printf("LIMITE Nodo:%i	%i - %i = %i\n", nodo->num, nodo->mat.LastCol(), nodo->mat.FirstCol(), nodo->mat.LastCol() - nodo->mat.FirstCol());
		return;
	}


	//Se comprueba si un nodo es homogeneo en el caso de que no se haya hecho antes
	if (nodo->homogeneo == -1)
		uniforme(nodo);


	if (nodo->homogeneo == 0) { //Si un nodo NO es homogeneo se divide
		//DEGUG: info sobre el nodo
		printf("DIVIDIENDO Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", nodo->num, nodo->mat.FirstRow(), nodo->mat.LastRow(), nodo->mat.FirstCol(), nodo->mat.LastCol());
		
		//Creamos las 4 regiones en las que se dividira el nodo
		region* hijo1 = new region;
		region* hijo2 = new region;
		region* hijo3 = new region;
		region* hijo4 = new region;

		//Arriba izquierda
		int primeraFila = nodo->mat.FirstRow();
		int ultimaFila = (nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow();
		int primeraCol = nodo->mat.FirstCol();
		int ultimaCol = (nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol();
		
		//Usamos el constructor que crea una submatriz para partir de la matriz del nodo
		C_Matrix mat1(nodo->mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);


		//Arriba derecha
		primeraFila = nodo->mat.FirstRow();
		ultimaFila = (nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow();
		primeraCol = ((nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol()) + 1; //Sumamos 1 para que no coincida con mat1
		ultimaCol = nodo->mat.LastCol();
		C_Matrix mat2(nodo->mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);

		//Abajo izquierda
		primeraFila = ((nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow()) + 1; //Sumamos 1 para que no coincida con mat1
		ultimaFila = nodo->mat.LastRow();
		primeraCol = nodo->mat.FirstCol();
		ultimaCol = (nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol();
		C_Matrix mat3(nodo->mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);

		//Abajo derecha
		primeraFila = ((nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow()) + 1; //Sumamos 1 para que no coincida con mat2
		ultimaFila = nodo->mat.LastRow();
		primeraCol = ((nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol()) + 1; //Sumamos 1 para que no coincida con mat3
		ultimaCol = nodo->mat.LastCol();
		C_Matrix mat4(nodo->mat,
			primeraFila, ultimaFila,
			primeraCol, ultimaCol,
			primeraFila, primeraCol);

		//Asignamos cada matriz al nodo correspondiente
		hijo1->mat = mat1;
		hijo2->mat = mat2;
		hijo3->mat = mat3;
		hijo4->mat = mat4;

		prepararRegion(hijo1);
		prepararRegion(hijo2);
		prepararRegion(hijo3);
		prepararRegion(hijo4);

		//DEBUG: info sobre cada hijo
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", hijo1->num, hijo1->mat.FirstRow(), hijo1->mat.LastRow(), hijo1->mat.FirstCol(), hijo1->mat.LastCol());
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", hijo2->num, hijo2->mat.FirstRow(), hijo2->mat.LastRow(), hijo2->mat.FirstCol(), hijo2->mat.LastCol());
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", hijo3->num, hijo3->mat.FirstRow(), hijo3->mat.LastRow(), hijo3->mat.FirstCol(), hijo3->mat.LastCol());
		printf("Nodo:%i FirstRow=%i LastRow=%i FirstCol=%i LastCol=%i\n", hijo4->num, hijo4->mat.FirstRow(), hijo4->mat.LastRow(), hijo4->mat.FirstCol(), hijo4->mat.LastCol());

		//Llamada recursiva al metodo dividir
		dividir(hijo1);
		dividir(hijo2);
		dividir(hijo3);
		dividir(hijo4);
	}

}

/*Metodo que fusiona una region con otra si se cumple el criterio establecido
*/
void fusionar() {

	//Recorremos el vector de regiones homogeneas, comprobando si son vecinos. En el caso de que lo sean comprobamos si cumplen el criterio
	bool regionesVecinas = false;
	for (int i = 0; i < regiones.size(); i++) { //Nos referiremos a este como Region 1 (region principal)

		for (int j = 0; j < regiones.size(); j++) {  //Nos referiremos a este como Region 2 (region secundaria que se fusionaria, pasandose a la principal)
			regionesVecinas = false;
			if (i != j && regiones[j]->disponible && regiones[i]->disponible) {

				if (vecinos(regiones[i], regiones[j])) {
					regionesVecinas = true;
				}
				else {
					//Comprobamos si alguna subregion de la region 1 es vecina con la region de region 2
					for (int k = 0; k < regiones[i]->subregiones.size(); k++) {
						if (vecinos(regiones[i]->subregiones[k], regiones[j])) {
							regionesVecinas = true;
							break;
						}
					}

					//Comprobamos si alguna subregion de la region 2 es vecina con la region de region 1
					for (int k = 0; k < regiones[j]->subregiones.size(); k++) {
						if (vecinos(regiones[j]->subregiones[k], regiones[i])) {
							regionesVecinas = true;
							break;
						}

					}

					//Comprobamos si alguna subregion de la region 1 es vecina con alguna subregion de la region 2
					for (int k = 0; k < regiones[i]->subregiones.size(); k++) {
						for (int z = 0; z < regiones[j]->subregiones.size(); z++) {
							if (vecinos(regiones[i]->subregiones[k], regiones[j]->subregiones[z])) {
								regionesVecinas = true;
								break;
							}

						}
					}
				}

				//En el caso de que haya algun vecino se comprueba si los dos nodos juntos cumplen el criterio
				if (regionesVecinas) {
					parejaUniforme(regiones[i], regiones[j]);
				}
			}

		}

	}


	//DEBUG: Muestra con que regiones se ha fusionado cada region principal
	for (int i = 0; i < regiones.size(); i++) {
		if (regiones[i]->disponible) { 
			printf("\nNodo %i se fusiona con: ", regiones[i]->num);
			for (int j = 0; j < regiones[i]->subregiones.size(); j++) {
				printf("nodo %i ", regiones[i]->subregiones[j]->num);
			}
		}
	}

	//Proceso de asignar color a las regiones
	COLOR = 0;
	for (int g = 0; g < regiones.size(); g++) {
		if (regiones[g]->disponible && regiones[g]->pixeles > 5) {
			COLOR++;

			//DEBUG: Muestra las regiones principales con su color asignado y las regiones con las que se ha fusionado
			printf("Region %i, Color %i, Subregiones :", regiones[g]->num, COLOR);
			for (int i = regiones[g]->mat.FirstRow(); i <= regiones[g]->mat.LastRow(); i++) {
				for (int j = regiones[g]->mat.FirstCol(); j <= regiones[g]->mat.LastCol(); j++) {
					salidaSegmentacion(i, j) =	COLOR;
				}
			}

			for (int k = 0; k < regiones[g]->subregiones.size(); k++) {
				//DEBUG: parte del debug anterior
				printf("%i ", regiones[g]->subregiones[k]->num);
				for (int i = regiones[g]->subregiones[k]->mat.FirstRow(); i <= regiones[g]->subregiones[k]->mat.LastRow(); i++) {
					for (int j = regiones[g]->subregiones[k]->mat.FirstCol(); j <= regiones[g]->subregiones[k]->mat.LastCol(); j++) {
						salidaSegmentacion(i, j) = COLOR;
					}
				}
			}
			printf("\n");
		}
	}

	//DEBUG: para comprobar si el metodo de vecinos funciona correctamente
	/*
	int indice = 4;
	printf("Nodo %i\n", (*regiones[indice]).num);
	for (int i = 0; i < regiones.size(); i++) {
		exportar(regiones[i]);
		if (vecinos(regiones[i], regiones[indice])) {
			printf("Vecino con %i\n", regiones[i]->num);
		}
	}*/

}

/*Metodo para asignar un id, numero de pixeles y suma de los pixeles a un nodo
*/
void prepararRegion(region* nodo) {
	nNodo++;
	nodo->num = nNodo;
	nodo->pixeles = calcularPixeles(nodo);
	nodo->suma = nodo->mat.Sum();
}

/*Metodo que comprueba si dos regiones son vecinas
*/
bool vecinos(region* nodo1, region* nodo2) {

	int izq1 = nodo1->mat.FirstCol(); 
	int der1 = nodo1->mat.LastCol(); 
	int izq2 = nodo2->mat.FirstCol(); 
	int der2 = nodo2->mat.LastCol(); 

	int arriba1 = nodo1->mat.FirstRow(); 
	int abajo1 = nodo1->mat.LastRow();
	int arriba2 = nodo2->mat.FirstRow();
	int abajo2 = nodo2->mat.LastRow();


	if (abs(izq1 - der2) == 1 || abs(der1 - izq2) == 1) { //Comprobamos si las regiones tienen columnas adyacentes

		//Comprobamos si alguna fila del nodo2 esta dentro del rango de filas del nodo1
		if (arriba1 <= arriba2 && abajo1 >= arriba2) { 
			return true;
		}

		//Comprobamos si alguna fila del nodo1 esta dentro del rango de filas del nodo2
		if (arriba2 <= arriba1 && abajo2 >= arriba1) {
			return true;
		}
	}

	if (abs(arriba1 - abajo2) == 1 || abs(abajo1 - arriba2) == 1) { //Comprobamos si las regiones tienen filas adyacentes
		
		//Comprobamos si alguna columna del nodo2 esta dentro del rango de columnas del nodo1
		if (izq1 <= izq2 && izq2 <= der1) {
			return true;
		}

		//Comprobamos si alguna columna del nodo1 esta dentro del rango de columnas del nodo2
		if (izq2 <= izq1 && izq1 <= der2) {
			return true;
		}
	}

	return false;
}

/*DEBUG:Metodo que exporta el nodo que se le pasa
*/
void exportar(region* nodo) {
	C_Image salida(nodo->mat);
	C_Image::IndexT row, col;
	string nombre = std::to_string(nodo->num) + ".bmp";

	salida.WriteBMP(nombre.c_str());
}

/*Metodo que devuelve el numero de pixeles de un nodo
*/
int calcularPixeles(region* nodo) {
	return (nodo->mat.LastRow() - nodo->mat.FirstRow() + 1) * (nodo->mat.LastCol() - nodo->mat.FirstCol() + 1);
}

/*Metodo que comprueba si un nodo cumple el criterio de homogenidad establecido, 
utilizando la media de su matriz y calculando el porcentaje de pixeles que estan fuera del rango permitido
*/
void uniforme(region* nodo) {
	int media = nodo->mat.Mean();
	int fallos = 0;
	int pixeles = calcularPixeles(nodo);

	fallos = calcularFallos(nodo, media);

	int porcentajeFallos = ((double)fallos / pixeles) * 100;

	//DEBUG: muestra el resultado de la comprobacion
	printf("UNIFORME Nodo %i Pixeles = %i Fallos = %i Porcentaje = %i\n", nodo->num, pixeles, fallos, porcentajeFallos);


	if (porcentajeFallos > PORCENTAJEDIVISION) { //No cumple el criterio
		nodo->homogeneo = 0; 
	}
	else { //Cumple el criterio
		nodo->homogeneo = 1; 
		regiones.push_back(nodo);
	}
}


/*Metodo que comprueba si la union de dos nodos y sus subregiones cumplen el criterio de homogenidad
*/
void parejaUniforme(region* nodo1, region* nodo2) {


	//Calculamos la media de las dos regiones juntas (incluye las subregiones tambien)
	int media = (nodo1->suma + nodo2->suma) / (nodo1->pixeles + nodo2->pixeles);
	int pixeles = nodo1->pixeles + nodo2->pixeles;
	int fallos = 0;

	//Calculamos los fallos del nodo1 y sus subregiones
	fallos += calcularFallos(nodo1, media);
	for (int i = 0; i < nodo1->subregiones.size(); i++) {
		fallos += calcularFallos(nodo1->subregiones[i], media);
	}

	//Calculamos los fallos del nodo2 y sus subregiones
	fallos += calcularFallos(nodo2, media);
	for (int i = 0; i < nodo2->subregiones.size(); i++) {
		fallos += calcularFallos(nodo2->subregiones[i], media);
	}

	int porcentajeFallos = ((double)fallos / pixeles) * 100;

	if (porcentajeFallos < PORCENTAJEFUSION) { //Cumple el criterio

		//Fusionamos el nodo2 al nodo1
		nodo1->subregiones.push_back(nodo2);
		nodo1->pixeles += nodo2->pixeles;
		nodo1->suma += nodo2->suma;

		for (int i = 0; i < nodo2->subregiones.size(); i++) {
			nodo1->subregiones.push_back(nodo2->subregiones[i]);
		}

		//Marcamos el nodo2 como no disponible para que no se pueda fusionar con otro nodo
		nodo2->disponible = false; 
	}
}

/*Metodo que separa el fondo de una imagen,
* solo funciona hay una diferencia de tamanio considerable entre las regiones del fondo y las del objeto
*/
void separacionFondo() {
	//Buscamos la region con una mayor numero de pixeles
	int max = regiones[0]->pixeles;
	int indice = 0;
	bool actualizado; //indica si se ha actualizado el max en la iteracion actual

	for (int g = 0; g < LIMITESEPARACIONFONDO; g++) {
		actualizado = false;
		//DEBUG: indica el max con el que se ha entrado en el bucle
		printf("Max entrada: %i\n", max);
		for (int i = 0; i < regiones.size(); i++) {
			if (regiones[i]->pixeles > max && regiones[i]->disponible) {
				indice = i;
				max = regiones[i]->pixeles;
				actualizado = true;
			}
		}

		//DEBUG: indica cuando no se ha encontrado un maximo nuevo
		if (!actualizado && LIMITESEPARACIONFONDO == 100) {
			printf("BREAK SEPARACIONFONDO");
			break;
		}

		//DEBUG: indica el max con el que se ha salido del bucle
		printf("Max escogido; %i\n", max);

		regiones[indice]->disponible = false; //Indicamos la region como no disponible para no tenerla en cuenta para un maximo nuevo
		max = max / FACTORDIVISION; //Actualizamos el maximo para que en la siguiente iteracion busque uno mayor que dividido


		//Actualizamos el valor correspondiente a la region en la matriz de la imagen de salida
		for (int i = regiones[indice]->mat.FirstRow(); i <= regiones[indice]->mat.LastRow(); i++) {
			for (int j = regiones[indice]->mat.FirstCol(); j <= regiones[indice]->mat.LastCol(); j++) {
				salidaSeparacionFondo(i, j) = 0;
			}
		}

		for (int k = 0; k < regiones[indice]->subregiones.size(); k++) {
			for (int i = regiones[indice]->subregiones[k]->mat.FirstRow(); i <= regiones[indice]->subregiones[k]->mat.LastRow(); i++) {
				for (int j = regiones[indice]->subregiones[k]->mat.FirstCol(); j <= regiones[indice]->subregiones[k]->mat.LastCol(); j++) {
					salidaSeparacionFondo(i, j) = 0;
				}
			}
		}

	}

	salidaSeparacionFondo.WriteBMP("salidaSeparacionFondo.bmp");
}

/*Metodo que calcula los fallos de un nodo comprobando si los pixeles estan dentro de un rango establecido
*/
int calcularFallos(region* nodo, int media) {
	//Dependiendo de si los estamos calculando para la fusion o la division debemos usar valores diferentes
	int rangoComparacion;
	if (modoDivision) {
		rangoComparacion = RANGOFALLODIVISION;
	}
	else {
		rangoComparacion = RANGOFALLOFUSION;
	}

	int rangoSuperior = media + rangoComparacion;
	int rangoInferior = abs(media - rangoComparacion);

	int fallos = 0;
	for (int row = nodo->mat.FirstRow(); row <= nodo->mat.LastRow(); row++) {
		for (int col = nodo->mat.FirstCol(); col <= nodo->mat.LastCol(); col++) {
			if (nodo->mat(row, col) <= rangoInferior || nodo->mat(row, col) >= rangoSuperior) {
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

/*DEBUG: metodo que divide nodos sin tener en cuenta el criterio de homogenidad
*/
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
	int primeraFila = nodo->mat.FirstRow();
	int ultimaFila = (nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow();
	int primeraCol = nodo->mat.FirstCol();
	int ultimaCol = (nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol();
	C_Matrix mat1(nodo->mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);


	//Arriba derecha
	primeraFila = nodo->mat.FirstRow();
	ultimaFila = (nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow();
	primeraCol = ((nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol()) + 1; //Para que no coincida con mat1
	ultimaCol = nodo->mat.LastCol();
	C_Matrix mat2(nodo->mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);

	//Abajo izquierda
	primeraFila = ((nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow()) + 1; //Para que no coincida con mat1
	ultimaFila = nodo->mat.LastRow();
	primeraCol = nodo->mat.FirstCol();
	ultimaCol = (nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol();
	C_Matrix mat3(nodo->mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);

	//Abajo derecha
	primeraFila = ((nodo->mat.LastRow() - nodo->mat.FirstRow()) / 2 + nodo->mat.FirstRow()) + 1; //Para que no coincida con mat2
	ultimaFila = nodo->mat.LastRow();
	primeraCol = ((nodo->mat.LastCol() - nodo->mat.FirstCol()) / 2 + nodo->mat.FirstCol()) + 1; //Para que no coincida con mat3
	ultimaCol = nodo->mat.LastCol();
	C_Matrix mat4(nodo->mat,
		primeraFila, ultimaFila,
		primeraCol, ultimaCol,
		primeraFila, primeraCol);

	hijo1->mat = mat1;
	hijo2->mat = mat2;
	hijo3->mat = mat3;
	hijo4->mat = mat4;

	nNodo++;
	hijo1->num = nNodo;
	nNodo++;
	hijo2->num = nNodo;
	nNodo++;
	hijo3->num = nNodo;
	nNodo++;
	hijo4->num = nNodo;


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