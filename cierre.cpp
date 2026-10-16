
#include <iostream>
#include <cstdio>
using namespace std;

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

const char* TEMPORAL = "comandas_semana_tmp.dat";


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

// Intercala A y B (ambos ordenados por idMozo) en C.
// Si el idMozo es igual, sale primero el de A (el acumulado) -> estable.
// Devuelve la cantidad de registros grabados en C.
int apareo(const char* nomA, const char* nomB, const char* nomC) {
    FILE* a = fopen(nomA, "rb");
    FILE* b = fopen(nomB, "rb");
    FILE* c = fopen(nomC, "wb");
    if (a == NULL || b == NULL || c == NULL) {
        cout << "Error abriendo archivos en el apareo." << endl;
        if (a) fclose(a);
        if (b) fclose(b);
        if (c) fclose(c);
        return -1;
    }

    Comanda ra, rb;
    int la = fread(&ra, sizeof(Comanda), 1, a);   // lectura anticipada
    int lb = fread(&rb, sizeof(Comanda), 1, b);
    int total = 0;

    while (la == 1 && lb == 1) {                  // mientras haya en AMBOS
        if (ra.idMozo <= rb.idMozo) {
            fwrite(&ra, sizeof(Comanda), 1, c);
            la = fread(&ra, sizeof(Comanda), 1, a);
        } else {
            fwrite(&rb, sizeof(Comanda), 1, c);
            lb = fread(&rb, sizeof(Comanda), 1, b);
        }
        total++;
    }
    while (la == 1) {                             // vuelco el resto de A
        fwrite(&ra, sizeof(Comanda), 1, c);
        la = fread(&ra, sizeof(Comanda), 1, a);
        total++;
    }
    while (lb == 1) {                             // vuelco el resto de B
        fwrite(&rb, sizeof(Comanda), 1, c);
        lb = fread(&rb, sizeof(Comanda), 1, b);
        total++;
    }

    fclose(a);
    fclose(b);
    fclose(c);
    return total;
}

// Copia todos los registros de origen en destino (destino se vacia primero).
// Devuelve la cantidad de registros copiados, o -1 si no pudo abrir algo.
int copiarArchivo(const char* nomOrigen, const char* nomDestino) {
    FILE* o = fopen(nomOrigen, "rb");
    FILE* d = fopen(nomDestino, "wb");     // "wb" vacia el destino
    if (o == NULL || d == NULL) {
        cout << "Error abriendo archivos al copiar." << endl;
        if (o) fclose(o);
        if (d) fclose(d);
        return -1;
    }

    Comanda r;
    int total = 0;
    while (fread(&r, sizeof(Comanda), 1, o) == 1) {
        fwrite(&r, sizeof(Comanda), 1, d);
        total++;
    }

    fclose(o);
    fclose(d);
    return total;
}

int main() {
    int semana, mes, cantDias;

    cout << "=== CIERRE SEMANAL ===" << endl;
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

    cout << "Cantidad de dias a juntar: ";
    cin >> cantDias;

    // Arranco con la semanal vacia (si ya existia, se rehace de cero)
    FILE* f = fopen(nomSemanal, "wb");
    if (f == NULL) {
        cout << "No se pudo crear " << nomSemanal << endl;
        return 1;
    }
    fclose(f);

    int diasUnidos = 0;
    int ventasTotales = 0;

    for (int i = 1; i <= cantDias; i++) {
        char fecha[11];
        cout << "Fecha del dia " << i << " (DD-MM-AAAA): ";
        cin.width(11);
        cin >> fecha;

        char nomDia[60];
        int posDia = 0;
        copiarCadena(nomDia, "comandas_", posDia);
        copiarCadena(nomDia, fecha, posDia);
        copiarCadena(nomDia, ".dat", posDia);

        // Caso raro: dia sin planilla (no hubo ventas o no se cargo)
        FILE* test = fopen(nomDia, "rb");
        if (test == NULL) {
            cout << "  -> No existe " << nomDia << ", se omite este dia." << endl;
        } else {
            fclose(test);

            int n = apareo(nomSemanal, nomDia, TEMPORAL);
            if (n < 0) {
                cout << "  -> Error al unir " << nomDia << ", se omite." << endl;
            } else {
                // paso el resultado del temporal a la semanal
                int copiados = copiarArchivo(TEMPORAL, nomSemanal);
                if (copiados < 0) {
                    cout << "  -> Error al actualizar " << nomSemanal << "." << endl;
                } else {
                    diasUnidos++;
                    ventasTotales = copiados;
                    cout << "  -> " << nomDia << " unido. Total acumulado: " << copiados << " ventas." << endl;
                }
            }
        }
    }

    cout << endl;
    if (diasUnidos == 0) {
        cout << "No se unio ningun dia. " << nomSemanal << " quedo vacio." << endl;
    } else {
        cout << "Cierre listo: " << diasUnidos << " dia(s) unidos, "
             << ventasTotales << " ventas en " << nomSemanal << endl;
    }
    return 0;
}
