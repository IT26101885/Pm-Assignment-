#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define G 9.81
#define CANVAS_SIZE 10000.0 // Fixed Canvas Size D x D in meters
#define MAX_E 100
#define MAX_K 50

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
    int isDestroyed; // Flag: 1 if destroyed, 0 if active
};

// Struct for Battleship
struct Battleship {
    char type; // 'U', 'M', 'R', 'S'
    char name[30];
    double x, y;
    double maxVel;
    double maxRange;
};

// Struct for Path Points
struct Point {
    double x, y;
};

// Calculate maximum projectile range: R_max = V^2 / g
double getRange(double v) {
    return (v * v) / G;
}

// Calculate projectile range given a minimum vertical angle constraint:
// R = (V^2 * sin(2 * theta_min)) / g  (for 45 deg <= theta_min <= 90 deg, angle adjustment applies)
double getJammedRange(double v, double thetaMinDeg) {
    double rad = thetaMinDeg * (3.141592653589793 / 180.0);
    // If min angle > 45 deg, max range occurs at theta_min; otherwise optimal range at 45 deg is still reachable
    if (thetaMinDeg > 45.0) {
        return (v * v * sin(2.0 * rad)) / G;
    }
    return (v * v) / G; 
}

// Calculate distance between two points
double getDistance(double x1, double y1, double x2, double y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Reset Escort ships to initial state for Simulation 2
void resetEscortShips(struct EscortShip E[], struct EscortShip E_copy[], int N) {
    for (int i = 0; i < N; i++) {
        E[i] = E_copy[i];
    }
}

int main() {
    srand(time(NULL));

    double D = CANVAS_SIZE;
    int N, K, T;
    double thetaMin;

    printf("=== NAVAL BATTLE SIMULATOR (PART 1-B) ===\n");
    printf("Canvas Size: %.0fx%.0f meters\n\n", D, D);

    // 1. Inputs
    printf("Enter number of escort ships (N): ");
    scanf("%d", &N);

    printf("Enter number of path points for Battleship (K): ");
    scanf("%d", &K);

    printf("Enter iteration step where gun gets jammed (t < K): ");
    scanf("%d", &T);

    printf("Enter minimum angle theta_min for jammed gun (0 < theta_min < 30): ");
    scanf("%lf", &thetaMin);

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

    B.maxVel = 400 + rand() % 400; // max speed between 400 and 800 m/s
    B.maxRange = getRange(B.maxVel);

    // Generate K path points for Battleship path
    struct Point path[MAX_K];
    for (int i = 0; i < K; i++) {
        path[i].x = rand() % (int)D;
        path[i].y = rand() % (int)D;
    }
    // Set initial position as path[0]
    B.x = path[0].x;
    B.y = path[0].y;

    // 3. Escort Ships Setup
    struct EscortShip E[MAX_E];
    struct EscortShip E_original[MAX_E]; // Master copy to reset for Simulation 2

    for (int i = 0; i < N; i++) {
        E[i].id = i;
        E[i].x = rand() % (int)D;
        E[i].y = rand() % (int)D;
        E[i].type = rand() % 5;
        E[i].isDestroyed = 0;

        E[i].minAngle = 10 + rand() % 20;
        E[i].minVel = 50 + rand() % 50;

        if (E[i].type == 0) {
            sprintf(E[i].typeName, "1936A-class Destroyer");
            E[i].maxAngle = E[i].minAngle + 20;
            E[i].maxVel = 1.2 * B.maxVel;
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
        E_original[i] = E[i]; // Store backup copy
    }

    // Save Initial Battlefield Setup to text file
    FILE *fInit = fopen("part1b_initial_conditions.txt", "w");
    if (fInit != NULL) {
        fprintf(fInit, "INITIAL BATTLEFIELD CONDITIONS (PART 1-B)\n");
        fprintf(fInit, "Canvas Size: %.2f x %.2f | Total Path Points K: %d\n\n", D, D, K);
        fprintf(fInit, "BATTLESHIP:\nName: %s\nMax Speed: %.2f\nMax Range: %.2f\n\n",
                B.name, B.maxVel, B.maxRange);

        fprintf(fInit, "PATH POINTS:\n");
        for (int i = 0; i < K; i++) {
            fprintf(fInit, "Point %d: (%.1f, %.1f)\n", i + 1, path[i].x, path[i].y);
        }

        fprintf(fInit, "\nESCORT SHIPS:\n");
        fprintf(fInit, "ID\tType\t\t\t\tX\tY\tMinVel\tMaxVel\tMaxRange\n");
        for (int i = 0; i < N; i++) {
            fprintf(fInit, "%d\t%-25s\t%.1f\t%.1f\t%.1f\t%.1f\t%.2f\n",
                    E[i].id, E[i].typeName, E[i].x, E[i].y,
                    E[i].minVel, E[i].maxVel, E[i].maxRange);
        }
        fclose(fInit);
    }

    // ==========================================
    // SIMULATION 1: Normal Movement along Path
    // ==========================================
    printf("\n--- RUNNING SIMULATION 1 (Normal Path Movement) ---\n");
    FILE *fSim1 = fopen("part1b_sim1_results.txt", "w");

    int bSunk1 = 0;
    int sunkStep1 = -1;
    int totalHits1 = 0;

    for (int k = 0; k < K; k++) {
        B.x = path[k].x;
        B.y = path[k].y;

        if (fSim1 != NULL) {
            fprintf(fSim1, "--- Step %d at Position (%.1f, %.1f) ---\n", k + 1, B.x, B.y);
        }

        // Check if any active E ship can hit B
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= E[i].maxRange) {
                    bSunk1 = 1;
                    sunkStep1 = k + 1;
                    if (fSim1 != NULL) {
                        fprintf(fSim1, "Battleship SUNK at step %d by Escort Ship ID %d\n", k + 1, E[i].id);
                    }
                    break;
                }
            }
        }

        if (bSunk1) break; // End simulation if B sinks

        // Battleship attacks remaining E ships in its range
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= B.maxRange) {
                    E[i].isDestroyed = 1; // Mark as destroyed for future steps
                    totalHits1++;
                    if (fSim1 != NULL) {
                        fprintf(fSim1, "Hit Escort Ship ID %d (Dist: %.2f m)\n", E[i].id, dist);
                    }
                }
            }
        }
    }

    if (fSim1 != NULL) {
        fprintf(fSim1, "\nSummary: B Sunk = %s, Total E Ships Destroyed = %d\n",
                bSunk1 ? "YES" : "NO", totalHits1);
        fclose(fSim1);
    }

    if (bSunk1) {
        printf("Simulation 1: Battleship SUNK at path point %d.\n", sunkStep1);
    } else {
        printf("Simulation 1: Battleship SURVIVED all %d points! Destroyed %d E-ships.\n", K, totalHits1);
    }

    // ==========================================
    // SIMULATION 2: Gun Jamming after 't' Steps
    // ==========================================
    printf("\n--- RUNNING SIMULATION 2 (Gun Jammed after step %d) ---\n", T);
    resetEscortShips(E, E_original, N); // Reset all E-ships state

    FILE *fSim2 = fopen("part1b_sim2_results.txt", "w");

    int bSunk2 = 0;
    int sunkStep2 = -1;
    int totalHits2 = 0;

    for (int k = 0; k < K; k++) {
        B.x = path[k].x;
        B.y = path[k].y;

        // Apply jammed range after step t
        double currentBRange = B.maxRange;
        if (k >= T) {
            currentBRange = getJammedRange(B.maxVel, thetaMin);
        }

        if (fSim2 != NULL) {
            fprintf(fSim2, "--- Step %d at Position (%.1f, %.1f) %s ---\n", 
                    k + 1, B.x, B.y, (k >= T) ? "[GUN JAMMED]" : "[GUN NORMAL]");
        }

        // Check if any active E ship can hit B
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= E[i].maxRange) {
                    bSunk2 = 1;
                    sunkStep2 = k + 1;
                    if (fSim2 != NULL) {
                        fprintf(fSim2, "Battleship SUNK at step %d by Escort Ship ID %d\n", k + 1, E[i].id);
                    }
                    break;
                }
            }
        }

        if (bSunk2) break; // End simulation if B sinks

        // Battleship attacks remaining E ships in its current range
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= currentBRange) {
                    E[i].isDestroyed = 1;
                    totalHits2++;
                    if (fSim2 != NULL) {
                        fprintf(fSim2, "Hit Escort Ship ID %d (Dist: %.2f m)\n", E[i].id, dist);
                    }
                }
            }
        }
    }

    if (fSim2 != NULL) {
        fprintf(fSim2, "\nSummary: B Sunk = %s, Total E Ships Destroyed = %d\n",
                bSunk2 ? "YES" : "NO", totalHits2);
        fclose(fSim2);
    }

    if (bSunk2) {
        printf("Simulation 2: Battleship SUNK at path point %d.\n", sunkStep2);
    } else {
        printf("Simulation 2: Battleship SURVIVED all %d points! Destroyed %d E-ships.\n", K, totalHits2);
    }

    printf("\nAll simulation logs saved to text files ('part1b_initial_conditions.txt', 'part1b_sim1_results.txt', 'part1b_sim2_results.txt').\n");

    return 0;
}
