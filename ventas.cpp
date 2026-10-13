#include <iostream>
#include <cstdio>

using namespace std;

const int K = 3;
const int MAX_MOZOS = 100;
const int MAX_PROD = 100;
const int MAX_VENTAS = 1000;

// Structs
struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Producto {
    int codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

// Funciones
void copiarCadena(char destino[], const char origen[]) {
    int i = 0;
    while (origen[i] != '\0') {
        destino[i] = origen[i];
        i++;
    }
    destino[i] = '\0';
}

int compararCadenas(const char cad1[], const char cad2[]) {
    int i = 0;
    while (cad1[i] != '\0' && cad1[i] == cad2[i]) {
        i++;
    }
    return cad1[i] - cad2[i];
}

void generarClaveEncriptada(const char* original, char* destino) {
    int i = 0;
    while (original[i] != '\0' && i < 19) {
        destino[i] = original[i] + K;
        i++;
    }
    destino[i] = '\0';
}

void armarNombreArchivo(char destino[], const char fecha[]) {
    int pos = 0;

    char prefijo[] = "comandas_";
    int i = 0;
    while (prefijo[i] != '\0') {
        destino[pos] = prefijo[i];
        pos++;
        i++;
    }

    int j = 0;
    while (fecha[j] != '\0') {
        destino[pos] = fecha[j];
        pos++;
        j++;
    }

    char sufijo[] = ".dat";
    int k = 0;
    while (sufijo[k] != '\0') {
        destino[pos] = sufijo[k];
        pos++;
        k++;
    }

    destino[pos] = '\0';
}


int buscarMozoPorID(Mozo arr[], int len, int idBuscado) {
    int i = 0;
    while (i < len && arr[i].idMozo != idBuscado) {
        i++;
    }
    if (i == len) return -1;
    return i;
}

int buscarProducto(Producto arr[], int len, int codBuscado) {
    int i = 0;
    while (i < len && arr[i].codigo != codBuscado) {
        i++;
    }
    if (i == len) return -1;
    return i;
}

void ordenamientoBurbujaComandas(Comanda vec[], int len) {
    for (int i = 0; i < len - 1; i++) {
        for (int j = 0; j < len - i - 1; j++) {
            if (vec[j].idMozo > vec[j+1].idMozo) {
                Comanda temp = vec[j];
                vec[j] = vec[j+1];
                vec[j+1] = temp;
            }
        }
    }
}

// Cargas en memoria
void cargarMozos(Mozo lista[], int& len) {
    FILE* arch = fopen("mozos.dat", "rb");
    if (arch != NULL) {
        while (len < MAX_MOZOS && fread(&lista[len], sizeof(Mozo), 1, arch) == 1) {
            len++;
        }
        fclose(arch);
    }
}

void cargarInventario(Producto lista[], int& len) {
    FILE* arch = fopen("inventario.dat", "rb");
    if (arch != NULL) {
        while (len < MAX_PROD && fread(&lista[len], sizeof(Producto), 1, arch) == 1) {
            len++;
        }
        fclose(arch);
    }
}

// Funcion principal
int main() {
    Mozo mozos[MAX_MOZOS];
    int lenMozos = 0;
    cargarMozos(mozos, lenMozos);

    Producto inventario[MAX_PROD];
    int lenProd = 0;
    cargarInventario(inventario, lenProd);

    Comanda ventasDia[MAX_VENTAS];
    int lenVentas = 0;

    char fechaHoy[11];
    cout << "Ingrese la fecha de hoy (DD-MM-AAAA): ";
    cin >> fechaHoy;

    char nombreArchivo[50];
    armarNombreArchivo(nombreArchivo, fechaHoy);

    FILE* archVentasPrevias = fopen(nombreArchivo, "rb");
    if (archVentasPrevias != NULL) {
        while (lenVentas < MAX_VENTAS && fread(&ventasDia[lenVentas], sizeof(Comanda), 1, archVentasPrevias) == 1) {
            lenVentas++;
        }
        fclose(archVentasPrevias);
        cout << "Se cargaron " << lenVentas << " ventas previas del dia " << fechaHoy << "." << endl;
    } else {
        cout << "Se iniciara una planilla nueva para el dia " << fechaHoy << "." << endl;
    }

    int opcion = 1;
    while (opcion != 0) {
        cout << "\n--- SISTEMA DE VENTAS ---" << endl;
        cout << "1. Ingresar como Mozo (Login)" << endl;
        cout << "0. Cerrar caja y salir" << endl;
        cout << "Opcion: ";
        cin >> opcion;

        if (opcion == 1) {
            int idIngresado;
            char passIngresada[20];
            char passEncriptada[20];

            cout << "ID de Mozo: ";
            cin >> idIngresado;
            
            cin.ignore(); 
            cout << "Contrasena: ";
            cin.getline(passIngresada, 20);

            generarClaveEncriptada(passIngresada, passEncriptada);

            int posMozo = buscarMozoPorID(mozos, lenMozos, idIngresado);

            if (posMozo != -1 && compararCadenas(mozos[posMozo].password, passEncriptada) == 0) {
                cout << "\nBienvenido/a, " << mozos[posMozo].nombre << "!" << endl;
                
                int cargarMas = 1;
                while (cargarMas != 0 && lenVentas < MAX_VENTAS) {
                    int codProd, cant;
                    cout << "Codigo de producto: ";
                    cin >> codProd;
                    
                    int posProd = buscarProducto(inventario, lenProd, codProd);
                    
                    if (posProd != -1) {
                        cout << "Stock disponible (" << inventario[posProd].descripcion << "): " << inventario[posProd].stockActual << endl;
                        cout << "Ingrese cantidad a vender: ";
                        cin >> cant;
                        
                        if (cant > 0 && cant <= inventario[posProd].stockActual) {
                            inventario[posProd].stockActual -= cant;
                            
                            ventasDia[lenVentas].idMozo = idIngresado;
                            ventasDia[lenVentas].codigoProducto = codProd;
                            ventasDia[lenVentas].cantidad = cant;
                            
                            // Comisión del 10% verificada con el lote de prueba
                            float totalVenta = inventario[posProd].precio * cant;
                            ventasDia[lenVentas].comision = totalVenta * 0.10; 
                            
                            lenVentas++;
                            cout << "Venta registrada. Total: $" << totalVenta << " | Comision (10%): $" << ventasDia[lenVentas-1].comision << endl;
                        } else {
                            cout << "Error: Cantidad invalida o stock insuficiente." << endl;
                        }
                    } else {
                        cout << "Producto no encontrado." << endl;
                    }
                    
                    cout << "\nDesea cargar otra comanda? (1 = Si, 0 = Volver al menu principal): ";
                    cin >> cargarMas;
                }
            } else {
                cout << "Credenciales incorrectas o Mozo inexistente." << endl;
            }
        }
    }

    // Guardado final
    if (lenVentas > 0) {
        ordenamientoBurbujaComandas(ventasDia, lenVentas);

        FILE* archVentas = fopen(nombreArchivo, "wb"); 
        if (archVentas != NULL) {
            for (int i = 0; i < lenVentas; i++) {
                fwrite(&ventasDia[i], sizeof(Comanda), 1, archVentas);
            }
            fclose(archVentas);
        }

        FILE* archInv = fopen("inventario.dat", "wb");
        if (archInv != NULL) {
            for (int i = 0; i < lenProd; i++) {
                fwrite(&inventario[i], sizeof(Producto), 1, archInv);
            }
            fclose(archInv);
        }
        cout << "\nCierre exitoso. Se guardo la planilla ordenada con " << lenVentas << " ventas totales y se actualizo el inventario." << endl;
    }

    return 0;
}