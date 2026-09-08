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
    int isDestroyed;  // 1 if destroyed by B, 0 if active
    int hasAttacked;  // 1 if this E ship has already attacked B (each E fires once)
};

// Struct for Battleship
struct Battleship {
    char type; // 'U', 'M', 'R', 'S'
    char name[30];
    double x, y;
    double maxVel;
    double maxRange;
    double cumulativeImpact; // Total damage taken (0.0 to 1.0+)
};

// Struct for Path Points
struct Point {
    double x, y;
};

// Calculate maximum projectile range: R_max = V^2 / g
double getRange(double v) {
    return (v * v) / G;
}

// Calculate projectile range when gun is jammed with minimum angle constraint
double getJammedRange(double v, double thetaMinDeg) {
    double rad = thetaMinDeg * (3.141592653589793 / 180.0);
    if (thetaMinDeg > 45.0) {
        return (v * v * sin(2.0 * rad)) / G;
    }
    return (v * v) / G;
}

// Distance formula between two points
double getDistance(double x1, double y1, double x2, double y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Reset Escort ships and Battleship state between simulation runs
void resetShips(struct EscortShip E[], struct EscortShip E_orig[], int N, struct Battleship *B) {
    for (int i = 0; i < N; i++) {
        E[i] = E_orig[i];
    }
    B->cumulativeImpact = 0.0;
}

int main() {
    srand(time(NULL));

    double D = CANVAS_SIZE;
    int N, K, T;
    double thetaMin;

    printf("=== NAVAL BATTLE SIMULATOR (PART 1-C) ===\n");
    printf("Canvas Size: %.0fx%.0f meters\n\n", D, D);

    // 1. User Inputs
    printf("Enter number of escort ships (N): ");
    scanf("%d", &N);

    printf("Enter number of path points for Battleship (K): ");
    scanf("%d", &K);

    printf("Enter step where gun gets jammed (t < K): ");
    scanf("%d", &T);

    printf("Enter min angle theta_min for jammed gun (0 < theta_min < 30): ");
    scanf("%lf", &thetaMin);

    // 2. Setup Battleship
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

    B.maxVel = 400 + rand() % 400;
    B.maxRange = getRange(B.maxVel);
    B.cumulativeImpact = 0.0;

    // Generate K path points for B
    struct Point path[MAX_K];
    for (int i = 0; i < K; i++) {
        path[i].x = rand() % (int)D;
        path[i].y = rand() % (int)D;
    }
    B.x = path[0].x;
    B.y = path[0].y;

    // 3. Setup Escort Ships
    struct EscortShip E[MAX_E];
    struct EscortShip E_original[MAX_E];

    for (int i = 0; i < N; i++) {
        E[i].id = i;
        E[i].x = rand() % (int)D;
        E[i].y = rand() % (int)D;
        E[i].type = rand() % 5;
        E[i].isDestroyed = 0;
        E[i].hasAttacked = 0;

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
        E_original[i] = E[i];
    }

    // Save Initial Battlefield Setup
    FILE *fInit = fopen("part1c_initial_conditions.txt", "w");
    if (fInit != NULL) {
        fprintf(fInit, "INITIAL BATTLEFIELD CONDITIONS (PART 1-C)\n");
        fprintf(fInit, "Canvas Size: %.2f x %.2f\n\n", D, D);
        fprintf(fInit, "BATTLESHIP:\nName: %s\nMax Speed: %.2f\nMax Range: %.2f\n\n",
                B.name, B.maxVel, B.maxRange);

        fprintf(fInit, "ESCORT SHIPS:\n");
        fprintf(fInit, "ID\tType\t\t\t\tX\tY\tMaxVel\tImpactPower\tMaxRange\n");
        for (int i = 0; i < N; i++) {
            fprintf(fInit, "%d\t%-25s\t%.1f\t%.1f\t%.1f\t%.2f\t\t%.2f\n",
                    E[i].id, E[i].typeName, E[i].x, E[i].y,
                    E[i].maxVel, E[i].impactPower, E[i].maxRange);
        }
        fclose(fInit);
    }

    // ==========================================
    // SIMULATION 1: Normal Movement with Partial Impact
    // ==========================================
    printf("\n--- RUNNING SIMULATION 1 (Cumulative Impact Model) ---\n");
    FILE *fSim1 = fopen("part1c_sim1_results.txt", "w");

    int bSunk1 = 0;
    int sunkStep1 = -1;
    int totalHits1 = 0;

    for (int k = 0; k < K; k++) {
        B.x = path[k].x;
        B.y = path[k].y;

        if (fSim1 != NULL) {
            fprintf(fSim1, "--- Step %d at Position (%.1f, %.1f) ---\n", k + 1, B.x, B.y);
        }

        // 1. Escort ships attack B if B is in range and ship hasn't fired yet
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed && !E[i].hasAttacked) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= E[i].maxRange) {
                    E[i].hasAttacked = 1; // Mark that this E ship fired its single shell
                    B.cumulativeImpact += E[i].impactPower;

                    if (fSim1 != NULL) {
                        fprintf(fSim1, "E-Ship ID %d attacked B! Impact added: %.2f (Total Impact: %.2f)\n",
                                E[i].id, E[i].impactPower, B.cumulativeImpact);
                    }

                    // Check if total impact reaches or exceeds 1.0 (100% destruction)
                    if (B.cumulativeImpact >= 1.0) {
                        bSunk1 = 1;
                        sunkStep1 = k + 1;
                        if (fSim1 != NULL) {
                            fprintf(fSim1, "Battleship %s SUNK at step %d!\n", B.name, k + 1);
                        }
                        break;
                    }
                }
            }
        }

        if (bSunk1) break;

        // 2. Battleship destroys any E ship within its attack range
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= B.maxRange) {
                    E[i].isDestroyed = 1;
                    totalHits1++;
                    if (fSim1 != NULL) {
                        fprintf(fSim1, "Battleship destroyed Escort Ship ID %d (Dist: %.2f m)\n", E[i].id, dist);
                    }
                }
            }
        }
    }

    if (fSim1 != NULL) {
        fprintf(fSim1, "\nSUMMARY SIMULATION 1:\n");
        fprintf(fSim1, "Battleship Status: %s\n", bSunk1 ? "SUNK" : "SURVIVED");
        fprintf(fSim1, "Final Cumulative Impact on Battleship: %.2f (%.1f%%)\n",
                B.cumulativeImpact, B.cumulativeImpact * 100.0);
        fprintf(fSim1, "Total E-Ships Destroyed: %d\n", totalHits1);
        fclose(fSim1);
    }

    if (bSunk1) {
        printf("Simulation 1: Battleship SUNK at step %d!\n", sunkStep1);
    } else {
        printf("Simulation 1: Battleship SURVIVED! Final Cumulative Impact: %.2f (%.1f%%)\n",
               B.cumulativeImpact, B.cumulativeImpact * 100.0);
    }

    // ==========================================
    // SIMULATION 2: Gun Jamming + Cumulative Impact
    // ==========================================
    printf("\n--- RUNNING SIMULATION 2 (Jammed Gun after step %d + Cumulative Impact) ---\n", T);
    resetShips(E, E_original, N, &B);

    FILE *fSim2 = fopen("part1c_sim2_results.txt", "w");

    int bSunk2 = 0;
    int sunkStep2 = -1;
    int totalHits2 = 0;

    for (int k = 0; k < K; k++) {
        B.x = path[k].x;
        B.y = path[k].y;

        double currentBRange = B.maxRange;
        if (k >= T) {
            currentBRange = getJammedRange(B.maxVel, thetaMin);
        }

        if (fSim2 != NULL) {
            fprintf(fSim2, "--- Step %d at Position (%.1f, %.1f) %s ---\n",
                    k + 1, B.x, B.y, (k >= T) ? "[GUN JAMMED]" : "[GUN NORMAL]");
        }

        // 1. Escort ships attack B
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed && !E[i].hasAttacked) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= E[i].maxRange) {
                    E[i].hasAttacked = 1;
                    B.cumulativeImpact += E[i].impactPower;

                    if (fSim2 != NULL) {
                        fprintf(fSim2, "E-Ship ID %d attacked B! Impact added: %.2f (Total Impact: %.2f)\n",
                                E[i].id, E[i].impactPower, B.cumulativeImpact);
                    }

                    if (B.cumulativeImpact >= 1.0) {
                        bSunk2 = 1;
                        sunkStep2 = k + 1;
                        if (fSim2 != NULL) {
                            fprintf(fSim2, "Battleship %s SUNK at step %d!\n", B.name, k + 1);
                        }
                        break;
                    }
                }
            }
        }

        if (bSunk2) break;

        // 2. Battleship attacks E ships
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= currentBRange) {
                    E[i].isDestroyed = 1;
                    totalHits2++;
                    if (fSim2 != NULL) {
                        fprintf(fSim2, "Battleship destroyed Escort Ship ID %d (Dist: %.2f m)\n", E[i].id, dist);
                    }
                }
            }
        }
    }

    if (fSim2 != NULL) {
        fprintf(fSim2, "\nSUMMARY SIMULATION 2:\n");
        fprintf(fSim2, "Battleship Status: %s\n", bSunk2 ? "SUNK" : "SURVIVED");
        fprintf(fSim2, "Final Cumulative Impact on Battleship: %.2f (%.1f%%)\n",
                B.cumulativeImpact, B.cumulativeImpact * 100.0);
        fprintf(fSim2, "Total E-Ships Destroyed: %d\n", totalHits2);
        fclose(fSim2);
    }

    if (bSunk2) {
        printf("Simulation 2: Battleship SUNK at step %d!\n", sunkStep2);
    } else {
        printf("Simulation 2: Battleship SURVIVED! Final Cumulative Impact: %.2f (%.1f%%)\n",
               B.cumulativeImpact, B.cumulativeImpact * 100.0);
    }

    printf("\nAll Part 1-C results saved to 'part1c_initial_conditions.txt', 'part1c_sim1_results.txt', and 'part1c_sim2_results.txt'.\n");

    return 0;
}
