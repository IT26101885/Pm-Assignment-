#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define G 9.81
#define CANVAS_SIZE 10000.0 // Fixed Canvas Size D x D in meters

// Struct for Escort Ship
struct EscortShip {
    int id;
    int type; // 0: EA, 1: EB, 2: EC, 3: ED, 4: EE
    char typeName[30];
    double x, y;
    double minAngle;
    double maxAngle;
    double minVel;
    double maxVel;
    double impactPower;
    double maxRange;
};

// Struct for Battleship
struct Battleship {
    char type; // 'U', 'M', 'R', 'S'
    char name[30];
    double x, y;
    double maxVel;
    double maxRange;
};

// Function to calculate maximum projectile range
double getRange(double v) {
    return (v * v) / G;
}

// Function to calculate distance between two points
double getDistance(double x1, double y1, double x2, double y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

int main() {
    srand(time(NULL));

    double D = CANVAS_SIZE; // Fixed canvas size set directly
    int N;

    printf("=== NAVAL BATTLE SIMULATOR (PART 1-A) ===\n");
    printf("Canvas Size set to fixed: %.0fx%.0f meters\n\n", D, D);

    // 1. Inputs
    printf("Enter number of escort ships (N): ");
    scanf("%d", &N);

    // 2. Battleship Setup
    struct Battleship B;
    printf("\nSelect Battleship Type (U = USS Iowa, M = MS King George, R = Richelieu, S = Sovetsky Soyuz): ");
    scanf(" %c", &B.type);

    if (B.type == 'M' || B.type == 'm') {
        sprintf(B.name, "MS King George V");
    } else if (B.type == 'R' || B.type == 'r') {
        sprintf(B.name, "Richelieu");
    } else if (B.type == 'S' || B.type == 's') {
        sprintf(B.name, "Sovetsky Soyuz-class");
    } else {
        B.type = 'U';
        sprintf(B.name, "USS Iowa (BB-61)");
    }

    // Battleship position and velocity setup
    B.x = rand() % (int)D;
    B.y = rand() % (int)D;
    B.maxVel = 400 + rand() % 400; // max speed between 400 and 800 m/s
    B.maxRange = getRange(B.maxVel);

    // 3. Escort Ships Setup
    struct EscortShip E[100]; // Array for up to 100 escort ships

    for (int i = 0; i < N; i++) {
        E[i].id = i;
        E[i].x = rand() % (int)D;
        E[i].y = rand() % (int)D;
        E[i].type = rand() % 5;

        E[i].minAngle = 10 + rand() % 20;
        E[i].minVel = 50 + rand() % 50;

        if (E[i].type == 0) {
            sprintf(E[i].typeName, "1936A-class Destroyer");
            E[i].maxAngle = E[i].minAngle + 20;
            E[i].maxVel = 1.2 * B.maxVel; // Rule from assignment table
            E[i].impactPower = 0.08;
        } else if (E[i].type == 1) {
            sprintf(E[i].typeName, "Gabbiano-class Corvette");
            E[i].maxAngle = E[i].minAngle + 30;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.06;
        } else if (E[i].type == 2) {
            sprintf(E[i].typeName, "Matsu-class Destroyer");
            E[i].maxAngle = E[i].minAngle + 25;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.07;
        } else if (E[i].type == 3) {
            sprintf(E[i].typeName, "F-class Escort Ship");
            E[i].maxAngle = E[i].minAngle + 50;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.05;
        } else {
            sprintf(E[i].typeName, "Japanese Kaibokan");
            E[i].maxAngle = E[i].minAngle + 70;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.04;
        }

        E[i].maxRange = getRange(E[i].maxVel);
    }

    // 4. Save Initial Battlefield Conditions to Text File
    FILE *fInit = fopen("initial_conditions.txt", "w");
    if (fInit != NULL) {
        fprintf(fInit, "INITIAL BATTLEFIELD CONDITIONS\n");
        fprintf(fInit, "Canvas Size: %.2f x %.2f\n\n", D, D);
        
        fprintf(fInit, "BATTLESHIP:\nName: %s\nPosition: (%.2f, %.2f)\nMax Speed: %.2f\nMax Range: %.2f\n\n",
                B.name, B.x, B.y, B.maxVel, B.maxRange);

        fprintf(fInit, "ESCORT SHIPS:\n");
        fprintf(fInit, "ID\tType\t\t\t\tX\tY\tMinVel\tMaxVel\tMinAngle\tMaxAngle\tMaxRange\n");
        for (int i = 0; i < N; i++) {
            fprintf(fInit, "%d\t%-25s\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t\t%.1f\t\t%.2f\n",
                    E[i].id, E[i].typeName, E[i].x, E[i].y,
                    E[i].minVel, E[i].maxVel, E[i].minAngle, E[i].maxAngle, E[i].maxRange);
        }
        fclose(fInit);
    }

    // 5. Simulation Logic (Part 1-A)
    int bSunk = 0;
    int sunkById = -1;

    // Check if any Escort Ship can hit B
    for (int i = 0; i < N; i++) {
        double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
        if (dist <= E[i].maxRange) {
            bSunk = 1;
            sunkById = E[i].id;
            break; // First E ship that can hit B destroys B
        }
    }

    // 6. Display Results & Save Files
    FILE *fFinal = fopen("final_conditions.txt", "w");

    if (bSunk) {
        printf("\n--- BATTLE RESULT ---\n");
        printf("Battleship %s was SUNK!\n", B.name);
        printf("Sunk by Escort Ship Index: %d\n", sunkById);

        if (fFinal != NULL) {
            fprintf(fFinal, "FINAL BATTLEFIELD CONDITIONS\n");
            fprintf(fFinal, "Battleship Status: SUNK\n");
            fprintf(fFinal, "Sunk by Escort Ship Index: %d\n", sunkById);
        }
    } else {
        printf("\n--- BATTLE RESULT ---\n");
        printf("Battleship %s SURVIVED!\n", B.name);

        int hitCount = 0;
        FILE *fHits = fopen("hits_details.txt", "w");
        if (fHits != NULL) {
            fprintf(fHits, "Index_of_E\tTime_to_hit\tDistance\n");
        }

        for (int i = 0; i < N; i++) {
            double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
            if (dist <= B.maxRange) {
                hitCount++;
                double timeToHit = dist / B.maxVel; // Time calculation assuming direct shell flight

                if (fHits != NULL) {
                    fprintf(fHits, "%d\t\t%.2f\t\t%.2f\n", E[i].id, timeToHit, dist);
                }
            }
        }

        if (fHits != NULL) fclose(fHits);

        printf("Escort ships hit by Battleship: %d\n", hitCount);
        printf("Time taken to end battle: 0 seconds (Instant reloading assumed)\n");

        if (fFinal != NULL) {
            fprintf(fFinal, "FINAL BATTLEFIELD CONDITIONS\n");
            fprintf(fFinal, "Battleship Status: SURVIVED\n");
            fprintf(fFinal, "Total Escort Ships Destroyed: %d\n", hitCount);
        }
    }

    if (fFinal != NULL) fclose(fFinal);

    printf("\nAll initial and final conditions have been saved to text files.\n");

    return 0;
}
