#include <iostream>
#include <cstdio>

using namespace std;

const int K = 3;
const int MAX_MOZOS = 100;    
const int MAX_PROD = 100;     
const int MAX_VENTAS = 1000;  

// Structs
struct ComandaHistorica {
    char fecha[11];
    char nombreMozo[50];
    int codigoProducto;
    int cantidad;
    float comision;
};

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

struct VentaTemporal {
    char fecha[11];
    Comanda info;
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

// Búsqueda Secuencial
int buscarMozoSec(Mozo arr[], int len, const char nombre[]) {
    int i = 0;
    // Causa 1: i == len | Causa 2: coincidencia
    while (i < len && compararCadenas(arr[i].nombre, nombre) != 0) {
        i++;
    }
    if (i == len) {
        return -1;
    } else {
        return i;
    }
}

// Ordenamiento Burbuja
void ordenamientoBurbuja(VentaTemporal vec[], int len) {
    for (int i = 0; i < len - 1; i++) {
        for (int j = 0; j < len - i - 1; j++) {
            int comparacionFecha = compararCadenas(vec[j].fecha, vec[j+1].fecha);
            if (comparacionFecha > 0 || (comparacionFecha == 0 && vec[j].info.idMozo > vec[j+1].info.idMozo)) {
                VentaTemporal temp = vec[j];
                vec[j] = vec[j+1];
                vec[j+1] = temp;
            }
        }
    }
}

// Corte de Control
void corteDeControlDiario(VentaTemporal vec[], int len) {
    int i = 0;
    int archivosGenerados = 0;
    
    while (i < len) {
        char fechaActual[11];
        copiarCadena(fechaActual, vec[i].fecha);
        
        char nombreArchivo[50];
        sprintf(nombreArchivo, "comandas_%s.dat", fechaActual);
        
        FILE* archDia = fopen(nombreArchivo, "wb");
        if (archDia != NULL) {
            while (i < len && compararCadenas(vec[i].fecha, fechaActual) == 0) {
                fwrite(&vec[i].info, sizeof(Comanda), 1, archDia);
                i++;
            }
            // Acción de cierre del grupo
            fclose(archDia);
            archivosGenerados++;
        } else {
            i++; 
        }
    }
    cout << "3. Se generaron " << archivosGenerados << " archivos diarios de comandas mediante corte de control." << endl;
}

// Funciones de la logica del negocio
void procesarMozos(Mozo listaMozos[], int& lenMozos) {
    FILE* archHistorico = fopen("datos/comandas_historicas.dat", "rb");
    if (archHistorico == NULL) return;

    ComandaHistorica comandaVieja;
    while (fread(&comandaVieja, sizeof(ComandaHistorica), 1, archHistorico) == 1) {
        int pos = buscarMozoSec(listaMozos, lenMozos, comandaVieja.nombreMozo);
        
        if (pos == -1 && lenMozos < MAX_MOZOS) {
            listaMozos[lenMozos].idMozo = lenMozos + 1;
            copiarCadena(listaMozos[lenMozos].nombre, comandaVieja.nombreMozo);
            listaMozos[lenMozos].totalComision = comandaVieja.comision;
            generarClaveEncriptada(comandaVieja.nombreMozo, listaMozos[lenMozos].password);
            lenMozos++;
        } else if (pos != -1) {
            listaMozos[pos].totalComision += comandaVieja.comision;
        }
    }
    fclose(archHistorico);

    FILE* archMozos = fopen("mozos.dat", "wb");
    if (archMozos != NULL) {
        for (int i = 0; i < lenMozos; i++) {
            fwrite(&listaMozos[i], sizeof(Mozo), 1, archMozos);
        }
        fclose(archMozos);
        cout << "1. Se genero mozos.dat con " << lenMozos << " mozos unicos." << endl;
    }
}

void cargarInventario(Producto inventario[], int& lenProd) {
    FILE* archInvOrig = fopen("datos/inventario.dat", "rb");
    if (archInvOrig != NULL) {
        while (lenProd < MAX_PROD && fread(&inventario[lenProd], sizeof(Producto), 1, archInvOrig) == 1) {
            lenProd++;
        }
        fclose(archInvOrig);
    }
}

void procesarVentasYStock(Mozo listaMozos[], int lenMozos, Producto inventario[], int lenProd, VentaTemporal ventas[], int& lenVentas) {
    FILE* archHistorico = fopen("datos/comandas_historicas.dat", "rb");
    if (archHistorico == NULL) return;

    ComandaHistorica comandaVieja;
    while (lenVentas < MAX_VENTAS && fread(&comandaVieja, sizeof(ComandaHistorica), 1, archHistorico) == 1) {
        
        // Descontar stock
        for (int i = 0; i < lenProd; i++) {
            if (inventario[i].codigo == comandaVieja.codigoProducto) {
                inventario[i].stockActual -= comandaVieja.cantidad;
                break;
            }
        }
        
        // Cargar array temporal
        copiarCadena(ventas[lenVentas].fecha, comandaVieja.fecha);
        int posMozo = buscarMozoSec(listaMozos, lenMozos, comandaVieja.nombreMozo);
        ventas[lenVentas].info.idMozo = listaMozos[posMozo].idMozo;
        ventas[lenVentas].info.codigoProducto = comandaVieja.codigoProducto;
        ventas[lenVentas].info.cantidad = comandaVieja.cantidad;
        ventas[lenVentas].info.comision = comandaVieja.comision;
        lenVentas++;
    }
    fclose(archHistorico);

    FILE* archInvNuevo = fopen("inventario.dat", "wb");
    if (archInvNuevo != NULL) {
        for (int i = 0; i < lenProd; i++) {
            fwrite(&inventario[i], sizeof(Producto), 1, archInvNuevo);
        }
        fclose(archInvNuevo);
        cout << "2. Se genero inventario.dat con el stock actualizado." << endl;
    }
}

// Funcion Principal
int main() {
    // Declaración con tamaños físicos
    Mozo listaMozos[MAX_MOZOS];
    int lenMozos = 0;

    Producto inventario[MAX_PROD];
    int lenProd = 0;

    VentaTemporal ventas[MAX_VENTAS];
    int lenVentas = 0;

    // Llamadas a las funciones
    procesarMozos(listaMozos, lenMozos);
    
    cargarInventario(inventario, lenProd);
    
    procesarVentasYStock(listaMozos, lenMozos, inventario, lenProd, ventas, lenVentas);
    
    ordenamientoBurbuja(ventas, lenVentas);
    
    corteDeControlDiario(ventas, lenVentas);

    return 0;
}