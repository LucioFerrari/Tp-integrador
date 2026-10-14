
#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

const char* TEMPORAL = "comandas_semana_tmp.dat";

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

int main() {
    int semana, mes, cantDias;

    cout << "=== CIERRE SEMANAL ===" << endl;
    cout << "Numero de semana (sX): ";
    cin >> semana;
    cout << "Mes (mm): ";
    cin >> mes;

    char nomSemanal[60];
    sprintf(nomSemanal, "comandas_semana_s%d-%02d.dat", semana, mes);

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
        sprintf(nomDia, "comandas_%s.dat", fecha);

        // Caso raro: dia sin planilla (no hubo ventas o no se cargo)
        FILE* test = fopen(nomDia, "rb");
        if (test == NULL) {
            cout << "  -> No existe " << nomDia << ", se omite este dia." << endl;
        } else {
            fclose(test);

            int n = apareo(nomSemanal, nomDia, TEMPORAL);
            if (n < 0) {
                remove(TEMPORAL);
                cout << "  -> Error al unir " << nomDia << ", se omite." << endl;
            } else {
                remove(nomSemanal);               // reemplazo la semanal por la nueva
                rename(TEMPORAL, nomSemanal);
                diasUnidos++;
                ventasTotales = n;
                cout << "  -> " << nomDia << " unido. Total acumulado: " << n << " ventas." << endl;
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
