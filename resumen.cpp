

#include <iostream>
#include <cstdio>    // solo para archivos: FILE, fopen, fread, fclose
using namespace std;

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};


void copiarCadena(char* destino, const char* origen, int& pos) {
    int i = 0;
    while (origen[i] != '\0') {
        destino[pos] = origen[i];
        pos++;
        i++;
    }
    destino[pos] = '\0';
}


void copiarNumero(char* destino, int n, int minimoDigitos, int& pos) {
    char tmp[12];
    int len = 0;
    if (n == 0) {
        tmp[len] = '0';
        len++;
    }
    while (n > 0) {
        tmp[len] = '0' + n % 10;       // ultimo digito (salen en orden inverso)
        len++;
        n = n / 10;
    }
    while (len < minimoDigitos) {
        tmp[len] = '0';
        len++;
    }
    for (int i = len - 1; i >= 0; i--) {   // los doy vuelta al copiar
        destino[pos] = tmp[i];
        pos++;
    }
    destino[pos] = '\0';
}

int main() {
    int semana, mes;

    cout << " RESUMEN DEL CIERRE " << endl;
    cout << "Numero de semana (sX): ";
    cin >> semana;
    cout << "Mes (mm): ";
    cin >> mes;

    char nomSemanal[60];
    int pos = 0;
    copiarCadena(nomSemanal, "comandas_semana_s", pos);
    copiarNumero(nomSemanal, semana, 1, pos);
    copiarCadena(nomSemanal, "-", pos);
    copiarNumero(nomSemanal, mes, 2, pos);
    copiarCadena(nomSemanal, ".dat", pos);

    FILE* f = fopen(nomSemanal, "rb");
    if (f == NULL) {
        cout << "No se pudo abrir " << nomSemanal << ". Corra primero cierre.cpp." << endl;
        return 1;
    }

    cout << endl;
    cout << "Semana " << semana << " - mes " << mes << endl;
    cout << "Mozo   Productos   Comision a pagar" << endl;
    cout << "------------------------------------" << endl;

    int totalBuffet = 0;                       // productos de todo el buffet

    Comanda r;
    int leido = fread(&r, sizeof(Comanda), 1, f);   // lectura anticipada

    if (leido != 1) {
        cout << "La planilla semanal esta vacia, no hay ventas." << endl;
    }

    while (leido == 1) {
        int idActual = r.idMozo;               // clave de control del grupo
        int productosMozo = 0;                 // acumuladores del grupo
        float comisionMozo = 0;

        while (leido == 1 && r.idMozo == idActual) {
            productosMozo += r.cantidad;
            comisionMozo += r.comision;
            leido = fread(&r, sizeof(Comanda), 1, f);
        }

        // cierre del grupo: tambien se ejecuta para el ultimo mozo, porque
        // el ciclo interno corta cuando el archivo se termina (leido == 0)
        cout << idActual << "      " << productosMozo << "          $" << comisionMozo << endl;
        totalBuffet += productosMozo;
    }

    fclose(f);

    cout << "Total de productos del buffet en la semana: " << totalBuffet << endl;

    return 0;
}
